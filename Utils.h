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