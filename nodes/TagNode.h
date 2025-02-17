#pragma once
#include "BaseNode.h"
#include "Pin.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>

class TagNode : public BaseNode {
public:
	TagNode(MainWindow* owner, int id, const char* name, bool input, ImColor color = ImColor(255, 255, 255));

	void Update() override;
	// 维护全局标签注册表
	static std::unordered_set<std::string> globalNames;
	static std::unordered_map<std::string, Pin*> outputPins;

	std::unique_ptr<Pin> inputPin;      // 唯一输入引脚
	std::unique_ptr<Pin> outputPin;        // 多个输出引脚
	bool isInput = true;

	// 引脚管理
	void UpdatePins();
	bool CheckInputConflicts();
};