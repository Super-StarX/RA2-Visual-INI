#include "PinStyle.h"
#include "LinkStyle.h"
#include "Localization.h"
#include "Utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <misc/cpp/imgui_stdlib.h>

PinStyleManager::PinStyleManager() {
	auto AddType = [this](const PinStyleInfo& type) {
		if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
			return;

		m_Types.push_back(type);
		m_TypeIndex[type.Identifier] = m_Types.size() - 1;
	};

	AddType({ "flow",    LOCALE["Style Flow"],    ImColor(255, 255, 255), 0 });
	AddType({ "bool",    LOCALE["Style Boolean"], ImColor(220, 48, 48),   1 });
	AddType({ "int",     LOCALE["Style Integer"], ImColor(68, 201, 156),  1 });
	AddType({ "float",   LOCALE["Style Float"],   ImColor(147, 226, 74),  1 });
	AddType({ "string",  LOCALE["Style String"],  ImColor(124, 21, 153),  1 });
	AddType({ "object",  LOCALE["Style Object"],  ImColor(51, 150, 215),  1 });
	AddType({ "function",LOCALE["Style Function"],ImColor(218, 0, 183),   1 });
	AddType({ "delegate",LOCALE["Style Delegate"],ImColor(255, 48, 48),   2 });
}

void PinStyleManager::Menu() {
	static int selected = -1;
	static auto iconTypes = Utils::GetComboItems({ LOCALE["Circle"], LOCALE["Square"], LOCALE["Triangle"], LOCALE["Diamond"] });

	const auto& types = PinStyleManager::Get().GetAllTypes();

	ImGui::Text(LOCALE["Pin Style"]);
	ImGui::BeginChild("##PinStyleList", ImVec2(150, 300), true);
	for (int i = 0; i < types.size(); ++i) {
		const auto& type = types[i];
		ImGui::PushID(i);

		// 给每一行留出删除按钮空间（比如 20 像素）
		float fullWidth = ImGui::GetContentRegionAvail().x;
		float buttonWidth = 20.0f;
		float labelWidth = fullWidth - buttonWidth - 5.0f; // 5 px 间距

		// 限制 Selectable 宽度
		if (ImGui::Selectable(type.DisplayName.c_str(), selected == i, 0, ImVec2(labelWidth, 0))) {
			selected = i;
		}

		// 删除按钮
		if (type.IsUserDefined) {
			ImGui::SameLine(labelWidth + 15.0f); // 靠右对齐
			if (ImGui::SmallButton("×")) {
				PinStyleManager::Get().RemoveCustomType(type.Identifier);

				if (selected == i)
					selected = -1;
				else if (selected > i)
					selected--;

				ImGui::PopID();
				break;
			}
		}

		ImGui::PopID();
	}

	ImGui::EndChild();

	ImGui::SameLine();

	static auto textWidth = Utils::max(
		ImGui::CalcTextSize(LOCALE["Identifier"]).x, 
		ImGui::CalcTextSize(LOCALE["Display Name"]).x, 
		ImGui::CalcTextSize(LOCALE["Pin Color"]).x, 
		ImGui::CalcTextSize(LOCALE["Pin Icon Type"]).x, 
		ImGui::CalcTextSize(LOCALE["Pin Link Style"]).x
	);

	// 编辑区域
	ImGui::BeginGroup();
	if (selected >= 0 && selected < types.size()) {
		PinStyleInfo& type = const_cast<PinStyleInfo&>(types[selected]);

		Utils::InputTextWithLeftLabel("##Identifier", LOCALE["Identifier"], textWidth, &type.Identifier, true);
		Utils::InputTextWithLeftLabel("##DisplayName", LOCALE["Display Name"], textWidth, &type.DisplayName);

		ImVec4 col = type.Color;
		Utils::InsertLeftLabelToNextItem(LOCALE["Pin Color"], textWidth);
		if (ImGui::ColorEdit4("##PinColor", (float*)&col))
			type.Color = ImColor(col);

		Utils::InsertLeftLabelToNextItem(LOCALE["Pin Icon Type"], textWidth);


		ImGui::Combo("##PinIconType", &type.IconType, iconTypes);

		Utils::InsertLeftLabelToNextItem(LOCALE["Pin Link Style"], textWidth);
		if (ImGui::BeginCombo("##PinLinkStyle", LinkStyleManager::Get().FindType(type.LinkType)->DisplayName.c_str())) {
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
			if (ImGui::Button(LOCALE["Delete"])) {
				PinStyleManager::Get().RemoveCustomType(type.Identifier);
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
	static ImVec4 newColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	static int newIconType = 0;
	static std::string newLinkType = LinkStyleManager::GetDefaultIdentifier();
	static std::string newLinkDisplayName = LinkStyleManager::GetDefaultDisplayName();

	ImGui::PushItemWidth(455.0f);
	ImGui::Text(LOCALE["Add New Style"]);

	Utils::InputTextWithLeftLabel("##IdentifierNew", LOCALE["Identifier"], textWidth, &newId);
	Utils::InputTextWithLeftLabel("##DisplayNameNew", LOCALE["Display Name"], textWidth, &newName);

	Utils::InsertLeftLabelToNextItem(LOCALE["Pin Color"], textWidth);
	ImGui::ColorEdit4("##PinColorNew", (float*)&newColor);

	Utils::InsertLeftLabelToNextItem(LOCALE["Pin Color"], textWidth);
	ImGui::Combo("##PinIconTypeNew", &newIconType, iconTypes);

	Utils::InsertLeftLabelToNextItem(LOCALE["Pin Link Style"], textWidth);
	if (ImGui::BeginCombo("##PinLinkStyleNew", newLinkDisplayName.c_str())) {
		for (const auto& linkType : LinkStyleManager::Get().GetAllTypes()) {
			bool isSelected = (newLinkType == linkType.Identifier);
			if (ImGui::Selectable(linkType.DisplayName.c_str(), isSelected)) {
				newLinkType = linkType.Identifier;
				newLinkDisplayName = linkType.DisplayName;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	if (ImGui::Button(LOCALE["Add"])) {
		if (!newId.empty() && !newName.empty() && !PinStyleManager::Get().FindType(newId)) {
			PinStyleManager::Get().AddCustomType({ newId,newName,ImColor(newColor),newIconType,newLinkType,true });
			newId.clear();
			newName.clear();
			newLinkType = LinkStyleManager::GetDefaultIdentifier();
			newLinkDisplayName = LinkStyleManager::GetDefaultDisplayName();
		}
	}
	ImGui::EndGroup();
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

void PinStyleManager::AddCustomType(const PinStyleInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}