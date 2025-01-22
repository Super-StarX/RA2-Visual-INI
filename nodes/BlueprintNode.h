#pragma once
#include "Node.h"

class BlueprintNode : public Node {
public:
	static ax::NodeEditor::Utilities::BlueprintNodeBuilder builder;
	using Node::Node;

	virtual void Update();
};

