#include "Link.h"

void Link::Menu() const {
	ImGui::Text("ID: %p", ID.AsPointer());
	ImGui::Text("From: %p", StartPinID.AsPointer());
	ImGui::Text("To: %p", EndPinID.AsPointer());
}