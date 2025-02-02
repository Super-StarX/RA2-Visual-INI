#include "MainWindow.h"
#include "PinTypeManager.h"

void MainWindow::Menu() {
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId))
		ImGui::OpenPopup("Pin Context Menu");
	else if (ed::ShowLinkContextMenu(&contextLinkId))
		ImGui::OpenPopup("Link Context Menu");
	else if (ed::ShowBackgroundContextMenu()) {
		ImGui::OpenPopup("Create New Node");
		newNodeLinkPin = nullptr;
	}
	ed::Resume();

	ed::Suspend();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("Node Context Menu"))
		NodeMenu();
	if (ImGui::BeginPopup("Pin Context Menu"))
		PinMenu();
	if (ImGui::BeginPopup("Link Context Menu"))
		LinkMenu();
	if (ImGui::BeginPopup("Create New Node"))
		LayerMenu();
	else
		createNewNode = false;
	ImGui::PopStyleVar();
	ed::Resume();
}

void MainWindow::LayerMenu() {
	auto openPopupPosition = ImGui::GetMousePos();
	//ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

	//auto drawList = ImGui::GetWindowDrawList();
	//drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

	Node* node = nullptr;
	if (ImGui::MenuItem("Section"))
		node = (Node*)SpawnSectionNode();
	ImGui::Separator();

	if (node) {
		BuildNodes();

		createNewNode = false;

		ed::SetNodePosition(node->ID, openPopupPosition);

		if (auto startPin = newNodeLinkPin) {
			auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

			for (auto& pin : pins) {
				if (Pin::CanCreateLink(startPin, &pin)) {
					auto endPin = &pin;
					if (startPin->Kind == PinKind::Input)
						std::swap(startPin, endPin);

					m_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
					m_Links.back().Color = startPin->GetIconColor();

					break;
				}
			}
		}
	}

	ImGui::EndPopup();
}

void MainWindow::NodeMenu() {
	auto node = FindNode(contextNodeId);

	ImGui::TextUnformatted("Node Context Menu");
	ImGui::Separator();
	if (node) {
		ImGui::Text("ID: %p", node->ID.AsPointer());
		ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
		ImGui::Text("Inputs: %d", (int)node->Inputs.size());
		ImGui::Text("Outputs: %d", (int)node->Outputs.size());
	}
	else
		ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		ed::DeleteNode(contextNodeId);
	ImGui::EndPopup();
}

void MainWindow::PinMenu() {
	auto pin = FindPin(contextPinId);

	ImGui::TextUnformatted("Pin Context Menu");
	ImGui::Separator();

	if (pin) {
		// 显示当前类型
		if (auto* currentType = PinTypeManager::Get().FindType(pin->TypeIdentifier)) {
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
					pin->TypeIdentifier = type.Identifier;
					// 标记节点需要刷新
					// 通过微小位置变化强制刷新
					auto pos = pin->Node->GetPosition();
					ed::SetNodePosition(pin->Node->ID, ImVec2(pos.x + 0.1f, pos.y + 0.1f));
					ed::SetNodePosition(pin->Node->ID, pos);
				}

				// 在菜单项显示颜色标记
				ImGui::SameLine();
				ImGui::ColorButton(("##color_" + type.Identifier).c_str(),
					type.Color, ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
			}
			ImGui::EndMenu();
		}

		// 自定义类型管理
		if (ImGui::MenuItem("Manage Custom Types..."))
			m_ShowPinTypeEditor = true;
	}
	else {
		ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());
	}

	ImGui::EndPopup();
}

// 类型编辑器窗口
void MainWindow::ShowPinTypeEditor() {
	if (!m_ShowPinTypeEditor) return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Pin Type Manager", &m_ShowPinTypeEditor)) {
		static char newIdentifier[128] = "";
		static char newDisplayName[128] = "";
		static ImColor newColor = ImColor(255, 255, 255);
		static int newIconType = 0;

		// 添加新类型
		ImGui::InputText("Identifier", newIdentifier, IM_ARRAYSIZE(newIdentifier));
		ImGui::InputText("Display Name", newDisplayName, IM_ARRAYSIZE(newDisplayName));
		ImGui::ColorEdit4("Color", &newColor.Value.x);
		ImGui::Combo("Icon", &newIconType, "Circle\0Square\0Triangle\0Diamond\0");

		if (ImGui::Button("Add New Type") && newIdentifier[0] != '\0') {
			PinTypeInfo newType;
			newType.Identifier = newIdentifier;
			newType.DisplayName = newDisplayName[0] ? newDisplayName : newIdentifier;
			newType.Color = newColor;
			newType.IconType = newIconType;
			newType.IsUserDefined = true;

			PinTypeManager::Get().AddCustomType(newType);

			// 清空输入
			memset(newIdentifier, 0, sizeof(newIdentifier));
			memset(newDisplayName, 0, sizeof(newDisplayName));
			newColor = ImColor(255, 255, 255);
		}

		ImGui::Separator();

		// 现有类型列表
		ImGui::Text("Existing Types:");
		ImGui::BeginChild("##type_list", ImVec2(0, 200), true);
		for (const auto& type : PinTypeManager::Get().GetAllTypes()) {
			ImGui::ColorButton("##color", type.Color,
				ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
			ImGui::SameLine();

			if (type.IsUserDefined) {
				if (ImGui::Button(("X##del_" + type.Identifier).c_str())) {
					PinTypeManager::Get().RemoveCustomType(type.Identifier);
				}
				ImGui::SameLine();
			}

			ImGui::Text("%s (%s)", type.DisplayName.c_str(),
				type.Identifier.c_str());
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void MainWindow::LinkMenu() {
	auto link = FindLink(contextLinkId);

	ImGui::TextUnformatted("Link Context Menu");
	ImGui::Separator();
	if (link)
		link->Menu();
	else
		ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		ed::DeleteLink(contextLinkId);
	ImGui::EndPopup();
}
