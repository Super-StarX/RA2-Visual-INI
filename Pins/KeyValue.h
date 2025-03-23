#pragma once
#include "ValuePin.h"
#include "MainWindow.h"
#include "Nodes/Node.h"
#include <memory>

class KeyValue : public ValuePin {
public:
	KeyValue(::Node* node, const std::string& key = "key", const std::string& value = "value", const std::string& comment = "", int id = 0);

	virtual void Tooltip() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	Pin InputPin;
	std::string Key;
	std::string Comment;
};
