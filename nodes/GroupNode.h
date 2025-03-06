#pragma once
#include "Node.h"

class GroupNode : public Node {
public:
	using Node::Node;
	virtual NodeType GetNodeType() const override { return NodeType::Group; }
	virtual void Update() override;

	ImVec2 Size;
};
