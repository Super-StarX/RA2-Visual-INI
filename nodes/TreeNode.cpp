﻿#define IMGUI_DEFINE_MATH_OPERATORS
#include "TreeNode.h"
#include "MainWindow.h"
#include <imgui_node_editor_internal.h>

void TreeNode::Update() {
	const float rounding = 5.0f;
	const float padding = 12.0f;

	const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
	ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
	ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

	ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
	ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
	ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
	ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
	ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
	ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
	ed::BeginNode(this->ID);

	ImGui::BeginVertical(this->ID.AsPointer());
	ImGui::BeginHorizontal("inputs");
	ImGui::Spring(0, padding * 2);

	ImRect inputsRect;
	float inputAlpha = 200;
	if (!this->Inputs.empty()) {
		auto& pin = this->Inputs[0];
		ImGui::Dummy(ImVec2(0, padding));
		ImGui::Spring(1, 0);
		inputsRect = ed::Detail::ImGui_GetItemRect();

		ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
		ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
#if IMGUI_VERSION_NUM > 18101
		ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
#else
		ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
#endif
		ed::BeginPin(pin.ID, ed::PinKind::Input);
		ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
		ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
		ed::EndPin();
		ed::PopStyleVar(3);

		inputAlpha = pin.GetAlpha();
	}
	else
		ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("content_frame");
	ImGui::Spring(1, padding);

	ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
	ImGui::Dummy(ImVec2(160, 0));
	ImGui::Spring(1);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::EndVertical();
	auto contentRect = ed::Detail::ImGui_GetItemRect();

	ImGui::Spring(1, padding);
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("outputs");
	ImGui::Spring(0, padding * 2);

	ImRect outputsRect;
	float outputAlpha = 200;
	if (!this->Outputs.empty()) {
		auto& pin = this->Outputs[0];
		ImGui::Dummy(ImVec2(0, padding));
		ImGui::Spring(1, 0);
		outputsRect = ed::Detail::ImGui_GetItemRect();

#if IMGUI_VERSION_NUM > 18101
		ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);
#else
		ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
#endif
		ed::BeginPin(pin.ID, ed::PinKind::Output);
		ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
		ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
		ed::EndPin();
		ed::PopStyleVar();

		outputAlpha = pin.GetAlpha();
	}
	else
		ImGui::Dummy(ImVec2(0, padding));

	ImGui::Spring(0, padding * 2);
	ImGui::EndHorizontal();

	ImGui::EndVertical();

	ed::EndNode();
	ed::PopStyleVar(7);
	ed::PopStyleColor(4);

	auto drawList = ed::GetNodeBackgroundDrawList(this->ID);

	//const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
	//const auto unitSize    = 1.0f / fringeScale;

	//const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
	//{
	//    if ((col >> 24) == 0)
	//        return;
	//    drawList->PathRect(a, b, rounding, rounding_corners);
	//    drawList->PathStroke(col, true, thickness);
	//};

#if IMGUI_VERSION_NUM > 18101
	const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
	const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;
#else
	const auto    topRoundCornersFlags = 1 | 2;
	const auto bottomRoundCornersFlags = 4 | 8;
#endif

	drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
		IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
	//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
		IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
	//ImGui::PopStyleVar();
	drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
		IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
	//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
		IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
	//ImGui::PopStyleVar();
	drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	drawList->AddRect(
		contentRect.GetTL(),
		contentRect.GetBR(),
		IM_COL32(48, 128, 255, 100), 0.0f);
	//ImGui::PopStyleVar();
}
