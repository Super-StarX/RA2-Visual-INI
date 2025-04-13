#include "VIWidget.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>

bool VIInputText::InputText() {
	return ImGui::InputText("##SectionName", Buffer, sizeof(Buffer),
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_AutoSelectAll);
}

bool VIInputText::InputTextMultiline() {
	const float MaxWidth = 500.0f;
	const float MaxHeight = 400.0f;
	const float Padding = 20.0f;

	// 1. 计算文本行数和最长一行长度
	std::vector<std::string> lines = Utils::SplitString(Buffer, '\n');
	int lineCount = static_cast<int>(lines.size());
	float maxTextWidth = 0.0f;

	ImGuiContext& g = *ImGui::GetCurrentContext();
	ImFont* font = g.Font;

	for (const auto& line : lines) {
		float textWidth = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str()).x;
		maxTextWidth = std::max(maxTextWidth, textWidth);
	}

	// 2. 计算文本框尺寸
	float width = std::min(maxTextWidth + Padding, MaxWidth);
	float height = std::min((lineCount + 1) * font->FontSize + Padding, MaxHeight);

	return ImGui::InputTextMultiline("##SectionName", Buffer, sizeof(Buffer),
		ImVec2(width, height),
		ImGuiInputTextFlags_AutoSelectAll);
}

void VIInputText::BeginEditing() {
	Editing = true;
	NeedFocus = true;
	strcpy_s(Buffer, this->c_str());
	Temp = *this;
	//ImGui::SetKeyboardFocusHere();
}

bool VIInputText::Render() {
	bool result = false;
	ImGui::PushID(Owner);
	Utils::SetNextInputWidth(*this, 130.f);
	if (Editing || this->empty()) {
		// 编辑模式：显示InputText
		if (Multiline)
			InputTextMultiline();  // 不检测返回值，回车不结束
		else {
			if (InputText()) { // 检测回车（仅限单行）
				*this = Buffer;
				Editing = false;
				result = true;
			}
		}

		// 失去焦点时保存
		if (!NeedFocus && !ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
			*this = Buffer;
			Editing = false;
		}

		if (NeedFocus) {
			ImGui::SetKeyboardFocusHere(-1);
			NeedFocus = false;
		}
	}
	else {
		// 普通模式：显示静态文本
		ImGui::Text("%s", this->c_str());
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			// 双击：进入编辑模式
			BeginEditing();
		}
	}
	// 处理ESC取消编辑
	if (Editing && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		*this = Temp; // 恢复原始值
		Editing = false;
	}
	ImGui::PopID();
	return result;
}
