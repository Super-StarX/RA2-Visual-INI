#include "MainWindow.h"
#include "Pins/KeyValue.h"
#include "Nodes/Node.h"
#include "Nodes/BlueprintNode.h"
#include "Nodes/CommentNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/TreeNode.h"
#include "Nodes/GroupNode.h"
#include "Nodes/SimpleNode.h"
#include "Nodes/HoudiniNode.h"
#include "Nodes/SectionNode.h"

void MainWindow::BuildNode(const std::unique_ptr<Node>& node) {
	for (auto& input : node->Inputs) {
		input.Node = node.get();
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs) {
		output.Node = node.get();
		output.Kind = PinKind::Output;
	}

	if (auto sectionNode = dynamic_cast<SectionNode*>(node.get())) {
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
		auto newkv = newNode->AddKeyValue(kv.Key, kv.Value, GetNextId(), kv.IsInherited, kv.IsComment, kv.IsFolded);

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
	Node::Array.emplace_back(std::make_unique<SectionNode>(this, GetNextId(), section.c_str()));
	auto node = reinterpret_cast<SectionNode*>(Node::Array.back().get());
	node->Type = NodeType::Section;
	node->InputPin = std::make_unique<Pin>(GetNextId(), "input");
	node->OutputPin = std::make_unique<Pin>(GetNextId(), "output");
	SectionNode::Map[section] = node;
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
	if (input && TagNode::Inputs.contains(section)) {
		auto exsit = TagNode::Inputs[section];
		ed::SelectNode(exsit->ID);
		ed::NavigateToSelection();
		return nullptr;
	}

	Node::Array.emplace_back(std::make_unique<TagNode>(this, GetNextId(), section.c_str(), input));
	auto node = Node::Array.back().get();
	node->Type = NodeType::Tag;
	node->Size = ImVec2(300, 200);

	return reinterpret_cast<TagNode*>(node);
}