#include "Pin.h"
#include "PinStyle.h"
#include "Utils.h"
#include "KeyValue.h"
#include "Nodes/SectionNode.h"
#include "MainWindow.h"
#include "utilities/widgets.h"

std::map<ed::PinId, Pin*, ComparePinId> Pin::Array;

Pin::Pin(::Node* node, const char* name, PinKind kind, int id) :
	Node(node), Name(name), Kind(kind){
	if (!id)
		ID = MainWindow::GetNextId();
	else
		ID = MainWindow::GetIdOffset() + id;
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
	ImGui::Text("%s: %d", LOCALE["ID"], ID);
	ImGui::Text("%s: %s", LOCALE["Pin Style"], PinStyleManager::Get().FindType(TypeIdentifier)->DisplayName);
	ImGui::Text("%s: %d", LOCALE["Pin Links"], Links.size());
}

void Pin::SetValue(const std::string& str) {
	Name = str; 
}

std::string Pin::GetValue() const {
	if (Kind == PinKind::Output)
		if (auto pNode = GetLinkedNode())
			return pNode->GetValue(GetLinkedPin());

	return Name;
}

void Pin::SaveToJson(json& j) const {
	j["ID"] = ID.Get();
	j["Name"] = Name;
	j["TypeIdentifier"] = TypeIdentifier;
	j["Kind"] = static_cast<int>(Kind);
}

void Pin::LoadFromJson(const json& j, bool newId) {
	Array.erase(ID);
	ID = newId ? MainWindow::GetNextId() : ed::PinId(j["ID"] + MainWindow::GetIdOffset());
	Array[ID] = this;
	Name = j["Name"];
	TypeIdentifier = j["TypeIdentifier"];
	Kind = static_cast<PinKind>(j["Kind"]);
}

void Pin::UpdateOutputLink(std::string value) {
	// 值变化后,判断自己连着的node的名字是否还是自己的值
	// 如果不是的话,就断开当前链接,并遍历node::array寻找是否有新的node可以链接
	// 如果value是空的，只断开链接
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

	if (value.empty())
		return;

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
	if (this->Node->PinNameChangable() && pin->Node->PinNameSyncable())
			this->SetValue(pin->Node->Name);
	this->Links[link->ID] = link.get();
	pin->Links[link->ID] = link.get();
	return link.get();
}

Pin* Pin::GetLinkedPin() const {
	if (!IsLinked())
		return nullptr;

	for (auto& [_, link] : Links)
		if (auto otherPin = link->StartPinID == ID ? Get(link->EndPinID) : Get(link->StartPinID))
			return otherPin;

	return nullptr;
}

Node* Pin::GetLinkedNode() const {
	if (auto pPin = GetLinkedPin())
		return pPin->Node;

	return nullptr;
}

SectionNode* Pin::GetLinkedSection() const {
	if (auto pNode = GetLinkedNode())
		return dynamic_cast<SectionNode*>(pNode);

	return nullptr;
}

ImColor Pin::GetIconColor() const {
	auto* typeInfo = PinStyleManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return ImColor(255, 255, 255);

	return typeInfo->Color;
}

std::string Pin::GetLinkType() const {
	auto* typeInfo = PinStyleManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return std::string();

	return typeInfo->LinkType;
}

void Pin::Menu() {
	// 显示当前类型
	if (auto* currentType = PinStyleManager::Get().FindType(TypeIdentifier)) {
		ImGui::Text("%s: ",LOCALE["Current Style"]);
		ImGui::SameLine();
		ImGui::TextColored(currentType->Color, "%s",
			currentType->DisplayName.c_str());
		ImGui::SameLine();
		ImGui::ColorButton("##color", currentType->Color,
			ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
	}

	ImGui::Separator();

	// 类型选择菜单
	if (ImGui::BeginMenu(LOCALE["Change Style"])) {
		for (const auto& type : PinStyleManager::Get().GetAllTypes()) {
			if (ImGui::MenuItem(type.DisplayName.c_str()))
				TypeIdentifier = type.Identifier;

			// 在菜单项显示颜色标记
			ImGui::SameLine();
			ImGui::ColorButton(("##color_" + type.Identifier).c_str(),
				type.Color, ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
		}
		ImGui::EndMenu();
	}
}

float Pin::GetAlpha() {
	auto alpha = ImGui::GetStyle().Alpha;
	if (MainWindow::newLinkPin && !this->CanCreateLink(MainWindow::newLinkPin) && this != MainWindow::newLinkPin)
		alpha *= 48.0f / 255.0f;
	return alpha;
}

void Pin::DrawPinIcon(bool connected, int alpha, bool isReverse) const {
	auto* typeInfo = PinStyleManager::Get().FindType(TypeIdentifier);

	using namespace ax::Widgets;
	ImColor color = typeInfo->Color;
	color.Value.w = alpha / 255.0f;

	Icon(ImVec2(IconSize, IconSize), IconType(typeInfo->IconType), connected, color, ImColor(32, 32, 32, alpha), isReverse);
};