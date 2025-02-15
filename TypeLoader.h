// TypeLoader.h
#pragma once
#include "IniFile.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <climits>
#include <variant>

// 类型分类枚举
enum class TypeCategory {
	Unknown,
	Basic,
	Section,
	NumberLimit,
	StringLimit,
	List
};

// 类型定义存储
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

// 完整类型信息结构
struct TypeInfo {
	TypeCategory Category{ TypeCategory::Unknown };
	std::string TypeName;
	std::variant<NumberLimit, StringLimit, ListType> Data;

	TypeInfo() : Category(TypeCategory::Unknown), TypeName("") {}
	~TypeInfo() = default;

	TypeInfo(const TypeInfo& other)
		: Category(other.Category), TypeName(other.TypeName), Data(other.Data) {
	}
};

class TypeSystem {
public:
	enum class ParseState {
		Global,
		InSections,
		InNumberLimits,
		InLimits,
		InLists
	};
	static TypeSystem& Get();
	TypeInfo GetTypeInfo(const std::string& typeName) const;
	TypeInfo GetKeyType(const std::string& sectionType, const std::string& key) const;
	void LoadFromINI(const std::string& path);

	std::unordered_set<std::string> BasicTypes = { "string", "int", "float", "bool" };
	std::unordered_map<std::string, Section> Sections;
	std::unordered_map<std::string, NumberLimit> NumberLimits;
	std::unordered_map<std::string, StringLimit> StringLimits;
	std::unordered_map<std::string, ListType> Lists;
};
