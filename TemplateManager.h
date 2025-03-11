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
		bool IsComment = false;
		bool IsFolded = false;
	};

	std::string Name{};
	std::string Type{};
	ImColor Color{};
	bool IsFolded{ false };
	bool IsComment{ false };
	std::vector<KeyValue> KeyValues;
};

class TemplateManager {
public:
	using NodeCreator = std::function<void(
		const TemplateSection& templa,
		ImVec2 position
	)>;

	void LoadTemplates(const std::string& iniPath);
	void ShowCreationMenu(NodeCreator creator);

private:
	std::vector<TemplateSection> m_Templates;
};