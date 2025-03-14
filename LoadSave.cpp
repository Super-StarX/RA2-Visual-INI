#include "MainWindow.h"
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/GroupNode.h"
#include "Nodes/ListNode.h"
#include "Pins/KeyValue.h"
#include "version.h"
#include "Utils.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <random>
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <shellapi.h>

bool OpenFileDialog(LPCSTR fliter, char* path, int maxPath, bool isSaving) {
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
	ClearAll();

	// 加载 Nodes
	for (const auto& nodeJson : root["Nodes"]) {
		switch (static_cast<NodeType>(nodeJson["Type"])) {
		case NodeType::Section: {
			auto node = std::make_unique<SectionNode>("", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			break;
		}
		case NodeType::List: {
			auto node = std::make_unique<ListNode>("", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			break;
		}
		case NodeType::Group: {
			auto node = std::make_unique<GroupNode>("", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
			break;
		}
		case NodeType::Tag: {
			std::string type = nodeJson["TagType"];
			bool isInput = false;
			bool isConst = false;
			if (type == "Input") {
				isInput = true;
				isConst = false;
			}
			else if (type == "Const") {
				isInput = false;
				isConst = true;
			}
			else if (type == "Output") {
				isInput = false;
				isConst = false;
			}
			auto node = std::make_unique<TagNode>(isInput, "", -1);
			node->IsInput = isInput;
			node->IsConstant = isConst;
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
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
		if (node->GetNodeType() == NodeType::IO)
			root["IO"].push_back(nodeJson);
		else
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
	std::wifstream file(path);
	std::string line, currentSection, comment;
	std::wstring wline;
	file.imbue(std::locale("zh_CN.UTF-8"));
	auto fromWString = [](const std::wstring& wstr) {
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string result(sizeNeeded - 1, 0); // -1 是为了去掉 null 终止符
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], sizeNeeded, nullptr, nullptr);
		return result;
	};

	while (std::getline(file, wline)) {
		line = fromWString(wline);

		if (!Utils::IsCommentSection(line)) {
			// 将注释与正文分隔开
			auto commentPos = line.find(';');
			if (commentPos != std::string::npos) {
				comment = Utils::Trim(line.substr(commentPos + 1));
				line = line.substr(0, commentPos);
			}
			else {
				comment.clear();
			}

			// 去除注释后如果没有任何内容，则跳过
			line = Utils::Trim(line);
			if (line.empty())
				continue;
		}

		auto bracketPos = line.find("[");
		if (bracketPos != std::string::npos) { // 新的Section开始了
			auto inheritPos = line.find(":");
			SectionNode* toConnect = nullptr;

			// 提取Section Name的函数
			auto substractSectionName = [](const std::string& str) -> std::string {
				if (size_t l = str.find('['); l != str.npos)
					if (size_t r = str.find(']', l + 1); r != str.npos)
						return str.substr(l + 1, r - l - 1);
				return "";
			};

			if (inheritPos == std::string::npos) { // 普通section
				currentSection = substractSectionName(line);
				if (currentSection.empty())
					continue;
				toConnect = nullptr;
			}
			else { // 有继承的Section []:[]
				auto former = line.substr(0, inheritPos);
				auto latter = line.substr(inheritPos + 1);
				currentSection = substractSectionName(former);
				if (currentSection.empty())
					continue;

				auto toConnectSection = substractSectionName(latter);
				if (auto it = SectionNode::Map.find(toConnectSection); it != SectionNode::Map.end())
					toConnect = it->second;
				else
					toConnect = nullptr;
			}

			auto sectionNode = Node::Create<SectionNode>(currentSection.c_str());
			if (Utils::IsCommentSection(line))
				sectionNode->IsComment = true;
			if (toConnect)
				sectionNode->OutputPin->LinkTo(toConnect->InputPin.get());
		}
	}

	// 重置文件指针到文件开头
	file.clear();
	file.seekg(0, std::ios::beg);

	auto it = Node::Array.begin();
	auto current = it;
	while (std::getline(file, wline)) {
		auto currentNode = reinterpret_cast<SectionNode*>(current->get());
		line = fromWString(wline);

		if (!Utils::IsCommentSection(line) && !currentNode->IsComment) {
			// 将注释与正文分隔开
			auto commentPos = line.find(';');
			if (commentPos != std::string::npos) {
				comment = Utils::Trim(line.substr(commentPos + 1));
				line = line.substr(0, commentPos);
			}
			else {
				comment.clear();
			}

			// 去除注释后如果没有任何内容，则跳过
			line = Utils::Trim(line);
			if (line.empty())
				continue;
		}

		auto bracketPos = line.find("[");
		if (bracketPos != std::string::npos) { // 新的Section开始了
			current = it++;
			continue;
		}
		else {
			// 如果当前节点是注释节点，移除所有语句前面的注释
			if (currentNode->IsComment) {
				if (line.find('=') == std::string::npos)
					continue;
				if (auto pos = line.find(';'); pos != std::string::npos)
					line = line.replace(line.find(';'), 1, "");
			}

			// 处理键值对逻辑
			std::istringstream iss(line);
			std::string key, value;
			if (std::getline(iss, key, '=') && std::getline(iss, value)) {
				// 去除键中的注释（分号开头）
				bool isComment = false;
				if (!key.empty() && key[0] == ';') {
					isComment = true;
					key = key.substr(1);
					while (!key.empty() && key[0] == ';') // 去除后续可能的前导分号（如 ";;key"）
						key = key.substr(1);
				}
				
				if (currentNode) {
					currentNode->SetPosition({ 0, 0 });
					auto kv = currentNode->AddKeyValue(key, value, comment, 0, false, isComment);
					// 如果场内有对应的 section 就连上 Link
					if (SectionNode::Map.contains(value)) {
						auto targetNode = SectionNode::Map[value];
						if (targetNode->InputPin->CanCreateLink(kv)) {
							auto link = kv->LinkTo(targetNode->InputPin.get());
							link->TypeIdentifier = kv->GetLinkType();
						}
					}
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

			if (kv->IsInherited) {
				if (auto sectionLinked = dynamic_cast<SectionNode*>(kv->GetLinkedNode())) {
					Collect(sectionLinked, output, visited);
					continue;
				}
			}

			output.emplace_back(kv->Key, kv->GetValue());
		}
	};

	// 主逻辑
	for (auto& node : SectionNode::Array) {
		if (node->IsComment)
			continue;

		file << "[" << node->Name << "]";

		if (auto output = node->OutputPin.get()->GetLinkedNode())
			//if (node->Name != output->GetValue())
				file << ":[" << output->GetValue() << "]";

		file << "\n";
			
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