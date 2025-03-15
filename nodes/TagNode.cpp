#include "TagNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/Pin.h"
#include "Nodes/SectionNode.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <memory>

namespace ed = ax::NodeEditor;

std::unordered_map<std::string, int> TagNode::GlobalNames;
std::unordered_map<std::string, TagNode*> TagNode::Inputs;
bool TagNode::HasInputChanged = false;
std::unordered_set<std::string> TagNode::HighlightedNodes;
TagNode::TagNode(bool input, const char* name, int id) :
	Node(name, id), IsInput(input){

	if (IsInput) {
		GlobalNames[Name]++;
		HasInputChanged = true;
	}

	InputPin = std::make_unique<Pin>(this, input ? "input" : "output", PinKind::Input);
}

TagNode::~TagNode() {
	if (!IsInput)
		return;

	GlobalNames[Name]--;
	HasInputChanged = true;
	if (GlobalNames[Name] == 0)
		GlobalNames.erase(Name);
}

void TagNode::UpdateSelectedName() {
	HighlightedNodes.clear();
	for (auto& node : Node::GetSelectedNodes())
		if (auto tagNode = dynamic_cast<TagNode*>(node))
			HighlightedNodes.insert(tagNode->Name);
}

void TagNode::UpdateInputs() {
	if (!HasInputChanged)
		return;

	HasInputChanged = false;
	Inputs.clear();
	for (auto& node : Node::Array)
		if (auto tag = dynamic_cast<TagNode*>(node.get()))
			if (tag->IsInput && GlobalNames[tag->Name] == 1)
				Inputs[tag->Name] = tag;

}

Pin* TagNode::GetFirstCompatiblePin(Pin* pin) {
	return InputPin.get();
}

void TagNode::SetName(const std::string& str) {
	if (Name == str)
		return;

	if (!IsInput)
		return Node::SetName(str);

	// 旧名字-1
	GlobalNames[Name]--;
	if (GlobalNames[Name] == 0)
		GlobalNames.erase(Name);

	Node::SetName(str);

	// 新名字+1
	GlobalNames[Name]++;
	HasInputChanged = true;
}

void TagNode::Update() {
	// 开始节点
	auto builder = BuilderNode::GetBuilder();

	if (!IsConstant) {
		// 冲突检测
		bool hasInputConflict = CheckInputConflicts();
		if (hasInputConflict)
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 0, 0, 1));
		else if (HighlightedNodes.contains(Name))
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 1, 0, 1));

		builder->Begin(this->ID);
		if (hasInputConflict) {
			ed::PopStyleColor();
			// 绘制标题（带冲突提示）
			ImGui::Text("(Conflict!)");
		}
		else if (HighlightedNodes.contains(Name))
			ed::PopStyleColor();

		{
			auto alpha = InputPin->GetAlpha();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ed::BeginPin(InputPin->ID, ed::PinKind::Input);
			InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255), IsInput);
			ed::EndPin();
			ImGui::PopStyleVar();
			ImGui::SameLine();
		}

		// 名称输入
		std::string name = Name;
		Utils::SetNextInputWidth(name, 120.f);
		if (ImGui::InputText("##Name", &name))
			SetName(name);
		builder->End();
	}
	else {
		if (HighlightedNodes.contains(Name))
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 1, 0, 1));

		builder->Begin(this->ID);

		if (HighlightedNodes.contains(Name))
			ed::PopStyleColor();

		Utils::SetNextInputWidth(Name, 120.f);
		ImGui::InputText("##Name", &Name);

		{
			ImGui::SameLine();
			auto alpha = InputPin->GetAlpha();
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ed::BeginPin(InputPin->ID, ed::PinKind::Output);
			InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
			ed::EndPin();
			ImGui::PopStyleVar();
		}

		builder->End();
	}

}

void TagNode::Menu() {
	Node::Menu();

	if (ImGui::MenuItem(IsConstant ? "Set Variable" : "Set Constant"))
		IsConstant = !IsConstant;
}

std::string TagNode::GetValue() const {
	if (IsConstant) {
		return Name;
	}
	else {
		std::unordered_set<Node*> tagVisited;
		auto pThis = const_cast<TagNode*>(this);
		return pThis->ResolveTagPointer(pThis, tagVisited);
	}
}

void TagNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	json inputJson;
	InputPin->SaveToJson(inputJson);
	j["Input"] = inputJson;
	j["TagType"] = IsConstant ? "Const" : IsInput ? "Input" : "Output";
}

void TagNode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);

	InputPin = std::make_unique<Pin>(this, "input", PinKind::Input);
	InputPin->Node = this;
	InputPin->LoadFromJson(j["Input"]);
}

// 递归解析指针类型TagNode的值
std::string TagNode::ResolveTagPointer(TagNode* tagNode, std::unordered_set<Node*>& visited) {
	if (!tagNode || visited.count(tagNode))
		return "";
	visited.insert(tagNode);

	if (tagNode->IsConstant)
		return tagNode->Name;

	if (!tagNode->InputPin)
		return "";

	auto inputNode = tagNode->GetInputTagNode();
	if (!inputNode)
		return "";

	if (auto section = dynamic_cast<SectionNode*>(inputNode)) {
		return section->Name;
	}
	else if (auto tag = dynamic_cast<TagNode*>(inputNode)) {
		if (tag->IsInput)
			return tag->Name;
		return ResolveTagPointer(tag, visited);
	}
	return "";
}

bool TagNode::CheckInputConflicts() const {
	if (IsInput && GlobalNames.contains(Name))
		return GlobalNames[Name] > 1;

	return false;
}

TagNode* TagNode::GetInputTagNode() const {
	return Inputs.contains(Name) ? Inputs[Name] : nullptr;
}
