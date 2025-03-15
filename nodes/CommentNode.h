#pragma once
#include "Node.h"

class CommentNode : public Node {
public:
	using Node::Node;
	virtual NodeType GetNodeType() const override { return NodeType::Comment; }
	virtual void Update() override;

public:
	std::string Content;   // 注释内容
	ImVec2      Size;      // 节点当前尺寸
	bool        ShowTitle; // 是否显示标题栏
};
