#include "TemplateManager.h"
#include "Utils.h"
#include "Log.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
void TemplateManager::LoadTemplates(const std::string& folderPath) {
	try {
		if (!fs::exists(folderPath)) {
			LOG_ERROR("模板目录不存在: {}", folderPath);
			return;
		}

		m_Root = TemplateItem{};
		LoadFolder(folderPath, m_Root);
	}
	catch (const std::exception& e) {
		LOG_ERROR("加载模板失败: {}", e.what());
	}
}

void TemplateManager::LoadFolder(const fs::path& path, TemplateItem& parent) {
	try {
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory()) {
				TemplateItem folderItem{ .name = entry.path().filename().string() };
				parent.children.push_back(folderItem);
				LoadFolder(entry.path(), parent.children.back());
			}
			else if (entry.path().extension() == ".ini") {
				ParseIniFile(entry.path(), parent);
			}
		}
	}
	catch (const std::exception& e) {
		LOG_ERROR("读取目录 {} 时出错: {}", path.string(), e.what());
	}
}

TemplateItem TemplateManager::ParseIniString(std::istringstream& file) {
	TemplateItem fileItem;
	std::string line;
	TemplateSection* currentSection = nullptr;

	while (std::getline(file, line)) {
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1);

		if (line.empty()) continue;

		if (line[0] == '[') {
			auto endPos = line.find(']');
			if (endPos != std::string::npos) {
				TemplateSection newSection{};
				newSection.Name = line.substr(1, endPos - 1);
				fileItem.sections.push_back(newSection);
				currentSection = &fileItem.sections.back();
			}
		}
		else if (currentSection) {
			auto eqPos = line.find('=');
			if (eqPos != std::string::npos) {
				if (auto lgPos = line.find(">"); lgPos != std::string::npos) {
					auto key = line.substr(lgPos + 1, eqPos - lgPos - 1);
					key = Utils::Trim(key);
					auto value = line.substr(eqPos + 1);
					if (key == "Style")
						currentSection->Style = value;
					else if (key == "Type")
						currentSection->Type = value;
					else if (key == "Folded")
						currentSection->IsFolded = value == "true";
					else if (key == "Comment")
						currentSection->IsComment = value == "true";
				}
				else {
					TemplateSection::KeyValue kv;
					kv.IsComment = line.find(";") == 0;
					kv.IsInherited = line.find("@") == 0;
					kv.IsFolded = line.find("#") == 0;
					line.erase(0, line.find_first_not_of(";@#"));
					eqPos = line.find('=');
					kv.Key = line.substr(0, eqPos);
					kv.Value = line.substr(eqPos + 1);
					currentSection->KeyValues.push_back(kv);
				}
			}
		}
	}

	return fileItem;
}

void TemplateManager::ParseIniFile(const fs::path& filePath, TemplateItem& parent) {
	try {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			LOG_ERROR("无法打开文件: {}", filePath.string());
			return;
		}
		std::string content((std::istreambuf_iterator<char>(file)),
						 std::istreambuf_iterator<char>());
		std::istringstream iss(content);
		auto fileItem = ParseIniString(iss);
		fileItem.name = filePath.stem().string(); // 使用文件名作为显示名称

		if (!fileItem.sections.empty()) {
			parent.children.push_back(fileItem);
		}
	}
	catch (const std::exception& e) {
		LOG_ERROR("解析文件 {} 时出错: {}", filePath.string(), e.what());
	}
}

void TemplateManager::ShowCreationMenu(NodeCreator creator) {
	ShowMenuRecursive(m_Root, creator);
}

void TemplateManager::ShowMenuRecursive(const TemplateItem& item, NodeCreator creator) {
	for (const auto& child : item.children) {
		if (!child.sections.empty()) {
			if (ImGui::MenuItem(child.name.c_str())) {
				ImVec2 mousePos = ImGui::GetMousePos();
				for (const auto& section : child.sections) {
					creator(section, mousePos);
				}
			}
		}
		else {
			if (ImGui::BeginMenu(child.name.c_str())) {
				ShowMenuRecursive(child, creator);
				ImGui::EndMenu();
			}
		}
	}
}