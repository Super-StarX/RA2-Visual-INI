// IONode.h
#pragma once
#include "Node.h"
#include "Pins/Pin.h"

class ModuleNode;
class IONode : public Node {
public:
	IONode(PinKind mode, const char* name = "", int id = 0, ModuleNode* parent = nullptr);
	virtual NodeType GetNodeType() const override { return NodeType::IO; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j, bool newId = false) override;
	virtual std::string GetValue(Pin* from = nullptr) const override;
	virtual bool PinNameChangable() const override { return false; }

	Pin* GetPin() const { return IOPin.get(); };
	void Update() override;

private:
	PinKind Mode;
	std::unique_ptr<Pin> IOPin;
	ModuleNode* Parent;
};
