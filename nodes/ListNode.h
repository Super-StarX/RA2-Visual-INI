#pragma once
#include "VINode.h"
#include "Pins/ValuePin.h"

class ListNode : public VINode<ValuePin> {
public:
	static std::unordered_map<std::string, ListNode*> Map;

	using VINode::VINode;

	virtual NodeType GetNodeType() const override { return NodeType::List; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void UnFoldedKeyValues(ValuePin& kv, bool override) override;
	virtual void AddKeyValue() override;
	virtual std::string GetValue() const override;

	ValuePin* AddKeyValue(const std::string& value, int pinid = 0, bool isInherited = false, bool isComment = false, bool isFolded = false);
};