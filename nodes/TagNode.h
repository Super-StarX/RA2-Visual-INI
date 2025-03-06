#pragma once
#include "Node.h"
#include "BuilderNode.h"
#include "Pins/Pin.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

class TagNode : public Node, public BuilderNode {
public:
	TagNode(const char* name, bool input, int id = -1);
	virtual ~TagNode();

	virtual NodeType GetNodeType() const override { return NodeType::Tag; }
	virtual void Menu();
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	virtual void SetName(const std::string& str) override;

	// 维护全局标签注册表
	static std::unordered_map<std::string, int> GlobalNames;
	static std::unordered_map<std::string, TagNode*> Inputs;
	static bool HasInputChanged;
	static std::unordered_set<std::string> HighlightedNodes;
	static void UpdateSelectedName();
	static void UpdateInputs();

	std::string ResolveTagPointer(TagNode* tagNode, std::unordered_set<Node*>& visited);
	void Update() override;

	std::unique_ptr<Pin> InputPin;			// 唯一输入引脚
	bool IsInput{ true };
	bool IsConstant{ false };

	// 引脚管理
	void UpdatePins();
	bool CheckInputConflicts() const;
	TagNode* GetInputTagNode() const;
};