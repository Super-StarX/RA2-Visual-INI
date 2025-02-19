#include "MainWindow.h"
#include "nodes/Node.h"
#include "nodes/BlueprintNode.h"
#include "nodes/CommentNode.h"
#include "nodes/TagNode.h"
#include "nodes/TreeNode.h"
#include "nodes/GroupNode.h"
#include "nodes/SimpleNode.h"
#include "nodes/HoudiniNode.h"
#include "nodes/SectionNode.h"

void MainWindow::BuildNode(const std::unique_ptr<Node>& node) {
	for (auto& input : node->Inputs) {
		input.Node = node.get();
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs) {
		output.Node = node.get();
		output.Kind = PinKind::Output;
	}

	if (node->Type == NodeType::Section) {
		auto sectionNode = reinterpret_cast<SectionNode*>(node.get());
		sectionNode->InputPin->Node = sectionNode;
		sectionNode->InputPin->Kind = PinKind::Input;
		sectionNode->OutputPin->Node = sectionNode;
		sectionNode->OutputPin->Kind = PinKind::Output;
	}
}

void MainWindow::BuildNodes() {
	for (const auto& node : Node::Array)
		BuildNode(node);
}

Node* MainWindow::SpawnNodeFromTemplate(const std::string& sectionName, const std::vector<TemplateSection::KeyValue>& keyValues, ImVec2 position) {
	// 转换屏幕坐标到画布坐标
	const auto canvasPos = ed::ScreenToCanvas(position);

	// 创建节点
	auto* newNode = SpawnSectionNode(sectionName);
	BuildNode(Node::Array.back());
	ed::SetNodePosition(newNode->ID, canvasPos);

	// 填充键值对
	for (const auto& kv : keyValues) {
		auto& keyvalue = newNode->AddKeyValue(kv.Key, kv.Value, GetNextId(), kv.IsInherited, kv.IsComment, kv.IsFolded);

		// 如果场内有对应的section就连上Link
		if (SectionNode::Map.contains(kv.Value)) {
			auto targetNode = SectionNode::Map[kv.Value];
			auto outpin = keyvalue.OutputPin.get();
			if (Pin::CanCreateLink(outpin, targetNode->InputPin.get())) {
				CreateLink(outpin, targetNode->InputPin.get())->TypeIdentifier = outpin->GetLinkType();
			}
		}
	}

	// 添加动画效果
	ed::SelectNode(newNode->ID);
	ed::NavigateToSelection(true);

	return newNode;
}

SectionNode* MainWindow::SpawnSectionNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<SectionNode>(this, GetNextId(), section.c_str()));
	auto node = Node::Array.back().get();
	node->Type = NodeType::Section;
	SectionNode::Map[section] = reinterpret_cast<SectionNode*>(node);
	SectionNode::Map[section]->InputPin = std::make_unique<Pin>(GetNextId(), "input");
	SectionNode::Map[section]->OutputPin = std::make_unique<Pin>(GetNextId(), "output");
	return reinterpret_cast<SectionNode*>(node);
}

GroupNode* MainWindow::SpawnGroupNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<GroupNode>(this, GetNextId(), section.c_str()));
	auto node = Node::Array.back().get();
	node->Type = NodeType::Group;
	node->Size = ImVec2(300, 200);

	return reinterpret_cast<GroupNode*>(node);
}

CommentNode* MainWindow::SpawnCommentNode(const std::string& section) {
	Node::Array.emplace_back(std::make_unique<CommentNode>(this, GetNextId(), section.c_str()));
	auto node = Node::Array.back().get();
	node->Type = NodeType::Comment;
	node->Size = ImVec2(300, 200);

	return reinterpret_cast<CommentNode*>(node);
}

TagNode* MainWindow::SpawnTagNode(bool input, const std::string& section) {
	Node::Array.emplace_back(std::make_unique<TagNode>(this, GetNextId(), section.c_str(), input));
	auto node = Node::Array.back().get();
	node->Type = NodeType::Tag;
	node->Size = ImVec2(300, 200);

	return reinterpret_cast<TagNode*>(node);
}