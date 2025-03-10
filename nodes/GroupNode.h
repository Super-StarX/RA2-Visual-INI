#pragma once
#include "Node.h"

class GroupNode : public Node {
public:
	using Node::Node;
	virtual NodeType GetNodeType() const override { return NodeType::Group; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void Menu() override;
	virtual void Update() override;

	bool IsEditingName{ false };
	ImVec2 Size{ 300,200 };
};
