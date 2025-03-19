#include "VIWidget.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>

bool VIInputText::Render() {
	bool result = false;
	ImGui::PushID(Owner);
	Utils::SetNextInputWidth(*this, 130.f);
	if (Editing) {
		// 编辑模式：显示InputText
		if (ImGui::InputText("##SectionName", Buffer, sizeof(Buffer),
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_AutoSelectAll)) {
			// 按下回车：保存修改
			*this = Buffer;
			Editing = false;
			// 更新关联Pin的值（保持原有逻辑）
			result = true;
		}
		// 失去焦点时保存
		if (!ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
			*this = Buffer;
			Editing = false;
		}
	}
	else {
		// 普通模式：显示静态文本
		ImGui::Text("%s", this->c_str());
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			// 双击：进入编辑模式
			Editing = true;
			strcpy_s(Buffer, this->c_str());
			Temp = *this;
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
