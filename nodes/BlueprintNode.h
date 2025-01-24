#pragma once
#include "BaseNode.h"

class BlueprintNode : public BaseNode {
public:
	using BaseNode::BaseNode;
	virtual void Update();
};

