#include "ModuleNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include <Utils.h>

#include "SectionNode.h"
#include "ListNode.h"
#include "GroupNode.h"
#include "TagNode.h"
#include "IONode.h"
#include "NodeStyle.h"
#include "PlaceholderReplacer.h"

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_node_editor_internal.h>
#include <Windows.h>
#include <filesystem>
#include <version.h>

ModuleNode::ModuleNode(const char* name, int id) :
	INENode(name, id) {
}

void ModuleNode::Update() {
	auto builder = BuilderNode::GetBuilder();

	builder->Begin(this->ID);
	auto* typeInfo = NodeStyleManager::Get().FindType(Style);
	if (!typeInfo) return;
	builder->Header(typeInfo->Color);
	ImGui::Spring(0);
	ImGui::PushID(this);
	Utils::SetNextInputWidth(Name, 130.f);
	ImGui::InputText("##SectionName", &Name);
	ImGui::PopID();

	ImGui::Spring(0);
	builder->EndHeader();

	for (auto& input : this->Inputs)
		BuilderNode::UpdateInput(input);

	for (auto& output : this->Outputs)
		BuilderNode::UpdateOutput(output);

	builder->End();

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		LoadProject();
		ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] = false;
		ImGui::GetIO().MouseDown[ImGuiMouseButton_Right] = false;
	}
}

void ModuleNode::Menu() {
	INENode::Menu();
	ImGui::Separator();

	if (ImGui::MenuItem(LOCALE["Load Project"]))
		LoadProject();
}

void ModuleNode::Tooltip() {
	INENode::Tooltip();
	ImGui::Separator();
	std::string input = LOCALE["ModuleNode Inputs"];
	for (const auto& valuePin : Inputs) {
		if (!input.empty())
			input += ",";
		input += valuePin.Name;
	}
	input += ": " + input;
	ImGui::Text(input.c_str());

	std::string output = LOCALE["ModuleNode Outputs"];
	for (const auto& valuePin : Outputs) {
		if (!output.empty())
			output += ",";
		output += valuePin.Name;
	}
	output += ": " + output;
	ImGui::Text(output.c_str());
}

void ModuleNode::SaveToJson(json& j) const {
	INENode::SaveToJson(j);
	j["Path"] = Path;
}

void ModuleNode::LoadFromJson(const json& j, bool newId) {
	INENode::LoadFromJson(j, newId);
	LoadProject(j["Path"]);
}

void ModuleNode::LoadProject() {
	char path[MAX_PATH] = { 0 };
	if (!Utils::OpenFileDialog("Project Files (*.viproj)\0*.viproj\0All Files (*.*)\0*.*\0",
		path, MAX_PATH, false)) {
		return;
	}
	std::string str(path);
	if (!str.ends_with(".viproj"))
		str += ".viproj";

	LoadProject(str);
}

void ModuleNode::LoadProject(std::string path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for reading.\n";
		return;
	}
	Path = path;
	Name = std::filesystem::path(path).stem().string();
	InternalProject = json::parse(file);

	{
		std::string version = InternalProject["Version"];
		int major, minor, revision, patch;
		sscanf_s(version.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &patch);
		if (VERSION_PATCH != patch) {
			wchar_t buffer[0x100] = { 0 };
			swprintf_s(buffer, L"当前版本：%hs\n项目版本：%hs\n\n要继续读取吗？", FILE_VERSION_STR, version.c_str());
			if (MessageBox(NULL, buffer, L"警告：版本不同，项目可能不兼容！", MB_YESNO | MB_ICONWARNING) == IDNO) {
				InternalProject.clear();
				return;
			}
		}
	}

	UpdatePins();
}

