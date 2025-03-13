#include "MainWindow.h"
#include <imgui_node_editor_internal.h>
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"
#include <unordered_map>
#include <queue>
#include <cmath>
#include <limits>

void MainWindow::ApplyForceDirectedLayout() {
	// 1. 动态创建标签节点
	CreateTagNodesForMultiInputs();

	// 2. 建立节点连接关系图
	auto childrenMap = BuildChildrenMap();

	// 3. 计算节点层级
	CalculateNodeLevels(childrenMap);

	// 4. 收集层级节点
	auto layers = CollectLayerNodes(childrenMap);

	// 5. 执行布局排列
	ArrangeNodesInLayers(layers);

	ed::NavigateToContent();
}

void MainWindow::CreateTagNodesForMultiInputs() {
	std::unordered_map<SectionNode*, std::vector<Link*>> multiInputSections;

	// 收集都有谁被很多人链接(inputpin有很多link)
	for (auto& node : Node::Array) {
		if (auto section = dynamic_cast<SectionNode*>(node.get())) {
			if (section->InputPin && section->InputPin->Links.size() > 1) {
				std::vector<Link*> links;
				for (const auto& linkPair : section->InputPin->Links)
					links.push_back(linkPair.second);
				multiInputSections[section] = links;
			}
		}
	}

	// 创建标签节点对
	for (auto& pair : multiInputSections) {
		auto section = pair.first; // 被链接的node
		auto& links = pair.second; // 该node的所有的inputs
		std::string tagName = section->Name;

		// 创建input标签节点
		if (auto inputTag = Node::Create<TagNode>(true, tagName.c_str())) {
			inputTag->SetPosition(section->GetPosition() - ImVec2(200, 0));
			section->OutputPin->LinkTo(inputTag->InputPin.get());
		}

		// 创建output标签节点
		for (auto link : links) {
			auto pin = Pin::Get(link->StartPinID);
			if (auto outputTag = Node::Create<TagNode>(false, tagName.c_str())) {
				outputTag->SetPosition(section->GetPosition() + ImVec2(200, 0));
				pin->LinkTo(outputTag->InputPin.get());
			}
		}
	}
}

std::unordered_map<Node*, std::vector<Node*>> MainWindow::BuildChildrenMap() {
	std::unordered_map<Node*, std::vector<Node*>> childrenMap;
	for (auto& node : Node::Array) {
		// 处理所有具有输出引脚的节点类型
		auto section = dynamic_cast<SectionNode*>(node.get());
		if (section && section->OutputPin) {
			for (auto& linkPair : section->OutputPin->Links) {
				Link* link = linkPair.second;
				Pin* endPin = Pin::Get(link->EndPinID);
				if (endPin && endPin->Node) {
					Node* targetNode = endPin->Node;

					// 处理output标签节点的重定向
					if (auto outputTag = dynamic_cast<TagNode*>(targetNode)) {
						if (!outputTag->IsInput) {
							if (auto inputTag = outputTag->GetInputTagNode()) {
								targetNode = inputTag;
								endPin = inputTag->InputPin.get();
							}
						}
					}

					childrenMap[node.get()].push_back(targetNode);
				}
			}
		}
	}

	return childrenMap;
}

void MainWindow::CalculateNodeLevels(const std::unordered_map<Node*, std::vector<Node*>>& childrenMap) {
	std::queue<Node*> queue;

	// 初始化层级（处理所有节点类型）
	for (auto& node : Node::Array) {
		bool hasInput = false;
		if (auto section = dynamic_cast<SectionNode*>(node.get())) {
			hasInput = section->InputPin && !section->InputPin->Links.empty();
		}
		node->level = hasInput ? -1 : 0;
		if (node->level == 0)
			queue.push(node.get());
	}

	// BFS计算层级（添加安全检查）
	while (!queue.empty()) {
		Node* current = queue.front();
		queue.pop();

		auto it = childrenMap.find(current);
		if (it == childrenMap.end()) continue;

		for (Node* child : it->second) {
			int newLevel = current->level + 1;
			if (newLevel > child->level) {
				child->level = newLevel;
				queue.push(child);
			}
		}
	}
}

