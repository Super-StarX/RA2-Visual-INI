#pragma once
#include "INENode.h"

class TreeNode : public INENode {
public:
	using INENode::INENode;
	virtual NodeType GetNodeType() const override { return NodeType::Tree; }
	virtual void Update();
};