void ModuleNode::OpenProject() {
	if (InternalProject.empty())
		return;

	json root = json::parse(PlaceholderReplacer::replaceManually(InternalProject.dump(), "${PROJECT}", Name));
	MainWindow::SetIdOffset(MainWindow::GetIdOffset() + root["Totals"] + 10);

	// 加载 Nodes
	for (const auto& nodeJson : root["Nodes"]) {
		switch (static_cast<NodeType>(nodeJson["Type"])) {
		case NodeType::Section: {
			auto node = std::make_unique<SectionNode>("", -1);
			node->LoadFromJson(nodeJson);
			Nodes.push_back(std::move(node));
			break;
		}
		case NodeType::List: {
			auto node = std::make_unique<ListNode>("", -1);
			node->LoadFromJson(nodeJson);
			Nodes.push_back(std::move(node));
			break;
		}
		case NodeType::Module: {
			auto node = std::make_unique<ModuleNode>("", -1);
			node->LoadFromJson(nodeJson);
			Nodes.push_back(std::move(node));
			break;
		}
		case NodeType::Group: {
			auto node = std::make_unique<GroupNode>("", -1);
			node->LoadFromJson(nodeJson);
			Nodes.push_back(std::move(node));
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
			Nodes.push_back(std::move(node));
			break;
		}
		default:
			throw "Unsupported node!";
		}
	}

	if (root.contains("IO")) {
		for (const auto& nodeJson : root["IO"]) {
			auto node = std::make_unique<IONode>(static_cast<PinKind>(nodeJson["Mode"]), "", -1, this);
			node->LoadFromJson(nodeJson);
			Nodes.push_back(std::move(node));
		}
	}

	// 加载 Links
	for (const auto& linkJson : root["Links"]) {
		auto link = std::make_unique<Link>(-1, 0, 0);
		link->LoadFromJson(linkJson);
		Links.push_back(std::move(link));
	}
}

void ModuleNode::CloseProject() {
	Nodes.clear();
	Links.clear();
}

void ModuleNode::UpdatePins() {
	std::vector<std::string> inputNames;
	std::vector<std::string> outputNames;

	for (const auto& ioNode : InternalProject["IO"]) {
		if (ioNode["Kind"] == PinKind::Output)
			inputNames.push_back(ioNode["Name"]);  // 工程的input node的pinkind是output, 对于模块Node来说, 就需要一个input的pin
		else
			outputNames.push_back(ioNode["Name"]);
	}

	UpdatePinSet(Inputs, inputNames, true);
	UpdatePinSet(Outputs, outputNames, false);
}

void ModuleNode::UpdatePinSet(std::vector<Pin>& pinSet, const std::vector<std::string>& newNames, bool isInput) {
	std::unordered_map<std::string, Pin*> existingPins;
	for (auto& pin : pinSet)
		existingPins[pin.Name] = &pin;

	std::vector<Pin> newPinSet;
	newPinSet.reserve(newNames.size());

	for (const auto& name : newNames) {
		if (existingPins.find(name) != existingPins.end()) {
			// 保留现有引脚（保持连接）
			newPinSet.push_back(std::move(*existingPins[name]));
			existingPins.erase(name);
		}
		else
			// 创建新引脚
			newPinSet.emplace_back(this, name.c_str(), isInput ? PinKind::Input : PinKind::Output);
	}

	// 移除未使用的旧引脚（自动断开连接）
	for (auto& [name, pin] : existingPins)
		pinSet.erase(std::remove_if(pinSet.begin(), pinSet.end(), [name](const Pin& p) { return p.Name == name; }), pinSet.end());

	pinSet = std::move(newPinSet);
}

std::string ModuleNode::GetValue(Pin* from) const {
	if(!from)
		return Node::GetValue();

	bool isOutput = false;
	const Pin* inputPin = nullptr;

	for (const auto& Pin : Inputs) {
		if (Pin.Name == from->Name) {
			isOutput = true;
			break;
		}
	}

	if (!isOutput) {
		for (const auto& Pin : Outputs) {
			if (Pin.Name == from->Name) {
				inputPin = &Pin;
				break;
			}
		}
	}

	if (isOutput) {
		for (const auto& Node : Nodes)
			if (Node.get()->GetNodeType() == NodeType::IO && Node.get()->Name == from->Name)
				return Node.get()->GetValue();

	}
	else if (inputPin) {
		return inputPin->GetValue();
	}

	return Node::GetValue();
}