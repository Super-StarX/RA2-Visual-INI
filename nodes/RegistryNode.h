#pragma once
#include "Node.h"

class RegistryNode : public Node {
public:
	RegistryNode(const std::string& name = "", int id = 0);

	virtual NodeType GetNodeType() const override;
	virtual void Update() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j, bool newId = false) override;

	std::vector<std::string> Entries;
	ImVec2 Size{ 300,200 };
};
