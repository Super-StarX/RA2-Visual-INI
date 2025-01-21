#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

class Value {
public:
	operator std::string() const { return value; }
	std::string operator()() const { return value; }

	friend std::ostream& operator<<(std::ostream& os, const Value& v) {
		os << v.value;
		return os;
	}

	std::string getFileName() const;

	std::string value{ };
	int line{ -1 };
	std::string origin{ };
	size_t fileIndex{ };
	std::string filetype{ };
	bool isInheritance{ };
};

template<>
struct std::formatter<Value> : std::formatter<std::string> {
	auto format(const Value& v, std::format_context& ctx) const {
		return std::formatter<std::string>::format(v(), ctx);
	}
};

class Section {
public:
	using Key = std::string;

	auto begin() const { return section.begin(); }
	auto end() const { return section.end(); }
	void insert(const Section& other) { return section.insert(other.begin(), other.end()); }
	bool contains(const std::string& key) const { return section.contains(key); }
	Value at(const std::string& key) const { return section.at(key); }
	Value& at(const std::string& key) { return section.at(key); }
	Value& operator[](const std::string& key) { return section[key]; }

	std::string name{ };
	int line{ -1 };
	std::string origin{ };
	size_t fileIndex{ };
	bool isScanned{ };
	int inheritanceLevel{ };
	std::unordered_map<Key, Value> section;
};

class IniFile {
public:
	using Sections = std::unordered_map<std::string, Section>;

	static std::string GetFileName(size_t index);
	static size_t GetFileIndex();
	static std::vector<std::string> FileNames;
	static size_t FileIndex;
	static std::string FileType;

	IniFile();
	IniFile(const std::string& filepath, bool isConfig = false);

	void load(const std::string& filepath, bool isInclude = false);
	void readSection(std::string& currentSection, std::string& line, std::string origin, int& lineNumber);
	void readKeyValue(std::string& currentSection, std::string& line, std::string origin, int lineNumber);

	bool isConfig{ false };
	Sections sections;
private:
	void processIncludes(const std::string& basePath);
	void processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& currentSection);
};