﻿#include "TagNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/Pin.h"
#include "Nodes/SectionNode.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <memory>
/*
Constant: 可重复
───╮	╭────────────────────────╮
kv-│----│---→    Value           │
───╯	╰────────────────────────╯
InputTag: 可重复
───╮	╭────────────────────────╮
kv-│----│---→    Name            │
───╯	╰────────────────────────╯
OutputTag: 不可重复
		╭────────────────────────╮	  ╭───
		│		    Name    ----→│----│-kv
		╰────────────────────────╯	  ╰───
*/
namespace ed = ax::NodeEditor;

std::unordered_map<std::string, int> TagNode::GlobalNames;
std::unordered_map<std::string, TagNode*> TagNode::Outputs;
bool TagNode::HasOutputChanged = false;
std::unordered_set<std::string> TagNode::HighlightedNodes;
TagNode::TagNode(const char* name, int id) :
	Node(name, id) {
	InputPin = std::make_unique<Pin>(this, "constant", PinKind::Input);
	IsConstant = true;
}

TagNode::TagNode(bool input, const char* name, int id) :
	Node(name, id), IsInput(input) {

	if (!IsInput) {
		GlobalNames[Name]++;
		HasOutputChanged = true;
	}

	InputPin = input ?
		std::make_unique<Pin>(this, "input", PinKind::Input) :
		std::make_unique<Pin>(this, "output", PinKind::Output);
}

TagNode::~TagNode() {
	if (IsInput)
		return;

	GlobalNames[Name]--;
	HasOutputChanged = true;
	if (GlobalNames[Name] == 0)
		GlobalNames.erase(Name);
}

void TagNode::UpdateSelectedName() {
	HighlightedNodes.clear();
	for (auto& node : Node::GetSelectedNodes())
		if (auto tagNode = dynamic_cast<TagNode*>(node))
			HighlightedNodes.insert(tagNode->Name);
}

void TagNode::UpdateOutputs() {
	if (!HasOutputChanged)
		return;

	HasOutputChanged = false;
	Outputs.clear();
	for (auto& node : Node::Array)
		if (auto tag = dynamic_cast<TagNode*>(node.get()))
			if (!tag->IsInput)
				Outputs[tag->Name] = tag;

}

Pin* TagNode::GetFirstCompatiblePin(Pin* pin) {
	return InputPin.get();
}

void TagNode::SetName(const std::string& str) {
	if (Name == str)
		return;

	if (IsInput)
		return Node::SetName(str);

	// 旧名字-1
	GlobalNames[Name]--;
	if (GlobalNames[Name] == 0)
		GlobalNames.erase(Name);

	Node::SetName(str);

	// 新名字+1
	GlobalNames[Name]++;
	HasOutputChanged = true;
}

void TagNode::Update() {
	// 开始节点
	auto builder = BuilderNode::GetBuilder();

	if (IsConstant) {
		if (HighlightedNodes.contains(Name))
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 1, 0, 1));

		builder->Begin(this->ID);

		{
			auto alpha = InputPin->GetAlpha();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ed::BeginPin(InputPin->ID, ed::PinKind::Input);
			InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
			ed::EndPin();
			ImGui::PopStyleVar();
			ImGui::SameLine();
		}
		if (HighlightedNodes.contains(Name))
			ed::PopStyleColor();

		Utils::SetNextInputWidth(Name, 120.f);
		ImGui::InputText("##Name", &Name);

	}
	else {
		// 冲突检测
		bool hasInputConflict = CheckOutputConflicts();
		if (hasInputConflict)
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 0, 0, 1));
		else if (HighlightedNodes.contains(Name))
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 1, 0, 1));

		builder->Begin(this->ID);

		if (IsInput) {
			auto alpha = InputPin->GetAlpha();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ed::BeginPin(InputPin->ID, ed::PinKind::Input);
			InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
			ed::EndPin();
			ImGui::PopStyleVar();
			ImGui::SameLine();
		}

		if (hasInputConflict) {
			ed::PopStyleColor();
			// 绘制标题（带冲突提示）
			ImGui::Text("(%s!)", LOCALE["Conflict"]);
		}
		else if (HighlightedNodes.contains(Name))
			ed::PopStyleColor();

		// 名称输入
		std::string name = Name;
		Utils::SetNextInputWidth(name, 120.f);
		if (ImGui::InputText("##Name", &name))
			SetName(name);

		if (!IsInput) {
			ImGui::SameLine();
			auto alpha = InputPin->GetAlpha();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ed::BeginPin(InputPin->ID, ed::PinKind::Output);
			InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
			ed::EndPin();
			ImGui::PopStyleVar();
		}
	}

	builder->End();
}

void TagNode::Menu() {
	Node::Menu();
}

void TagNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	json inputJson;
	InputPin->SaveToJson(inputJson);
	j["Input"] = inputJson;
	j["TagType"] = IsConstant ? "Const" : IsInput ? "Input" : "Output";
}

void TagNode::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);

	InputPin = std::make_unique<Pin>(this, "input", PinKind::Input);
	InputPin->Node = this;
	InputPin->LoadFromJson(j["Input"], newId);
}

std::string TagNode::GetValue(Pin* from) const {
	if (IsConstant)
		return Name;
	else {
		std::unordered_set<Node*> tagVisited;
		auto pThis = const_cast<TagNode*>(this);
		return pThis->ResolveTagPointer(pThis, tagVisited);
	}
}

// 递归解析指针类型TagNode的值
std::string TagNode::ResolveTagPointer(TagNode* tagNode, std::unordered_set<Node*>& visited) {
	if (!tagNode || visited.count(tagNode))
		return "";
	visited.insert(tagNode);

	auto outputTag = tagNode->GetOutputTagNode();
	if (!outputTag)
		return "";

	if (auto linked = outputTag->InputPin->GetLinkedNode()) {
		if (auto tag = dynamic_cast<TagNode*>(linked))
			return ResolveTagPointer(tag, visited);
		else
			return linked->GetValue(outputTag->InputPin.get());
	}

	return "";
}

bool TagNode::CheckOutputConflicts() const {
	if (!IsInput && GlobalNames.contains(Name))
		return GlobalNames[Name] > 1;

	return false;
}

TagNode* TagNode::GetOutputTagNode() const {
	return Outputs.contains(Name) ? Outputs[Name] : nullptr;
}
