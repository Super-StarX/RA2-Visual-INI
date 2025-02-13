#include "TypeLoader.h"
#include "Utils.h"
#include <fstream>

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
		if (section.KeyTypes.count(key)) {
			return GetTypeInfo(section.KeyTypes.at(key));
		}
	}
	return {};
}

TypeSystem TypeLoader::LoadFromINI(const std::string& path) {
	TypeSystem ts;
	std::ifstream file(path);
	std::string line;
	std::string currentSection;
	ParseState state = ParseState::Global;
	std::string currentMainCategory;

	while (std::getline(file, line)) {
		line = line.substr(0, line.find(';'));
		line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
		if (line.empty()) continue;

		if (line[0] == '[') {
			currentSection = line.substr(1, line.find(']') - 1);

			if (currentSection == "Sections") {
				state = ParseState::InSections;
			}
			else if (currentSection == "NumberLimits") {
				state = ParseState::InNumberLimits;
			}
			else if (currentSection == "Limits") {
				state = ParseState::InLimits;
			}
			else if (currentSection == "Lists") {
				state = ParseState::InLists;
			}
			else {
				if (state == ParseState::InSections) {
					ts.Sections[currentSection].KeyTypes.clear();
					currentMainCategory = currentSection;
				}
				else if (ts.NumberLimits.count(currentSection)) {
					state = ParseState::InNumberLimits;
					currentMainCategory = currentSection;
				}
				else if (ts.StringLimits.count(currentSection)) {
					state = ParseState::InLimits;
					currentMainCategory = currentSection;
				}
				else if (ts.Lists.count(currentSection)) {
					state = ParseState::InLists;
					currentMainCategory = currentSection;
				}
			}
			continue;
		}

		size_t eqPos = line.find('=');
		if (eqPos != std::string::npos) {
			std::string key = line.substr(0, eqPos);
			std::string value = line.substr(eqPos + 1);

			switch (state) {
			case ParseState::InSections: {
				if (currentSection == "Sections") {
					ts.Sections[value] = {};
				}
				else if (!currentMainCategory.empty()) {
					ts.Sections[currentMainCategory].KeyTypes[key] = value;
				}
				break;
			}
			case ParseState::InNumberLimits: {
				if (key == "Range") {
					auto parts = Utils::SplitString(value, ',');
					if (parts.size() == 2) {
						ts.NumberLimits[currentMainCategory].Min = std::stoi(parts[0]);
						ts.NumberLimits[currentMainCategory].Max = std::stoi(parts[1]);
					}
				}
				break;
			}
			case ParseState::InLimits: {
				auto& limit = ts.StringLimits[currentMainCategory];
				if (key == "StartWith") limit.StartWith = value;
				else if (key == "EndWith") limit.EndWith = value;
				else if (key == "LimitIn") limit.ValidValues = Utils::SplitString(value, ',');
				else if (key == "CaseSensitive") limit.CaseSensitive = (value == "true");
				break;
			}
			case ParseState::InLists: {
				auto& list = ts.Lists[currentMainCategory];
				if (key == "Type") list.ElementType = value;
				else if (key == "Range") {
					auto parts = Utils::SplitString(value, ',');
					if (parts.size() == 2) {
						list.MinLength = std::stoi(parts[0]);
						list.MaxLength = std::stoi(parts[1]);
					}
				}
				break;
			}
			default: break;
			}
		}
		else {
			switch (state) {
			case ParseState::InSections:
				if (currentSection == "Sections") ts.Sections[line] = {};
				break;
			case ParseState::InNumberLimits:
				ts.NumberLimits[line] = {};
				break;
			case ParseState::InLimits:
				ts.StringLimits[line] = {};
				break;
			case ParseState::InLists:
				ts.Lists[line] = {};
				break;
			default: break;
			}
		}
	}
	return ts;
}