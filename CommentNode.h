#pragma once
#include "Node.h"

class CommentNode : public Node {
public:
	using Node::Node;
	virtual void Update();
};

