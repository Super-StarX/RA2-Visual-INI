#pragma once
#include "Node.h"

template <typename T>
class INENode : public Node {
public:
	INENode(const char* name, int id = 0);
	virtual T* GetFirstCompatiblePin(Pin* pin) override;
	std::vector<T> Inputs;
	std::vector<T> Outputs;
};

