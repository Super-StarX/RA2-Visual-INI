#pragma once
#include "Node.h"
class SimpleNode : public Node {
public:
	static ImTextureID m_HeaderBackground;

	SimpleNode(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255));
	~SimpleNode();

	virtual void Update();
};

