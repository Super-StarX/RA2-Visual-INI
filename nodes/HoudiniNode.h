#pragma once
#include "INENode.h"
#include "Pins/Pin.h"

class HoudiniNode :	public INENode<Pin> {
public:
	using INENode::INENode;
	virtual NodeType GetNodeType() const override { return NodeType::Houdini; }
	virtual void Update();
};

