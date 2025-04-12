// LinkStyle.cpp
#include "LinkStyle.h"
#include "Localization.h"
#include "Utils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <misc/cpp/imgui_stdlib.h>

const char* LinkStyleManager::GetDefaultIdentifier() {
	return "default";
}
const char* LinkStyleManager::GetDefaultDisplayName() {
	return LOCALE["Style Default"];
}

LinkStyleManager::LinkStyleManager() {
	auto AddType = [this](const LinkStyleInfo& type) {
		if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
			return;

		m_Types.push_back(type);
		m_TypeIndex[type.Identifier] = m_Types.size() - 1;
	};

	AddType({ GetDefaultIdentifier(), GetDefaultDisplayName(),
			IM_COL32(255, 255, 255, 255), IM_COL32(200, 200, 255, 255),
			1.0f, 0 });
	AddType({ "red", LOCALE["Style Red"],
			IM_COL32(180, 0, 0, 255), IM_COL32(220, 0, 0, 255),
			1.0f, 1 });
	AddType({ "blue", LOCALE["Style Blue"],
			IM_COL32(0, 0, 180, 255), IM_COL32(0, 0, 255, 255),
			2.0f, 1 });
}

void LinkStyleManager::Menu() {
	static int selected = -1;
	const auto& types = LinkStyleManager::Get().GetAllTypes();

	ImGui::Text(LOCALE["Link Style"]);
	ImGui::BeginChild("##LinkStyleList", ImVec2(150, 300), true);
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
				LinkStyleManager::Get().RemoveCustomType(type.Identifier);

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
		ImGui::CalcTextSize(LOCALE["Link Base Color"]).x,
		ImGui::CalcTextSize(LOCALE["Link Highlight Color"]).x,
		ImGui::CalcTextSize(LOCALE["Link Thickness"]).x
	);

	// 编辑区域
	ImGui::BeginGroup();
	if (selected >= 0 && selected < types.size()) {
		auto& type = const_cast<LinkStyleInfo&>(types[selected]); // 注意：这里直接修改内存，确保只对用户定义项启用修改

		Utils::InputTextWithLeftLabel("##Identifier", LOCALE["Identifier"], textWidth, &type.Identifier, true);
		Utils::InputTextWithLeftLabel("##DisplayName", LOCALE["Display Name"], textWidth, &type.DisplayName);

		ImVec4 col = type.Color;
		Utils::InsertLeftLabelToNextItem(LOCALE["Link Base Color"], textWidth);
		if (ImGui::ColorEdit4("##BaseColor", (float*)&col))
			type.Color = ImColor(col);

		ImVec4 highlight = ImGui::ColorConvertU32ToFloat4(type.HighlightColor);
		Utils::InsertLeftLabelToNextItem(LOCALE["Link Highlight Color"], textWidth);
		if (ImGui::ColorEdit4("##HighlightColor", (float*)&highlight))
			type.HighlightColor = ImGui::ColorConvertFloat4ToU32(highlight);

		Utils::InsertLeftLabelToNextItem(LOCALE["Link Thickness"], textWidth);
		ImGui::SliderFloat("##Thickness", &type.Thickness, 0.5f, 10.0f);

		if (type.IsUserDefined) {
			if (ImGui::Button(LOCALE["Delete"])) {
				LinkStyleManager::Get().RemoveCustomType(type.Identifier);
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
	static ImVec4 newHighlight = ImVec4(0.8f, 0.8f, 1.0f, 1.0f);
	static float newThickness = 1.0f;

	ImGui::Text(LOCALE["Add New Style"]);
	Utils::InputTextWithLeftLabel("##IdentifierNew", LOCALE["Identifier"], textWidth, &newId);
	Utils::InputTextWithLeftLabel("##DisplayNameNew", LOCALE["Display Name"], textWidth, &newName);
	Utils::InsertLeftLabelToNextItem(LOCALE["Link Base Color"], textWidth);
	ImGui::ColorEdit4("##BaseColorNew", (float*)&newColor);
	Utils::InsertLeftLabelToNextItem(LOCALE["Link Highlight Color"], textWidth);
	ImGui::ColorEdit4("##HighlightColorNew", (float*)&newHighlight);
	Utils::InsertLeftLabelToNextItem(LOCALE["Link Thickness"], textWidth);
	ImGui::SliderFloat("##ThicknessNew", &newThickness, 0.5f, 10.0f);

	if (ImGui::Button(LOCALE["Add"])) {
		if (!newId.empty() && !newName.empty() && !LinkStyleManager::Get().FindType(newId)) {
			LinkStyleInfo newType;
			newType.Identifier = newId;
			newType.DisplayName = newName;
			newType.Color = ImColor(newColor);
			newType.HighlightColor = ImGui::ColorConvertFloat4ToU32(newHighlight);
			newType.Thickness = newThickness;
			newType.IsUserDefined = true;

			LinkStyleManager::Get().AddCustomType(newType);
			newId.clear();
			newName.clear();
		}
	}
	ImGui::EndGroup();
}

const LinkStyleInfo* LinkStyleManager::FindType(const std::string& identifier) const {
	auto it = m_TypeIndex.find(identifier);
	if (it != m_TypeIndex.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}

void LinkStyleManager::AddCustomType(const LinkStyleInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}

void LinkStyleManager::RemoveCustomType(const std::string& identifier) {
	auto it = m_TypeIndex.find(identifier);
	if (it == m_TypeIndex.end() || !m_Types[it->second].IsUserDefined)
		return;

	m_Types.erase(m_Types.begin() + it->second);
	m_TypeIndex.clear();
	for (size_t i = 0; i < m_Types.size(); ++i)
		m_TypeIndex[m_Types[i].Identifier] = i;
}

bool LinkStyleManager::LoadFromFile(const std::string& path) {
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
		for (auto& t : j["LinkTypes"]) {
			auto& color = t["Color"];
			LinkStyleInfo info{
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

bool LinkStyleManager::SaveToFile(const std::string& path) {
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