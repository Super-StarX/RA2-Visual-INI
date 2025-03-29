#pragma once
#include <string>
#include <unordered_map>

#define LOCALE Localization::GetInstance()

class Localization {
public:
	static Localization& GetInstance();

	const char* operator[](const std::string& key) const;
	void Init();

private:
	std::unordered_map<std::string, std::string> m_Localization;
};