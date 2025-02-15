#include "TypeLoader.h"
#include "Utils.h"
#include <IniFile.h>

TypeSystem& TypeSystem::Get() {
	static TypeSystem instance;
	return instance;
}

TypeInfo TypeSystem::GetTypeInfo(const std::string& typeName) const {
	TypeInfo info;
	info.TypeName = typeName;

	if (Sections.count(typeName)) {
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
		for (auto& [_, typeName] : regSection.section) {
			auto a = typeName;
			a.getFileName();
			if (ini.sections.count(typeName.value))
				Sections[typeName] = ini.sections.at(typeName.value);
		}
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