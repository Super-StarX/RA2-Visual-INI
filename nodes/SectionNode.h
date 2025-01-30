#pragma once
#include "BaseNode.h"
class SectionNode : public BaseNode {
public:
	struct KeyValuePair {
		std::string Key;
		std::string Value;
		Pin         OutputPin;
	};

	using BaseNode::BaseNode;
	virtual void Update();

	std::vector<KeyValuePair> KeyValues;
};

