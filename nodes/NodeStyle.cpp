#include "NodeStyle.h"
#include "Utils.h"
#include <imgui.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include "Localization.h"

NodeStyleManager::NodeStyleManager() {
	AddCustomType({ "default", LOCALE["Style Default"], ImColor(0, 64, 128), false });
	AddCustomType({ "red",LOCALE["Style Red"], ImColor(255, 0, 0), false });
	AddCustomType({ "green",LOCALE["Style Green"], ImColor(0, 255, 0), false });
	AddCustomType({ "blue",LOCALE["Style Blue"], ImColor(0, 0, 255), false });
	AddCustomType({ "white",LOCALE["Style White"], ImColor(255, 255, 255), true });
	AddCustomType({ "black",LOCALE["Style Black"], ImColor(0, 0, 0), true });
	AddCustomType({ "dark",LOCALE["Style Dark Blue"], ImColor(30, 30, 80), true });
}
void NodeStyleManager::Menu() {
	static int selected = -1;
	const auto& types = NodeStyleManager::Get().GetAllTypes();

	ImGui::Text(LOCALE["Node Styles"]);
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

	static auto textWidth = Utils::max(
		ImGui::CalcTextSize(LOCALE["Identifier"]).x, 
		ImGui::CalcTextSize(LOCALE["Display Name"]).x,
		ImGui::CalcTextSize(LOCALE["Title Color"]).x
	);

	if (selected >= 0 && selected < types.size()) {
		auto& type = const_cast<NodeStyleInfo&>(types[selected]);
		Utils::InputTextWithLeftLabel("##Identifier", LOCALE["Identifier"], textWidth, &type.Identifier, true);
		Utils::InputTextWithLeftLabel("##DisplayName", LOCALE["Display Name"], textWidth, &type.Identifier);

		ImVec4 color = type.Color;
		Utils::InsertLeftLabelToNextItem(LOCALE["Title Color"], textWidth);
		if (ImGui::ColorEdit4("##NodeColor", (float*)&color))
			type.Color = ImColor(color);

		if (type.IsUserDefined) {
			if (ImGui::Button(LOCALE["Delete"])) {
				NodeStyleManager::Get().RemoveCustomType(type.Identifier);
				selected = -1;
			}
		}
	}

	ImGui::EndGroup();
	ImGui::Separator();

	// 添加新样式
	ImGui::BeginGroup();
	static std::string newId{};
	static std::string newName{};
	static ImVec4 newColor = ImVec4(0.3f, 0.3f, 0.6f, 1.0f);

	ImGui::Text(LOCALE["Add New Style"]);

	Utils::InputTextWithLeftLabel("##IdentifierNew", LOCALE["Identifier"], textWidth, &newId);
	Utils::InputTextWithLeftLabel("##DisplayNameNew", LOCALE["Display Name"], textWidth, &newName);

	Utils::InsertLeftLabelToNextItem(LOCALE["Title Color"], textWidth);
	ImGui::ColorEdit4("##NodeColorNew", (float*)&newColor);

	if (ImGui::Button(LOCALE["Add"])) {
		if (!newId.empty() && !newName.empty() && !NodeStyleManager::Get().FindType(newId)) {
			NodeStyleManager::Get().AddCustomType({ newId,newName,ImColor(newColor),true });

			newId.clear();
			newName.clear();
		}
	}

	ImGui::EndGroup();
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

bool NodeStyleManager::LoadFromFile(const std::string& path) {
	try {
		std::ifstream file(path);
		if (!file.is_open())
			return false;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string jsonContent = buffer.str();

		using json = nlohmann::json;
		json j = json::parse(jsonContent);

		if (!j.contains("NodeStyles"))
			return false;

		for (const auto& type : j["NodeStyles"]) {
			NodeStyleInfo info;
			info.Identifier = type["Identifier"].get<std::string>();
			info.DisplayName = type["DisplayName"].get<std::string>();

			auto& color = type["Color"];
			info.Color = ImColor(
				color[0].get<float>(),
				color[1].get<float>(),
				color[2].get<float>(),
				color[3].get<float>());

			info.IsUserDefined = type["IsUserDefined"].get<bool>();

			// 替换或添加用户自定义样式
			if (info.IsUserDefined) {
				auto existing = m_Index.find(info.Identifier);
				if (existing != m_Index.end() &&
					m_Types[existing->second].IsUserDefined) {
					m_Types[existing->second] = info;
				}
				else {
					AddCustomType(info);
				}
			}
		}
		return true;
	}
	catch (...) {
		return false;
	}
}
bool NodeStyleManager::SaveToFile(const std::string& path) {
	using json = nlohmann::json;
	json j;

	for (const auto& type : m_Types) {
		if (!type.IsUserDefined)
			continue;

		json t;
		t["Identifier"] = type.Identifier;
		t["DisplayName"] = type.DisplayName;

		json color;
		color.push_back(type.Color.Value.x);
		color.push_back(type.Color.Value.y);
		color.push_back(type.Color.Value.z);
		color.push_back(type.Color.Value.w);
		t["Color"] = color;

		t["IsUserDefined"] = type.IsUserDefined;

		j["NodeStyles"].push_back(t);
	}

	std::ofstream file(path);
	if (!file.is_open())
		return false;

	file << std::setw(4) << j << std::endl;
	return true;
}
