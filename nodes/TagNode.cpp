#include "TagNode.h"
#include "Pin.h"
#include "MainWindow.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <memory.h>

namespace ed = ax::NodeEditor;

std::unordered_set<std::string> TagNode::GlobalNames;
TagNode::TagNode(MainWindow* owner, int id, const char* name, bool input, ImColor color) :
	BaseNode(owner, id, name, color), IsInput(input){
	GlobalNames.insert(name);
	
	InputPin = std::make_unique<Pin>(MainWindow::GetNextId(), input ? "input" : "output");
	InputPin->Node = this;
	InputPin->Kind = PinKind::Input;
}

void TagNode::Update() {
	// 冲突检测
	bool hasInputConflict = CheckInputConflicts();

	// 开始节点
	auto builder = GetBuilder();

	if (hasInputConflict)
		ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1, 0, 0, 1));

	builder->Begin(this->ID);

	if (hasInputConflict) {
		ed::PopStyleColor();
		// 绘制标题（带冲突提示）
		ImGui::Text("(Conflict!)");
	}

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
	ImGui::PushItemWidth(120);
	if (ImGui::InputText("##Name", &Name)) {
		// 名称去重处理
		if (GlobalNames.count(Name)) {
			hasInputConflict = true;
		}
		else {
			GlobalNames.erase(this->Name);
			GlobalNames.insert(Name);
			hasInputConflict = false;
		}
	}
	ImGui::PopItemWidth();

	builder->End();

	// 绘制冲突描边
	if (hasInputConflict) {
		constexpr float conflictOutlineWidth = 3.0f;
		const ImColor conflictColor = (255, 0, 0, 255);
		auto drawList = ed::GetNodeBackgroundDrawList(ID);
		const ImVec2 padding(conflictOutlineWidth, conflictOutlineWidth);
		const auto nodeRect = ed::Detail::ImGui_GetItemRect();
		drawList->AddRect(
			nodeRect.Min - padding, nodeRect.Max + padding,
			conflictColor, 0.0f, 0, conflictOutlineWidth
		);
	}
}

bool TagNode::CheckInputConflicts() {
	int count = 0;
	for (auto& node : Node::Array)
		if (IsInput && node->Type == NodeType::Tag)
			if (auto tagNode = reinterpret_cast<TagNode*>(node.get()))
				if (tagNode->IsInput && tagNode->Name == Name)
					if (++count > 1)
						return true;
	return false;
}
