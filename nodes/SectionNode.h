// SectionNode.h
#pragma once
#include "BaseNode.h"
#include "TypeLoader.h"
#include <memory>

struct KeyValue {
	std::string Key;
	std::string Value;
	std::unique_ptr<Pin> OutputPin;
	bool IsInherited = false;
	bool IsComment = false;
	bool IsFolded = false;

	KeyValue(const std::string& key, const std::string& value, std::unique_ptr<Pin>&& outputPin, bool isInherited = false, bool isComment = false, bool isFolded = false)
		: Key(key),
		Value(value),
		OutputPin(std::move(outputPin)),
		IsInherited(isInherited),
		IsComment(isComment),
		IsFolded(isFolded) {}

	KeyValue(KeyValue&& other) noexcept
		: Key(std::move(other.Key)),
		Value(std::move(other.Value)),
		OutputPin(std::move(other.OutputPin)),
		IsInherited(other.IsInherited),
		IsComment(other.IsComment),
		IsFolded(other.IsFolded) {}

	KeyValue(const KeyValue& other)
		: Key(other.Key),
		Value(other.Value),
		OutputPin(other.OutputPin ? std::make_unique<Pin>(*other.OutputPin) : nullptr),
		IsInherited(other.IsInherited),
		IsComment(other.IsComment),
		IsFolded(other.IsFolded) {}

	KeyValue& operator=(KeyValue&& other) noexcept {
		if (this != &other) {
			Key = std::move(other.Key);
			Value = std::move(other.Value);
			OutputPin = std::move(other.OutputPin);
			IsInherited = other.IsInherited;
			IsComment = other.IsComment;
			IsFolded = other.IsFolded;
		}
		return *this;
	}

	KeyValue& operator=(const KeyValue& other) {
		if (this != &other) {
			Key = other.Key;
			Value = other.Value;
			OutputPin = other.OutputPin ? std::make_unique<Pin>(*other.OutputPin) : nullptr;
			IsInherited = other.IsInherited;
			IsComment = other.IsComment;
			IsFolded = other.IsFolded;
		}
		return *this;
	}
};

class SectionNode : public BaseNode {
public:
	static std::unordered_map<std::string, SectionNode*> Map;

	using BaseNode::BaseNode;
	virtual void Update() override;

	std::vector<KeyValue> KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;

	KeyValue& AddKeyValue(const std::string& key, const std::string& value, int pinid = 0, bool isInherited = false, bool isComment = false, bool isFolded = false);

private:
	TypeInfo GetKeyTypeInfo(const std::string& sectionType, const std::string& key) const {
		return TypeSystem::Get().GetKeyType(sectionType, key);
	}

	TypeInfo GetTypeInfo(const std::string& typeName) const {
		return TypeSystem::Get().GetTypeInfo(typeName);
	}

	void DrawValueWidget(std::string& value, const TypeInfo& type);

	void DrawListInput(std::string& listValue, const ListType& listType);
	void OpenListEditor(std::string& listValue, const ListType& listType);
	bool DrawElementEditor(std::string& value, const TypeInfo& type);
};