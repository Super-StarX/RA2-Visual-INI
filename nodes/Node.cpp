#include "Node.h"
#include "MainWindow.h"

ImVec2 Node::GetPosition() {
	return ed::GetNodePosition(ID);
}

void Node::SetPosition(ImVec2 pos) {
	return ed::SetNodePosition(ID, pos);
}

int Node::GetConnectedLinkCount() {
	int count = 0;
	for (auto& link : Owner->m_Links) {
		if (auto pin = Owner->FindPin(link.StartPinID)) {
			if (pin->Node == this) ++count;
		}
		if (auto pin = Owner->FindPin(link.EndPinID)) {
			if (pin->Node == this) ++count;
		}
	}
	return count;
}

void Node::Menu() {
	ImGui::Text("ID: %p", ID.AsPointer());
	ImGui::Text("Type: %s", Type == NodeType::Section ? "Section" : "Unexcepted");
	ImGui::Text("Inputs: %d", (int)Inputs.size());
	ImGui::Text("Outputs: %d", (int)Outputs.size());
	ImGui::Separator();

	if (Type == NodeType::Section) {
		if (ImGui::MenuItem(IsComment ? "Uncomment" : "Set Comment"))
			IsComment = !IsComment;
		if (ImGui::MenuItem(IsFolded ? "Unfold" : "Fold"))
			IsFolded = !IsFolded;
	}
}
