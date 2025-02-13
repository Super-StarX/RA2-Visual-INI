#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct TypeSystem {
	// 类型定义存储
	struct SectionType {
		std::unordered_map<std::string, std::string> KeyTypes; // 键值类型映射
	};

	struct NumberLimit {
		int Min = INT_MIN;
		int Max = INT_MAX;
	};

	struct StringLimit {
		std::vector<std::string> ValidValues;
		std::string StartWith;
		std::string EndWith;
		bool CaseSensitive = false;
	};

	struct ListType {
		std::string ElementType;
		int MinLength = 0;
		int MaxLength = INT_MAX;
	};


	std::unordered_map<std::string, SectionType> Sections;
	std::unordered_map<std::string, NumberLimit> NumberLimits;
	std::unordered_map<std::string, StringLimit> StringLimits;
	std::unordered_map<std::string, ListType> Lists;
};

class TypeLoader {
private:
	enum class ParseState {
		Global,
		InSections,
		InNumberLimits,
		InLimits,
		InLists
	};

	static std::vector<std::string> SplitString(const std::string& s, char delimiter);

public:
	static TypeSystem LoadFromINI(const std::string& path);
};