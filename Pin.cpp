#include "Pin.h"
#include "PinTypeManager.h"
#include "nodes/Node.h"
#include "utilities/widgets.h"

bool Pin::CanCreateLink(Pin* a, Pin* b) {
	if (!a || !b || a == b || a->Kind == b->Kind || a->Node == b->Node)
		return false;

	return true;
}

ImColor Pin::GetIconColor() const {
	auto* typeInfo = PinTypeManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return ImColor(255, 255, 255);

	return typeInfo->Color;
};

void Pin::Menu() const {
	ImGui::Text("ID: %p", ID.AsPointer());
	if (Node)
		ImGui::Text("Node: %p", Node->ID.AsPointer());
	else
		ImGui::Text("Node: %s", "<none>");
}

float Pin::GetAlpha(Pin* newLinkPin) {
	auto alpha = ImGui::GetStyle().Alpha;
	if (newLinkPin && !Pin::CanCreateLink(newLinkPin, this) && this != newLinkPin)
		alpha *= 48.0f / 255.0f;
	return alpha;
}

void Pin::DrawPinIcon(bool connected, int alpha) const {
	auto* typeInfo = PinTypeManager::Get().FindType(TypeIdentifier);

	using namespace ax::Widgets;
	ImColor color = typeInfo->Color;
	color.Value.w = alpha / 255.0f;

	Icon(ImVec2(IconSize, IconSize), IconType(typeInfo->IconType), connected, color, ImColor(32, 32, 32, alpha));
};