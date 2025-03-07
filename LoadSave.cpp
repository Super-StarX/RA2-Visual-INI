#include "MainWindow.h"
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/ListNode.h"
#include "Pins/KeyValue.h"
#include "version.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <random>
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <shellapi.h>

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
	if (!OpenFileDialog("INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0",
		path, MAX_PATH, isSaving)) {
		return;
	}

	std::filesystem::path filePath(path);
	if (isSaving) {
		// 强制添加.ini后缀（不覆盖原有扩展名）
		if (filePath.extension() != ".ini") {
			filePath += ".ini";
		}

		Owner->ExportINI(filePath.string());
		ShellExecuteA(nullptr, "open", path, nullptr, nullptr, SW_SHOW);
	}
	else {
		Owner->ImportINI(filePath.string());
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

	{
		std::string version = root["Version"];
		int major, minor, revision, patch;
		sscanf_s(version.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &patch);
		if (VERSION_PATCH != patch) {
			wchar_t buffer[0x100] = { 0 };
			swprintf_s(buffer, L"当前版本：%hs\n存档版本：%hs\n\n要继续读取吗？", FILE_VERSION_STR, version.c_str());
			if (MessageBox(NULL, buffer, L"警告：版本不同，存档可能不兼容！", MB_YESNO | MB_ICONWARNING) == IDNO) {
				return;
			}
		}
	}

	// 清空现有数据
	Node::Array.clear();
	Pin::Array.clear();
	Link::Array.clear();

	// 加载 Nodes
	for (const auto& nodeJson : root["Nodes"]) {
		switch (static_cast<NodeType>(nodeJson["Type"])) {
		case NodeType::Section: {
			auto node = std::make_unique<SectionNode>("", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			break;
		}
		case NodeType::Tag: {
			/*
			auto node = std::make_unique<TagNode>("", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			*/
			break;
		}
		default:
			throw "Unsupported node!";
		}
	}

	// 加载 Links
	for (const auto& linkJson : root["Links"]) {
		auto link = std::make_unique<Link>(-1, 0, 0);
		link->LoadFromJson(linkJson);
		Link::Array.push_back(std::move(link));
	}
	SetNextId(root["Totals"].get<int>() + 1);

	std::cout << "Data loaded from " << filePath << "\n";
	ed::NavigateToContent();
}

void MainWindow::SaveProject(const std::string& filePath) {
	using json = nlohmann::json;
	json root;

	// 保存 Nodes
	root["Totals"] = GetNextId() - 1;
	root["Version"] = FILE_VERSION_STR;
	for (const auto& node : Node::Array) {
		json nodeJson;
		node->SaveToJson(nodeJson);
		root["Nodes"].push_back(nodeJson);
	}

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
	ApplyForceDirectedLayout();
	ed::NavigateToContent();
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

			auto linkedNode = kv->GetLinkedNode();
			if (!linkedNode) {
				output.emplace_back(kv->Key, kv->Value);
				continue;
			}

			// 处理SectionNode链接
			if (auto sectionLinked = dynamic_cast<SectionNode*>(linkedNode)) {
				if (!kv->IsInherited) {
					output.emplace_back(kv->Key, sectionLinked->Name);
				}
				else {
					Collect(sectionLinked, output, visited);
				}
			}
			// 处理TagNode链接
			else if (auto tagLinked = dynamic_cast<TagNode*>(linkedNode)) {
				if (tagLinked->IsConstant) {
					output.emplace_back(kv->Key, tagLinked->Name);
				}
				else {
					std::unordered_set<Node*> tagVisited;
					std::string value = tagLinked->ResolveTagPointer(tagLinked, tagVisited);
					output.emplace_back(kv->Key, value);
				}
			}
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