#pragma once
#include "utilities/builders.h"

#include <unordered_map>
#include <string>

struct PinStyleInfo {
	std::string Identifier;  // 唯一标识符
	std::string DisplayName; // 显示名称
	ImColor     Color;       // 显示颜色
	int         IconType;    // 图标类型（0=圆形，1=方形...）
	std::string LinkType = "default";
	bool        IsUserDefined = false; // 是否用户自定义类型

	// 序列化支持
	template<class Archive>
	void serialize(Archive& archive) {
		archive(Identifier, DisplayName, Color, IconType, IsUserDefined);
	}
};

class PinStyleManager {
public:
	static PinStyleManager& Get() {
		static PinStyleManager instance;
		return instance;
	}
	static void Menu();

	PinStyleManager();
	bool LoadFromFile(const std::string& path);
	bool SaveToFile(const std::string& path);

	const PinStyleInfo* FindType(const std::string& identifier) const;
	const std::vector<PinStyleInfo>& GetAllTypes() const { return m_Types; }

	void AddCustomType(const PinStyleInfo& type);
	void RemoveCustomType(const std::string& identifier);

private:
	std::vector<PinStyleInfo> m_Types;
	std::unordered_map<std::string, size_t> m_TypeIndex;
};