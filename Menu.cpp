#include "MainWindow.h"
#include "PinType.h"
#include "nodes/SectionNode.h"
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>

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
	else if (ImGui::MenuItem("Group"))
		node = (Node*)SpawnCommentNode();
	ImGui::Separator();
	m_TemplateManager.ShowCreationMenu([this, &node](auto&&... args) {
		node = SpawnNodeFromTemplate(std::forward<decltype(args)>(args)...);
	});

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
					m_Links.back().TypeIdentifier = startPin->GetLinkType();

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
		ImGui::Text("Type: %s", node->Type == NodeType::Section ? "Section" : "Unexcepted");
		ImGui::Text("Inputs: %d", (int)node->Inputs.size());
		ImGui::Text("Outputs: %d", (int)node->Outputs.size());
	}
	else
		ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
	ImGui::Separator();

	if (node && node->Type == NodeType::Section) {
		if (ImGui::MenuItem("Section Editor")) {
			m_ShowSectionEditor = true;
			m_SectionEditorNode = reinterpret_cast<SectionNode*>(node);
		}
		if (!node->IsComment && ImGui::MenuItem("Hide"))
			node->IsComment = true;
		if (node->IsComment && ImGui::MenuItem("Unhide"))
			node->IsComment = false;
	}

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
							[&key](const SectionNode::KeyValuePair& kv) { return kv.Key == key; });

						if (it != m_SectionEditorNode->KeyValues.end()) {
							// Key exists, just update the value
							it->Value = value;
						}
						else {
							// Key doesn't exist, add new entry
							m_SectionEditorNode->KeyValues.emplace_back(key, value, Pin(GetNextId(), key.c_str()));
							auto& kv = m_SectionEditorNode->KeyValues.back();
							kv.OutputPin.Node = m_SectionEditorNode;
							kv.OutputPin.Kind = PinKind::Output;
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
				std::vector<SectionNode::KeyValuePair> orderedKeyValues;
				for (const auto& key : order) {
					auto it = std::find_if(m_SectionEditorNode->KeyValues.begin(), m_SectionEditorNode->KeyValues.end(),
										   [&key](const SectionNode::KeyValuePair& kv) { return kv.Key == key; });
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
