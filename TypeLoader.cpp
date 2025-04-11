#include "TypeLoader.h"
#include "Utils.h"
#include "Localization.h"
#include <IniFile.h>
#include <misc/cpp/imgui_stdlib.h>

TypeSystem& TypeSystem::Get() {
	static TypeSystem instance;
	return instance;
}

TypeInfo TypeSystem::GetTypeInfo(const std::string& typeName) const {
	TypeInfo info;
	info.TypeName = typeName;

	std::unordered_map<std::string, TypeCategory> BasicTypes = {
		{"bool", TypeCategory::Bool},
		{"ColorStruct", TypeCategory::Color},
	};

	if (BasicTypes.contains(typeName)) {
		info.Category = BasicTypes.at(typeName);
	}
	else if (Sections.count(typeName)) {
		info.Category = TypeCategory::Section;
	}
	else if (NumberLimits.count(typeName)) {
		info.Category = TypeCategory::NumberLimit;
		info.Data = NumberLimits.at(typeName);
	}
	else if (StringLimits.count(typeName)) {
		info.Category = TypeCategory::StringLimit;
		info.Data = StringLimits.at(typeName);
	}
	else if (Lists.count(typeName)) {
		info.Category = TypeCategory::List;
		info.Data = Lists.at(typeName);
	}
	else if (BasicTypes.count(typeName)) {
		info.Category = TypeCategory::Basic;
	}
	return info;
}

TypeInfo TypeSystem::GetKeyType(const std::string& sectionType, const std::string& key) const {
	if (Sections.count(sectionType)) {
		auto& section = Sections.at(sectionType);
		if (section.contains(key)) {
			return GetTypeInfo(section.at(key));
		}
	}
	return {};
}

void TypeSystem::LoadFromINI(const std::string& path) {
	IniFile ini(path);

	// 处理[Sections]主段（特殊处理）
	if (ini.sections.count("Sections")) {
		auto& regSection = ini.sections.at("Sections");
		for (auto& [typeName, _] : regSection.section)
			if (ini.sections.count(typeName))
				Sections[typeName] = ini.sections.at(typeName);
	}

	// 类型分类处理函数
	auto processCategory = [&](const std::string& category, auto& typeMap, auto handler) {
		if (ini.sections.count(category)) {
			auto& mainSection = ini.sections.at(category);
			for (auto& [name, _] : mainSection.section) {
				typeMap[name] = {}; // 注册类型

				// 处理类型定义子段
				if (ini.sections.count(name)) {
					auto& typeSection = ini.sections.at(name);
					handler(typeSection, typeMap[name]);
				}
			}
		}
	};

	// 处理NumberLimits类型
	processCategory("NumberLimits", NumberLimits, [](auto& section, auto& limit) {
		if (section.section.count("Range")) {
			auto parts = Utils::SplitString(section.section.at("Range"), ',');
			if (parts.size() == 2) {
				limit.Min = std::stoi(parts[0]);
				limit.Max = std::stoi(parts[1]);
			}
		}
	});

	// 处理Limits类型
	processCategory("Limits", StringLimits, [](auto& section, auto& limit) {
		if (section.section.count("StartWith"))
			limit.StartWith = static_cast<std::string>(section.section.at("StartWith"));
		if (section.section.count("EndWith"))
			limit.EndWith = static_cast<std::string>(section.section.at("EndWith"));
		if (section.section.count("LimitIn"))
			limit.ValidValues = Utils::SplitString(section.section.at("LimitIn"), ',');
		if (section.section.count("CaseSensitive"))
			limit.CaseSensitive = (static_cast<std::string>(section.section.at("CaseSensitive")) == "true");
	});

	// 处理Lists类型
	processCategory("Lists", Lists, [](auto& section, auto& list) {
		if (section.section.count("Type"))
			list.ElementType = static_cast<std::string>(section.section.at("Type"));
		if (section.section.count("Range")) {
			auto parts = Utils::SplitString(section.section.at("Range"), ',');
			if (parts.size() == 2) {
				list.MinLength = std::stoi(parts[0]);
				list.MaxLength = std::stoi(parts[1]);
			}
		}
	});
}

