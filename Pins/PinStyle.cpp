#include "PinStyle.h"
#include "LinkStyle.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <misc/cpp/imgui_stdlib.h>

void PinStyleManager::Menu() {

	static int selected = -1;
	const auto& types = PinStyleManager::Get().GetAllTypes();

	ImGui::Text("Pin Styles:");
	ImGui::BeginChild("##PinStyleList", ImVec2(200, 300), true);
	for (int i = 0; i < types.size(); ++i) {
		const auto& type = types[i];
		if (ImGui::Selectable(type.DisplayName.c_str(), selected == i)) {
			selected = i;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 编辑区域
	ImGui::BeginGroup();
	if (selected >= 0 && selected < types.size()) {
		PinStyleInfo& type = const_cast<PinStyleInfo&>(types[selected]);

		ImGui::Text("Edit Pin Style:");
		ImGui::Text("Identifier: %s", type.Identifier.c_str());
		ImGui::InputText("Name", &type.DisplayName);

		ImVec4 col = type.Color;
		if (ImGui::ColorEdit4("Color", (float*)&col))
			type.Color = ImColor(col);

		ImGui::Combo("Icon Type", &type.IconType, "Circle\0Square\0Triangle\0Diamond\0");

		if (ImGui::BeginCombo("Link Type", type.LinkType.c_str())) {
			for (const auto& linkType : LinkStyleManager::Get().GetAllTypes()) {
				bool isSelected = (type.LinkType == linkType.Identifier);
				if (ImGui::Selectable(linkType.DisplayName.c_str(), isSelected)) {
					type.LinkType = linkType.Identifier;
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (type.IsUserDefined) {
			if (ImGui::Button("Delete")) {
				PinStyleManager::Get().RemoveCustomType(type.Identifier);
				selected = -1;
			}
		}
	}
	ImGui::EndGroup();

	ImGui::Separator();

	// 添加新样式
	static char newId[64] = "";
	static char newName[64] = "";
	static ImVec4 newColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	static int newIconType = 0;
	static std::string newLinkType = "default";

	ImGui::Text("Add New Pin Style:");
	ImGui::InputText("Identifier", newId, IM_ARRAYSIZE(newId));
	ImGui::InputText("Display Name", newName, IM_ARRAYSIZE(newName));
	ImGui::ColorEdit4("Color##new", (float*)&newColor);
	ImGui::Combo("Icon Type##new", &newIconType, "Circle\0Square\0Triangle\0Diamond\0");

	if (ImGui::BeginCombo("Link Type##new", newLinkType.c_str())) {
		for (const auto& linkType : LinkStyleManager::Get().GetAllTypes()) {
			bool isSelected = (newLinkType == linkType.Identifier);
			if (ImGui::Selectable(linkType.DisplayName.c_str(), isSelected)) {
				newLinkType = linkType.Identifier;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Add")) {
		if (strlen(newId) > 0 && strlen(newName) > 0 && !PinStyleManager::Get().FindType(newId)) {
			PinStyleInfo newType;
			newType.Identifier = newId;
			newType.DisplayName = newName;
			newType.Color = ImColor(newColor);
			newType.IconType = newIconType;
			newType.LinkType = newLinkType;
			newType.IsUserDefined = true;

			PinStyleManager::Get().AddCustomType(newType);
			memset(newId, 0, sizeof(newId));
			memset(newName, 0, sizeof(newName));
			newLinkType = "default";
		}
	}

}

const PinStyleInfo* PinStyleManager::FindType(const std::string& identifier) const {
	auto it = m_TypeIndex.find(identifier);
	if (it != m_TypeIndex.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}

void PinStyleManager::RemoveCustomType(const std::string& identifier) {
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

bool PinStyleManager::LoadFromFile(const std::string& path) {
	try {
		// 打开文件并读取内容
		std::ifstream file(path);
		if (!file.is_open()) {
			// 文件无法打开时返回 false
			return false;
		}

		// 将文件内容读取到字符串中
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string jsonContent = buffer.str();

		// 解析 JSON 数据
		using json = nlohmann::json;
		json j = json::parse(jsonContent);
		for (const auto& type : j["Types"]) {

			PinStyleInfo info;
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

bool PinStyleManager::SaveToFile(const std::string& path) {
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

PinStyleManager::PinStyleManager() {
	auto AddType = [this](const PinStyleInfo& type) {
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
	//AddType({ "tag",     "Tag",     ImColor(220, 48, 48),   0 });
}

void PinStyleManager::AddCustomType(const PinStyleInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}