#pragma once
#include "INENode.h"

class HoudiniNode :	public INENode {
public:
	using INENode::INENode;
	virtual NodeType GetNodeType() const override { return NodeType::Houdini; }
	virtual void Update();
};

