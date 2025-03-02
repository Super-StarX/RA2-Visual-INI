// SectionNode.h
#pragma once
#include "BaseNode.h"
#include "TypeLoader.h"
#include "Pins/Pin.h"
#include <memory>
#include <string>

class SectionNode : public BaseNode {
public:
	static std::unordered_map<std::string, SectionNode*> Map;
	using vector_kv = std::vector<std::unique_ptr<KeyValue>>;

	using BaseNode::BaseNode;
	virtual void Update() override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;

	vector_kv::iterator FindPin(const Pin& key);
	vector_kv::iterator FindPin(const std::string& key);
	KeyValue* AddKeyValue(const std::string& key, const std::string& value, int pinid = 0, bool isInherited = false, bool isComment = false, bool isFolded = false);
	void FoldedKeyValues(size_t& i);
	void UnFoldedKeyValues(KeyValue& kv, ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder);


	vector_kv KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;
	float maxSize;
	float lastMaxSize;
};