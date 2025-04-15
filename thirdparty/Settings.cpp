// Settings.cpp
#include "IniFile.h"
#include "Settings.h"
#include <filesystem>
#include <fstream>

// --- 内部辅助函数定义 ---
namespace SettingsDetail {

	// 辅助函数：去除字符串首尾空白
	std::string trim(const std::string& str) {
		auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
		auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
		return (start < end ? std::string(start, end) : std::string());
	}

	// --- 值解析函数定义 ---

	// 通用模板定义 (数字和字符串)
	template<typename T>
	T parseValue(const std::string& valueStr, const std::string& key) {
		if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
			std::stringstream ss(valueStr);
			T result{};
			ss >> result;
			if (ss.fail()) {
				throw std::invalid_argument("Invalid numeric format for key '" + key + "'");
			}
			std::string remaining;
			ss >> remaining;
			if (!trim(remaining).empty()) {
				throw std::invalid_argument("Extra characters after numeric value for key '" + key + "'");
			}
			return result;
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			return valueStr;
		}
		else {
			static_assert(!std::is_same_v<T, T>, "Unsupported setting type encountered in generic parseValue. Handle with specialization or specific function.");
		}
	}

	// bool 特化定义
	template<>
	bool parseValue<bool>(const std::string& valueStr, const std::string& key) {
		if (valueStr.empty()) {
			return false; // 或者抛出异常，取决于你希望如何处理空字符串
		}
		char firstCharLower = std::tolower(valueStr[0]);
		if (firstCharLower == 't' || firstCharLower == '1' || firstCharLower == 'y') {
			return true;
		}
		else if (firstCharLower == 'f' || firstCharLower == '0' || firstCharLower == 'n') {
			return false;
		}
		else {
			throw std::invalid_argument("Unrecognized boolean value for key '" + key + "': " + valueStr);
		}
	}

	// --- 特殊类型解析函数定义 ---

	// vector<string> 解析函数定义 (具有外部链接)
	std::vector<std::string> parseStringVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<std::string> result;
		if (valueStr.empty()) {
			return result;
		}
		std::stringstream ss(valueStr);
		std::string item;
		while (std::getline(ss, item, delimiter)) {
			result.push_back(trim(item));
		}
		return result;
	}

	// vector<int> 解析函数定义 (具有外部链接)
	std::vector<int> parseIntVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<int> result;
		if (valueStr.empty()) {
			return result;
		}
		std::stringstream ss(valueStr);
		std::string itemStr;
		while (std::getline(ss, itemStr, delimiter)) {
			std::string trimmedItem = trim(itemStr);
			if (!trimmedItem.empty()) {
				try {
					result.push_back(parseValue<int>(trimmedItem, key + "[vector item]"));
				}
				catch (const std::exception& e) {
					throw std::invalid_argument("Invalid integer format within vector for key '" + key + "': " + trimmedItem);
				}
			}
		}
		return result;
	}

	// vector<double> 解析函数定义 (具有外部链接)
	std::vector<double> parseDoubleVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<double> result;
		if (valueStr.empty()) {
			return result;
		}
		std::stringstream ss(valueStr);
		std::string itemStr;
		while (std::getline(ss, itemStr, delimiter)) {
			std::string trimmedItem = trim(itemStr);
			if (!trimmedItem.empty()) {
				try {
					result.push_back(parseValue<double>(trimmedItem, key + "[vector item]"));
				}
				catch (const std::exception& e) {
					throw std::invalid_argument("Invalid double format within vector for key '" + key + "': " + trimmedItem);
				}
			}
		}
		return result;
	}

	// vector<bool> 解析函数定义 (具有外部链接)
	std::vector<bool> parseBoolVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<bool> result;
		if (valueStr.empty()) {
			return result;
		}
		std::stringstream ss(valueStr);
		std::string itemStr;
		while (std::getline(ss, itemStr, delimiter)) {
			std::string trimmedItem = trim(itemStr);
			if (!trimmedItem.empty()) {
				if (trimmedItem.empty()) {
					result.push_back(false); // 或者根据你的需求处理空项
					continue;
				}
				char firstCharLower = std::tolower(trimmedItem[0]);
				if (firstCharLower == 't' || firstCharLower == '1' || firstCharLower == 'y') {
					result.push_back(true);
				}
				else if (firstCharLower == 'f' || firstCharLower == '0' || firstCharLower == 'n') {
					result.push_back(false);
				}
				else {
					throw std::invalid_argument("Invalid boolean format within vector for key '" + key + "': " + trimmedItem);
				}
			}
		}
		return result;
	}

} // namespace SettingsDetail

std::map<std::string, std::map<std::string, Settings::SettingValueGetter>> Settings::registeredSettingsForSave;
std::string Settings::defaultSectionName = "";

bool Settings::load(const std::string& filename) {
	IniFile iniFile(filename);
	if (!std::filesystem::exists(filename)) {
		std::cerr << "Info: Settings file not found or could not be opened: " << filename
			<< ". Using default settings." << std::endl;
		return false;
	}

	auto& registry = SettingsDetail::getRegistry();
	for (const auto& sectionPair : registry) {
		const std::string& sectionName = sectionPair.first;
		if (iniFile.sections.contains(sectionName)) {
			for (const auto& keyPair : sectionPair.second) {
				const std::string& key = keyPair.first;
				if (iniFile.sections.at(sectionName).contains(key)) {
					try {
						keyPair.second(iniFile.sections.at(sectionName).at(key));
						std::cout << "Applying setting from file: [" << sectionName << "]." << key << " = \"" << static_cast<std::string>(iniFile.sections.at(sectionName).at(key)) << "\"" << std::endl;
					}
					catch (const std::exception& e) {
						std::cerr << "Warning: Failed to apply value for key '" << key << "' in section [" << sectionName << "]. Reason: " << e.what() << std::endl;
					}
				}
			}
		}
		else {
			std::cerr << "Warning: Section [" << sectionName << "] not found in ini file." << std::endl;
		}
	}

	std::cout << "Settings loading process completed for " << filename << "." << std::endl;
	return true;
}

bool Settings::save(const std::string& filename) {
	IniFile iniFile;
	for (const auto& sectionPair : registeredSettingsForSave) {
		for (const auto& keyPair : sectionPair.second) {
			iniFile.sections[sectionPair.first][keyPair.first] = Value{ keyPair.second() }; // 显式创建 Value 对象
		}
	}

	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open settings file for writing: " << filename << std::endl;
		return false;
	}

	for (const auto& sectionPair : iniFile.sections) {
		if (!sectionPair.first.empty()) {
			file << "[" << sectionPair.first << "]" << std::endl;
		}
		for (const auto& keyValuePair : sectionPair.second.section) {
			file << keyValuePair.first << " = " << keyValuePair.second << std::endl;
		}
		file << std::endl;
	}

	file.close();
	std::cout << "Settings saved to " << filename << std::endl;
	return true;
}