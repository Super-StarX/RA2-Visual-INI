#pragma once
#include "INENode.h"
using namespace ax::NodeEditor::Utilities;

class BuilderNode {
public:
	static ImTextureID m_HeaderBackground;
	static void CreateHeader();
	static void DestroyHeader();

	virtual void UpdateInput(Pin& input);
	virtual void UpdateOutput(Pin& output);

	BlueprintNodeBuilder* GetBuilder();
};

