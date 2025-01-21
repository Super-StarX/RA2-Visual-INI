#include "MainWindow.h"
#include "Node.h"
#include "BlueprintNode.h"
#include "TreeNode.h"
#include "CommentNode.h"
#include "HoudiniNode.h"

Node* MainWindow::SpawnInputActionNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "InputAction Fire", ImColor(255, 128, 128)));	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnBranchNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Branch"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnDoNNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Do N"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "N", PinType::Int);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnOutputActionNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "OutputAction"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnPrintStringNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Print String"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "In String", PinType::String);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnMessageNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "", ImColor(128, 195, 248)));
	m_Nodes.back()->Type = NodeType::Simple;
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Message", PinType::String);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnSetTimerNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Set Timer", ImColor(128, 195, 248)));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnLessNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "<", ImColor(128, 195, 248)));
	m_Nodes.back()->Type = NodeType::Simple;
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnWeirdNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "o.O", ImColor(128, 195, 248)));
	m_Nodes.back()->Type = NodeType::Simple;
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTraceByChannelNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64)));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Start", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "End", PinType::Int);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Trace Channel", PinType::Float);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Trace Complex", PinType::Bool);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Actors to Ignore", PinType::Int);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Draw Debug Type", PinType::Bool);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Ignore Self", PinType::Bool);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Out Hit", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Return Value", PinType::Bool);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeSequenceNode() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Sequence"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeTaskNode() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Move To"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeTask2Node() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Random Wait"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnComment() {
	m_Nodes.emplace_back(std::make_unique<CommentNode>(this, GetNextId(), "Test Comment"));
	m_Nodes.back()->Size = ImVec2(300, 200);

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnHoudiniTransformNode() {
	m_Nodes.emplace_back(std::make_unique<HoudiniNode>(this, GetNextId(), "Transform"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnHoudiniGroupNode() {
	m_Nodes.emplace_back(std::make_unique<HoudiniNode>(this, GetNextId(), "Group"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back().get());

	return m_Nodes.back().get();
}
