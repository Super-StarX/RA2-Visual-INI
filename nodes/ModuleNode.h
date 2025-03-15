#pragma once
#include "INENode.h"
#include "Pins/Pin.h"

class ModuleNode : public INENode<Pin> {
public:
	ModuleNode(const char* name = "", int id = 0);
	virtual NodeType GetNodeType() const override { return NodeType::Module; }
	virtual void Update() override;
	virtual void Menu() override;
	virtual void Tooltip() override;
	void LoadProject();
	void LoadProject(std::string path);
	void UpdatePins();
	void UpdatePinSet(std::vector<Pin>& pinSet, const std::vector<std::string>& newNames, bool direction);

	std::string Path;
	json InternalProject;
};