void TypeSystem::Draw(bool* show) {
	static int selectedCategory = -1; // 0: NumberLimit, 1: StringLimit, 2: List, 3: Section
	static std::string selectedName;
	static bool showAddPopup = false;

	auto& ts = TypeSystem::Get();
	ImGui::Begin(LOCALE["Type Manager"], show);

	ImGui::BeginGroup();
	ImGui::BeginChild("##TypeList", ImVec2(250, 500), true);
	ImGui::Text("Types:");

	auto DrawTypeGroup = [&](const char* label, const auto& container, int categoryIndex) {
		if (ImGui::TreeNode(label)) {
			int idx = 0;
			for (const auto& [name, data] : container) {
				ImGui::PushID((int)name[0] + idx++);

				float fullWidth = ImGui::GetContentRegionAvail().x;
				float deleteBtnWidth = 20.0f;
				float labelWidth = fullWidth - deleteBtnWidth - 6.0f;

				// 选中逻辑
				if (ImGui::Selectable(name.c_str(), selectedName == name, 0, ImVec2(labelWidth, 0))) {
					selectedName = name;
					selectedCategory = categoryIndex;
				}

				// 删除按钮
				ImGui::SameLine(labelWidth + 6.0f);
				if (ImGui::SmallButton("×")) {
					if (selectedName == name)
						selectedName.clear();

					if constexpr (std::is_same_v<decltype(data), NumberLimit>)
						ts.NumberLimits.erase(name);
					else if constexpr (std::is_same_v<decltype(data), StringLimit>)
						ts.StringLimits.erase(name);
					else if constexpr (std::is_same_v<decltype(data), ListType>)
						ts.Lists.erase(name);
					else if constexpr (std::is_same_v<decltype(data), Section>)
						ts.Sections.erase(name);

					ImGui::PopID();
					break;
				}

				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	};

	DrawTypeGroup("NumberLimits", ts.NumberLimits, 0);
	DrawTypeGroup("StringLimits", ts.StringLimits, 1);
	DrawTypeGroup("Lists", ts.Lists, 2);
	DrawTypeGroup("Sections", ts.Sections, 3);
	ImGui::EndChild();

	if (ImGui::Button("Add Type")) {
		showAddPopup = true;
		ImGui::OpenPopup("AddNewType");
	}
	ImGui::EndGroup();

	// 右侧属性编辑面板
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Type Info:");
	ImGui::Separator();

	if (!selectedName.empty()) {
		TypeInfo info = ts.GetTypeInfo(selectedName);

		ImGui::Text("Name: %s", selectedName.c_str());
		// ImGui::Text("Category: %d", (int)info.Category);

		if (info.Category == TypeCategory::NumberLimit) {
			auto& limit = std::get<NumberLimit>(info.Data);
			ImGui::InputInt("Min", &limit.Min);
			ImGui::InputInt("Max", &limit.Max);
			if (ImGui::Button("Apply")) {
				ts.NumberLimits[selectedName] = limit;
			}
		}
		else if (info.Category == TypeCategory::StringLimit) {
			auto& lim = std::get<StringLimit>(info.Data);
			std::string bufStart = lim.StartWith;
			std::string bufEnd = lim.EndWith;
			std::string validBuf = Utils::JoinStrings(lim.ValidValues, ",");
			ImGui::InputText("StartWith", &bufStart);
			ImGui::InputText("EndWith", &bufEnd);
			ImGui::Checkbox("CaseSensitive", &lim.CaseSensitive);
			ImGui::InputText("ValidValues", &validBuf);

			if (ImGui::Button("Apply")) {
				lim.StartWith = bufStart;
				lim.EndWith = bufEnd;
				lim.ValidValues = Utils::SplitString(validBuf, ',');
				ts.StringLimits[selectedName] = lim;
			}
		}
		else if (info.Category == TypeCategory::List) {
			auto& list = std::get<ListType>(info.Data);
			char bufType[128] = {};
			strcpy_s(bufType, list.ElementType.c_str());
			ImGui::InputText("ElementType", bufType, sizeof(bufType));
			ImGui::InputInt("MinLength", &list.MinLength);
			ImGui::InputInt("MaxLength", &list.MaxLength);

			if (ImGui::Button("Apply")) {
				list.ElementType = bufType;
				ts.Lists[selectedName] = list;
			}
		}
		else if (info.Category == TypeCategory::Section) {
			auto& section = ts.Sections[selectedName];

			ImGui::Text("Key-Value Pairs:");

			// ---------- Add key-value row ----------
			static std::string newKey;
			static std::string newValue;

			ImGui::Text("Add Key-Value:");

			float totalWidth = ImGui::GetContentRegionAvail().x;
			float keyWidth = totalWidth * 0.25f;
			float valueWidth = totalWidth * 0.50f;
			float buttonWidth = totalWidth - keyWidth - valueWidth - 8.0f; // 留出些间距

			ImGui::PushID("AddKVRow");

			// key input
			ImGui::PushItemWidth(keyWidth);
			ImGui::InputText("##newKey", &newKey);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			// value input
			ImGui::PushItemWidth(valueWidth);
			ImGui::InputText("##newValue", &newValue);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			// add button
			if (ImGui::Button("Add", ImVec2(buttonWidth, 0))) {
				if (!newKey.empty() && !section.section.contains(newKey)) {
					section.section[newKey] = Value{ newValue };
					newKey.clear();
					newValue.clear();
				}
			}

			ImGui::PopID();
			ImGui::Separator();

			if (ImGui::BeginTable("SectionTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, 30.0f);

				for (auto it = section.section.begin(); it != section.section.end(); ) {
					const std::string& key = it->first;
					Value& value = it->second;

					ImGui::TableNextRow();

					// --- Column 0: Key ---
					ImGui::TableSetColumnIndex(0);
					ImGui::PushTextWrapPos(0.0f); // 防止被剪裁太狠
					ImGui::TextUnformatted(key.c_str());
					ImGui::PopTextWrapPos();


					// --- Column 1: Value ---
					ImGui::TableSetColumnIndex(1);
					char valBuf[256] = {};
					strcpy_s(valBuf, value.value.c_str());
					ImGui::PushItemWidth(-1);
					if (ImGui::InputText(("##val_" + key).c_str(), valBuf, sizeof(valBuf))) {
						value.value = valBuf;
					}
					ImGui::PopItemWidth();

					// --- Column 2: Delete ---
					ImGui::TableSetColumnIndex(2);
					if (ImGui::SmallButton(("×##" + key).c_str())) {
						it = section.section.erase(it);
						continue;
					}

					++it;
				}

				ImGui::EndTable();
			}
		}
		else {
			ImGui::Text("该类型无需编辑或不支持编辑。");
		}
	}

	ImGui::EndGroup();

	// 添加类型弹窗
	if (ImGui::BeginPopupModal("AddNewType", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static char newName[128] = "";
		static int newCategory = 0;

		ImGui::InputText("Type Name", newName, sizeof(newName));
		ImGui::Combo("Category", &newCategory, "NumberLimit\0StringLimit\0List\0Section\0");

		if (ImGui::Button("Create")) {
			std::string typeName = newName;
			if (typeName.empty()) {
				ImGui::CloseCurrentPopup();
			}
			else {
				switch (newCategory) {
				case 0: ts.NumberLimits[typeName] = NumberLimit(); break;
				case 1: ts.StringLimits[typeName] = StringLimit(); break;
				case 2: ts.Lists[typeName] = ListType(); break;
				case 3: ts.Sections[typeName] = Section(); break;
				}
				selectedName = typeName;
				selectedCategory = newCategory;
				newName[0] = '\0';
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			newName[0] = '\0';
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}