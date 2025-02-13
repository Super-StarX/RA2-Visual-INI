#include "TypeLoader.h"

std::vector<std::string> TypeLoader::SplitString(const std::string& s, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) tokens.push_back(token);
	}
	return tokens;
}

TypeSystem TypeLoader::LoadFromINI(const std::string& path) {
	TypeSystem ts;
	std::ifstream file(path);
	std::string line;
	std::string currentSection;
	ParseState state = ParseState::Global;
	std::string currentMainCategory;

	while (std::getline(file, line)) {
		// 移除前后空格和注释
		line = line.substr(0, line.find(';'));
		line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
		if (line.empty()) continue;

		// 解析段头
		if (line[0] == '[') {
			currentSection = line.substr(1, line.find(']') - 1);

			// 确定主分类状态
			if (currentSection == "Sections")
				state = ParseState::InSections;
			else if (currentSection == "NumberLimits")
				state = ParseState::InNumberLimits;
			else if (currentSection == "Limits")
				state = ParseState::InLimits;
			else if (currentSection == "Lists")
				state = ParseState::InLists;
			else {
				// 处理子段
				if (state == ParseState::InSections) {
					// 记录Sections类型
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

		// 解析键值对
		size_t eqPos = line.find('=');
		if (eqPos != std::string::npos) {
			std::string key = line.substr(0, eqPos);
			std::string value = line.substr(eqPos + 1);

			switch (state) {
			case ParseState::InSections: {
				// 主[Sections]段记录类型名称
				if (currentSection == "Sections")
					ts.Sections[value] = {}; // 注册新类型
				// 子段处理键值类型
				else if (!currentMainCategory.empty())
					ts.Sections[currentMainCategory].KeyTypes[key] = value;
				break;
			}
			case ParseState::InNumberLimits: {
				if (key == "Range") {
					auto parts = SplitString(value, ',');
					if (parts.size() == 2) {
						ts.NumberLimits[currentMainCategory].Min = std::stoi(parts[0]);
						ts.NumberLimits[currentMainCategory].Max = std::stoi(parts[1]);
					}
				}
				break;
			}
			case ParseState::InLimits: {
				auto& limit = ts.StringLimits[currentMainCategory];
				if (key == "StartWith")
					limit.StartWith = value;
				else if (key == "EndWith")
					limit.EndWith = value;
				else if (key == "LimitIn")
					limit.ValidValues = SplitString(value, ',');
				else if (key == "CaseSensitive")
					limit.CaseSensitive = (value == "true");
				break;
			}
			case ParseState::InLists: {
				auto& list = ts.Lists[currentMainCategory];
				if (key == "Type")
					list.ElementType = value;
				else if (key == "Range") {
					auto parts = SplitString(value, ',');
					if (parts.size() == 2) {
						list.MinLength = std::stoi(parts[0]);
						list.MaxLength = std::stoi(parts[1]);
					}
				}
				break;
			}
			default:
				// 处理全局键值对（如果有）
				break;
			}
		}
		else {
			// 处理无键值的情况（如注册类型名）
			if (state == ParseState::InSections && currentSection == "Sections")
				ts.Sections[line] = {}; // 直接注册类型名
			else if (state == ParseState::InNumberLimits && currentSection == "NumberLimits")
				ts.NumberLimits[line] = {}; // 注册数值类型
			else if (state == ParseState::InLimits && currentSection == "Limits")
				ts.StringLimits[line] = {}; // 注册限制类型
			else if (state == ParseState::InLists && currentSection == "Lists")
				ts.Lists[line] = {}; // 注册列表类型
		}
	}
	return ts;
}
