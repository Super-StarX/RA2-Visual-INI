#include "Utils.h"
#include <algorithm>
#include <regex>
#include <windows.h>
#include <misc/cpp/imgui_stdlib.h>

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
	tokens.resize(std::count(s.begin(), s.end(), ',') + 1);
	std::string token;
	std::istringstream tokenStream(s);
	int i = 0;
	while (std::getline(tokenStream, token, delimiter)) {
		tokens[i++] = token;
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

std::string Utils::Trim(const std::string& str) {
	static const std::string whitespace = " \t\n\r";
	auto start = str.find_first_not_of(whitespace);
	if (start == std::string::npos) return "";
	auto end = str.find_last_not_of(whitespace);
	return str.substr(start, end - start + 1);
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

bool Utils::IsCommentSection(const std::string& str) {
	static std::regex pattern(R"(;\s*\[)");
	return std::regex_search(str, pattern);
}

bool Utils::OpenFileDialog(const char* fliter, char* path, int maxPath, bool isSaving) {
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = fliter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (isSaving) {
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	else {
		if (GetOpenFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	return false;
}

void Utils::InputText(const char* label, std::string* str, bool disable) {
	if (disable) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));   // 淡化文字颜色
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f); // 降低透明度
		ImGui::InputText(label, str, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar();
	}
	else
		ImGui::InputText(label, str);
}

void Utils::InputTextWithLeftLabel(const char* label, const char* text, float textWidth, std::string* string, bool disable) {
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(text);
	ImGui::SameLine(textWidth + ImGui::GetStyle().ItemSpacing.x);
	ImGui::SetNextItemWidth(-FLT_MIN);
	Utils::InputText(label, string, disable);
}