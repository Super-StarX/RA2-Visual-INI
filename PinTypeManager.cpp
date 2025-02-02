#include "PinTypeManager.h"
#include <fstream>
#include <sstream>

const PinTypeInfo* PinTypeManager::FindType(const std::string& identifier) const {
	auto it = m_TypeIndex.find(identifier);
	if (it != m_TypeIndex.end() && it->second < m_Types.size())
		return &m_Types[it->second];
	return nullptr;
}

void PinTypeManager::RemoveCustomType(const std::string& identifier) {
	auto it = m_TypeIndex.find(identifier);
	if (it == m_TypeIndex.end())
		return;

	// 确保只删除用户自定义类型
	if (!m_Types[it->second].IsUserDefined)
		return;

	// 从vector中移除
	m_Types.erase(m_Types.begin() + it->second);

	// 重建索引
	m_TypeIndex.clear();
	for (size_t i = 0; i < m_Types.size(); ++i) {
		m_TypeIndex[m_Types[i].Identifier] = i;
	}
}

bool PinTypeManager::LoadFromFile(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open())
		return false;

	try {
		std::string content((std::istreambuf_iterator<char>(file)),
						   std::istreambuf_iterator<char>());

		/*value json;
		if (!parser::parse(content, json))
			return false;

		auto& types = json["Types"].get<array>();
		for (auto& typeVal : types) {
			auto& typeObj = typeVal.get<object>();

			PinTypeInfo info;
			info.Identifier = typeObj["Identifier"].get<string>();
			info.DisplayName = typeObj["DisplayName"].get<string>();

			auto& color = typeObj["Color"].get<array>();
			info.Color = ImColor(
				static_cast<float>(color[0].get<double>()),
				static_cast<float>(color[1].get<double>()),
				static_cast<float>(color[2].get<double>()),
				static_cast<float>(color[3].get<double>())
			);

			info.IconType = static_cast<int>(typeObj["IconType"].get<double>());
			info.IsUserDefined = typeObj["IsUserDefined"].get<boolean>();

			// 替换已存在的自定义类型
			if (info.IsUserDefined) {
				auto existing = m_TypeIndex.find(info.Identifier);
				if (existing != m_TypeIndex.end() &&
					m_Types[existing->second].IsUserDefined) {
					m_Types[existing->second] = info;
				}
				else {
					AddCustomType(info);
				}
			}
		}*/
		return true;
	}
	catch (...) {
		return false;
	}
}

bool PinTypeManager::SaveToFile(const std::string& path) {
	/*value json(object_val);
	array typesArr;

	for (const auto& type : m_Types) {
		if (!type.IsUserDefined)
			continue;

		value typeObj(object_val);
		typeObj["Identifier"] = value(type.Identifier);
		typeObj["DisplayName"] = value(type.DisplayName);

		array color;
		color.push_back(type.Color.Value.x);
		color.push_back(type.Color.Value.y);
		color.push_back(type.Color.Value.z);
		color.push_back(type.Color.Value.w);
		typeObj["Color"] = color;

		typeObj["IconType"] = value(static_cast<double>(type.IconType));
		typeObj["IsUserDefined"] = value(type.IsUserDefined);

		typesArr.push_back(typeObj);
	}

	json["Types"] = typesArr;

	std::ofstream file(path);
	if (!file.is_open())
		return false;

	file << json.serialize();*/
	return true;
}

void PinTypeManager::InitializeDefaults() {
	auto AddType = [this](const PinTypeInfo& type) {
		if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
			return;

		m_Types.push_back(type);
		m_TypeIndex[type.Identifier] = m_Types.size() - 1;
	};

	AddType({ "flow",    "Flow",    ImColor(255, 255, 255), 0 });
	AddType({ "bool",    "Boolean", ImColor(220, 48, 48),   0 });
	AddType({ "int",     "Integer", ImColor(68, 201, 156),  0 });
	AddType({ "float",   "Float",   ImColor(147, 226, 74),  0 });
	AddType({ "string",  "String",  ImColor(124, 21, 153),  0 });
	AddType({ "object",  "Object",  ImColor(51, 150, 215),  0 });
	AddType({ "function","Function",ImColor(218, 0, 183),   0 });
	AddType({ "delegate","Delegate",ImColor(255, 48, 48),   0 });
}

void PinTypeManager::AddCustomType(const PinTypeInfo& type) {
	if (m_TypeIndex.find(type.Identifier) != m_TypeIndex.end())
		return;

	m_Types.push_back(type);
	m_TypeIndex[type.Identifier] = m_Types.size() - 1;
}