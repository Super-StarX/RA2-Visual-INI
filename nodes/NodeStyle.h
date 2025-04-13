#pragma once
#include <string>
#include <imgui.h>
#include <unordered_map>
#include <vector>

struct NodeStyleInfo {
	std::string Identifier;     // 唯一标识
	std::string DisplayName;    // 显示名
	ImColor Color{ 0, 64, 128 }; // 节点颜色
	bool IsUserDefined = false;

	template<class Archive>
	void serialize(Archive& archive) {
		archive(Identifier, DisplayName, Color, IsUserDefined);
	}
};

class NodeStyleManager {
public:
	static NodeStyleManager& Get() {
		static NodeStyleManager instance;
		return instance;
	}
	static void Menu(); // 样式配置 UI

	const NodeStyleInfo* FindType(const std::string& identifier) const;
	const std::vector<NodeStyleInfo>& GetAllTypes() const { return m_Types; }

	void AddCustomType(const NodeStyleInfo& type);
	void RemoveCustomType(const std::string& identifier);
	bool LoadFromFile(const std::string& path);
	bool SaveToFile(const std::string& path);

private:
	std::vector<NodeStyleInfo> m_Types;
	std::unordered_map<std::string, size_t> m_Index;

	NodeStyleManager();
};
