﻿#include "MainWindow.h"
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/ModuleNode.h"
#include "Nodes/GroupNode.h"
#include "Nodes/ListNode.h"
#include "Nodes/IONode.h"
#include "Pins/KeyValue.h"
#include "version.h"
#include "PlaceholderReplacer.h"
#include "Utils.h"
#include "Log.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <random>
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <shellapi.h>

void LeftPanelClass::ShowINIFileDialog(bool isSaving) {
	char path[MAX_PATH] = { 0 };
	if (!Utils::OpenFileDialog("INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0",
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
	if (Utils::OpenFileDialog("Project Files (*.viproj)\0*.viproj\0All Files (*.*)\0*.*\0", path, MAX_PATH, isSaving)) {
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
	LOG_INFO("正在载入工程[{}]", filePath);
	using json = nlohmann::json;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for reading.\n";
		LOG_WARN("无法打开文件[{}]!", filePath);
		return;
	}

	json root;
	file >> root;
	file.close();

	{
		std::string version = root["Version"];
		LOG_INFO("目标工程版本：{}", version);
		int major, minor, revision, patch;
		sscanf_s(version.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &patch);
		if (VERSION_PATCH != patch) {
			wchar_t buffer[0x100] = { 0 };
			swprintf_s(buffer, L"当前版本：%hs\n项目版本：%hs\n\n要继续读取吗？", FILE_VERSION_STR, version.c_str());
			if (MessageBox(NULL, buffer, L"警告：版本不同，项目可能不兼容！", MB_YESNO | MB_ICONWARNING) == IDNO) {
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
		case NodeType::Module: {
			auto node = std::make_unique<ModuleNode>("", -1);
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
		default: {
			LOG_WARN("未知节点类型！节点是[{}]", nodeJson["Name"].get<std::string>());
		}
		}
	}

	if (root.contains("IO")) {
		for (const auto& nodeJson : root["IO"]) {
			auto node = std::make_unique<IONode>(static_cast<PinKind>(nodeJson["Mode"]),"", -1);
			node->LoadFromJson(nodeJson);
			Node::Array.push_back(std::move(node));
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
	LOG_INFO("工程载入完毕!");
	ed::NavigateToContent();
}

void MainWindow::SaveProject(const std::string& filePath) {
	using json = nlohmann::json;
	json root;
	LOG_INFO("正在保存工程[{}]", filePath);
	// 保存 Nodes
	root["Totals"] = GetNextId() - 1;
	root["Version"] = FILE_VERSION_STR;
	LOG_INFO("当前版本：{}", FILE_VERSION_STR);
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
		LOG_INFO("工程保存完毕!");
	}
	else {
		LOG_ERROR("文件[{}]打开失败，保存中止", filePath);
		std::cerr << "Failed to open file for writing.\n";
	}
}

void MainWindow::ImportINI(const std::string& path) {
	LOG_INFO("正在准备载入[{}]", path);
	ClearAll();
	SetNextId(1);
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
			if (currentNode)
				currentNode->AutoSelectType();
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
	LOG_INFO("载入完毕");
	EnableApplyForceDirectedLayout();
	ed::NavigateToContent();
}

static void ParseNode(std::vector<SectionNode*>& ret, const std::vector<std::unique_ptr<Node>>& array) {
	for (auto& node : array) {
		if (auto sectionNode = dynamic_cast<SectionNode*>(node.get())) {
			ret.push_back(sectionNode);
		}
		else if (auto moduleNode = dynamic_cast<ModuleNode*>(node.get())) {
			moduleNode->OpenProject();
			ParseNode(ret, moduleNode->Nodes);
		}
	}
}

static void UnparseNode() {
	for (auto& node : Node::Array) {
		if (auto moduleNode = dynamic_cast<ModuleNode*>(node.get())) {
			moduleNode->CloseProject();
		}
	}
}

void MainWindow::ExportINI(const std::string& path) {
	std::ofstream file(path);
	PlaceholderReplacer replacer({ .projectName = std::filesystem::path(path).stem().string() });

	int id = MainWindow::GetNextId();
	MainWindow::SetIdOffset(id + 10);
	std::vector<SectionNode*> Array;
	ParseNode(Array, Node::Array);

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

	// 已导出节点集合
	std::unordered_set<SectionNode*> exported;

	// 循环注释：node -> "Cycle: A -> B -> A"
	std::unordered_map<SectionNode*, std::string> cycleAnnotations;

	// 递归导出函数
	std::function<void(SectionNode*, std::vector<SectionNode*>)> ExportNode;
	ExportNode = [&](SectionNode* node, std::vector<SectionNode*> callStack) {
		if (!node || node->IsComment)
			return;

		// 检测循环
		if (std::find(callStack.begin(), callStack.end(), node) != callStack.end()) {
			std::string cycle = "Cycle: ";
			bool started = false;
			for (auto* n : callStack) {
				if (n == node || started) {
					if (!cycle.empty() && cycle.back() != ' ')
						cycle += " -> ";
					cycle += n->Name;
					started = true;
				}
			}
			cycle += " -> " + node->Name;
			cycleAnnotations[node] = cycle;
			return;
		}

		if (exported.count(node))
			return;

		callStack.push_back(node);

		// 先导出 OutputPin 指向的父节点（继承来源）
		if (auto* output = dynamic_cast<SectionNode*>(node->OutputPin.get()->GetLinkedNode())) {
			ExportNode(output, callStack);
		}

		exported.insert(node);

		// 写 section 标头
		file << "[" << replacer.replace(node->Name) << "]";

		auto* output = dynamic_cast<SectionNode*>(node->OutputPin.get()->GetLinkedNode());
		if (output) {
			file << ":[" << replacer.replace(output->GetValue()) << "]";

			// 如果继承目标未被导出，添加提醒
			if (!exported.count(output)) {
				file << " ; Warning: [" << replacer.replace(output->GetValue()) << "] not declared before use (possible cycle or invalid order)";
			}
		}

		// 如果当前节点存在循环记录，也输出注释
		if (cycleAnnotations.count(node)) {
			file << " ; " << cycleAnnotations[node];
		}

		file << "\n";


		// 收集键值
		std::vector<std::pair<std::string, std::string>> outputEntries;
		std::unordered_set<SectionNode*> visited;
		Collect(node, outputEntries, visited);

		// 保留最后值的键
		std::unordered_map<std::string, size_t> finalMap;
		for (size_t i = 0; i < outputEntries.size(); ++i)
			finalMap[outputEntries[i].first] = i;

		std::vector<std::pair<std::string, size_t>> vec(finalMap.begin(), finalMap.end());
		std::sort(vec.begin(), vec.end(), [](auto& a, auto& b) { return a.second < b.second; });

		for (auto& [key, idx] : vec)
			file << key << "=" << replacer.replace(outputEntries[idx].second) << "\n";

		file << "\n";
	};

	// 遍历所有节点，递归导出
	for (auto* node : Array) {
		std::vector<SectionNode*> callStack;
		ExportNode(node, callStack);
	}

	MainWindow::SetNextId(id);
	MainWindow::SetIdOffset(0);
	UnparseNode();
}