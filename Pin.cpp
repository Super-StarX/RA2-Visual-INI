#include "Pin.h"
#include "nodes/Node.h"

void Pin::Menu() const {
	ImGui::Text("ID: %p", ID.AsPointer());
	if (Node)
		ImGui::Text("Node: %p", Node->ID.AsPointer());
	else
		ImGui::Text("Node: %s", "<none>");
}