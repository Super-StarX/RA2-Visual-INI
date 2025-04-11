#include "NodeStyle.h"
#include "NodeStyle.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

NodeStyleManager::NodeStyleManager() {
	AddCustomType({ "default", "Default", ImColor(0, 64, 128), false });
	AddCustomType({ "red", "Red", ImColor(255, 0, 0), false });
	AddCustomType({ "green", "Green", ImColor(0, 255, 0), false });
	AddCustomType({ "blue", "Blue", ImColor(0, 0, 255), false });
	AddCustomType({ "dark", "Dark Blue", ImColor(30, 30, 80), true });
}
void NodeStyleManager::Menu() {
	static int selected = -1;
	const auto& types = NodeStyleManager::Get().GetAllTypes();

	ImGui::Text("Node Styles:");
	ImGui::BeginChild("##NodeStyleList", ImVec2(150, 300), true);
	for (int i = 0; i < types.size(); ++i) {
		const auto& type = types[i];
		ImGui::PushID(i);

		float fullWidth = ImGui::GetContentRegionAvail().x;
		float buttonWidth = 20.0f;
		float labelWidth = fullWidth - buttonWidth - 5.0f;

		if (ImGui::Selectable(type.DisplayName.c_str(), selected == i, 0, ImVec2(labelWidth, 0)))
			selected = i;

		if (type.IsUserDefined) {
			ImGui::SameLine(labelWidth + 15.0f);
			if (ImGui::SmallButton("×")) {
				NodeStyleManager::Get().RemoveCustomType(type.Identifier);

				if (selected == i)
					selected = -1;
				else if (selected > i)
					--selected;

				ImGui::PopID();
				break;
			}
		}

		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginGroup();

	if (selected >= 0 && selected < types.size()) {
		NodeStyleInfo& type = const_cast<NodeStyleInfo&>(types[selected]);

		ImGui::Text("Edit Node Style:");
		ImGui::Text("Identifier: %s", type.Identifier.c_str());

		ImGui::Text("Display Name:");
		ImGui::InputText("##DisplayName", &type.DisplayName);

		ImVec4 color = type.Color;
		if (ImGui::ColorEdit4("Node Color", (float*)&color))
			type.Color = ImColor(color);

		if (type.IsUserDefined) {
			if (ImGui::Button("Delete")) {
				NodeStyleManager::Get().RemoveCustomType(type.Identifier);
				selected = -1;
			}
		}
	}

	ImGui::EndGroup();
	ImGui::Separator();

	// Add New Node Style
	static char newId[64] = "";
	static char newName[64] = "";
	static ImVec4 newColor = ImVec4(0.3f, 0.3f, 0.6f, 1.0f);

	ImGui::Text("Add New Style:");
	ImGui::InputText("Identifier", newId, IM_ARRAYSIZE(newId));
	ImGui::InputText("Display Name", newName, IM_ARRAYSIZE(newName));
	ImGui::ColorEdit4("Node Color##new", (float*)&newColor);

	if (ImGui::Button("Add")) {
		if (strlen(newId) > 0 && strlen(newName) > 0 && !NodeStyleManager::Get().FindType(newId)) {
			NodeStyleInfo style;
			style.Identifier = newId;
			style.DisplayName = newName;
			style.Color = ImColor(newColor);
			style.IsUserDefined = true;

			NodeStyleManager::Get().AddCustomType(style);

			memset(newId, 0, sizeof(newId));
			memset(newName, 0, sizeof(newName));
		}
	}
}

void NodeStyleManager::AddCustomType(const NodeStyleInfo& type) {
	if (m_Index.contains(type.Identifier)) return;
	m_Types.push_back(type);
	m_Index[type.Identifier] = m_Types.size() - 1;
}

void NodeStyleManager::RemoveCustomType(const std::string& identifier) {
	auto it = m_Index.find(identifier);
	if (it == m_Index.end() || !m_Types[it->second].IsUserDefined) return;

	m_Types.erase(m_Types.begin() + it->second);
	m_Index.clear();
	for (size_t i = 0; i < m_Types.size(); ++i)
		m_Index[m_Types[i].Identifier] = i;
}

const NodeStyleInfo* NodeStyleManager::FindType(const std::string& identifier) const {
	auto it = m_Index.find(identifier);
	if (it != m_Index.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}
