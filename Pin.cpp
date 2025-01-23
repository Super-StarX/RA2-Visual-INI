#include "Pin.h"
#include "nodes/Node.h"
#include "utilities/widgets.h"

ImColor Pin::GetIconColor(PinType type) {
	switch (type) {
	default:
	case PinType::Flow:     return ImColor(255, 255, 255);
	case PinType::Bool:     return ImColor(220, 48, 48);
	case PinType::Int:      return ImColor(68, 201, 156);
	case PinType::Float:    return ImColor(147, 226, 74);
	case PinType::String:   return ImColor(124, 21, 153);
	case PinType::Object:   return ImColor(51, 150, 215);
	case PinType::Function: return ImColor(218, 0, 183);
	case PinType::Delegate: return ImColor(255, 48, 48);
	}
};

void Pin::Menu() const {
	ImGui::Text("ID: %p", ID.AsPointer());
	if (Node)
		ImGui::Text("Node: %p", Node->ID.AsPointer());
	else
		ImGui::Text("Node: %s", "<none>");
}

void Pin::DrawPinIcon(bool connected, int alpha) const {
	using ax::Widgets::IconType;
	IconType iconType;
	ImColor color = GetIconColor(Type);
	color.Value.w = alpha / 255.0f;
	switch (Type) {
	case PinType::Flow:     iconType = IconType::Flow;   break;
	case PinType::Bool:     iconType = IconType::Circle; break;
	case PinType::Int:      iconType = IconType::Circle; break;
	case PinType::Float:    iconType = IconType::Circle; break;
	case PinType::String:   iconType = IconType::Circle; break;
	case PinType::Object:   iconType = IconType::Circle; break;
	case PinType::Function: iconType = IconType::Circle; break;
	case PinType::Delegate: iconType = IconType::Square; break;
	default:
		return;
	}

	ax::Widgets::Icon(ImVec2(IconSize, IconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
};
