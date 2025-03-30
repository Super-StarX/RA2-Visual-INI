#include "TemplateManager.h"
#include "Utils.h"
#include "Log.h"
#include <filesystem>
#include <fstream>
#include <sstream>

// 递归构建菜单树结构
void TemplateManager::LoadTemplates(const std::string& folderPath) {
	m_Root = TemplateItem{};
	LoadFolder(folderPath, m_Root);
}

void TemplateManager::LoadFolder(const std::filesystem::path& path, TemplateItem& parent) {
	namespace fs = std::filesystem;
	try {
		for (const auto& entry : fs::directory_iterator(path)) {
			try {
				if (entry.is_directory()) {
					// 处理子目录
					TemplateItem folderItem{ .name = entry.path().filename().string() };
					parent.children.push_back(folderItem);
					LoadFolder(entry.path(), parent.children.back());
				}
				else if (entry.path().extension() == ".ini") {
					// 处理 .ini 文件
					ParseIniFile(entry.path(), parent);
				}
			}
			catch (const fs::filesystem_error& e) {
				LOG_ERROR("处理条目 {} 时出错: {}", entry.path().string(), e.what());
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		LOG_ERROR("读取目录 {} 时出错: {}", path.string(), e.what());
	}
}

void TemplateManager::ParseIniFile(const std::filesystem::path& filePath, TemplateItem& parent) {
	std::ifstream file(filePath);
	std::string line;
	TemplateSection* currentSection = nullptr;

	TemplateItem fileItem;
	fileItem.name = filePath.stem().string(); // 使用文件名作为父节点

	while (std::getline(file, line)) {
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1);

		if (line.empty()) continue;

		if (line[0] == '[') {
			auto endPos = line.find(']');
			if (endPos != std::string::npos) {
				TemplateSection newSection;
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
					if (key == "Color") {
						std::istringstream iss(value);
						int r, g, b;
						iss >> r >> g >> b;
						currentSection->Color = ImColor(r, g, b);
					}
					else if (key == "Type")
						currentSection->Type = value;
					else if (key == "Folded")
						currentSection->IsFolded = value == "true";
					else if (key == "Comment")
						currentSection->IsComment = value == "true";
				}
				else {
					TemplateSection::KeyValue kv;
					kv.IsComment = kv.Key.find(";") == 0;
					kv.IsInherited = kv.Key.find("@") == 0;
					kv.IsFolded = kv.Key.find("#") == 0;
					line.erase(0, line.find_first_not_of(";@#"));
					eqPos = line.find('=');
					kv.Key = line.substr(0, eqPos);
					kv.Value = line.substr(eqPos + 1);
					currentSection->KeyValues.push_back(kv);
				}
			}
		}
	}

	if (!fileItem.sections.empty()) {
		parent.children.push_back(fileItem);
	}
}

void TemplateManager::ShowCreationMenu(NodeCreator creator) {
	ShowMenuRecursive(m_Root, creator);
}

void TemplateManager::ShowMenuRecursive(const TemplateItem& item, NodeCreator creator) {
	for (const auto& child : item.children) {
		if (!child.sections.empty()) {
			// 叶子节点（包含section）
			for (const auto& section : child.sections) {
				if (ImGui::MenuItem(section.Name.c_str())) {
					creator(section, ImGui::GetMousePos());
				}
			}
		}
		else {
			// 文件夹节点
			if (ImGui::BeginMenu(child.name.c_str())) {
				ShowMenuRecursive(child, creator);
				ImGui::EndMenu();
			}
		}
	}
}