#pragma once
#include "BaseNode.h"
class CommentNode : public BaseNode {
public:
	using BaseNode::BaseNode;
	virtual void Update() override;


public:
	std::string Content;   // 注释内容
	ImVec2      Size;      // 节点当前尺寸
	bool        ShowTitle; // 是否显示标题栏
};
