﻿#pragma once
#include "Pin.h"
#include "TypeLoader.h"

class ValuePin : public Pin {
public:
	ValuePin(::Node* node, std::string value = "value", int id = -1);

	float DrawValueWidget(std::string& value, const TypeInfo& type);

	void DrawListInput(std::string& listValue, const ListType& listType);
	static void OpenListEditor(std::string& listValue, const ListType& listType);
	static bool DrawElementEditor(std::string& value, const TypeInfo& type);

	std::string Value;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;
};

