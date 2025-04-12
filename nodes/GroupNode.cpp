#define IMGUI_DEFINE_MATH_OPERATORS
#include "GroupNode.h"
#include "Utils.h"
#include "NodeStyle.h"
#include <imgui_node_editor_internal.h>
#include <misc/cpp/imgui_stdlib.h>

GroupNode::GroupNode(const char* name, int id) :
	Node(name, id) {
	Style = "white";
}

void GroupNode::Update() {
	const float commentAlpha = 0.75f;

	auto* typeInfo = NodeStyleManager::Get().FindType(Style);
	if (!typeInfo) return;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
	auto color = typeInfo->Color;
	color.Value.w = 64;
	ed::PushStyleColor(ed::StyleColor_NodeBg, color);
	ed::PushStyleColor(ed::StyleColor_NodeBorder, color);
	ed::BeginNode(this->ID);
	ImGui::PushID(this->ID.AsPointer());
	ImGui::BeginVertical("content");
	ImGui::BeginHorizontal("horizontal");
	ImGui::Spring(1);

	Name.Render();

	ImGui::Spring(1);
	ImGui::EndHorizontal();
	ed::Group(this->Size);
	ImGui::EndVertical();
	ImGui::PopID();
	ed::EndNode();
	ed::PopStyleColor(2);
	ImGui::PopStyleVar();

	if (ed::BeginGroupHint(this->ID)) {
		//auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
		auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

		//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

		auto min = ed::GetGroupMin();
		//auto max = ed::GetGroupMax();

		ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
		ImGui::BeginGroup();
		ImGui::TextUnformatted(this->Name.c_str());
		ImGui::EndGroup();

		auto drawList = ed::GetHintBackgroundDrawList();

		auto hintBounds = ed::Detail::ImGui_GetItemRect();
		auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

		drawList->AddRectFilled(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

		drawList->AddRect(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

		//ImGui::PopStyleVar();
	}
	ed::EndGroupHint();
}

void GroupNode::Menu() {
	Node::Menu();
}

void GroupNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	j["Size"] = { Size.x, Size.y };
}

void GroupNode::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);

	Size = {
		j["Size"][0].get<float>(),
		j["Size"][1].get<float>()
	};
}
