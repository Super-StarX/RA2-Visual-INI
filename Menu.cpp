#include "MainWindow.h"
#include "PinType.h"
#include "nodes/SectionNode.h"
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>

void MainWindow::Menu() {
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId)) {
		auto pin = FindPin(contextPinId);
		if (pin && pin->Node->Type == NodeType::Section) {
			auto sectionNode = reinterpret_cast<SectionNode*>(pin->Node);
			auto it = std::find_if(sectionNode->KeyValues.begin(), sectionNode->KeyValues.end(),
			   [pin](const KeyValue& kv) { return kv.OutputPin.get() == pin; });

			if (it == sectionNode->KeyValues.end() || !it->IsFolded)
				ImGui::OpenPopup("Pin Context Menu");
		}
	}
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

	// 悬停节点提示处理
	static Node* lastHoveredNode = nullptr;
	Node* hoveredNode = GetHoverNode();
	if (hoveredNode) {
		if (hoveredNode != lastHoveredNode) {
			hoveredNode->HoverTimer = 0.0f;
			lastHoveredNode = hoveredNode;
		}
		hoveredNode->HoverTimer += ImGui::GetIO().DeltaTime;
		if (hoveredNode->HoverTimer > 0.5f)
			ShowNodeTypeTooltip(hoveredNode);
	}
	else
		lastHoveredNode = nullptr;
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
	else if (ImGui::MenuItem("Group"))
		node = (Node*)SpawnGroupNode();
	else if (ImGui::MenuItem("InputTag"))
		node = (Node*)SpawnTagNode(true);
	else if (ImGui::MenuItem("OutputTag"))
		node = (Node*)SpawnTagNode(false);
	else if (ImGui::MenuItem("Comment"))
		node = (Node*)SpawnCommentNode();
	ImGui::Separator();
	m_TemplateManager.ShowCreationMenu([this, &node](auto&&... args) {
		node = SpawnNodeFromTemplate(std::forward<decltype(args)>(args)...);
	});

	if (node) {
		BuildNodes();

		createNewNode = false;
		
		const ImVec2 canvasPos = ed::ScreenToCanvas(openPopupPosition);
		ed::SetNodePosition(node->ID, canvasPos);

		if (auto startPin = newNodeLinkPin) {
			if (startPin->Node->Type == NodeType::Section) {
				auto sectionNode = reinterpret_cast<SectionNode*>(node);
				auto pin = startPin->Kind == PinKind::Input ? sectionNode->OutputPin.get() : sectionNode->InputPin.get();
				if (startPin->Kind == PinKind::Input)
					std::swap(startPin, pin);
				if (Pin::CanCreateLink(startPin, pin))
					CreateLink(startPin, pin)->TypeIdentifier = startPin->GetLinkType();
			}
			else {
				auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;
				for (auto& pin : pins) {
					if (Pin::CanCreateLink(startPin, &pin)) {
						auto endPin = &pin;
						if (startPin->Kind == PinKind::Input)
							std::swap(startPin, endPin);
						CreateLink(startPin, endPin)->TypeIdentifier = startPin->GetLinkType();
						break;
					}
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
		node->Menu();
		if (ImGui::MenuItem("Section Editor")) {
			m_ShowSectionEditor = true;
			m_SectionEditorNode = reinterpret_cast<SectionNode*>(node);
		}
	}
	else
		ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
	ImGui::Separator();

	if (ImGui::MenuItem("Delete"))
		ed::DeleteNode(contextNodeId);
	ImGui::EndPopup();
}

void MainWindow::ShowSectionEditor() {
	if (!m_ShowSectionEditor) return;

	static std::string textBuffer;  // 临时缓冲区
	static std::string titleBuffer; // 临时缓冲区

	// 将编辑器内容加载到缓冲区
	if (titleBuffer.empty()) {
		titleBuffer = m_SectionEditorNode->Name;
	}

	if (textBuffer.empty()) {
		for (auto& output : m_SectionEditorNode->KeyValues)
			textBuffer += output.Key + "=" + output.Value + "\n";
	}

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Section Editor", &m_ShowSectionEditor, ImGuiWindowFlags_AlwaysAutoResize)) {
		//ImGui::Text("Edit:");

		// 创建多行文本框
		ImGui::InputText("##SectionEditorTitle", &titleBuffer, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::InputTextMultiline("##SectionEditorTextEditor", &textBuffer, ImVec2(-1, 200));

		if (ImGui::Button("Save")) {
			{
				std::istringstream stream(textBuffer);
				std::string line;
				std::unordered_map<std::string, bool> seenKeys;
				std::vector<std::string> order;

				// First, let's handle the new lines and update or add KeyValues
				while (std::getline(stream, line)) {
					auto delimiter = line.find('=');
					if (delimiter != std::string::npos) {
						auto key = line.substr(0, delimiter);
						auto value = line.substr(delimiter + 1);

						seenKeys[key] = true;
						order.push_back(key); // Record the order of keys

						// Check if key already exists in KeyValues
						auto it = std::find_if(m_SectionEditorNode->KeyValues.begin(), m_SectionEditorNode->KeyValues.end(),
							[&key](const KeyValue& kv) { return kv.Key == key; });

						if (it != m_SectionEditorNode->KeyValues.end()) {
							// Key exists, just update the value
							it->Value = value;
						}
						else {
							// Key doesn't exist, add new entry
							m_SectionEditorNode->AddKeyValue(key, value);
						}
					}
				}

				// Second, remove any keys that are not present in textBuffer
				auto it = m_SectionEditorNode->KeyValues.begin();
				while (it != m_SectionEditorNode->KeyValues.end()) {
					if (seenKeys.find(it->Key) == seenKeys.end()) {
						it = m_SectionEditorNode->KeyValues.erase(it); // Remove key if not found in textBuffer
					}
					else {
						++it;
					}
				}

				// Now reorder KeyValues to match the order in the textBuffer
				std::vector<KeyValue> orderedKeyValues;
				for (const auto& key : order) {
					auto it = std::find_if(m_SectionEditorNode->KeyValues.begin(), m_SectionEditorNode->KeyValues.end(),
										   [&key](const KeyValue& kv) { return kv.Key == key; });
					if (it != m_SectionEditorNode->KeyValues.end()) {
						orderedKeyValues.push_back(*it); // Add the element in the correct order
					}
				}

				// Update KeyValues to match the order
				m_SectionEditorNode->KeyValues = std::move(orderedKeyValues);
			}
			m_SectionEditorNode->Name = titleBuffer;
			textBuffer.clear();
			titleBuffer.clear();
			m_ShowSectionEditor = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			textBuffer.clear();
			titleBuffer.clear();
			m_ShowSectionEditor = false;
		}
	}
	ImGui::End();
}

void MainWindow::PinMenu() {
	auto pin = FindPin(contextPinId);

	ImGui::TextUnformatted("Pin Context Menu");
	ImGui::Separator();

	if (pin) {
		pin->Menu();
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
		PinTypeManager::Menu();
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

void MainWindow::ShowNodeTypeTooltip(Node* node) {
	ImGui::BeginTooltip();

	// 类型名称
	ImGui::Text("Type:%s", node->TypeName.c_str());

	// 类型详细信息
	auto typeInfo = TypeSystem::Get().GetTypeInfo(node->TypeName);
	switch (typeInfo.Category) {
	case TypeCategory::NumberLimit:
		ImGui::Separator();
		ImGui::Text("Value Range:");
		ImGui::BulletText("%d to %d",
			std::get<NumberLimit>(typeInfo.Data).Min,
			std::get<NumberLimit>(typeInfo.Data).Max);
		break;

	case TypeCategory::List:
		ImGui::Separator();
		ImGui::Text("List Properties:");
		ImGui::BulletText("Element: %s", std::get<ListType>(typeInfo.Data).ElementType.c_str());
		ImGui::BulletText("Length: %d-%d",
			std::get<ListType>(typeInfo.Data).MinLength,
			std::get<ListType>(typeInfo.Data).MaxLength);
		break;

	case TypeCategory::StringLimit:
		if (!std::get<StringLimit>(typeInfo.Data).ValidValues.empty()) {
			ImGui::Separator();
			ImGui::Text("Valid Options:");
			for (auto& val : std::get<StringLimit>(typeInfo.Data).ValidValues) {
				ImGui::BulletText("%s", val.c_str());
			}
		}
		break;
	}

	ImGui::EndTooltip();
}