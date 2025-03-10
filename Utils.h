#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <vector>
#include <sstream>

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y) {
	ImRect result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

class Utils {
public:
	static void DrawTextOnCursor(const char* label, ImColor color);
	static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
	static std::vector<std::string> SplitString(const std::string& s, char delimiter);
	static std::string JoinStrings(const std::vector<std::string>& elements, const std::string& delim);
	static int GetComboIndex(const std::string& value, const std::vector<std::string>& options);
	static const char* GetComboItems(const std::vector<std::string>& options);
	static float SetNextInputWidth(const std::string& value, float minSize = 0.f, float maxSize = 400.f);
	static std::string StringTrim(const std::string& str);
	static bool IsCommentSection(const std::string& str);
};
