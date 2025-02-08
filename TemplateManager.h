#pragma once
#include "utilities/widgets.h"
#include <vector>
#include <string>
#include <functional>

struct TemplateSection {
	struct KeyValue {
		std::string Key;
		std::string Value;
		bool IsInherited = false;
		bool IsHide = false;
	};

	std::string Name;
	std::vector<KeyValue> KeyValues;
};

class TemplateManager {
public:
	using NodeCreator = std::function<void(
		const std::string& sectionName,
		const std::vector<TemplateSection::KeyValue>& keyValues,
		ImVec2 position
	)>;

	void LoadTemplates(const std::string& iniPath);
	void ShowCreationMenu(NodeCreator creator);

	const auto& GetTemplates() const { return m_Templates; }

private:
	std::vector<TemplateSection> m_Templates;
};