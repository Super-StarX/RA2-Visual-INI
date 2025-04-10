#include "Link.h"
#include "LinkStyle.h"
#include "MainWindow.h"

std::vector<std::unique_ptr<Link>> Link::Array;

Link::Link(int id, ed::PinId startPinId, ed::PinId endPinId) :
	ID(id), StartPinID(startPinId), EndPinID(endPinId) {
	if (!id)
		ID = MainWindow::GetNextId();
	else
		ID = MainWindow::GetIdOffset() + id;
}

Link::~Link() {
	if (auto pin = Pin::Get(StartPinID))
		pin->Links.erase(ID);

	if (auto pin = Pin::Get(EndPinID))
		pin->Links.erase(ID);
}

Link* Link::Get(ed::LinkId id) {
	for (auto& link : Link::Array)
		if (link->ID == id)
			return link.get();

	return nullptr;
}

void Link::Draw() const {
	auto* typeInfo = LinkStyleManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return;

	ed::Link(ID, StartPinID, EndPinID, typeInfo->Color, typeInfo->Thickness);
}

void Link::Menu() {
	// 显示当前类型
	if (auto* currentType = LinkStyleManager::Get().FindType(TypeIdentifier)) {
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
		for (const auto& type : LinkStyleManager::Get().GetAllTypes()) {
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

void Link::SaveToJson(json& j) const {
	j["ID"] = ID.Get();
	j["StartID"] = StartPinID.Get();
	j["EndID"] = EndPinID.Get();
	j["TypeIdentifier"] = TypeIdentifier;
}

void Link::LoadFromJson(const json& j, bool newId) {
	ID = newId ? MainWindow::GetNextId() : ed::LinkId(j["ID"] + MainWindow::GetIdOffset());
	StartPinID = ed::PinId(j["StartID"] + MainWindow::GetIdOffset());
	EndPinID = ed::PinId(j["EndID"] + MainWindow::GetIdOffset());
	TypeIdentifier = j["TypeIdentifier"];

	if (auto startPin = Pin::Get(StartPinID)) {
		if (auto endPin = Pin::Get(EndPinID)) {
			if (endPin->Node)
				startPin->SetValue(endPin->Node->Name);
			startPin->Links[this->ID] = this;
			endPin->Links[this->ID] = this;
		}
	}
}

void Link::Tooltip() {
	ImGui::BeginTooltip();
	ImGui::Text("Link %d", ID);
	ImGui::Text("Type: %s", TypeIdentifier.c_str());
	ImGui::Text("Start Pin: %d", StartPinID.Get());
	ImGui::Text("End Pin: %d", EndPinID.Get());
	ImGui::EndTooltip();
}
