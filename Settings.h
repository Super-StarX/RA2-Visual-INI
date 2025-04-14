
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
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include "IniFile.h"

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
		// 构造函数：将设置项注册到 RegistryMap
		// 使用可变参数模板 DefaultValuePack 处理包含逗号的默认值
		template <typename... DefaultValuePack>
		SettingRegistrar(const std::string& key, T& variable, DefaultValuePack&&... /*unused_default (value already set by DECLARE_SETTING)*/) {
			std::cout << "Registering setting: " << key << std::endl; // Debug 输出
			Settings::registerSetting(key, [&variable]() {
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
				else {
					static_assert(!std::is_same_v<T, T>, "Unsupported type for saving.");
				}
				return ss.str();
			});

			// 使用 if constexpr 根据类型选择不同的解析逻辑
			// (注意：特殊类型的解析函数声明需要在此处可见)
			if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				// 需要 Settings::parseStringVector 的声明
				// 我们将在 Settings 类中声明它
				// forward declare class Settings; // Not enough for static member
				// Requires full Settings class definition before this point OR make parseStringVector a free function in SettingsDetail
				extern std::vector<std::string> parseStringVector(const std::string & valueStr, const std::string & key, char delimiter); // Declare external linkage

				getRegistry()[key] = [&variable, key](const std::string& valueStr) {
					try {
						// 调用外部（或类静态）的特定解析函数
						variable = parseStringVector(valueStr, key, ','); // Default delimiter ','
					}
					catch (const std::exception& e) {
						std::cerr << "Warning: Failed to apply value '" << valueStr
							<< "' for key '" << key << "'. Reason: " << e.what() << std::endl;
					}
				};

			}
			else if constexpr (std::is_same_v<T, std::vector<int>>) {
				// 需要 Settings::parseIntVector 的声明
				extern std::vector<int> parseIntVector(const std::string & valueStr, const std::string & key, char delimiter); // Declare external linkage

				getRegistry()[key] = [&variable, key](const std::string& valueStr) {
					try {
						// 调用外部（或类静态）的特定解析函数
						variable = parseIntVector(valueStr, key, ','); // Default delimiter ','
					}
					catch (const std::exception& e) {
						std::cerr << "Warning: Failed to apply value '" << valueStr
							<< "' for key '" << key << "'. Reason: " << e.what() << std::endl;
					}
				};
			}
			else {
				// 基本类型的通用逻辑
				getRegistry()[key] = [&variable, key](const std::string& valueStr) {
					try {
						// 调用 SettingsDetail::parseValue 的对应版本
						variable = SettingsDetail::parseValue<T>(valueStr, key);
					}
					catch (const std::exception& e) {
						std::cerr << "Warning: Failed to apply value '" << valueStr
							<< "' for key '" << key << "'. Reason: " << e.what() << std::endl;
					}
				};
			}
		}
	};

} // namespace SettingsDetail


// --- 主 Settings 类 ---

// 宏定义：使用 Variadic Macro (__VA_ARGS__) 处理包含逗号的默认值
// 注意：## 连接符用于创建唯一的注册器变量名
#define DECLARE_SETTING(Type, Name, ...) \
	public: \
		/* 公开的内联静态成员变量，带默认值 (__VA_ARGS__ 捕获所有默认值参数) */ \
		inline static Type Name = __VA_ARGS__; \
	private: \
		/* 私有的内联静态注册器实例，在静态初始化时自动注册 */ \
		/* 使用 #Name 将变量名转换为字符串作为配置文件的键 */ \
		/* 将默认值也传递给注册器构造函数（虽然当前未使用，但保持一致性）*/ \
		inline static ::SettingsDetail::SettingRegistrar<Type> reg_##Name{ #Name, Name /*, __VA_ARGS__ */ }

// 前向声明特殊解析函数 (如果它们是 Settings 的静态成员)
// 或者在 SettingsDetail 中声明为 extern 自由函数 (如上)

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
	// --- 特殊类型解析函数的声明 (如果作为类成员) ---
	// static std::vector<std::string> parseStringVector(const std::string& valueStr, const std::string& key, char delimiter = ',');
	// static std::vector<int> parseIntVector(const std::string& valueStr, const std::string& key, char delimiter = ',');
	// 注意: 如果是类静态成员，SettingRegistrar 需要 Settings 的完整定义或通过友元访问
	// 将其设为 SettingsDetail 中的 extern 自由函数更简单（如上所做）

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

	// 如果需要支持 vector<int>，也需要在 SettingRegistrar 的 if constexpr 中添加分支
	// （为了简洁，下面的代码示例中省略了 vector<int> 的注册器修改，但原理相同）

} // namespace SettingsDetail