#pragma once
#include "Node.h"
class BaseNode : public Node {
public:
	static ImTextureID m_HeaderBackground;

	BaseNode(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255));
	~BaseNode();

	virtual void UpdateInput(Pin& input);
	virtual void UpdateOutput(Pin& output);

	ax::NodeEditor::Utilities::BlueprintNodeBuilder GetBuilder();
};

