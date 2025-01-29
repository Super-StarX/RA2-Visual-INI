#include "MainWindow.h"
#include "nodes/Node.h"
#include "nodes/BlueprintNode.h"
#include "nodes/TreeNode.h"
#include "nodes/CommentNode.h"
#include "nodes/SimpleNode.h"
#include "nodes/HoudiniNode.h"

void MainWindow::BuildNode(const std::unique_ptr<Node>& node) {
	for (auto& input : node->Inputs) {
		input.Node = node.get();
		input.Kind = PinKind::Input;
	}

	for (auto& output : node->Outputs) {
		output.Node = node.get();
		output.Kind = PinKind::Output;
	}
}

Node* MainWindow::SpawnInputActionNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "InputAction Fire", ImColor(255, 128, 128)));	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnBranchNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Branch"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnDoNNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Do N"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "N", PinType::Int);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnOutputActionNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "OutputAction"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnPrintStringNode() {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), "Print String"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "In String", PinType::String);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnMessageNode() {
	m_Nodes.emplace_back(std::make_unique<SimpleNode>(this, GetNextId(), "", ImColor(128, 195, 248)));
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "Message", PinType::String);

	BuildNode(m_Nodes.back());

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

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnLessNode() {
	m_Nodes.emplace_back(std::make_unique<SimpleNode>(this, GetNextId(), "<", ImColor(128, 195, 248)));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnWeirdNode() {
	m_Nodes.emplace_back(std::make_unique<SimpleNode>(this, GetNextId(), "o.O", ImColor(128, 195, 248)));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Float);

	BuildNode(m_Nodes.back());

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

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeSequenceNode() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Sequence"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeTaskNode() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Move To"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnTreeTask2Node() {
	m_Nodes.emplace_back(std::make_unique<TreeNode>(this, GetNextId(), "Random Wait"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back());

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

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

Node* MainWindow::SpawnHoudiniGroupNode() {
	m_Nodes.emplace_back(std::make_unique<HoudiniNode>(this, GetNextId(), "Group"));
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Inputs.emplace_back(GetNextId(), "", PinType::Flow);
	m_Nodes.back()->Outputs.emplace_back(GetNextId(), "", PinType::Flow);

	BuildNode(m_Nodes.back());

	return m_Nodes.back().get();
}

void MainWindow::CreateInitNodes() {
	Node* node;
	node = SpawnInputActionNode();      ed::SetNodePosition(node->ID, ImVec2(-252, 220));
	node = SpawnBranchNode();           ed::SetNodePosition(node->ID, ImVec2(-300, 351));
	node = SpawnDoNNode();              ed::SetNodePosition(node->ID, ImVec2(-238, 504));
	node = SpawnOutputActionNode();     ed::SetNodePosition(node->ID, ImVec2(71, 80));
	node = SpawnSetTimerNode();         ed::SetNodePosition(node->ID, ImVec2(168, 316));

	node = SpawnTreeSequenceNode();     ed::SetNodePosition(node->ID, ImVec2(1028, 329));
	node = SpawnTreeTaskNode();         ed::SetNodePosition(node->ID, ImVec2(1204, 458));
	node = SpawnTreeTask2Node();        ed::SetNodePosition(node->ID, ImVec2(868, 538));

	node = SpawnComment();              ed::SetNodePosition(node->ID, ImVec2(112, 576)); ed::SetGroupSize(node->ID, ImVec2(384, 154));
	node = SpawnComment();              ed::SetNodePosition(node->ID, ImVec2(800, 224)); ed::SetGroupSize(node->ID, ImVec2(640, 400));

	node = SpawnLessNode();             ed::SetNodePosition(node->ID, ImVec2(366, 652));
	node = SpawnWeirdNode();            ed::SetNodePosition(node->ID, ImVec2(144, 652));
	node = SpawnMessageNode();          ed::SetNodePosition(node->ID, ImVec2(-348, 698));
	node = SpawnPrintStringNode();      ed::SetNodePosition(node->ID, ImVec2(-69, 652));

	node = SpawnHoudiniTransformNode(); ed::SetNodePosition(node->ID, ImVec2(500, -70));
	node = SpawnHoudiniGroupNode();     ed::SetNodePosition(node->ID, ImVec2(500, 42));

	m_Links.push_back(Link(GetNextLinkId(), m_Nodes[5]->Outputs[0].ID, m_Nodes[6]->Inputs[0].ID));
	m_Links.push_back(Link(GetNextLinkId(), m_Nodes[5]->Outputs[0].ID, m_Nodes[7]->Inputs[0].ID));

	m_Links.push_back(Link(GetNextLinkId(), m_Nodes[14]->Outputs[0].ID, m_Nodes[15]->Inputs[0].ID));
}

void MainWindow::CreateNewNode() {
	auto openPopupPosition = ImGui::GetMousePos();
	//ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

	//auto drawList = ImGui::GetWindowDrawList();
	//drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

	Node* node = nullptr;
	if (ImGui::MenuItem("Input Action"))
		node = SpawnInputActionNode();
	if (ImGui::MenuItem("Output Action"))
		node = SpawnOutputActionNode();
	if (ImGui::MenuItem("Branch"))
		node = SpawnBranchNode();
	if (ImGui::MenuItem("Do N"))
		node = SpawnDoNNode();
	if (ImGui::MenuItem("Set Timer"))
		node = SpawnSetTimerNode();
	if (ImGui::MenuItem("Less"))
		node = SpawnLessNode();
	if (ImGui::MenuItem("Weird"))
		node = SpawnWeirdNode();
	if (ImGui::MenuItem("Trace by Channel"))
		node = SpawnTraceByChannelNode();
	if (ImGui::MenuItem("Print String"))
		node = SpawnPrintStringNode();
	ImGui::Separator();
	if (ImGui::MenuItem("Comment"))
		node = SpawnComment();
	ImGui::Separator();
	if (ImGui::MenuItem("Sequence"))
		node = SpawnTreeSequenceNode();
	if (ImGui::MenuItem("Move To"))
		node = SpawnTreeTaskNode();
	if (ImGui::MenuItem("Random Wait"))
		node = SpawnTreeTask2Node();
	ImGui::Separator();
	if (ImGui::MenuItem("Message"))
		node = SpawnMessageNode();
	ImGui::Separator();
	if (ImGui::MenuItem("Transform"))
		node = SpawnHoudiniTransformNode();
	if (ImGui::MenuItem("Group"))
		node = SpawnHoudiniGroupNode();

	if (node) {
		BuildNodes();

		createNewNode = false;

		ed::SetNodePosition(node->ID, openPopupPosition);

		if (auto startPin = newNodeLinkPin) {
			auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

			for (auto& pin : pins) {
				if (Pin::CanCreateLink(startPin, &pin)) {
					auto endPin = &pin;
					if (startPin->Kind == PinKind::Input)
						std::swap(startPin, endPin);

					m_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
					m_Links.back().Color = Pin::GetIconColor(startPin->Type);

					break;
				}
			}
		}
	}

	ImGui::EndPopup();
}
