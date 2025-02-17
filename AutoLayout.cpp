#include "MainWindow.h"
#include <imgui_node_editor_internal.h>

float REPULSION_FORCE = 400000.0f;
float ATTRACTION_FORCE = 0.02f;

// 添加网格划分辅助结构
struct GridCell {
	std::vector<Node*> Nodes;
};

class SpatialGrid {
private:
	float m_CellSize;
	std::unordered_map<int, std::unordered_map<int, GridCell>> m_Grid;

public:
	explicit SpatialGrid(float cellSize = 200.0f) : m_CellSize(cellSize) {}

	void Clear() {
		m_Grid.clear();
	}

	void Insert(Node* node) {
		ImVec2 pos = node->GetPosition();
		int x = static_cast<int>(pos.x / m_CellSize);
		int y = static_cast<int>(pos.y / m_CellSize);
		m_Grid[x][y].Nodes.push_back(node);
	}

	std::vector<Node*> GetNearbyNodes(Node* node) {
		std::vector<Node*> result;
		ImVec2 pos = node->GetPosition();
		int x = static_cast<int>(pos.x / m_CellSize);
		int y = static_cast<int>(pos.y / m_CellSize);

		// 检查3x3区域
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				if (m_Grid.count(x + i) && m_Grid[x + i].count(y + j)) {
					auto& cell = m_Grid[x + i][y + j];
					result.insert(result.end(), cell.Nodes.begin(), cell.Nodes.end());
				}
			}
		}
		return result;
	}
};

void MainWindow::ApplyForceDirectedLayout() {
	// 初始化空间网格
	SpatialGrid grid(150.0f); // 根据节点平均大小调整网格尺寸
	for (auto& node : m_Nodes)
		grid.Insert(node.get());

	// 初始化物理状态
	std::unordered_map<Node*, ImVec2> velocities;
	std::unordered_map<Node*, ImVec2> accelerations;
	for (auto& node : m_Nodes) {
		velocities[node.get()] = ImVec2(0, 0);
		accelerations[node.get()] = ImVec2(0, 0);
	}

	// 迭代优化参数
	constexpr int ITERATIONS = 500;
	constexpr float DAMPING = 0.85f;
	constexpr float MAX_SPEED = 15.0f;
	constexpr float PADDING = 30.0f;

	for (int iter = 0; iter < ITERATIONS; ++iter) {
		// 重置加速度
		for (auto& [node, acc] : accelerations)
			acc = ImVec2(0, 0);

		// 节点间斥力（使用空间网格优化）
		for (auto& node : m_Nodes) {
			auto nearbyNodes = grid.GetNearbyNodes(node.get());

			const ImVec2 pos1 = node->GetPosition();
			const ImVec2 size1 = GetNodeSize(node->ID) + ImVec2(PADDING, PADDING);

			for (auto& other : nearbyNodes) {
				if (node.get() == other) continue;

				const ImVec2 pos2 = other->GetPosition();
				const ImVec2 size2 = GetNodeSize(other->ID) + ImVec2(PADDING, PADDING);

				// 计算实际边界距离
				const float minDistX = (size1.x + size2.x) / 2;
				const float minDistY = (size1.y + size2.y) * 0.5f;
				const float overlapX = minDistX - std::abs(pos1.x - pos2.x);
				const float overlapY = minDistY - std::abs(pos1.y - pos2.y);

				if (overlapX > 0 && overlapY > 0) {
					const float dist = std::hypot(pos1.x - pos2.x, pos1.y - pos2.y);
					const float safeDist = std::hypot(minDistX, minDistY);
					const float force = REPULSION_FORCE / (dist * dist + 1e-6f);
					const ImVec2 dir = ImNormalized(ImVec2(pos2.x - pos1.x, pos2.y - pos1.y));

					accelerations[node.get()] -= dir * (force + (std::min)(overlapX, overlapY) * 2.0f);
					accelerations[other] += dir * (force + (std::min)(overlapX, overlapY) * 2.0f);
				}
			}
		}

		// 连接引力
		for (auto& link : m_Links) {
			auto startPin = FindPin(link->StartPinID);
			auto endPin = FindPin(link->EndPinID);
			if (!startPin || !endPin) continue;

			Node* startNode = startPin->Node;
			Node* endNode = endPin->Node;

			const ImVec2 delta = endNode->GetPosition() - startNode->GetPosition();
			const float dist = (std::max)(ImLength(delta), 1.0f);

			// 动态调整理想距离
			const int linkCount = startNode->GetConnectedLinkCount();
			const float idealDist = 150.0f / std::sqrtf(linkCount + 1.f);
			const float attraction = ATTRACTION_FORCE * (dist - idealDist) / dist;

			accelerations[startNode] += delta * attraction;
			accelerations[endNode] -= delta * attraction;
		}

		// 更新运动状态
		for (auto& [node, acc] : accelerations) {
			ImVec2& vel = velocities[node];

			// 更新速度
			vel = vel * DAMPING + acc;

			// 限速
			const float speed = ImLength(vel);
			if (speed > MAX_SPEED)
				vel = ImNormalized(vel) * MAX_SPEED;

			// 更新位置
			node->SetPosition(node->GetPosition() + vel);
		}

		// 每5次迭代更新一次网格（平衡精度和性能）
		if (iter % 5 == 0) {
			grid.Clear();
			for (auto& node : m_Nodes)
				grid.Insert(node.get());
		}
	}

	// 最终对齐（保持原逻辑）
	for (auto& node : m_Nodes) {
		ImVec2 pos = node->GetPosition();
		pos.x = std::round(pos.x / 10) * 10;
		pos.y = std::round(pos.y / 10) * 10;
		node->SetPosition(pos);
	}
}
