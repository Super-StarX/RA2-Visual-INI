// SectionNode.h
#pragma once
#include "BaseNode.h"
#include "TypeLoader.h"
#include <memory>

class SectionNode : public BaseNode {
public:
	struct KeyValuePair {
		std::string Key;
		std::string Value;
		Pin OutputPin;
		bool IsInherited = false;
		bool IsComment = false;
		bool IsFolded = false;
	};

	using BaseNode::BaseNode;
	virtual void Update() override;

	std::vector<KeyValuePair> KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;

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