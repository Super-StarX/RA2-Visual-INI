#include "MainWindow.h"
#include <imgui_node_editor_internal.h>
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"
#include <unordered_map>
#include <queue>
#include <cmath>
#include <limits>
void MainWindow::ApplyForceDirectedLayout() {
	const float horizontalSpacing = 80.0f;
	const float verticalSpacing = 60.0f;
	const float layerSpacing = 120.0f;
	const ImVec2 startPos(50.0f, 50.0f);
	const float maxRowWidth = std::sqrtf(Node::Array.size()) * 500;

	//------------------------------
	// 0. 动态创建标签节点（内联处理）
	//------------------------------
    std::unordered_map<SectionNode*, std::vector<Link*>> multiInputSections;
    for (auto& node : Node::Array) {
        if (auto section = dynamic_cast<SectionNode*>(node.get())) {
            if (section->InputPin) {
                // 修复C2672错误：手动提取Link指针
                std::vector<Link*> links;
                for (const auto& linkPair : section->InputPin->Links) {
                    links.push_back(linkPair.second);
                }
                if (links.size() > 1) {
                    multiInputSections[section] = links;
                }
            }
        }
    }

	for (auto& pair : multiInputSections) {
		auto section = pair.first;
		auto& links = pair.second;

		// 创建input标签节点
		std::string tagName = section->Name + "_input";
		auto inputTag = SpawnTagNode(true, tagName);
		inputTag->SetPosition(section->GetPosition() - ImVec2(200, 0));

		// 创建output标签节点并连接回section
		auto outputTag = SpawnTagNode(false, tagName);
		outputTag->SetPosition(section->GetPosition() + ImVec2(200, 0));

		// 重新路由原始连接
		for (auto link : links) {
			link->EndPinID = inputTag->InputPin->ID;
		}

		// 创建output到section的连接
		auto newLink = outputTag->InputPin->LinkTo(section->InputPin.get());
	}

	//------------------------------
	// 1. 建立连接关系（处理标签节点查找）
	//------------------------------
	std::unordered_map<Node*, std::vector<Node*>> childrenMap;
	for (auto& node : Node::Array) {
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

	//------------------------------
	// 2. BFS计算层级（保持原逻辑）
	//------------------------------
	std::queue<Node*> queue;
	for (auto& node : Node::Array) {
		auto section = dynamic_cast<SectionNode*>(node.get());
		if (section) {
			bool hasInput = section->InputPin && !section->InputPin->Links.empty();
			node->level = hasInput ? -1 : 0;
			if (node->level == 0)
				queue.push(node.get());
		}
	}

	while (!queue.empty()) {
		Node* current = queue.front();
		queue.pop();
		for (Node* child : childrenMap[current]) {
			int newLevel = current->level + 1;
			if (newLevel > child->level) {
				child->level = newLevel;
				queue.push(child);
			}
		}
	}

	//------------------------------
	// 3. 收集层级节点（保持原逻辑）
	//------------------------------
	std::map<int, std::vector<Node*>> layers;
	for (auto& node : Node::Array) {
		if (node->level >= 0)
			layers[node->level].push_back(node.get());
		else
			layers[0].push_back(node.get());
	}

	// 按父节点输出顺序排序
	for (auto& layer : layers) {
		auto& nodes = layer.second;
		std::sort(nodes.begin(), nodes.end(), [&childrenMap](Node* a, Node* b) {
			bool aHasChild = !childrenMap[a].empty();
			bool bHasChild = !childrenMap[b].empty();
			if (aHasChild != bHasChild) return bHasChild;
			return a->ID.Get() < b->ID.Get();
		});
	}

	//------------------------------
	// 4. 计算布局位置（保持原逻辑）
	//------------------------------
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
}
