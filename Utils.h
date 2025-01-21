#pragma once
#include <imgui.h>
#include <imgui_internal.h>

static inline ImRect ImGui_GetItemRect() {
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}
