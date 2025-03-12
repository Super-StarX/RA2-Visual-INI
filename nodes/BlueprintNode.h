#pragma once
#include "INENode.h"
#include "BuilderNode.h"
#include "Pins/Pin.h"

class BlueprintNode : public INENode<Pin>, public BuilderNode {
public:
	using BuilderNode::BuilderNode;
	virtual NodeType GetNodeType() const override { return NodeType::Blueprint; }
	virtual void Update();
};

