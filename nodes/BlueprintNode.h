#pragma once
#include "Node.h"

class BlueprintNode : public Node {
public:
	static ImTextureID m_HeaderBackground;

	BlueprintNode(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255));
	~BlueprintNode();

	virtual void Update();
};

