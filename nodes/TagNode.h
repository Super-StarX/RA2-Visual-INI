#pragma once
#include "Node.h"
#include "Pins/Pin.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

class TagNode : public Node {
public:
	TagNode(const char* name = "", int id = 0); // constant
	TagNode(bool input, const char* name = "", int id = 0);
	virtual ~TagNode();

	virtual NodeType GetNodeType() const override { return NodeType::Tag; }
	virtual void Menu();
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j, bool newId = false) override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	virtual void SetName(const std::string& str) override;
	virtual std::string GetValue(Pin* from = nullptr) const override;

	// 维护全局标签注册表
	static std::unordered_map<std::string, int> GlobalNames;
	static std::unordered_map<std::string, TagNode*> Outputs;
	static bool HasOutputChanged;
	static std::unordered_set<std::string> HighlightedNodes;
	static void UpdateSelectedName();
	static void UpdateOutputs();

	std::string ResolveTagPointer(TagNode* tagNode, std::unordered_set<Node*>& visited);
	void Update() override;

	std::unique_ptr<Pin> InputPin;			// 唯一输入引脚
	bool IsInput{ true };
	bool IsConstant{ false };

	// 引脚管理
	void UpdatePins();
	bool CheckOutputConflicts() const;
	TagNode* GetOutputTagNode() const;
};