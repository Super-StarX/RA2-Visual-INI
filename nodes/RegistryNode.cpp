#include "RegistryNode.h"
#include "BuilderNode.h"
#include "imgui.h"
#include "Utils.h"
#include <imgui_node_editor_internal.h>
#include <misc/cpp/imgui_stdlib.h>

RegistryNode::RegistryNode(const std::string& name, int id)
	: Node(name.c_str(), id) {}

NodeType RegistryNode::GetNodeType() const {
	return NodeType::Registry;
}

void RegistryNode::Update() {
	const float commentAlpha = 0.75f;

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
	ed::BeginNode(this->ID);
	ImGui::PushID(this->ID.AsPointer());

	ImGui::BeginVertical("content");
	ImGui::BeginHorizontal("horizontal");
	ImGui::Spring(1);

	// 渲染节点名
	Name.Render();

	ImGui::Spring(1);
	ImGui::EndHorizontal();

	// 根据节点大小自动计算行列数
	ImVec2 size = ed::GetNodeSize(ID);
	float width = size.x - 20.0f; // 预留边距
	float entryHeight = ImGui::GetTextLineHeightWithSpacing();
	int columns = static_cast<int>(width / 120.0f); // 每列约120px
	if (columns < 1) columns = 1;

	size_t rows = (Entries.size() + columns - 1) / columns;

	ImGui::Separator();
	if (ImGui::BeginTable("RegistryTable", columns, ImGuiTableFlags_SizingStretchProp)) {
		for (size_t row = 0; row < rows; ++row) {
			ImGui::TableNextRow();
			for (size_t col = 0; col < columns; ++col) {
				size_t idx = row * columns + col;
				ImGui::TableSetColumnIndex(col);
				if (idx < Entries.size()) {
					ImGui::TextWrapped("%s", Entries[idx].c_str());
				}
			}
		}
		ImGui::EndTable();
	}

	ed::Group(this->Size);
	ImGui::EndVertical();
	ImGui::PopID();
	ed::EndNode();
	ed::PopStyleColor(2);
	ImGui::PopStyleVar();

	// Group Hint (悬浮提示名称)
	if (ed::BeginGroupHint(this->ID)) {
		auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);
		auto min = ed::GetGroupMin();

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
	}
	ed::EndGroupHint();
}


void RegistryNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);
	j["Entries"] = Entries;
	j["Size"] = { Size.x, Size.y };
}

void RegistryNode::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);
	if (j.contains("Entries"))
		Entries = j["Entries"].get<std::vector<std::string>>();
	Size = {
		j["Size"][0].get<float>(),
		j["Size"][1].get<float>()
	};
}
