#include "Pin.h"
#include "PinType.h"
#include "nodes/Node.h"
#include "nodes/SectionNode.h"
#include "MainWindow.h"
#include "utilities/widgets.h"

std::map<ed::PinId, Pin*, ComparePinId> Pin::m_Pins;

Pin::Pin(int id, const char* name, std::string type, PinKind kind) :
	ID(id), Node(nullptr), Name(name), TypeIdentifier(type), Kind(kind) {
	m_Pins[ID] = this;
}

Pin::~Pin() {
	m_Pins.erase(ID);
}

Pin* Pin::FindPin(ed::PinId id) {
	if (!id)
		return nullptr;

	return m_Pins.count(id) ? m_Pins.at(id) : nullptr;
}

bool Pin::CanCreateLink(Pin* a, Pin* b) {
	if (!a || !b || a == b || a->Kind == b->Kind || a->Node == b->Node)
		return false;

	return true;
}

bool Pin::IsLinked() const {
	return !Links.empty();
}

Node* Pin::GetLinkedNode() const {
	if (Links.empty())
		return nullptr;

	auto begin = Links.begin();
	if (auto endpin = FindPin(begin->second->EndPinID))
		return endpin->Node;

	return nullptr;
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
				   [this](const KeyValue& kv) { return kv.OutputPin.get() == this; });

		if (ImGui::MenuItem("Add Key Value")) {
			// 需要在中途加入，因此不能使用Add函数
			auto kv = KeyValue{ "key", "value", std::make_unique<Pin>(MainWindow::GetNextId(), "key") };
			kv.OutputPin->Node = sectionNode;
			kv.OutputPin->Kind = PinKind::Output;

			sectionNode->KeyValues.insert(it, kv);
		}

		if (ImGui::MenuItem("Delete"))
			sectionNode->KeyValues.erase(it);

		if (ImGui::MenuItem("Fold"))
			it->IsFolded = true;

		if (ImGui::MenuItem(it->IsComment ? "Uncomment" : "Set Comment"))
			it->IsComment = !it->IsComment;

		if (ImGui::MenuItem(it->IsInherited ? "Cancel Inherited" : "Set Inherited"))
			it->IsInherited = !it->IsInherited;
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