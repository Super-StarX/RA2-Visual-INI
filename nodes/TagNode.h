#pragma once
#include "BaseNode.h"
#include "Pins/Pin.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

class TagNode : public BaseNode {
public:
	TagNode(MainWindow* owner, int id, const char* name, bool input, ImColor color = ImColor(255, 255, 255));
	virtual ~TagNode();

	virtual void Menu();
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void SetName(const std::string& str) override;

	// 维护全局标签注册表
	static std::unordered_map<std::string, int> GlobalNames;
	static std::unordered_map<std::string, TagNode*> Inputs;
	static bool HasInputChanged;
	static std::unordered_set<std::string> HighlightedNodes;
	static void UpdateSelectedName();
	static void UpdateInputs();

	void Update() override;

	std::unique_ptr<Pin> InputPin;			// 唯一输入引脚
	bool IsInput{ true };
	bool IsConstant{ false };

	// 引脚管理
	void UpdatePins();
	bool CheckInputConflicts();
	TagNode* GetInputTagNode();
};