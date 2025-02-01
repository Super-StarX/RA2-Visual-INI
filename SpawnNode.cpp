#include "MainWindow.h"
#include "nodes/Node.h"
#include "nodes/BlueprintNode.h"
#include "nodes/TreeNode.h"
#include "nodes/CommentNode.h"
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
	for (const auto& node : m_Nodes)
		BuildNode(node);
}

SectionNode* MainWindow::SpawnSectionNode(const std::string& section) {
	m_Nodes.emplace_back(std::make_unique<SectionNode>(this, GetNextId(), section.c_str()));
	auto node = m_Nodes.back().get();
	node->Type = NodeType::Section;
	m_SectionMap[section] = reinterpret_cast<SectionNode*>(node);
	m_NodeSections[node->ID] = section;
	m_SectionMap[section]->InputPin = std::make_unique<Pin>(GetNextId(), "input", PinType::Flow);
	m_SectionMap[section]->OutputPin = std::make_unique<Pin>(GetNextId(), "output", PinType::Flow);
	return reinterpret_cast<SectionNode*>(node);
}
