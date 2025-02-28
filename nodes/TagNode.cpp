#include "TagNode.h"
#include "Pins/Pin.h"
#include "MainWindow.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <memory>

namespace ed = ax::NodeEditor;

std::unordered_set<std::string> TagNode::HighlightedNodes;
TagNode::TagNode(MainWindow* owner, int id, const char* name, bool input, ImColor color) :
	BaseNode(owner, id, name, color), IsInput(input){
	InputPin = std::make_unique<Pin>(MainWindow::GetNextId(), input ? "input" : "output");
	InputPin->Node = this;
	InputPin->Kind = PinKind::Input;
}

void TagNode::UpdateSelectedName() {
	HighlightedNodes.clear();
	for (auto& node : Node::GetSelectedNodes())
		if (auto tagNode = dynamic_cast<TagNode*>(node))
			HighlightedNodes.insert(tagNode->Name);
}

void TagNode::Update() {
	// 开始节点
	auto builder = GetBuilder();

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
		Utils::SetNextInputWidth(Name, 120.f);
		ImGui::InputText("##Name", &Name);
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

void TagNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	json inputJson;
	InputPin->SaveToJson(inputJson);
	j["Input"] = inputJson;
	j["Type"] = IsInput ? "Input" : IsConstant ? "Const" : "Output";
}

void TagNode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);

	InputPin = std::make_unique<Pin>(-1, "input");
	InputPin->Node = this;
	InputPin->LoadFromJson(j["Input"]);
	auto type = j["Type"].get<std::string>();
	if (type == "Input") {
		IsInput = true;
		IsConstant = false;
	}
	else if (type == "Const") {
		IsInput = false;
		IsConstant = true;
	}
	else if (type == "Output") {
		IsInput = false;
		IsConstant = false;
	}
}

bool TagNode::CheckInputConflicts() {
	int count = 0;
	for (auto& node : Node::Array) {
		if (!IsInput)
			continue;
		auto tagNode = dynamic_cast<TagNode*>(node.get());
		if (tagNode && tagNode->IsInput && tagNode->Name == Name)
			if (++count > 1)
				return true;
	}
	return false;
}

TagNode* TagNode::GetInputTagNode() {
	for (auto& node : Node::Array) {
		auto tag = dynamic_cast<TagNode*>(node.get());
		if (tag->IsInput && Name == tag->Name)
			return tag;
	}
	return nullptr;
}
