#include "MainWindow.h"
#include <imgui_node_editor_internal.h>
#include "Nodes/SectionNode.h"
#include <unordered_map>
#include <queue>
#include <cmath>
#include <limits>

void MainWindow::ApplyForceDirectedLayout() {
	const float horizontalSpacing = 80.0f;   // 节点水平间距
	const float verticalSpacing = 60.0f;     // 行间垂直间距
	const float layerSpacing = 120.0f;       // 层间垂直间距（原verticalSpacing*2）
	const ImVec2 startPos(50.0f, 50.0f);     // 起始布局位置
	const float maxRowWidth = std::sqrtf(Node::Array.size())*500;      // 最大行宽

	//------------------------------
	// 1. 建立连接关系
	//------------------------------
	std::unordered_map<Node*, std::vector<Node*>> childrenMap;
	for (auto& node : Node::Array) {
		auto section = dynamic_cast<SectionNode*>(node.get());
		if (section && section->OutputPin) {
			for (auto& linkPair : section->OutputPin->Links) {
				Link* link = linkPair.second;
				Pin* endPin = Pin::Get(link->EndPinID);
				if (endPin && endPin->Node)
					childrenMap[node.get()].push_back(endPin->Node);
			}
		}
	}

	//------------------------------
	// 2. BFS计算层级
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
	// 3. 收集层级节点
	//------------------------------
	std::map<int, std::vector<Node*>> layers;
	for (auto& node : Node::Array) {
		if (node->level >= 0) layers[node->level].push_back(node.get());
		else layers[0].push_back(node.get());
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
	// 4. 计算布局位置
	//------------------------------
	float currentY = startPos.y;
	for (auto& layer : layers) {
		auto& nodes = layer.second;

		struct Row {
			float y = 0.0f;        // 行起始Y坐标
			float width = 0.0f;    // 行当前宽度
			float height = 0.0f;   // 行当前高度
		};

		std::vector<Row> rows;
		rows.push_back({ currentY }); // 初始行

		float currentX = startPos.x;
		Row* currentRow = &rows.back();

		for (size_t i = 0; i < nodes.size(); ++i) {
			auto node = nodes[i];
			ImVec2 size = node->GetNodeSize();

			// 计算潜在宽度（包含间距）
			float potentialWidth = currentRow->width;
			if (i > 0) potentialWidth += horizontalSpacing; // 非首节点加间距
			potentialWidth += size.x;

			// 换行条件：当前行已有内容且超出最大宽度且是新节点
			auto section = dynamic_cast<SectionNode*>(node);
			bool shouldWrap = currentRow->width > 0 && potentialWidth > maxRowWidth && section->InputPin->Links.empty();

			if (shouldWrap) {
				// 创建新行
				rows.push_back({
					currentRow->y + currentRow->height + verticalSpacing,
					0.0f,
					0.0f
				});
				currentRow = &rows.back();
				currentX = startPos.x;
			}

			// 设置节点位置
			node->SetPosition(ImVec2(currentX, currentRow->y));

			// 更新行参数
			currentRow->width = currentX - startPos.x + size.x;
			currentRow->height = std::max(currentRow->height, size.y);
			currentX += size.x + horizontalSpacing;
		}

		// 计算层总高度
		float layerHeight = 0.0f;
		for (const auto& row : rows) {
			layerHeight = std::max(layerHeight, row.y + row.height - currentY);
		}
		currentY += layerHeight + layerSpacing;
	}
}