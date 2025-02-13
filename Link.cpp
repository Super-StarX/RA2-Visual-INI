#include "Link.h"
#include "LinkType.h"
#include "MainWindow.h"

Link::~Link() {
	if (auto pin = MainWindow::Instance->FindPin(StartPinID))
		pin->Links.erase(ID);

	if (auto pin = MainWindow::Instance->FindPin(EndPinID))
		pin->Links.erase(ID);
}

void Link::Draw() const {
	auto* typeInfo = LinkTypeManager::Get().FindType(TypeIdentifier);
	if (!typeInfo) return;

	ed::Link(ID, StartPinID, EndPinID, typeInfo->Color, typeInfo->Thickness);
}

void Link::Menu() {
	// 显示当前类型
	if (auto* currentType = LinkTypeManager::Get().FindType(TypeIdentifier)) {
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
		for (const auto& type : LinkTypeManager::Get().GetAllTypes()) {
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