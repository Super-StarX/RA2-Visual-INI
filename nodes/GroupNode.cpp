#define IMGUI_DEFINE_MATH_OPERATORS
#include "GroupNode.h"
#include "Utils.h"

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y) {
	ImRect result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

void GroupNode::Update() {
	const float commentAlpha = 0.75f;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	ed::BeginNode(this->ID);
	ImGui::PushID(this->ID.AsPointer());
	ImGui::BeginVertical("content");
	ImGui::BeginHorizontal("horizontal");
	ImGui::Spring(1);
	ImGui::TextUnformatted(this->Name.c_str());
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

		auto hintBounds = ImGui_GetItemRect();
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
