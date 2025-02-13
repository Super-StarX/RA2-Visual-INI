// TypeLoader.h
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <climits>

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
struct SectionType {
	std::unordered_map<std::string, std::string> KeyTypes;
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

// 完整类型信息结构
struct TypeInfo {
	TypeCategory Category = TypeCategory::Unknown;
	std::string TypeName;

	union {
		NumberLimit NumberLimit;
		StringLimit StringLimit;
		ListType ListType;
	};

	TypeInfo() = default;
	~TypeInfo();

	TypeInfo(const TypeInfo& other) {
		Category = other.Category;
		TypeName = other.TypeName;
		switch (Category) {
		case TypeCategory::NumberLimit:
			NumberLimit = other.NumberLimit; break;
		case TypeCategory::StringLimit:
			StringLimit = other.StringLimit; break;
		case TypeCategory::List:
			ListType = other.ListType; break;
		default: break;
		}
	}
};

class TypeSystem {
public:

	static TypeSystem& Get();
	TypeInfo GetTypeInfo(const std::string& typeName) const;
	TypeInfo GetKeyType(const std::string& sectionType, const std::string& key) const;

private:
	std::unordered_set<std::string> BasicTypes = { "string", "int", "float", "bool" };
	std::unordered_map<std::string, SectionType> Sections;
	std::unordered_map<std::string, NumberLimit> NumberLimits;
	std::unordered_map<std::string, StringLimit> StringLimits;
	std::unordered_map<std::string, ListType> Lists;

	friend class TypeLoader;
};

class TypeLoader {
public:
	static TypeSystem LoadFromINI(const std::string& path);
	static std::vector<std::string> SplitString(const std::string& s, char delimiter);

private:
	enum class ParseState {
		Global,
		InSections,
		InNumberLimits,
		InLimits,
		InLists
	};
};