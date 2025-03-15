#include "ModuleNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include <Utils.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <imgui_node_editor_internal.h>
#include <Windows.h>
#include <filesystem>

ModuleNode::ModuleNode(const char* name, int id) :
	INENode(name, id) {
}

void ModuleNode::Update() {
	auto builder = BuilderNode::GetBuilder();

	builder->Begin(this->ID);
	builder->Header(this->Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	ImGui::Spring(0);
	builder->EndHeader();

	for (auto& input : this->Inputs)
		BuilderNode::UpdateInput(input);

	for (auto& output : this->Outputs)
		if (output.TypeIdentifier != "delegate")
			BuilderNode::UpdateOutput(output);

	builder->End();
}

void ModuleNode::Menu() {
	INENode::Menu();
	ImGui::Separator();

	if (ImGui::MenuItem("Load Project"))
		LoadProject();
}

void ModuleNode::Tooltip() {
	INENode::Tooltip();
	ImGui::Separator();
	std::string input = "Inputs: ";
	for (const auto& pin : Inputs)
		input += pin.Name + ", ";
	input.pop_back();
	input.pop_back();
	ImGui::Text(input.c_str());
	std::string output = "Outputs: ";
	for (const auto& pin : Outputs)
		output += pin.Name + ", ";
	output.pop_back();
	output.pop_back();
	ImGui::Text(output.c_str());
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
	UpdatePins();
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
	file.close();
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