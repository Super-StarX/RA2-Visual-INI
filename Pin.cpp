#include "Pin.h"
#include "PinType.h"
#include "Utils.h"
#include "nodes/Node.h"
#include "nodes/SectionNode.h"
#include "MainWindow.h"
#include "utilities/widgets.h"

std::map<ed::PinId, Pin*, ComparePinId> Pin::Array;

Pin::Pin(int id, const char* name, std::string type, PinKind kind) :
	ID(id), Node(nullptr), Name(name), TypeIdentifier(type), Kind(kind) {
	Array[ID] = this;
}

Pin::~Pin() {
	Array.erase(ID);
}

Pin* Pin::Get(ed::PinId id) {
	if (!id)
		return nullptr;

	return Array.count(id) ? Array.at(id) : nullptr;
}

void Pin::UpdateLink(std::string value) {
	// 值变化后,判断自己连着的node的名字是否还是自己的值
	// 如果不是的话,就断开当前链接,并遍历node::array寻找是否有新的node可以链接
	if (!Node || Kind != PinKind::Output)
		return;
	for (auto it = Links.begin(); it != Links.end(); ) {
		if (auto endpin = Get(it->second->EndPinID)) {
			if (endpin->Node->Name != value) {
				it = Links.erase(it);
				Link::Array.erase(std::remove_if(Link::Array.begin(), Link::Array.end(),
					[endpin](auto& link) { return link->EndPinID == endpin->ID; }), Link::Array.end());
			}
			else {
				++it;
			}
		}
	}
	for (auto& pNode : Node::Array)
		if (pNode->Name == value)
			MainWindow::CreateLink(this, pNode->GetFirstCompatiblePin(this));
}

bool Pin::CanCreateLink(Pin* b) {
	return b && b != this && b->Kind != Kind && b->Node != Node;
}

bool Pin::IsLinked() const {
	return !Links.empty();
}

Node* Pin::GetLinkedNode() const {
	if (!IsLinked())
		return nullptr;

	auto begin = Links.begin();
	if (auto endpin = Get(begin->second->EndPinID))
		return endpin->Node;

	return nullptr;
}

SectionNode* Pin::GetLinkedSection() const {
	if (auto pNode = GetLinkedNode())
		if (pNode->Type == NodeType::Section)
			return reinterpret_cast<SectionNode*>(pNode);

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
		auto kv = reinterpret_cast<KeyValue*>(this);

		auto it = sectionNode->FindPin(*this);
		if (ImGui::MenuItem("Add Key Value"))
			sectionNode->KeyValues.insert(it, std::make_unique<KeyValue>(sectionNode)); // 需要在中途加入，因此不能使用Add函数

		if (ImGui::MenuItem("Delete"))
			sectionNode->KeyValues.erase(it);

		if (ImGui::MenuItem("Fold"))
			kv->IsFolded = true;

		if (ImGui::MenuItem(kv->IsComment ? "Uncomment" : "Set Comment"))
			kv->IsComment = !kv->IsComment;

		if (ImGui::MenuItem(kv->IsInherited ? "Cancel Inherited" : "Set Inherited"))
			kv->IsInherited = !kv->IsInherited;
	}
}

void Pin::Tooltip() {
	if (!Node) return;
	if (Node->Type == NodeType::Section) {
		auto pKv = reinterpret_cast<KeyValue*>(this);
		ImGui::BeginTooltip();
			
		auto type = TypeSystem::Get().GetKeyType(Node->TypeName, pKv->Key);
		ImGui::Text("Type: %s", type.TypeName.c_str());
		switch (type.Category) {
		case TypeCategory::NumberLimit:
			ImGui::Text("Range: [%d, %d]",
				std::get<NumberLimit>(type.Data).Min, std::get<NumberLimit>(type.Data).Max);
			break;
		case TypeCategory::StringLimit:
			if (!std::get<StringLimit>(type.Data).ValidValues.empty()) {
				ImGui::Text("Options: %s",
					Utils::JoinStrings(std::get<StringLimit>(type.Data).ValidValues, ", ").c_str());
			}
			break;
		case TypeCategory::List:
			ImGui::Text("Element: %s (%d-%d items)",
				std::get<ListType>(type.Data).ElementType.c_str(),
				std::get<ListType>(type.Data).MinLength,
				std::get<ListType>(type.Data).MaxLength);
			break;
		}
		ImGui::EndTooltip();
	}
}

float Pin::GetAlpha() {
	auto alpha = ImGui::GetStyle().Alpha;
	if (MainWindow::newLinkPin && !this->CanCreateLink(MainWindow::newLinkPin) && this != MainWindow::newLinkPin)
		alpha *= 48.0f / 255.0f;
	return alpha;
}

void Pin::DrawPinIcon(bool connected, int alpha, bool isReverse) const {
	auto* typeInfo = PinTypeManager::Get().FindType(TypeIdentifier);

	using namespace ax::Widgets;
	ImColor color = typeInfo->Color;
	color.Value.w = alpha / 255.0f;

	Icon(ImVec2(IconSize, IconSize), IconType(typeInfo->IconType), connected, color, ImColor(32, 32, 32, alpha), isReverse);
};