#include "Pin.h"
#include "PinType.h"
#include "Utils.h"
#include "KeyValue.h"
#include "Nodes/SectionNode.h"
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

void Pin::Tooltip() {
	ImGui::Text("Pin %d", ID);
	ImGui::Text("Type: %s", TypeIdentifier.c_str());
	ImGui::Text("Node: %s", Node ? Node->Name.c_str() : "None");
	ImGui::Text("Links: %d", Links.size());
}

void Pin::SaveToJson(json& j) const {
	j["ID"] = ID.Get();
	j["Name"] = Name;
	j["TypeIdentifier"] = TypeIdentifier;
	j["Kind"] = static_cast<int>(Kind);
}

void Pin::LoadFromJson(const json& j) {
	Array.erase(ID);
	ID = ed::PinId(j["ID"]);
	Array[ID] = this;
	Name = j["Name"];
	TypeIdentifier = j["TypeIdentifier"];
	Kind = static_cast<PinKind>(j["Kind"]);
}

void Pin::UpdateOutputLink(std::string value) {
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
			this->LinkTo(pNode->GetFirstCompatiblePin(this));
}

bool Pin::CanCreateLink(Pin* b) {
	return b && b != this && b->Kind != Kind && b->Node != Node;
}

bool Pin::IsLinked() const {
	return !Links.empty();
}

Link* Pin::LinkTo(Pin* pin) {
	if (!pin)
		return nullptr;

	// 删除所有起始点的链接
	Link::Array.erase(std::remove_if(Link::Array.begin(), Link::Array.end(),
		[this](auto& link) { return link->StartPinID == this->ID; }), Link::Array.end());
	this->Links.clear();

	// 生成新的链接
	auto& link = Link::Array.emplace_back(std::make_unique<Link>(MainWindow::GetNextId(), this->ID, pin->ID));
	if (pin->Node)
		this->SetValue(pin->Node->Name);
	this->Links[link->ID] = link.get();
	pin->Links[link->ID] = link.get();
	return link.get();
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
		return dynamic_cast<SectionNode*>(pNode);

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

	if (auto sectionNode = dynamic_cast<SectionNode*>(this->Node)) {
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