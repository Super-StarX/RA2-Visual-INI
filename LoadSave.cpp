#include "MainWindow.h"
#include "nodes/SectionNode.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <random>
#include <windows.h>
#include <commdlg.h>

bool OpenFileDialog(LPCSTR fliter,char* path, int maxPath, bool isSaving) {
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = fliter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (isSaving) {
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	else {
		if (GetOpenFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	return false;
}

void LeftPanelClass::ShowINIFileDialog(bool isSaving) {
	char path[MAX_PATH] = { 0 };
	if (OpenFileDialog("INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0", path, MAX_PATH, isSaving)) {
		if (isSaving)
			Owner->ExportINI(path);
		else
			Owner->ImportINI(path);
	}
}

void LeftPanelClass::ShowProjFileDialog(bool isSaving) {
	char path[MAX_PATH] = { 0 };
	if (OpenFileDialog("Project Files (*.viproj)\0*.viproj\0All Files (*.*)\0*.*\0", path, MAX_PATH, isSaving)) {
		if (isSaving)
			Owner->SaveProject(path);
		else
			Owner->LoadProject(path);
	}
}

void MainWindow::LoadProject(const std::string& filePath) {
	using json = nlohmann::json;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		// 处理文件打开失败的情况
		return;
	}

	json j = json::parse(file);

	// 清除现有数据
	m_SectionMap.clear();
	m_Links.clear();
	
	// 加载节点
	for (const auto& nodeJson : j["nodes"]) {
		std::string section = nodeJson["section"].get<std::string>();
		ImVec2 position = { 
			float(nodeJson["position"][0].get<int>()),
			float(nodeJson["position"][1].get<int>())
		};

		auto node = SpawnSectionNode(section);
		node->SetPosition(position);

		for (const auto& kvJson : nodeJson["key_values"]) {
			std::string key = kvJson["key"].get<std::string>();
			std::string value = kvJson["value"].get<std::string>();
			int outputPinId = kvJson["output_pin_id"].get<int>();

			auto& kv = node->KeyValues.emplace_back(key, value,
				std::make_unique<Pin>(outputPinId, key.c_str())
			);
			kv.OutputPin->Node = node;
			kv.OutputPin->Kind = PinKind::Output;
		}

		node->InputPin = std::make_unique<Pin>(GetNextId(), "input");
		node->OutputPin = std::make_unique<Pin>(GetNextId(), "output");
	}

	// 加载链接
	for (const auto& linkJson : j["links"]) {
		int startPinId = linkJson["start_pin_id"].get<int>();
		int endPinId = linkJson["end_pin_id"].get<int>();

		auto startPin = FindPin(startPinId);
		auto endPin = FindPin(endPinId);

		if (startPin && endPin) {
			CreateLink(startPinId, endPinId);
		}
	}
	
}

//把注释取消，就会开局弹框，我不理解
void MainWindow::SaveProject(const std::string& filePath) {
	using json = nlohmann::json;
	json j;

	// 保存节点信息
	for (const auto& [section, node] : m_SectionMap) {
		json nodeJson;
		nodeJson["section"] = node->Name;

		auto pos = node->GetPosition();
		nodeJson["position"] = { pos.x, pos.y };

		json keyValuesJson;
		for (const auto& kv : node->KeyValues) {
			json kvJson;
			kvJson["key"] = kv.Key;
			kvJson["value"] = kv.Value;
			kvJson["output_pin_id"] = kv.OutputPin->ID.Get();
			keyValuesJson.push_back(kvJson);
		}
		nodeJson["key_values"] = keyValuesJson;

		j["nodes"].push_back(nodeJson);
	}

	// 保存链接信息
	for (const auto& link : m_Links) {
		json linkJson;
		linkJson["start_pin_id"] = link.StartPinID.Get();
		linkJson["end_pin_id"] = link.EndPinID.Get();
		j["links"].push_back(linkJson);
	}

	std::ofstream file(filePath);
	if (!file.is_open()) {
		// 处理文件打开失败的情况
		return;
	}
	file << std::setw(4) << j << std::endl;
}

void MainWindow::ImportINI(const std::string& path) {
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
					std::make_unique<Pin>(GetNextId(), key.c_str())
				);
				kv.OutputPin->Node = currentNode;
				kv.OutputPin->Kind = PinKind::Output;
				kv.IsComment = line.find(';') != std::string::npos;
				// 这里还差一句把';'去掉

				// 创建连线
				if (m_SectionMap.contains(value)) {
					auto targetNode = m_SectionMap[value];
					if (Pin::CanCreateLink(kv.OutputPin.get(), targetNode->InputPin.get())) {
						CreateLink(kv.OutputPin->ID, targetNode->InputPin->ID)->TypeIdentifier = kv.OutputPin->GetLinkType();
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
				if (Pin::CanCreateLink(kv.OutputPin.get(), targetNode->InputPin.get())) {
					CreateLink(kv.OutputPin->ID, targetNode->InputPin->ID)->TypeIdentifier = kv.OutputPin->GetLinkType();
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

void MainWindow::ExportINI(const std::string& path) {
	std::ofstream file(path);

	// 定义处理单个键值对的 lambda
	auto ProcessKeyValue = [&](SectionNode::KeyValuePair& kv, std::vector<std::pair<std::string, std::string>>& output, std::unordered_set<SectionNode*>& visited, bool isRootProcessing) {
		// 这里两处(SectionNode*)会导致如果有非Section的Node会在node->KeyValues处弹框, 待修复
		auto linkedNode = (SectionNode*)GetLinkedNode(kv.OutputPin->ID);
		if (!linkedNode) {
			output.emplace_back(kv.Key, kv.Value);
			return;
		}

		if (!kv.IsInherited) {
			output.emplace_back(kv.Key, linkedNode->Name);
			return;
		}

		// 递归处理继承节点
		std::function<void(SectionNode*)> Collect = [&](SectionNode* node) {
			if (!node || visited.count(node)) return;
			visited.insert(node);

			for (auto& childKv : node->KeyValues) {
				if (childKv.IsComment) continue;

				auto childLinkedNode = (SectionNode*)GetLinkedNode(childKv.OutputPin->ID);
				if (!childLinkedNode) {
					output.emplace_back(childKv.Key, childKv.Value);
					continue;
				}

				if (childKv.IsInherited)
					Collect(childLinkedNode);
				else
					output.emplace_back(childKv.Key, childLinkedNode->Name);
			}
		};

		// 根节点的继承需要新建 visited 集合
		if (isRootProcessing) {
			std::unordered_set<SectionNode*> newVisited;
			Collect(linkedNode);
		}
		else
			Collect(linkedNode);
	};

	// 主逻辑
	for (auto& [section, node] : m_SectionMap) {
		if (node->IsComment)
			continue;

		file << "[" << section << "]\n";
		std::vector<std::pair<std::string, std::string>> outputEntries;
		std::unordered_set<SectionNode*> visited;

		for (auto& kv : node->KeyValues) {
			if (kv.IsComment) continue;

			ProcessKeyValue(kv, outputEntries, visited, true); // true 表示根节点处理
		}

		// 写入文件（保留最后出现的重复键）
		std::unordered_map<std::string, std::string> finalMap;
		for (auto& entry : outputEntries)
			finalMap[entry.first] = entry.second; // 自动覆盖重复键
		for (auto& [key, val] : finalMap)
			file << key << "=" << val << "\n";

		file << "\n";
	}
}