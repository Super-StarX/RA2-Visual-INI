#pragma once
#include "utilities/widgets.h"
#include <vector>
#include <string>
#include <functional>
#include <filesystem>

struct TemplateSection {
	struct KeyValue {
		std::string Key;
		std::string Value;
		bool IsInherited = false;
		bool IsComment = false;
		bool IsFolded = false;
	};

	std::string Name{};
	std::string Type{};
	std::string Style{};
	bool IsFolded{ false };
	bool IsComment{ false };
	std::vector<KeyValue> KeyValues;
};

// 新增模板项结构体
struct TemplateItem {
	std::string name;
	std::vector<TemplateSection> sections; // 非空表示叶子节点
	std::vector<TemplateItem> children;    // 非空表示文件夹节点
};

class TemplateManager {
public:
	using NodeCreator = std::function<void(
		const TemplateSection& templa,
		ImVec2 position
	)>;

	TemplateManager() : m_Root() {}
	void LoadTemplates(const std::string& folderPath);
	void ShowCreationMenu(NodeCreator creator);
	static TemplateItem ParseIniString(std::istringstream& file);

private:
	TemplateItem m_Root;

	void LoadFolder(const std::filesystem::path& path, TemplateItem& parent);
	void ParseIniFile(const std::filesystem::path& filePath, TemplateItem& parent);
	void ShowMenuRecursive(const TemplateItem& item, NodeCreator creator);
};