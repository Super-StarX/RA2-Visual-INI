#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

static inline ImRect ImGui_GetItemRect() {
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}
static inline void showLabel(const char* label, ImColor color) {
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
	auto size = ImGui::CalcTextSize(label);

	auto padding = ImGui::GetStyle().FramePadding;
	auto spacing = ImGui::GetStyle().ItemSpacing;

	ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

	auto rectMin = ImGui::GetCursorScreenPos() - padding;
	auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

	auto drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
	ImGui::TextUnformatted(label);
};

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f) {
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}
