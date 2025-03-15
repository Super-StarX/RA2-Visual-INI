#pragma once
#include "INENode.h"
#include "Pins/Pin.h"

class BlueprintNode : public INENode<Pin> {
public:
	using INENode::INENode;
	virtual NodeType GetNodeType() const override { return NodeType::Blueprint; }
	virtual void Update();
};

