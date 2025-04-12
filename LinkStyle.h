// LinkStyle.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>

struct LinkStyleInfo {
	std::string Identifier;  // 唯一标识符
	std::string DisplayName; // 显示名称
	ImColor       Color;       // 基础颜色
	ImU32       HighlightColor; // 高亮颜色
	float       Thickness;   // 线宽
	bool        IsUserDefined = false;

	// 序列化支持
	template<class Archive>
	void serialize(Archive& archive) {
		archive(Identifier, DisplayName, Color, HighlightColor,
				Thickness, IsUserDefined);
	}
};

class LinkStyleManager {
public:
	static LinkStyleManager& Get() {
		static LinkStyleManager instance;
		return instance;
	}

	LinkStyleManager();

	static void Menu();
	static const char* GetDefaultIdentifier();
	static const char* GetDefaultDisplayName();
	const LinkStyleInfo* FindType(const std::string& identifier) const;
	const std::vector<LinkStyleInfo>& GetAllTypes() const { return m_Types; }

	void AddCustomType(const LinkStyleInfo& type);
	void RemoveCustomType(const std::string& identifier);

	bool LoadFromFile(const std::string& path);
	bool SaveToFile(const std::string& path);

private:
	std::vector<LinkStyleInfo> m_Types;
	std::unordered_map<std::string, size_t> m_TypeIndex;
};