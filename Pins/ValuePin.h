#pragma once
#include "Pin.h"
#include "TypeLoader.h"

class ValuePin : public Pin {
public:
	static bool DrawElementEditor(std::string& value, const TypeInfo& type);

	static std::string EditBuffer;
	static ListType EditType;
	static ValuePin* EditPin;

	ValuePin(::Node* node, std::string value = "value", int id = -1);
	virtual void SetValue(const std::string& str) override;

	float DrawValueWidget(std::string& value, const TypeInfo& type);

	void DrawListInput(std::string& listValue, const ListType& listType);

	std::string Value;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;
};

