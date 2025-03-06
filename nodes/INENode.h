#pragma once
#include "Node.h"
class INENode : public Node {
public:
	INENode(const char* name, int id = 0);
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	std::vector<Pin> Inputs;
	std::vector<Pin> Outputs;
};

