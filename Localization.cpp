#include "Localization.h"
#include <fstream>
#include <nlohmann/json.hpp>

Localization& Localization::GetInstance() {
	static Localization instance;
	return instance;
}

const char* Localization::operator[](const std::string& key) const {
	auto it = m_Localization.find(key);
	if (it != m_Localization.end())
		return it->second.c_str();
	else
		return key.c_str();
}

void Localization::Init() {
	std::ifstream file("locales.json");
	nlohmann::json data = nlohmann::json::parse(file);
	m_Localization = data.get<std::unordered_map<std::string, std::string>>();
	return;
}