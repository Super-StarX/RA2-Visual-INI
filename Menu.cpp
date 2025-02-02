#include "MainWindow.h"
#include "PinTypeManager.h"

void MainWindow::Menu() {
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId))
		ImGui::OpenPopup("Pin Context Menu");
	else if (ed::ShowLinkContextMenu(&contextLinkId))
		ImGui::OpenPopup("Link Context Menu");
	else if (ed::ShowBackgroundContextMenu()) {
		ImGui::OpenPopup("Create New Node");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Node Context Menu"))
		NodeMenu();
	if (ImGui::BeginPopup("Pin Context Menu"))
		PinMenu();
	if (ImGui::BeginPopup("Link Context Menu"))
		LinkMenu();
	if (ImGui::BeginPopup("Create New Node"))
		LayerMenu();
	else
		createNewNode = false;
	ImGui::PopStyleVar();
	ed::Resume();
}

void MainWindow::LayerMenu() {
	auto openPopupPosition = ImGui::GetMousePos();
	//ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

	//auto drawList = ImGui::GetWindowDrawList();
	//drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

	Node* node = nullptr;
	if (ImGui::MenuItem("Section"))
		node = (Node*)SpawnSectionNode();
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
					m_Links.back().Color = startPin->GetIconColor();

					break;
				}
			}
		}
	}

	ImGui::EndPopup();
}

void MainWindow::NodeMenu() {
	auto node = FindNode(contextNodeId);

	ImGui::TextUnformatted("Node Context Menu");
	ImGui::Separator();
	if (node) {
		ImGui::Text("ID: %p", node->ID.AsPointer());
		ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
		ImGui::Text("Inputs: %d", (int)node->Inputs.size());
		ImGui::Text("Outputs: %d", (int)node->Outputs.size());
	}
	else
		ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		ed::DeleteNode(contextNodeId);
	ImGui::EndPopup();
}

void MainWindow::PinMenu() {
	auto pin = FindPin(contextPinId);

	ImGui::TextUnformatted("Pin Context Menu");
	ImGui::Separator();
	if (pin)
		pin->Menu();
	else
		ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

	ImGui::EndPopup();
}

void MainWindow::LinkMenu() {
	auto link = FindLink(contextLinkId);

	ImGui::TextUnformatted("Link Context Menu");
	ImGui::Separator();
	if (link)
		link->Menu();
	else
		ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		ed::DeleteLink(contextLinkId);
	ImGui::EndPopup();
}
