#include "MainWindow.h"
#include "Pins/KeyValue.h"
#include "Nodes/Node.h"
#include "Nodes/BlueprintNode.h"
#include "Nodes/CommentNode.h"
#include "Nodes/GroupNode.h"
#include "Nodes/HoudiniNode.h"
#include "Nodes/ListNode.h"
#include "Nodes/ModuleNode.h"
#include "Nodes/SectionNode.h"
#include "Nodes/SimpleNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/TreeNode.h"

Node* MainWindow::SpawnNodeFromTemplate(const TemplateSection& templa, ImVec2 position) {
	// 转换屏幕坐标到画布坐标
	const auto canvasPos = ed::ScreenToCanvas(position);

	// 创建节点
	auto* newNode = SpawnSectionNode(templa.Name);
	ed::SetNodePosition(newNode->ID, canvasPos);
	newNode->TypeName = templa.Type;
	newNode->Color = templa.Color;
	newNode->IsFolded = templa.IsFolded;
	newNode->IsComment = templa.IsComment;

	// 填充键值对
	for (const auto& kv : templa.KeyValues) {
		auto newkv = newNode->AddKeyValue(kv.Key, kv.Value, "", GetNextId(), kv.IsInherited, kv.IsComment, kv.IsFolded);

		// 如果场内有对应的section就连上Link
		if (SectionNode::Map.contains(kv.Value)) {
			auto targetNode = SectionNode::Map[kv.Value];
			if (targetNode->InputPin->CanCreateLink(newkv))
				newkv->LinkTo(targetNode->InputPin.get())->TypeIdentifier = newkv->GetLinkType();
		}
	}

	// 添加动画效果
	ed::SelectNode(newNode->ID);
	ed::NavigateToSelection(true);

	return newNode;
}

SectionNode* MainWindow::SpawnSectionNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<SectionNode>(section.c_str()));
	auto node = reinterpret_cast<SectionNode*>(Node::Array.back().get());
	node->Color = ImColor(0, 64, 128);
	return node;
}

ListNode* MainWindow::SpawnListNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<ListNode>(section.c_str()));
	auto node = reinterpret_cast<ListNode*>(Node::Array.back().get());

	return node;
}

ModuleNode* MainWindow::SpawnModuleNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<ModuleNode>(section.c_str()));
	auto node = reinterpret_cast<ModuleNode*>(Node::Array.back().get());

	return node;
}

GroupNode* MainWindow::SpawnGroupNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<GroupNode>(section.c_str()));
	auto node = reinterpret_cast<GroupNode*>(Node::Array.back().get());

	return node;
}

CommentNode* MainWindow::SpawnCommentNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<CommentNode>(section.c_str()));
	auto node = reinterpret_cast<CommentNode*>(Node::Array.back().get());

	return node;
}

TagNode* MainWindow::SpawnTagNode(bool input, const std::string& section) {
	if (input && TagNode::Inputs.contains(section)) {
		auto exsit = TagNode::Inputs[section];
		ed::SelectNode(exsit->ID);
		ed::NavigateToSelection();
		return nullptr;
	}

	Node::Array.emplace_back(std::make_unique<TagNode>(section.c_str(), input));
	auto node = reinterpret_cast<TagNode*>(Node::Array.back().get());

	return node;
}