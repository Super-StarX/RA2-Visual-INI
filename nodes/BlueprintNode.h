#pragma once
#include "INENode.h"
#include "BuilderNode.h"

class BlueprintNode : public INENode, public BuilderNode {
public:
	using BuilderNode::BuilderNode;
	virtual NodeType GetNodeType() const override { return NodeType::Blueprint; }
	virtual void Update();
};

