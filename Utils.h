#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <vector>
#include <sstream>

static inline ImRect ImGui_GetItemRect() {
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline float ImLength(const ImVec2& vec) {
	return std::hypot(vec.x, vec.y);
}

static inline ImVec2 ImNormalized(const ImVec2& vec) {
	float len = ImLength(vec);
	return len > 0.0f ? ImVec2(vec.x / len, vec.y / len) : ImVec2(0, 0);
}

class Utils {
public:
	static void DrawTextOnCursor(const char* label, ImColor color);
	static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
	static std::vector<std::string> SplitString(const std::string& s, char delimiter);
	static std::string JoinStrings(const std::vector<std::string>& elements, const std::string& delim);
	static int GetComboIndex(const std::string& value, const std::vector<std::string>& options);
	static const char* GetComboItems(const std::vector<std::string>& options);
};
