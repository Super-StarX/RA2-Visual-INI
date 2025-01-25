#pragma once
#include "Node.h"
using namespace ax::NodeEditor::Utilities;

class BaseNode : public Node {
public:
	static ImTextureID m_HeaderBackground;

	BaseNode(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255));
	~BaseNode();

	virtual void UpdateInput(Pin& input, BlueprintNodeBuilder& builder);
	virtual void UpdateOutput(Pin& output, BlueprintNodeBuilder& builder);

	BlueprintNodeBuilder GetBuilder();
};