std::map<int, std::vector<Node*>> MainWindow::CollectLayerNodes(const std::unordered_map<Node*, std::vector<Node*>>& childrenMap) {
	std::map<int, std::vector<Node*>> layers;

	// 按层级收集节点
	for (auto& node : Node::Array) {
		if (dynamic_cast<TagNode*>(node.get())) continue; // 跳过标签节点
		if (node->level >= 0)
			layers[node->level].push_back(node.get());
		else
			layers[0].push_back(node.get());
	}

	// 层内排序（添加安全检查）
	for (auto& layer : layers) {
		auto& nodes = layer.second;
		std::sort(nodes.begin(), nodes.end(), [&childrenMap](Node* a, Node* b) {
			auto aIt = childrenMap.find(a);
			auto bIt = childrenMap.find(b);
			bool aHasChild = (aIt != childrenMap.end()) && !aIt->second.empty();
			bool bHasChild = (bIt != childrenMap.end()) && !bIt->second.empty();
			if (aHasChild != bHasChild) return bHasChild;
			return a->ID.Get() < b->ID.Get();
		});
	}

	return layers;
}

void MainWindow::ArrangeNodesInLayers(const std::map<int, std::vector<Node*>>& layers) {
	const float horizontalSpacing = 80.0f;
	const float verticalSpacing = 60.0f;
	const float layerSpacing = 120.0f;
	const ImVec2 startPos(50.0f, 50.0f);
	const float maxRowWidth = std::sqrtf(static_cast<float>(Node::Array.size())) * 500;
	float currentY = startPos.y;

	for (auto& layer : layers) {
		auto& nodes = layer.second;

		struct Row {
			float y = 0.0f;
			float width = 0.0f;
			float height = 0.0f;
		};

		std::vector<Row> rows;
		rows.push_back({ currentY });

		float currentX = startPos.x;
		Row* currentRow = &rows.back();

		for (size_t i = 0; i < nodes.size(); ++i) {
			auto node = nodes[i];
			ImVec2 size = node->GetNodeSize();

			// 跳过标签节点的布局
			if (dynamic_cast<TagNode*>(node)) continue;

			float potentialWidth = currentRow->width;
			if (i > 0) potentialWidth += horizontalSpacing;
			potentialWidth += size.x;

			auto section = dynamic_cast<SectionNode*>(node);
			bool shouldWrap = currentRow->width > 0 && potentialWidth > maxRowWidth &&
				(!section || (section->InputPin && section->InputPin->Links.empty()));

			if (shouldWrap) {
				rows.push_back({
					currentRow->y + currentRow->height + verticalSpacing,
					0.0f,
					0.0f
				});
				currentRow = &rows.back();
				currentX = startPos.x;
			}

			node->SetPosition(ImVec2(currentX, currentRow->y));

			currentRow->width = currentX - startPos.x + size.x;
			currentRow->height = std::max(currentRow->height, size.y);
			currentX += size.x + horizontalSpacing;
		}

		float layerHeight = 0.0f;
		for (const auto& row : rows) {
			layerHeight = std::max(layerHeight, row.y + row.height - currentY);
		}
		currentY += layerHeight + layerSpacing;
	}

	// 单独处理标签节点的位置
	for (auto& node : Node::Array) {
		if (auto tagNode = dynamic_cast<TagNode*>(node.get())) {
			if (tagNode->IsInput) {
				// 输入标签固定在关联节点左侧
				if (auto inputTag = TagNode::Inputs[tagNode->Name])
					if (auto associatedNode = inputTag->InputPin->GetLinkedNode())
						tagNode->SetPosition(associatedNode->GetPosition() - ImVec2(200, 0));
			}
			else if (auto sourceNode = tagNode->InputPin->GetLinkedNode()) {
				tagNode->SetPosition(sourceNode->GetPosition() + ImVec2(200, 0));
			}
		}
	}
}