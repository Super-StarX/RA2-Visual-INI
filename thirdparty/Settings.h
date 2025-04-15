
/**
 * @file Settings.h By 小星星
 * @brief 一个简单的头文件配置管理库。
 *
 * 该库提供了一种方便的方式来管理应用程序的配置设置。
 * 它允许你声明各种类型的设置，并自动处理从 INI 文件加载和保存这些设置。
 *
 * **主要特点:**
 * - 使用宏 `DECLARE_SETTING` 声明配置项，减少重复代码。
 * - 自动处理多种数据类型：`int`, `double`, `std::string`, `bool`, `std::vector<int>`, `std::vector<std::string>`.
 * - 使用 `IniFile.h` 库进行 INI 文件的读写操作。
 * - 提供 `load` 方法从文件加载配置。
 * - 提供 `save` 方法将当前配置保存到文件。
 *
 * **如何使用:**
 * 1. 在你的项目中包含 `Settings.h` 和 `IniFile.h` 文件。
 * 2. 创建一个类（例如 `MyAppSettings`）并继承自 `Settings`。
 * 3. 在你的类中使用 `DECLARE_SETTING(Type, Name, DefaultValue)` 宏来声明你的配置项。
 * - `Type`: 配置项的数据类型（例如 `int`, `std::string`）。
 * - `Name`: 配置项的名称，这将用作 INI 文件中的键。
 * - `DefaultValue`: 配置项的默认值。
 * 4. 在你的应用程序启动时，调用 `YourSettingsClass::load(filename)` 来加载配置（默认文件名为 "settings.ini"）。
 * 5. 通过 `YourSettingsClass::Name` 访问和修改配置项（例如 `MyAppSettings::a = 20;`）。
 * 6. 在你的应用程序退出时或需要保存配置时，调用 `YourSettingsClass::save(filename)` 来保存配置（默认文件名为 "settings.ini"）。
 *
 * **支持的数据类型:**
 * - `int`
 * - `double`
 * - `std::string`
 * - `bool` (解析 "true", "false", "1", "0", "yes", "no", "on", "off", 忽略大小写)
 * - `std::vector<int>` (元素之间使用 ',' 分隔)
 * - `std::vector<std::string>` (元素之间使用 ',' 分隔)
 *
 * **配置文件格式 (INI):**
 * 配置文件是标准的 INI 格式。配置项以 `key = value` 的形式存在于文件中。
 * 注释行以 `#` 或 `;` 开头。
 * 默认情况下，配置项位于文件的根部分（没有节）。
 *
 * **示例:**
 *	```cpp
 *	// MyAppSettings.h
 *	#pragma once
 *	#include "Settings.h"
 *
 * 	class MySettings : public Settings {
 * 	public:
 * 		// --- 使用宏来定义你的设置项 ---
 * 		// 只需在这里添加/修改/删除行即可
 * 		DECLARE_SETTING(int, windowWidth, 1024);
 * 		DECLARE_SETTING(int, windowHeight, 768);
 * 		DECLARE_SETTING(std::string, windowTitle, "My Application");
 * 		DECLARE_SETTING(std::string, defaultUsername, "guest");
 * 		DECLARE_SETTING(bool, enableAutoLogin, false);
 * 		DECLARE_SETTING(double, networkTimeoutSeconds, 15.0);
 *
 * 		DECLARE_SETTING(std::vector<std::string>, pluginSearchPaths, { "/usr/lib/plugins", "./plugins" });
 * 		DECLARE_SETTING(std::vector<int>, initialEnemyIDs, { 101, 205, 300 });
 * 	};
 *
 *	// main.cpp
 *	#include "MyAppSettings.h"
 *	#include <iostream>
 *
 *	int main() {
 * 		MyAppSettings::load();
 * 		std::cout << "Window Width: " << MyAppSettings::windowWidth << std::endl;
 *		MyAppSettings::windowWidth = 1280;
 *		MyAppSettings::save();
 *		return 0;
 *	}
 *	```
 */

// Settings.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <iostream>
#include <type_traits>
#include <iomanip>

class Settings; // 前向声明 Settings 类

// --- 内部实现细节 ---
namespace SettingsDetail {

	// 类型：存储配置项名称到其更新函数的映射
	using SettingUpdater = std::function<void(const std::string&)>;
	using RegistryMap = std::map<std::string, SettingUpdater>;

	// 获取注册表的静态函数
	inline RegistryMap& getRegistry() {
		// Function-local static ensures proper initialization order
		static RegistryMap registry;
		return registry;
	}

	// --- 值解析函数声明 ---
	// (实现将在 settings.cpp 中)

	// 通用模板声明
	template<typename T>
	T parseValue(const std::string& valueStr, const std::string& key);

	// bool 特化声明
	template<>
	bool parseValue<bool>(const std::string& valueStr, const std::string& key);

