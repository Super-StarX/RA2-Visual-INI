// LinkType.cpp
#include "LinkType.h"
#include <nlohmann/json.hpp>
#include <fstream>

LinkTypeManager::LinkTypeManager() {
	auto AddType = [this](const LinkTypeInfo& type) {
		if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
			return;

		m_Types.push_back(type);
		m_TypeIndex[type.Identifier] = m_Types.size() - 1;
	};

	AddType({ "default", "Default",
			IM_COL32(255, 255, 255, 255), IM_COL32(200, 200, 255, 255),
			1.0f, 0 });
	AddType({ "red", "Red",
			IM_COL32(180, 0, 0, 255), IM_COL32(220, 0, 0, 255),
			1.0f, 1 });
	AddType({ "blue", "Blue",
			IM_COL32(0, 0, 180, 255), IM_COL32(0, 0, 255, 255),
			2.0f, 0 });
}

const LinkTypeInfo* LinkTypeManager::FindType(const std::string& identifier) const {
	auto it = m_TypeIndex.find(identifier);
	if (it != m_TypeIndex.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}

void LinkTypeManager::AddCustomType(const LinkTypeInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}

void LinkTypeManager::RemoveCustomType(const std::string& identifier) {
	auto it = m_TypeIndex.find(identifier);
	if (it == m_TypeIndex.end() || !m_Types[it->second].IsUserDefined)
		return;

	m_Types.erase(m_Types.begin() + it->second);
	m_TypeIndex.clear();
	for (size_t i = 0; i < m_Types.size(); ++i)
		m_TypeIndex[m_Types[i].Identifier] = i;
}

bool LinkTypeManager::LoadFromFile(const std::string& path) {
	try {
		using json = nlohmann::json;
		json j = json::parse(path);
		for (auto& t : j["LinkTypes"]) {
			auto& color = t["Color"];
			LinkTypeInfo info{
				t["Identifier"].get<std::string>(),
				t["DisplayName"].get<std::string>(),
				ImColor(
				color[0].get<float>(), color[1].get<float>(),
				color[2].get<float>(), color[3].get<float>()
				),
				t["HighlightColor"].get<ImU32>(),
				t["Thickness"].get<float>(),
				t["IsUserDefined"].get<bool>()
			};
			AddCustomType(info);
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

bool LinkTypeManager::SaveToFile(const std::string& path) {
	using json = nlohmann::json;
	json j = json::parse(path);
	for (const auto& type : m_Types) {
		if (!type.IsUserDefined) continue;

		json color;
		color.push_back(type.Color.Value.x);
		color.push_back(type.Color.Value.y);
		color.push_back(type.Color.Value.z);
		color.push_back(type.Color.Value.w);

		json t;
		t["Identifier"] = type.Identifier;
		t["DisplayName"] = type.DisplayName;
		t["Color"] = color;
		t["HighlightColor"] = type.HighlightColor;
		t["Thickness"] = type.Thickness;
		t["IsUserDefined"] = type.IsUserDefined;

		j["LinkTypes"].push_back(t);
	}

	std::ofstream file(path);
	if (!file.is_open())
		return false;

	file << std::setw(4) << j << std::endl;
	return true;
}