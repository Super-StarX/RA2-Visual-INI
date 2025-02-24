#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Object {
public:
	
	Object() = default;
	virtual ~Object() = default;
	virtual void Menu() = 0;
	virtual void Tooltip() = 0;
	virtual void SaveToJson(json& j) const = 0;
	virtual void LoadFromJson(const json& j) = 0;
};
