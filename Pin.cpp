#include "Pin.h"
#include "PinType.h"
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

void Pin::Menu() {
	// 显示当前类型
	if (auto* currentType = PinTypeManager::Get().FindType(TypeIdentifier)) {
		ImGui::Text("Current Type: %s", currentType->DisplayName.c_str());
		ImGui::ColorButton("##color", currentType->Color,
			ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
		ImGui::SameLine();
		ImGui::TextColored(currentType->Color, "%s",
			currentType->DisplayName.c_str());
	}

	ImGui::Separator();

	// 类型选择菜单
	if (ImGui::BeginMenu("Change Type")) {
		for (const auto& type : PinTypeManager::Get().GetAllTypes()) {
			if (ImGui::MenuItem(type.DisplayName.c_str())) {
				TypeIdentifier = type.Identifier;
				// 标记节点需要刷新
				// 通过微小位置变化强制刷新
				auto pos = Node->GetPosition();
				ed::SetNodePosition(Node->ID, ImVec2(pos.x + 0.1f, pos.y + 0.1f));
				ed::SetNodePosition(Node->ID, pos);
			}

			// 在菜单项显示颜色标记
			ImGui::SameLine();
			ImGui::ColorButton(("##color_" + type.Identifier).c_str(),
				type.Color, ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
		}
		ImGui::EndMenu();
	}

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