#include "TemplateManager.h"
#include <fstream>
#include <sstream>

void TemplateManager::LoadTemplates(const std::string& iniPath) {
	m_Templates.clear();

	std::ifstream file(iniPath);
	std::string line;
	TemplateSection* currentSection = nullptr;

	while (std::getline(file, line)) {
		// 清理行内容
		line.erase(0, line.find_first_not_of(" \t")); // 去除前导空白
		line.erase(line.find_last_not_of(" \t\r\n") + 1); // 去除尾部空白

		if (line.empty()) continue; // 跳过空行

		if (line[0] == '[') { // 解析Section
			auto endPos = line.find(']');
			if (endPos != std::string::npos) {
				TemplateSection newSection;
				newSection.Name = line.substr(1, endPos - 1);
				m_Templates.push_back(newSection);
				currentSection = &m_Templates.back();
			}
		}
		else if (currentSection) { // 解析键值对
			auto eqPos = line.find('=');
			if (eqPos != std::string::npos) {
				TemplateSection::KeyValue kv;
				kv.IsComment = kv.Key.find(";") == 0;
				kv.IsInherited = kv.Key.find("@") == 0;
				kv.IsFolded = kv.Key.find("#") == 0;
				line.erase(0, line.find_first_not_of(";@#")); // 去除标识符
				eqPos = line.find('='); // 去除之后再找一遍
				kv.Key = line.substr(0, eqPos);
				kv.Value = line.substr(eqPos + 1);
				currentSection->KeyValues.push_back(kv);
			}
		}
	}
}

void TemplateManager::ShowCreationMenu(NodeCreator creator) {
	for (const auto& section : m_Templates) {
		if (ImGui::MenuItem(section.Name.c_str())) {
			const ImVec2 spawnPos = ImGui::GetMousePos();
			creator(section.Name, section.KeyValues, spawnPos);
		}
	}
}