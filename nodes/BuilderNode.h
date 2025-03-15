#pragma once
#include "INENode.h"
using namespace ax::NodeEditor::Utilities;

class BuilderNode {
public:
	static ImTextureID m_HeaderBackground;
	static void CreateHeader();
	static void DestroyHeader();

	static void UpdateInput(Pin& input);
	static void UpdateOutput(Pin& output);

	static BlueprintNodeBuilder* GetBuilder();
};

