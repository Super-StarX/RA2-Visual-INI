#pragma once
#include "VINode.h"
#include "TypeLoader.h"
#include "Pins/Pin.h"
#include <memory>
#include <string>

class SectionNode : public VINode<KeyValue> {
public:
	static std::unordered_map<std::string, SectionNode*> Map;
	using vector_kv = std::vector<std::unique_ptr<KeyValue>>;

	SectionNode(const char* name, int id = 0);

	virtual NodeType GetNodeType() const override { return NodeType::Section; }
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void UnFoldedKeyValues(KeyValue& kv, bool override) override;
	virtual void AddKeyValue() override;
	KeyValue* AddKeyValue(const std::string& key, const std::string& value, int pinid = 0, bool isInherited = false, bool isComment = false, bool isFolded = false);

	vector_kv::iterator FindPin(const Pin& key);
	vector_kv::iterator FindPin(const std::string& key);
};