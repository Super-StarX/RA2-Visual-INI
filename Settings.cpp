// Settings.cpp
#include "settings.h"
#include <fstream>
#include <filesystem> // 可选，用于文件检查
#include <limits>     // For numeric_limits
#include <sstream>    // For std::stringstream
#include <iomanip>    // For std::fixed and std::setprecision

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
		// 注意：bool 和 vector<string> 由特化/特定函数处理

		if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
			// 处理整数和浮点数
			std::stringstream ss(valueStr);
			T result{}; // Zero-initialize
			ss >> result;

			// 严格检查：转换是否成功，以及是否消耗了所有非空白字符
			if (ss.fail()) {
				throw std::invalid_argument("Invalid numeric format for key '" + key + "'");
			}
			// 检查是否有剩余的非空白字符
			std::string remaining;
			ss >> remaining;
			if (!trim(remaining).empty()) {
				throw std::invalid_argument("Extra characters after numeric value for key '" + key + "'");
			}
			// (可选) 范围检查
			// if (result < std::numeric_limits<T>::lowest() || result > std::numeric_limits<T>::max()) {
			//     throw std::out_of_range("Value out of range for type for key '" + key + "'");
			// }
			return result;

		}
		else if constexpr (std::is_same_v<T, std::string>) {
			return valueStr; // 字符串直接返回

		}
		else {
			// 静态断言捕获编译时不支持的类型
			static_assert(!std::is_same_v<T, T>, "Unsupported setting type encountered in generic parseValue. Handle with specialization or specific function.");
			// 或者在运行时抛出异常（如果静态断言不适用）
			// throw std::logic_error("Unsupported setting type for key '" + key + "' in generic parseValue");
		}
	}

	// bool 特化定义
	template<>
	bool parseValue<bool>(const std::string& valueStr, const std::string& key) {
		std::string lowerVal = valueStr;
		// 转换为小写方便比较
		std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(),
						[](unsigned char c) { return std::tolower(c); });

		if (lowerVal == "true" || lowerVal == "1" || lowerVal == "yes" || lowerVal == "on") {
			return true;
		}
		else if (lowerVal == "false" || lowerVal == "0" || lowerVal == "no" || lowerVal == "off") {
			return false;
		}
		else {
			// 无法识别的布尔值，抛出异常
			throw std::invalid_argument("Unrecognized boolean value for key '" + key + "'");
		}
	}

	// --- 特殊类型解析函数定义 ---

	// vector<string> 解析函数定义 (具有外部链接)
	std::vector<std::string> parseStringVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<std::string> result;
		if (valueStr.empty()) { // 处理空字符串输入
			return result;
		}
		std::stringstream ss(valueStr);
		std::string item;
		while (std::getline(ss, item, delimiter)) {
			result.push_back(trim(item)); // 去除每个元素的首尾空白
		}
		// 检查流状态不是必须的，因为 getline 到达末尾是正常情况
		return result;
	}

	// vector<int> 解析函数定义 (如果需要)
	std::vector<int> parseIntVector(const std::string& valueStr, const std::string& key, char delimiter) {
		std::vector<int> result;
		if (valueStr.empty()) {
			return result;
		}
		std::stringstream ss(valueStr);
		std::string itemStr;
		while (std::getline(ss, itemStr, delimiter)) {
			std::string trimmedItem = trim(itemStr);
			if (!trimmedItem.empty()) { // 跳过由连续分隔符产生的空项
				try {
					// 复用单值的解析逻辑（或直接转换）
					result.push_back(parseValue<int>(trimmedItem, key + "[vector item]"));
				}
				catch (const std::exception& e) {
					// 更具体的错误报告
					throw std::invalid_argument("Invalid integer format within vector for key '" + key + "': " + trimmedItem);
				}
			}
		}
		return result;
	}


	// --- 显式模板实例化 (如果需要确保某些类型被编译进库) ---
	// 通常对于头文件中定义的内联模板（如 Registrar 构造函数）不需要显式实例化
	// 但对于 parseValue，如果希望确保特定类型的代码生成在库中，可以这样做：
	// template int parseValue<int>(const std::string&, const std::string&);
	// template double parseValue<double>(const std::string&, const std::string&);
	// template std::string parseValue<std::string>(const std::string&, const std::string&);


} // namespace SettingsDetail

std::map<std::string, Settings::SettingValueGetter> Settings::registeredSettingsForSave;

bool Settings::load(const std::string& filename) {
	IniFile iniFile(filename);
	if (!std::filesystem::exists(filename)) {
		std::cerr << "Info: Settings file not found or could not be opened: " << filename
			<< ". Using default settings." << std::endl;
		return false;
	}

	auto& registry = SettingsDetail::getRegistry();
	for (const auto& pair : registry) {
		const std::string& key = pair.first;
		if (iniFile.sections.contains("")) {
			if (iniFile.sections.at("").contains(key)) {
				try {
					pair.second(iniFile.sections.at("").at(key));
					std::cout << "Applying setting from file: " << key << " = \"" << static_cast<std::string>(iniFile.sections.at("").at(key)) << "\"" << std::endl;
				}
				catch (const std::exception& e) {
					std::cerr << "Warning: Failed to apply value for key '" << key << "'. Reason: " << e.what() << std::endl;
				}
			}
		}
		else {
			std::cerr << "Warning: No default section in ini file." << std::endl;
		}
	}

	std::cout << "Settings loading process completed for " << filename << "." << std::endl;
	return true;
}

bool Settings::save(const std::string& filename) {
	IniFile iniFile;
	for (const auto& pair : registeredSettingsForSave) {
		iniFile.sections[""][pair.first] = Value{ pair.second() }; // 显式创建 Value 对象
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