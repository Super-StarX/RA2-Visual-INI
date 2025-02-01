#include "MainWindow.h"
#include "nodes/SectionNode.h"

#include <fstream>
#include <sstream>
#include <random>

void MainWindow::LoadINI(const std::string& path) {
	ClearAll();
	std::ifstream file(path);
	std::string line, currentSection;

	std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> keyValuePairs;

	// 第一次遍历：记录所有的 section 和 key-value 对
	while (std::getline(file, line)) {
		if (line.find('[') != std::string::npos) {
			currentSection = line.substr(1, line.find(']') - 1);
			SpawnSectionNode(currentSection);
		}
		else if (!currentSection.empty() && line.find('=') != std::string::npos) {
			std::istringstream iss(line);
			std::string key, value;
			if (std::getline(iss, key, '=') && std::getline(iss, value))
				keyValuePairs[currentSection].emplace_back(key, value);
		}
	}

	// 重建所有节点
	BuildNodes();

	// 重置文件指针到文件开头
	file.clear();
	file.seekg(0, std::ios::beg);

	// 第二次遍历：处理所有的 key-value 对并进行连线
	while (std::getline(file, line)) {
		if (line.find('[') != std::string::npos) {
			currentSection = line.substr(1, line.find(']') - 1);
		}
		else if (!currentSection.empty() && line.find('=') != std::string::npos) {
			std::istringstream iss(line);
			std::string key, value;
			if (std::getline(iss, key, '=') && std::getline(iss, value)) {
				auto currentNode = m_SectionMap[currentSection];
				currentNode->SetPosition({ 0, 0 });

				// 添加输出引脚
				auto& kv = currentNode->KeyValues.emplace_back(key, value,
					Pin{ GetNextId(), key.c_str(), PinType::Flow }
				);
				kv.OutputPin.Node = currentNode;
				kv.OutputPin.Kind = PinKind::Output;

				// 创建连线
				if (m_SectionMap.contains(value)) {
					auto targetNode = m_SectionMap[value];
					if (Pin::CanCreateLink(&kv.OutputPin, targetNode->InputPin.get())) {
						m_Links.emplace_back(Link(GetNextId(), kv.OutputPin.ID, targetNode->InputPin->ID));
						m_Links.back().Color = Pin::GetIconColor(kv.OutputPin.Type);
					}
				}
			}
		}
	}

	// 遍历所有 key-value 对进行最终连线检查
	for (const auto& [section, pairs] : keyValuePairs) {
		for (const auto& [key, value] : pairs) {
			auto currentNode = m_SectionMap[section];
			auto& kv = currentNode->KeyValues.back(); // 假设每个 key-value 对都已添加到 KeyValues 中
			if (m_SectionMap.contains(value)) {
				auto targetNode = m_SectionMap[value];
				if (Pin::CanCreateLink(&kv.OutputPin, targetNode->InputPin.get())) {
					m_Links.emplace_back(Link(GetNextId(), kv.OutputPin.ID, targetNode->InputPin->ID));
					m_Links.back().Color = Pin::GetIconColor(kv.OutputPin.Type);
				}
			}
		}
	}
	// 初始化位置（圆形布局）
	{
		// 获取可用布局区域（示例值，根据实际UI调整）
		const ImVec2 layoutSize = ImGui::GetContentRegionAvail();
		const ImVec2 center(layoutSize.x / 2, layoutSize.y / 2);
		const float radius = (std::min)(layoutSize.x, layoutSize.y) * 0.4f; // 修复std::min宏问题
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(-0.5f, 0.5f);

		const float angleStep = 2 * 3.14159f / m_Nodes.size();
		float currentAngle = 0;
		for (auto& node : m_Nodes) {
			ImVec2 pos = {
				float(center.x + radius * cos(currentAngle) + dis(gen) * 50),
				float(center.y + radius * sin(currentAngle) + dis(gen) * 50)
			};
			node->SetPosition(pos);
			currentAngle += angleStep;
		}
	}

	ApplyForceDirectedLayout();
}

void MainWindow::SaveINI(const std::string& path) {
	std::ofstream file(path);

	for (auto& [section, node] : m_SectionMap) {
		file << "[" << section << "]\n";
		for (auto& output : node->KeyValues)
			file << output.Key << "=" << output.Value << "\n";
		file << "\n";
	}
}