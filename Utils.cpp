#include "Utils.h"
#include <algorithm>

void Utils::DrawTextOnCursor(const char* label, ImColor color) {
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
}

bool Utils::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size) {
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

std::vector<std::string> Utils::SplitString(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) tokens.push_back(token);
	}
	return tokens;
}

// 辅助函数实现
std::string Utils::JoinStrings(const std::vector<std::string>& elements, const std::string& delim) {
	std::string result;
	for (size_t i = 0; i < elements.size(); ++i) {
		if (i != 0) result += delim;
		result += elements[i];
	}
	return result;
}

int Utils::GetComboIndex(const std::string& value, const std::vector<std::string>& options) {
	auto it = std::find(options.begin(), options.end(), value);
	return (it != options.end()) ? static_cast<int>(it - options.begin()) : 0;
}

const char* Utils::GetComboItems(const std::vector<std::string>& options) {
	static std::string buffer;
	buffer.clear();
	for (auto& item : options) {
		buffer += item;
		buffer += '\0';
	}
	buffer += '\0';
	return buffer.c_str();
}

float Utils::SetNextInputWidth(const std::string& value, float minSize, float maxSize) {
	float textWidth = std::clamp(ImGui::CalcTextSize(value.c_str()).x, minSize, maxSize);
	float inputWidth = textWidth + 20.0f;
	ImGui::SetNextItemWidth(inputWidth);
	return inputWidth;
}
