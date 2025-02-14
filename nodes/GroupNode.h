#pragma once
#include "Node.h"

class GroupNode : public Node {
public:
	using Node::Node;
	virtual void Update();
};

