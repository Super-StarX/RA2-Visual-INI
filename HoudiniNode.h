#pragma once
#include "Node.h"

class HoudiniNode :	public Node {
public:
	using Node::Node;
	virtual void Update();
};

