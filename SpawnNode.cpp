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

void MainWindow::CreateNewNode() {
	auto openPopupPosition = ImGui::GetMousePos();
	//ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

	//auto drawList = ImGui::GetWindowDrawList();
	//drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

	Node* node = nullptr;
	if (ImGui::MenuItem("Section"))
		node = SpawnSectionNode();
	ImGui::Separator();

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
