#include "MainWindow.h"
#include "Utils.h"
#include "Pins/PinType.h"
#include "Pins/KeyValue.h"
#include "Nodes/SectionNode.h"
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>

static bool createNewNode = false;
static ed::NodeId contextNodeId;
static ed::LinkId contextLinkId;
static ed::PinId  contextPinId;
static Pin* newNodeLinkPin = nullptr;
void MainWindow::Menu() {
	ed::Suspend();
	if (ed::ShowNodeContextMenu(&contextNodeId))
		ImGui::OpenPopup("Node Context Menu");
	else if (ed::ShowPinContextMenu(&contextPinId)) {
		auto pin = Pin::Get(contextPinId);
		auto kv = dynamic_cast<KeyValue*>(pin);
		if (!kv || !kv->IsFolded)
			ImGui::OpenPopup("Pin Context Menu");
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

	ToolTip();
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
	else if (ImGui::MenuItem("ListNode"))
		node = (Node*)SpawnListNode();
	ImGui::Separator();
	m_TemplateManager.ShowCreationMenu([this, &node](auto&&... args) {
		node = SpawnNodeFromTemplate(std::forward<decltype(args)>(args)...);
	});

	if (node) {
		createNewNode = false;
		
		const ImVec2 canvasPos = ed::ScreenToCanvas(openPopupPosition);
		ed::SetNodePosition(node->ID, canvasPos);

		if (auto startPin = newNodeLinkPin)
			if(auto pin = node->GetFirstCompatiblePin(startPin))
				if (startPin->CanCreateLink(pin))
					startPin->LinkTo(pin)->TypeIdentifier = startPin->GetLinkType();
	}

	ImGui::EndPopup();
}

void MainWindow::NodeMenu() {
	auto node = Node::Get(contextNodeId);

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

void MainWindow::NodeEditor() {
	auto cursorTopLeft = ImGui::GetCursorScreenPos();

	if (createNewNode) {
		ImGui::SetCursorScreenPos(cursorTopLeft);
		return;
	}

	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f)) {
		ed::PinId startPinId = 0, endPinId = 0;
		if (ed::QueryNewLink(&startPinId, &endPinId)) {
			auto startPin = Pin::Get(startPinId);
			auto endPin = Pin::Get(endPinId);

			newLinkPin = startPin ? startPin : endPin;

			if (startPin && startPin->Kind == PinKind::Input) {
				std::swap(startPin, endPin);
				std::swap(startPinId, endPinId);
			}

			if (startPin && endPin) {
				if (endPin == startPin)
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				else if (endPin->Kind == startPin->Kind) {
					Utils::DrawTextOnCursor("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				//else if (endPin->Node == startPin->Node)
				//{
				//    Utils::DrawTextOnCursor("x Cannot connect to self", ImColor(45, 32, 32, 180));
				//    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
				//}
				else if (endPin->TypeIdentifier != startPin->TypeIdentifier) {
					Utils::DrawTextOnCursor("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
				}
				else {
					Utils::DrawTextOnCursor("+ Create Link", ImColor(32, 45, 32, 180));
					if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
						startPin->LinkTo(endPin)->TypeIdentifier = startPin->GetLinkType();
				}
			}
		}

		ed::PinId pinId = 0;
		if (ed::QueryNewNode(&pinId)) {
			newLinkPin = Pin::Get(pinId);
			if (newLinkPin)
				Utils::DrawTextOnCursor("+ Create Node", ImColor(32, 45, 32, 180));

			if (ed::AcceptNewItem()) {
				createNewNode = true;
				newNodeLinkPin = Pin::Get(pinId);
				newLinkPin = nullptr;
				ed::Suspend();
				ImGui::OpenPopup("Create New Node");
				ed::Resume();
			}
		}
	}
	else
		newLinkPin = nullptr;

	ed::EndCreate();

	if (ed::BeginDelete()) {
		ed::NodeId nodeId = 0;
		while (ed::QueryDeletedNode(&nodeId)) {
			if (ed::AcceptDeletedItem()) {
				auto id = std::find_if(Node::Array.begin(), Node::Array.end(), [nodeId](auto& node) { return node->ID == nodeId; });
				if (id != Node::Array.end())
					Node::Array.erase(id);
			}
		}

		ed::LinkId linkId = 0;
		while (ed::QueryDeletedLink(&linkId)) {
			if (ed::AcceptDeletedItem()) {
				auto id = std::find_if(Link::Array.begin(), Link::Array.end(), [linkId](auto& link) { return link->ID == linkId; });
				if (id != Link::Array.end())
					Link::Array.erase(id);
			}
		}
	}
	ed::EndDelete();
	ImGui::SetCursorScreenPos(cursorTopLeft);
}

void MainWindow::ShowSectionEditor() {
	if (!m_ShowSectionEditor) return;

	static std::string textBuffer;  // 临时缓冲区
	static std::string titleBuffer; // 临时缓冲区

	// 将编辑器内容加载到缓冲区
	if (titleBuffer.empty())
		titleBuffer = m_SectionEditorNode->Name;

	if (textBuffer.empty())
		for (auto& output : m_SectionEditorNode->KeyValues)
			textBuffer += output->Key + "=" + output->Value + "\n";

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
						auto it = m_SectionEditorNode->FindPin(key);
						if (it != m_SectionEditorNode->KeyValues.end())
							it->get()->Value = value; // Key exists, just update the value
						else
							m_SectionEditorNode->AddKeyValue(key, value); // Key doesn't exist, add new entry
					}
				}

				// Second, remove any keys that are not present in textBuffer
				auto it = m_SectionEditorNode->KeyValues.begin();
				while (it != m_SectionEditorNode->KeyValues.end()) {
					if (seenKeys.find(it->get()->Key) == seenKeys.end())
						it = m_SectionEditorNode->KeyValues.erase(it); // Remove key if not found in textBuffer
					else
						++it;
				}

				// Now reorder KeyValues to match the order in the textBuffer
				std::vector<std::unique_ptr<KeyValue>> orderedKeyValues;
				for (const auto& key : order) {
					auto it = m_SectionEditorNode->FindPin(key);
					if (it != m_SectionEditorNode->KeyValues.end())
						orderedKeyValues.push_back(std::make_unique<KeyValue>(*it->get())); // Add the element in the correct order
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
	auto pin = Pin::Get(contextPinId);

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
	auto link = Link::Get(contextLinkId);

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

void MainWindow::ToolTip() {
	// 悬停节点提示处理
	static Node* lastHoveredNode = nullptr;
	static Pin* lastHoveredPin = nullptr;

	// 判断是否悬浮在 Pin 上
	if (auto hoveredPinId = ed::GetHoveredPin()) {
		auto hoveredPin = Pin::Get(hoveredPinId);
		if (hoveredPin && hoveredPin != lastHoveredPin) {
			hoveredPin->HoverTimer = 0.0f;
			lastHoveredPin = hoveredPin;
			lastHoveredNode = nullptr;
		}
		if (hoveredPin) {
			hoveredPin->HoverTimer += ImGui::GetIO().DeltaTime;
			if (hoveredPin->HoverTimer > 0.5f) {
				hoveredPin->Tooltip();
			}
		}
	}
	// 如果没有悬浮在 Pin 上，再判断是否悬浮在 Node 上
	else if (auto hoveredNodeId = ed::GetHoveredNode()) {
		auto hoveredNode = Node::Get(hoveredNodeId);
		if (hoveredNode && hoveredNode != lastHoveredNode) {
			hoveredNode->HoverTimer = 0.0f;
			lastHoveredNode = hoveredNode;
			lastHoveredPin = nullptr;
		}
		if (hoveredNode) {
			hoveredNode->HoverTimer += ImGui::GetIO().DeltaTime;
			if (hoveredNode->HoverTimer > 0.5f) {
				hoveredNode->Tooltip();
			}
		}
	}
	// 如果既没有悬浮在 Pin 上，也没有悬浮在 Node 上
	else {
		lastHoveredNode = nullptr;
		lastHoveredPin = nullptr;
	}
}
