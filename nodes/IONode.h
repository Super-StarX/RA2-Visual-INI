// IONode.h
#pragma once
#include "Node.h"
#include "Pins/Pin.h"

class IONode : public Node {
public:
	IONode(PinKind mode, const char* name = "", int id = 0);
	virtual NodeType GetNodeType() const override { return NodeType::IO; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	PinKind GetMode() const { return mode; }
	Pin* GetPin() const { return IOPin.get(); };
	void Update() override;

private:
	PinKind mode;
	std::unique_ptr<Pin> IOPin;
};
