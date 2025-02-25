#include "MainWindow.h"
#include "Nodes/SectionNode.h"
#include "Pins/KeyValue.h"
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
		std::string str(path);
		if (!str.ends_with(".ini"))
			str += ".ini";
		if (isSaving)
			Owner->ExportINI(path);
		else
			Owner->ImportINI(path);
	}
}

void LeftPanelClass::ShowProjFileDialog(bool isSaving) {
	char path[MAX_PATH] = { 0 };
	if (OpenFileDialog("Project Files (*.viproj)\0*.viproj\0All Files (*.*)\0*.*\0", path, MAX_PATH, isSaving)) {
		std::string str(path);
		if (!str.ends_with(".viproj"))
			str += ".viproj";
		if (isSaving)
			Owner->SaveProject(str);
		else
			Owner->LoadProject(str);
	}
}

void MainWindow::LoadProject(const std::string& filePath) {
	using json = nlohmann::json;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for reading.\n";
		return;
	}

	json root;
	file >> root;
	file.close();

	// 清空现有数据
	Node::Array.clear();
	Pin::Array.clear();
	Link::Array.clear();

	// 加载 Nodes
	for (const auto& nodeJson : root["Nodes"]) {
		switch (static_cast<NodeType>(nodeJson["Type"])) {
		case NodeType::Section: {
			auto node = std::make_unique<SectionNode>(this, -1, "");
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			break;
		}
		default:
			throw "Unsupported node!";
		}
	}

	// 加载 Pins
	/*
	for (const auto& pinJson : root["Pins"]) {
		auto pin = new Pin(0, ""); // 创建临时对象
		pin->LoadFromJson(pinJson);
		Pin::Array[pin->ID] = pin;
	}
	*/
	// 加载 Links
	/*
	for (const auto& linkJson : root["Links"]) {
		auto link = std::make_unique<Link>(0, 0, 0);
		link->LoadFromJson(linkJson);
		Link::Array.push_back(std::move(link));
	}
	*/
	SetNextId(root["Totals"].get<int>() + 1);

	std::cout << "Data loaded from " << filePath << "\n";
}

//把注释取消，就会开局弹框，我不理解
void MainWindow::SaveProject(const std::string& filePath) {
	using json = nlohmann::json;
	json root;

	// 保存 Nodes
	root["Totals"] = GetNextId() - 1;
	for (const auto& node : Node::Array) {
		json nodeJson;
		node->SaveToJson(nodeJson);
		root["Nodes"].push_back(nodeJson);
	}

	// 保存 Pins
	/*
	for (const auto& [id, pin] : Pin::Array) {
		json pinJson;
		pin->SaveToJson(pinJson);
		root["Pins"].push_back(pinJson);
	}
	*/
	// 保存 Links
	for (const auto& link : Link::Array) {
		json linkJson;
		link->SaveToJson(linkJson);
		root["Links"].push_back(linkJson);
	}

	// 写入文件
	std::ofstream file(filePath);
	if (file.is_open()) {
		file << root.dump(4); // 格式化输出
		file.close();
		std::cout << "Data saved to " << filePath << "\n";
	}
	else {
		std::cerr << "Failed to open file for writing.\n";
	}
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
	auto& map = SectionNode::Map;
	while (std::getline(file, line)) {
		if (line.find('[') != std::string::npos) {
			currentSection = line.substr(1, line.find(']') - 1);
		}
		else if (!currentSection.empty() && line.find('=') != std::string::npos) {
			std::istringstream iss(line);
			std::string key, value;
			if (std::getline(iss, key, '=') && std::getline(iss, value)) {
				auto currentNode = map[currentSection];
				currentNode->SetPosition({ 0, 0 });
				bool isComment = line.find(';') != std::string::npos;
				// TODO: 这里还差一句把';'去掉

				// 添加输出引脚
				auto kv = currentNode->AddKeyValue(key, value, 0, false, isComment);

				// 创建连线
				if (map.contains(value)) {
					auto targetNode = map[value];
					if (targetNode->InputPin->CanCreateLink(kv)) {
						auto link = kv->LinkTo(targetNode->InputPin.get());
						link->TypeIdentifier = kv->GetLinkType();
					}
				}
			}
		}
	}

	// 遍历所有 key-value 对进行最终连线检查
	for (const auto& [section, pairs] : keyValuePairs) {
		for (const auto& [key, value] : pairs) {
			auto currentNode = map[section];
			if (map.contains(value)) {
				auto targetNode = map[value];
				if (currentNode->KeyValues.empty())
					continue;
				auto kv = currentNode->KeyValues.back().get(); // 假设每个 key-value 对都已添加到 KeyValues 中
				if (targetNode->InputPin->CanCreateLink(kv)){
					auto link = kv->LinkTo(targetNode->InputPin.get());
					link->TypeIdentifier = kv->GetLinkType();
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

		const float angleStep = 2 * 3.14159f / Node::Array.size();
		float currentAngle = 0;
		for (auto& node : Node::Array) {
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

	// 递归处理继承节点
	// 处理流程：
	// 遍历全部键值对
	// 1. 注释直接忽略
	// 2. 未链接节点的输出
	// 3. 未继承的，输出key=链接的节点名（目前节点名与value名同步，似乎没必要单独判断，不过还是保留了）
	// 4. 继承的，递归调用函数
	std::function<void(SectionNode*, std::vector<std::pair<std::string, std::string>>&, std::unordered_set<SectionNode*>&)> Collect =
		[&](SectionNode* node, std::vector<std::pair<std::string, std::string>>& output, std::unordered_set<SectionNode*>& visited) {
		
		if (!node || visited.count(node)) 
			return;

		visited.insert(node);

		for (auto& kv : node->KeyValues) {
			if (kv->IsComment)
				continue;

			auto linkedNode = kv->GetLinkedSection();
			if (!linkedNode) {
				output.emplace_back(kv->Key, kv->Value);
				continue;
			}

			if (!kv->IsInherited) {
				output.emplace_back(kv->Key, linkedNode->Name);
				continue;
			}

			// 继承的情况，递归调用
			Collect(linkedNode, output, visited);
		}
	};

	// 主逻辑
	for (auto& [section, node] : SectionNode::Map) {
		if (node->IsComment)
			continue;

		file << "[" << section << "]\n";
		std::vector<std::pair<std::string, std::string>> outputEntries;
		std::unordered_set<SectionNode*> visited;

		Collect(node, outputEntries, visited);

		// 写入文件（保留最后出现的重复键）
		std::unordered_map<std::string, size_t> finalMap;

		for (size_t i = 0; i < outputEntries.size(); i++)
			finalMap[outputEntries[i].first] = i; // 自动覆盖重复键

		std::vector<std::pair<std::string, size_t>> vec(finalMap.begin(), finalMap.end());
		std::sort(vec.begin(), vec.end(), [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) {
			return a.second < b.second;
		});

		for (auto& [key, val] : vec)
			file << key << "=" << outputEntries[val].second << "\n";

		file << "\n";
	}
}