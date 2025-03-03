#pragma once
#include "BaseNode.h"
#include "TypeLoader.h"
#include "Pins/ValuePin.h"
#include <memory>
#include <string>

class ListNode : public BaseNode {
public:
	static std::unordered_map<std::string, ListNode*> Map;
	using vector_v = std::vector<std::unique_ptr<ValuePin>>;

	using BaseNode::BaseNode;
	virtual void Update() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	ValuePin* AddKeyValue(const std::string& value, int pinid = 0, bool isInherited = false, bool isComment = false, bool isFolded = false);
	void FoldedKeyValues(size_t& i);
	void UnFoldedKeyValues(ValuePin& kv, ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder);


	vector_v KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;
	float maxSize;
	float lastMaxSize;
};