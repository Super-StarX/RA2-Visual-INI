#pragma once
#include "INENode.h"
#include "Pins/Pin.h"

class SimpleNode : public INENode<Pin> {
public:
	using INENode::INENode;

	virtual NodeType GetNodeType() const override { return NodeType::Simple; }
	virtual void Update();
};

