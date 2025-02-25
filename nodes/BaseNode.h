#pragma once
#include "Node.h"
using namespace ax::NodeEditor::Utilities;

class BaseNode : public Node {
public:
	static ImTextureID m_HeaderBackground;
	static void CreateHeader();
	static void DestroyHeader();

	BaseNode(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255));
	~BaseNode();

	virtual void UpdateInput(Pin& input);
	virtual void UpdateOutput(Pin& output);

	BlueprintNodeBuilder* GetBuilder();
};

