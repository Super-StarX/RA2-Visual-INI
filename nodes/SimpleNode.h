#pragma once
#include "INENode.h"
#include "BuilderNode.h"
class SimpleNode : public INENode, public BuilderNode {
public:
	using INENode::INENode;

	virtual NodeType GetNodeType() const override { return NodeType::Simple; }
	virtual void Update();
};

