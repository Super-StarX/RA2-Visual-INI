#pragma once
#include "Node.h"

class TreeNode : public Node {
public:
	using Node::Node;
	virtual void Update();
};

