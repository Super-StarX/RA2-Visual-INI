#pragma once
#include "INENode.h"
#include "Pins/Pin.h"

class TreeNode : public INENode<Pin> {
public:
	using INENode::INENode;
	virtual NodeType GetNodeType() const override { return NodeType::Tree; }
	virtual void Update();
};

