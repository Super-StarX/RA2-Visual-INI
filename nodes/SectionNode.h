// SectionNode.h
#pragma once
#include "BaseNode.h"
#include "TypeLoader.h"
#include <memory>
#include <string>

class SectionNode;
class KeyValue : public Pin {
public:
	KeyValue(SectionNode* node, std::string key = "key", std::string value = "value");

	virtual void SetValue(std::string str) { Value = str; }
	virtual void ToolTip();

	std::string Key;
	std::string Value;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;
};

class SectionNode : public BaseNode {
public:
	static std::unordered_map<std::string, SectionNode*> Map;
	using vector_kv = std::vector<std::unique_ptr<KeyValue>>;

	using BaseNode::BaseNode;
	virtual void Update() override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin);

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

private:
	TypeInfo GetKeyTypeInfo(const std::string& sectionType, const std::string& key) const {
		return TypeSystem::Get().GetKeyType(sectionType, key);
	}

	TypeInfo GetTypeInfo(const std::string& typeName) const {
		return TypeSystem::Get().GetTypeInfo(typeName);
	}

	void DrawValueWidget(KeyValue& value, const TypeInfo& type);

	void DrawListInput(std::string& listValue, const ListType& listType);
	void OpenListEditor(std::string& listValue, const ListType& listType);
	bool DrawElementEditor(std::string& value, const TypeInfo& type);
};