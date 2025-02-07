#pragma once
#include "BaseNode.h"
#include <memory>

class SectionNode : public BaseNode {
public:
	struct KeyValuePair {
		std::string Key;
		std::string Value;
		Pin         OutputPin;
		bool		IsInherited = false;
		bool		IsHide = false;
	};

	using BaseNode::BaseNode;
	virtual void Update();

	std::vector<KeyValuePair> KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;
};

