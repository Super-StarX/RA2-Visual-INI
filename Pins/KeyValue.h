#pragma once
#include "Pin.h"
#include "MainWindow.h"
#include "Nodes/Node.h"
#include "TypeLoader.h"

class KeyValue : public Pin {
public:
	KeyValue(::Node* node, std::string key = "key", std::string value = "value", int id = MainWindow::GetNextId());

	virtual void Tooltip() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	float DrawValueWidget(const TypeInfo& type);

	static void DrawListInput(std::string& listValue, const ListType& listType);
	static void OpenListEditor(std::string& listValue, const ListType& listType);
	static bool DrawElementEditor(std::string& value, const TypeInfo& type);

	std::string Key;
	std::string Value;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;
};
