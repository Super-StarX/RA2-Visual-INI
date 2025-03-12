#pragma once
#include "INENode.h"
#include "Pins/ValuePin.h"

class ModuleNode : public INENode<ValuePin> {
public:
	ModuleNode(const char* name, int id = 0);
	virtual NodeType GetNodeType() const override { return NodeType::Module; }
	virtual void Update();
	void LoadProject();

	ImVec2 Size{ 300,200 };
	json json;
};

