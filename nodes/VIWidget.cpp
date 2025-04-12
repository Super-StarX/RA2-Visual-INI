#include "VIWidget.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>

bool VIInputText::InputText() {
	return ImGui::InputText("##SectionName", Buffer, sizeof(Buffer),
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_AutoSelectAll);
}

bool VIInputText::InputTextMultiline() {
	return ImGui::InputTextMultiline("##SectionName", Buffer, sizeof(Buffer), {-1,-1});
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