	// --- 注册辅助类 (定义保留在头文件以便内联实例化) ---
	template<typename T>
	struct SettingRegistrar {
	private:
		// 保存值的辅助函数
		static std::string saveValue(const T& variable) {
			std::stringstream ss;
			if constexpr (std::is_same_v<T, std::string>) {
				ss << variable;
			}
			else if constexpr (std::is_arithmetic_v<T>) {
				ss << variable;
			}
			else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				for (size_t i = 0; i < variable.size(); ++i) {
					ss << variable[i];
					if (i < variable.size() - 1) {
						ss << ";";
					}
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<int>>) {
				for (size_t i = 0; i < variable.size(); ++i) {
					ss << variable[i];
					if (i < variable.size() - 1) {
						ss << ";";
					}
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<double>>) {
				for (size_t i = 0; i < variable.size(); ++i) {
					ss << std::fixed << std::setprecision(6) << variable[i];
					if (i < variable.size() - 1) {
						ss << ";";
					}
				}
			}
			else if constexpr (std::is_same_v<T, std::vector<bool>>) {
				for (size_t i = 0; i < variable.size(); ++i) {
					ss << (variable[i] ? "true" : "false");
					if (i < variable.size() - 1) {
						ss << ";";
					}
				}
			}
			else {
				static_assert(!std::is_same_v<T, T>, "Unsupported type for saving.");
			}
			return ss.str();
		}

		// 加载值的辅助函数
		static void loadValue(T& variable, const std::string& key, const std::string& valueStr) {
			try {
				if constexpr (std::is_same_v<T, std::vector<std::string>>) {
					extern std::vector<std::string> parseStringVector(const std::string & valueStr, const std::string & key, char delimiter);
					variable = parseStringVector(valueStr, key, ',');
				}
				else if constexpr (std::is_same_v<T, std::vector<int>>) {
					extern std::vector<int> parseIntVector(const std::string & valueStr, const std::string & key, char delimiter);
					variable = parseIntVector(valueStr, key, ',');
				}
				else if constexpr (std::is_same_v<T, std::vector<double>>) {
					extern std::vector<double> parseDoubleVector(const std::string & valueStr, const std::string & key, char delimiter);
					variable = parseDoubleVector(valueStr, key, ',');
				}
				else if constexpr (std::is_same_v<T, std::vector<bool>>) {
					extern std::vector<bool> parseBoolVector(const std::string & valueStr, const std::string & key, char delimiter);
					variable = parseBoolVector(valueStr, key, ',');
				}
				else {
					variable = SettingsDetail::parseValue<T>(valueStr, key);
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Warning: Failed to apply value '" << valueStr
					<< "' for key '" << key << "'. Reason: " << e.what() << std::endl;
			}
		}

	public:
		// 构造函数：使用默认键
		SettingRegistrar(const std::string& defaultKey, T& variable) {
			std::cout << "Registering setting (default key): " << defaultKey << std::endl;
			Settings::registerSetting(defaultKey, [&variable]() { return saveValue(variable); });
			getRegistry()[defaultKey] = [&](const std::string& valueStr) { loadValue(variable, defaultKey, valueStr); };
		}

		// 构造函数：使用自定义键
		SettingRegistrar(const std::string& defaultKey, T& variable, const std::string& customKey) {
			std::cout << "Registering setting (custom key): " << customKey << std::endl;
			Settings::registerSetting(customKey, [&variable]() { return saveValue(variable); });
			getRegistry()[customKey] = [&](const std::string& valueStr) { loadValue(variable, customKey, valueStr); };
		}
	};

} // namespace SettingsDetail


// --- 主 Settings 类 ---

// 宏定义：处理不带自定义键的情况
#define DECLARE_SETTING(Type, Name, DefaultValue) \
	public: \
		inline static Type Name = DefaultValue; \
	private: \
		inline static ::SettingsDetail::SettingRegistrar<Type> reg_##Name{ #Name, Name }

// 宏定义：处理带自定义键的情况
#define DECLARE_SETTING(Type, Name, DefaultValue, CustomKey) \
	public: \
		inline static Type Name = DefaultValue; \
	private: \
		inline static ::SettingsDetail::SettingRegistrar<Type> reg_##Name{ #Name, Name, CustomKey }

class Settings {
public: // 确保宏展开后成员是 public
	// --- 公共静态方法 ---
	/**
	 * @brief 从指定文件加载设置。
	 * @param filename 配置文件的路径。默认为 "Settings.ini"。
	 * @return 如果加载成功（文件存在且可读）返回 true，否则返回 false。
	 */
	static bool load(const std::string& filename = "Settings.ini");

	/**
	 * @brief 将当前设置保存到指定文件。
	 * @param filename 配置文件的路径。默认为 "Settings.ini"。
	 * @return 如果保存成功返回 true，否则返回 false。
	 */
	static bool save(const std::string& filename = "Settings.ini");

private:
	using SettingValueGetter = std::function<std::string()>;
	static std::map<std::string, SettingValueGetter> registeredSettingsForSave;

	static void registerSetting(const std::string& key, SettingValueGetter getter) {
		registeredSettingsForSave[key] = getter;
	}

	// 声明 SettingRegistrar 为友元类
	template <typename T> friend struct SettingsDetail::SettingRegistrar;
};

// --- 为 vector<int> 添加外部解析函数声明 (如果需要) ---
namespace SettingsDetail {
	extern std::vector<int> parseIntVector(const std::string& valueStr, const std::string& key, char delimiter);
	extern std::vector<std::string> parseStringVector(const std::string& valueStr, const std::string& key, char delimiter);
	extern std::vector<double> parseDoubleVector(const std::string& valueStr, const std::string& key, char delimiter);
	extern std::vector<bool> parseBoolVector(const std::string& valueStr, const std::string& key, char delimiter);

} // namespace SettingsDetail