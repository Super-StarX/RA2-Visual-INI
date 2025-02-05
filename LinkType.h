// LinkType.h
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>

struct LinkTypeInfo {
	std::string Identifier;  // 唯一标识符
	std::string DisplayName; // 显示名称
	ImU32       Color;       // 基础颜色
	ImU32       HighlightColor; // 高亮颜色
	float       Thickness;   // 线宽
	int         Style;       // 0=实线, 1=虚线
	bool        IsUserDefined = false;

	// 序列化支持
	template<class Archive>
	void serialize(Archive& archive) {
		archive(Identifier, DisplayName, Color, HighlightColor,
				Thickness, Style, IsUserDefined);
	}
};

class LinkTypeManager {
public:
	static LinkTypeManager& Get() {
		static LinkTypeManager instance;
		return instance;
	}

	void InitializeDefaults();
	bool LoadFromFile(const std::string& path);
	bool SaveToFile(const std::string& path);

	const LinkTypeInfo* FindType(const std::string& identifier) const;
	const std::vector<LinkTypeInfo>& GetAllTypes() const { return m_Types; }

	void AddCustomType(const LinkTypeInfo& type);
	void RemoveCustomType(const std::string& identifier);

private:
	std::vector<LinkTypeInfo> m_Types;
	std::unordered_map<std::string, size_t> m_TypeIndex;
};