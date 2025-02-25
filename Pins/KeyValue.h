#pragma once
#include "Pin.h"
#include "MainWindow.h"
#include "Nodes/Node.h"

class KeyValue : public Pin {
public:
	KeyValue(::Node* node, std::string key = "key", std::string value = "value", int id = MainWindow::GetNextId());

	virtual void SetValue(std::string str) { Value = str; }
	virtual void ToolTip();
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	std::string Key;
	std::string Value;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;
};
