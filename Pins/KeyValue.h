#pragma once
#include "ValuePin.h"
#include "MainWindow.h"
#include "Nodes/Node.h"

class KeyValue : public ValuePin {
public:
	KeyValue(::Node* node, std::string key = "key", std::string value = "value", int id = 0);

	virtual void Tooltip() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	std::string Key;
};
