#include "MainWindow.h"
#include "Node.h"

Node* MainWindow::SpawnInputActionNode() {
	m_Nodes.emplace_back(GetNextId(), "InputAction Fire", ImColor(255, 128, 128));
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnBranchNode() {
	m_Nodes.emplace_back(GetNextId(), "Branch");
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnDoNNode() {
	m_Nodes.emplace_back(GetNextId(), "Do N");
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "N", PinType::Int);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnOutputActionNode() {
	m_Nodes.emplace_back(GetNextId(), "OutputAction");
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnPrintStringNode() {
	m_Nodes.emplace_back(GetNextId(), "Print String");
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "In String", PinType::String);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnMessageNode() {
	m_Nodes.emplace_back(GetNextId(), "", ImColor(128, 195, 248));
	m_Nodes.back().Type = NodeType::Simple;
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Message", PinType::String);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnSetTimerNode() {
	m_Nodes.emplace_back(GetNextId(), "Set Timer", ImColor(128, 195, 248));
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnLessNode() {
	m_Nodes.emplace_back(GetNextId(), "<", ImColor(128, 195, 248));
	m_Nodes.back().Type = NodeType::Simple;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnWeirdNode() {
	m_Nodes.emplace_back(GetNextId(), "o.O", ImColor(128, 195, 248));
	m_Nodes.back().Type = NodeType::Simple;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnTraceByChannelNode() {
	m_Nodes.emplace_back(GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Start", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "End", PinType::Int);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Channel", PinType::Float);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Complex", PinType::Bool);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Actors to Ignore", PinType::Int);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Draw Debug Type", PinType::Bool);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "Ignore Self", PinType::Bool);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Out Hit", PinType::Float);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "Return Value", PinType::Bool);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnTreeSequenceNode() {
	m_Nodes.emplace_back(GetNextId(), "Sequence");
	m_Nodes.back().Type = NodeType::Tree;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnTreeTaskNode() {
	m_Nodes.emplace_back(GetNextId(), "Move To");
	m_Nodes.back().Type = NodeType::Tree;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnTreeTask2Node() {
	m_Nodes.emplace_back(GetNextId(), "Random Wait");
	m_Nodes.back().Type = NodeType::Tree;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnComment() {
	m_Nodes.emplace_back(GetNextId(), "Test Comment");
	m_Nodes.back().Type = NodeType::Comment;
	m_Nodes.back().Size = ImVec2(300, 200);

	return &m_Nodes.back();
}

Node* MainWindow::SpawnHoudiniTransformNode() {
	m_Nodes.emplace_back(GetNextId(), "Transform");
	m_Nodes.back().Type = NodeType::Houdini;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}

Node* MainWindow::SpawnHoudiniGroupNode() {
	m_Nodes.emplace_back(GetNextId(), "Group");
	m_Nodes.back().Type = NodeType::Houdini;
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(&m_Nodes.back());

	return &m_Nodes.back();
}
