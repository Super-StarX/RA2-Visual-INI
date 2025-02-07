#include "Pin.h"
#include "PinType.h"
#include "nodes/Node.h"
#include "nodes/SectionNode.h"
#include "MainWindow.h"
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
}

std::string Pin::GetLinkType() const {
	auto* typeInfo = PinTypeManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return std::string();

	return typeInfo->LinkType;
}

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
			if (ImGui::MenuItem(type.DisplayName.c_str()))
				TypeIdentifier = type.Identifier;

			// 在菜单项显示颜色标记
			ImGui::SameLine();
			ImGui::ColorButton(("##color_" + type.Identifier).c_str(),
				type.Color, ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
		}
		ImGui::EndMenu();
	}

	if (this->Node->Type == NodeType::Section) {
		auto sectionNode = reinterpret_cast<SectionNode*>(this->Node);
		auto it = std::find_if(sectionNode->KeyValues.begin(), sectionNode->KeyValues.end(),
				   [this](const SectionNode::KeyValuePair& kv) { return &kv.OutputPin == this; });
		
		if (ImGui::MenuItem("Add Key Value")) {
			auto kv = SectionNode::KeyValuePair{ "", "", Pin(MainWindow::GetNextId(), "") };
			kv.OutputPin.Node = sectionNode;
			kv.OutputPin.Kind = PinKind::Output;

			sectionNode->KeyValues.insert(it, kv);
		}
		if (ImGui::MenuItem("Delete")) {
			sectionNode->KeyValues.erase(it);
		}
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