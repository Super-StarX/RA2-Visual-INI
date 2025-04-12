#pragma once
#include "Node.h"

class GroupNode : public Node {
public:
	GroupNode(const char* name = "", int id = 0);
	virtual NodeType GetNodeType() const override { return NodeType::Group; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j, bool newId = false) override;
	virtual void Menu() override;
	virtual void Update() override;

	ImVec2 Size{ 300,200 };
};
