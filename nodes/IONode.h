// IONode.h
#pragma once
#include "Node.h"

enum class IOMode { Input, Output };

class IONode : public Node {
public:
	IONode(const char* name, IOMode mode, int id = 0);
	virtual NodeType GetNodeType() const override {
		return mode == IOMode::Input ? NodeType::Input : NodeType::Output;
	}
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	IOMode GetMode() const { return mode; }
	Pin* GetPin() const { return IOPin.get(); };

private:
	IOMode mode;
	std::unique_ptr<Pin> IOPin;
};
