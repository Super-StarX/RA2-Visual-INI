#include "PinType.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

void PinTypeManager::Menu() {
	static char newIdentifier[128] = "";
	static char newDisplayName[128] = "";
	static ImColor newColor = ImColor(255, 255, 255);
	static int newIconType = 0;

	// 添加新类型
	ImGui::InputText("Identifier", newIdentifier, IM_ARRAYSIZE(newIdentifier));
	ImGui::InputText("Display Name", newDisplayName, IM_ARRAYSIZE(newDisplayName));
	ImGui::ColorEdit4("Color", &newColor.Value.x);
	ImGui::Combo("Icon", &newIconType, "Flow\0Circle\0Square\0Grid\0RoundSquare\0Diamond\0");

	if (ImGui::Button("Add New Type") && newIdentifier[0] != '\0') {
		PinTypeInfo newType;
		newType.Identifier = newIdentifier;
		newType.DisplayName = newDisplayName[0] ? newDisplayName : newIdentifier;
		newType.Color = newColor;
		newType.IconType = newIconType;
		newType.IsUserDefined = true;

		PinTypeManager::Get().AddCustomType(newType);

		// 清空输入
		memset(newIdentifier, 0, sizeof(newIdentifier));
		memset(newDisplayName, 0, sizeof(newDisplayName));
		newColor = ImColor(255, 255, 255);
	}

	ImGui::Separator();

	// 现有类型列表
	ImGui::Text("Existing Types:");
	ImGui::BeginChild("##type_list", ImVec2(0, 200), true);
	for (const auto& type : PinTypeManager::Get().GetAllTypes()) {
		ImGui::ColorButton("##color", type.Color,
			ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
		ImGui::SameLine();

		if (type.IsUserDefined) {
			if (ImGui::Button(("X##del_" + type.Identifier).c_str())) {
				PinTypeManager::Get().RemoveCustomType(type.Identifier);
			}
			ImGui::SameLine();
		}

		ImGui::Text("%s (%s)", type.DisplayName.c_str(),
			type.Identifier.c_str());
	}
	ImGui::EndChild();
}

const PinTypeInfo* PinTypeManager::FindType(const std::string& identifier) const {
	auto it = m_TypeIndex.find(identifier);
	if (it != m_TypeIndex.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}

void PinTypeManager::RemoveCustomType(const std::string& identifier) {
	auto it = m_TypeIndex.find(identifier);
	if (it == m_TypeIndex.end())
		return;

	// 确保只删除用户自定义类型
	if (!m_Types[it->second].IsUserDefined)
		return;

	// 从vector中移除
	m_Types.erase(m_Types.begin() + it->second);

	// 重建索引
	m_TypeIndex.clear();
	for (size_t i = 0; i < m_Types.size(); ++i) {
		m_TypeIndex[m_Types[i].Identifier] = i;
	}
}

bool PinTypeManager::LoadFromFile(const std::string& path) {
	try {
		using json = nlohmann::json;
		json j = json::parse(path);
		for (const auto& type : j["Types"]) {

			PinTypeInfo info;
			info.Identifier = type["Identifier"].get<std::string>();
			info.DisplayName = type["DisplayName"].get<std::string>();

			auto& color = type["Color"];
			info.Color = ImColor(
				color[0].get<float>(), color[1].get<float>(),
				color[2].get<float>(), color[3].get<float>()
			);

			info.IconType = static_cast<int>(type["IconType"].get<double>());
			info.IsUserDefined = type["IsUserDefined"].get<bool>();

			// 替换已存在的自定义类型
			if (info.IsUserDefined) {
				auto existing = m_TypeIndex.find(info.Identifier);
				if (existing != m_TypeIndex.end() &&
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

bool PinTypeManager::SaveToFile(const std::string& path) {
	using json = nlohmann::json;
	json j;
	for (const auto& type : m_Types) {
		if (!type.IsUserDefined)
			continue;

		json typeObj;
		typeObj["Identifier"] = type.Identifier;
		typeObj["DisplayName"] = type.DisplayName;

		json color;
		color.push_back(type.Color.Value.x);
		color.push_back(type.Color.Value.y);
		color.push_back(type.Color.Value.z);
		color.push_back(type.Color.Value.w);
		typeObj["Color"] = color;

		typeObj["IconType"] = type.IconType;
		typeObj["IsUserDefined"] = type.IsUserDefined;

		j["Types"].push_back(typeObj);
	}

	std::ofstream file(path);
	if (!file.is_open())
		return false;

	file << std::setw(4) << j << std::endl;
	return true;
}

PinTypeManager::PinTypeManager() {
	auto AddType = [this](const PinTypeInfo& type) {
		if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
			return;

		m_Types.push_back(type);
		m_TypeIndex[type.Identifier] = m_Types.size() - 1;
	};

	AddType({ "flow",    "Flow",    ImColor(255, 255, 255), 0 });
	AddType({ "bool",    "Boolean", ImColor(220, 48, 48),   1 });
	AddType({ "int",     "Integer", ImColor(68, 201, 156),  1 });
	AddType({ "float",   "Float",   ImColor(147, 226, 74),  1 });
	AddType({ "string",  "String",  ImColor(124, 21, 153),  1 });
	AddType({ "object",  "Object",  ImColor(51, 150, 215),  1 });
	AddType({ "function","Function",ImColor(218, 0, 183),   1 });
	AddType({ "delegate","Delegate",ImColor(255, 48, 48),   2 });
}

void PinTypeManager::AddCustomType(const PinTypeInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}