#define IMGUI_DEFINE_MATH_OPERATORS
#include "GroupNode.h"
#include "Utils.h"
#include <imgui_node_editor_internal.h>
#include <misc/cpp/imgui_stdlib.h>

static std::string NameBuffer;
void GroupNode::Update() {
	const float commentAlpha = 0.75f;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	ed::BeginNode(this->ID);
	ImGui::PushID(this->ID.AsPointer());
	ImGui::BeginVertical("content");
	ImGui::BeginHorizontal("horizontal");
	ImGui::Spring(1);
	if (IsEditingName) {
		// 开始编辑模式
		ImGui::SetKeyboardFocusHere();
		float inputWidth = Size.x - 16.0f;
		ImGui::SetNextItemWidth(inputWidth);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, IM_COL32(255, 255, 255, 96));
		if (ImGui::InputText("##NameEdit", &NameBuffer,
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_AutoSelectAll)) {
			// 按下回车保存
			Name = NameBuffer;
			IsEditingName = false;
		}
		// ESC取消处理
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			NameBuffer = Name;
			IsEditingName = false;
		}
		// 外部点击保存处理
		else if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			Name = NameBuffer;
			IsEditingName = false;
		}
	}
	else {
		ImGui::TextUnformatted(this->Name.c_str());
	}
	ImGui::Spring(1);
	ImGui::EndHorizontal();
	ed::Group(this->Size);
	ImGui::EndVertical();
	ImGui::PopID();
	ed::EndNode();
	ed::PopStyleColor(2);
	ImGui::PopStyleVar();

	if (ed::BeginGroupHint(this->ID)) {
		//auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
		auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

		//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

		auto min = ed::GetGroupMin();
		//auto max = ed::GetGroupMax();

		ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
		ImGui::BeginGroup();
		ImGui::TextUnformatted(this->Name.c_str());
		ImGui::EndGroup();

		auto drawList = ed::GetHintBackgroundDrawList();

		auto hintBounds = ed::Detail::ImGui_GetItemRect();
		auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

		drawList->AddRectFilled(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

		drawList->AddRect(
			hintFrameBounds.GetTL(),
			hintFrameBounds.GetBR(),
			IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

		//ImGui::PopStyleVar();
	}
	ed::EndGroupHint();

	if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		IsEditingName = true;
		NameBuffer = Name;
	}
}

void GroupNode::Menu() {
	Node::Menu();
	// 添加编辑名称的菜单项
	if (ImGui::MenuItem("Edit Name")) {
		IsEditingName = true;
		NameBuffer = Name;
	}
}

void GroupNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	j["Size"] = { Size.x, Size.y };
}

void GroupNode::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);

	Size = {
		j["Size"][0].get<float>(),
		j["Size"][1].get<float>()
	};
}
