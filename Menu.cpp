#include "MainWindow.h"
#include "Utils.h"
#include "LinkStyle.h"
#include "Nodes/BlueprintNode.h"
#include "Nodes/CommentNode.h"
#include "Nodes/GroupNode.h"
#include "Nodes/HoudiniNode.h"
#include "Nodes/ListNode.h"
#include "Nodes/ModuleNode.h"
#include "Nodes/SectionNode.h"
#include "Nodes/SimpleNode.h"
#include "Nodes/TagNode.h"
#include "Nodes/TreeNode.h"
#include "Nodes/IONode.h"
#include "Nodes/NodeStyle.h"
#include "Pins/PinStyle.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include "Log.h"
#include <sstream>

static bool createNewNode = false;
static ed::NodeId contextNodeId;
static ed::LinkId contextLinkId;
static ed::PinId  contextPinId;
static Pin* newNodeLinkPin = nullptr;
static ImVec2 newNodePosition;
static bool m_ShowNodeStyleEditor{ false };
static bool m_ShowPinStyleEditor{ false };
static bool m_ShowLinkStyleEditor{ false };
static SectionNode* m_SectionEditorNode{ nullptr };
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
		newNodePosition = ImGui::GetMousePos(); // 记录右键点击位置
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
	// 添加节点菜单
	ImGui::TextUnformatted(LOCALE["Create Menu"]);
	ImGui::Separator();
	if (auto node = CreateNodeByName()) {
		createNewNode = false;
		const ImVec2 canvasPos = ed::ScreenToCanvas(newNodePosition);
		ed::SetNodePosition(node->ID, canvasPos);
		if (auto startPin = newNodeLinkPin)
			if (auto pin = node->GetFirstCompatiblePin(startPin))
				if (startPin->CanCreateLink(pin))
					startPin->LinkTo(pin)->TypeIdentifier = startPin->GetLinkType();
		LOG_INFO("创建新空白节点，节点类型为{}", static_cast<int>(node->GetNodeType()));
	}

	// 添加模板菜单
	ImGui::Separator();
	m_TemplateManager.ShowCreationMenu([this](auto&&... args) {
		SpawnNodeFromTemplate(std::forward<decltype(args)>(args)...);
	});

	// 添加模块菜单
	ImGui::Separator();
	AddModuleMenuItems("Modules");

	ImGui::EndPopup();
}

// 递归添加模块菜单项
void MainWindow::AddModuleMenuItems(const std::string& path) {
	try {
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			std::string filename = entry.path().filename().string();

			if (entry.is_directory()) {
				// 递归处理子目录，创建子菜单
				if (ImGui::BeginMenu(filename.c_str())) {
					AddModuleMenuItems(entry.path().string());
					ImGui::EndMenu();
				}
			}
			else if (entry.path().extension() == ".viproj") {
				// 处理.viproj文件
				if (ImGui::MenuItem(filename.c_str())) {
					auto moduleNode = Node::Create<ModuleNode>();
					moduleNode->LoadProject(entry.path().string());

					// 设置节点位置到右键点击位置
					const ImVec2 canvasPos = ed::ScreenToCanvas(newNodePosition);
					ed::SetNodePosition(moduleNode->ID, canvasPos);
				}
			}
		}
	}
	catch (const std::exception& e) {
		LOG_ERROR("模块加载失败: {}", e.what());
	}
}

void MainWindow::NodeMenu() {
	auto node = Node::Get(contextNodeId);

	ImGui::TextUnformatted(LOCALE["Node Context Menu"]);
	ImGui::Separator();
	if (node) {
		node->Menu();
		if (ImGui::MenuItem(LOCALE["Section Editor"])) {
			m_ShowSectionEditor = true;
			m_SectionEditorNode = reinterpret_cast<SectionNode*>(node);
		}
		if (ImGui::MenuItem("Manage Custom Styles..."))
			m_ShowNodeStyleEditor = true;
	}
	else
		ImGui::Text("%s: %p", LOCALE["Unknown node"], contextNodeId.AsPointer());
	ImGui::Separator();

	if (ImGui::MenuItem(LOCALE["Delete"]))
		ed::DeleteNode(contextNodeId);
	ImGui::EndPopup();
}

void MainWindow::ShowNodeStyleEditor() {
	if (!m_ShowNodeStyleEditor) return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Node Sytle Manager", &m_ShowNodeStyleEditor)) {
		NodeStyleManager::Menu();
	}
	ImGui::End();
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
		while (ed::QueryDeletedNode(&nodeId))
			if (ed::AcceptDeletedItem())
				std::erase_if(Node::Array, [nodeId](auto& node) { return node->ID == nodeId; });

		ed::LinkId linkId = 0;
		while (ed::QueryDeletedLink(&linkId))
			if (ed::AcceptDeletedItem())
				std::erase_if(Link::Array, [linkId](auto& link) { return link->ID == linkId; });
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
			textBuffer += output->Key + "=" + output->GetValue() + "\n";

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
							it->get()->SetValue(value); // Key exists, just update the value
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
		if (ImGui::MenuItem("Manage Custom Styles..."))
			m_ShowPinStyleEditor = true;
	}
	else {
		ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());
	}

	ImGui::EndPopup();
}

// 类型编辑器窗口
void MainWindow::ShowPinStyleEditor() {
	if (!m_ShowPinStyleEditor) return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Pin Sytle Manager", &m_ShowPinStyleEditor)) {
		PinStyleManager::Menu();
	}
	ImGui::End();
}

void MainWindow::ShowListEditor() {
	if (!m_ShowListEditor) return;

	if (ImGui::Begin("List Editor", &m_ShowListEditor,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		auto& editBuffer = KeyValue::EditBuffer;
		auto& editType = KeyValue::EditType;
		auto Pin = KeyValue::EditPin;

		if (Pin == nullptr) {
			m_ShowListEditor = false;
			return;
		}

		auto elements = Utils::SplitString(editBuffer, ',');

		// 自动填充/截断
		if (elements.size() < editType.MinLength)
			elements.resize(editType.MinLength);
		else if (elements.size() > editType.MaxLength)
			elements.resize(editType.MaxLength);

		// 元素类型信息
		TypeInfo elemType = TypeSystem::Get().GetTypeInfo(editType.ElementType);
		
		if (auto kv = dynamic_cast<KeyValue*>(Pin)) {
			ImGui::PushID(Pin->ID.AsPointer());
			std::string title = kv->Node->Name + " - " + kv->Key;
			ImGui::TextUnformatted(title.c_str());
			ImGui::PopID();
		}

		// 动态绘制元素
		bool modified = false;
		for (size_t i = 0; i < elements.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			ImGui::Text("Item %d:", static_cast<int>(i + 1));
			ImGui::SameLine();

			std::string elemValue = elements[i];
			if (KeyValue::DrawElementEditor(elemValue, elemType)) {
				elements[i] = elemValue;
				modified = true;
			}
			ImGui::PopID();
		}

		// 长度控制按钮
		if (elements.size() < editType.MaxLength) {
			if (ImGui::Button("+ Add Item")) {
				elements.emplace_back("");
				modified = true;
			}
		}
		if (elements.size() > editType.MinLength) {
			ImGui::SameLine();
			if (ImGui::Button("- Remove Last")) {
				elements.pop_back();
				modified = true;
			}
		}

		// 确认操作
		ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			Pin->SetValue(editBuffer);
			KeyValue::EditPin = nullptr;
			m_ShowListEditor = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			KeyValue::EditPin = nullptr;
			m_ShowListEditor = false;
		}

		if (modified)
			editBuffer = Utils::JoinStrings(elements, ",");

		ImGui::End();
	}
}


void MainWindow::LinkMenu() {
	auto link = Link::Get(contextLinkId);

	ImGui::TextUnformatted("Link Context Menu");
	ImGui::Separator();
	if (link) {
		link->Menu();
		if (ImGui::MenuItem("Manage Custom Styles..."))
			m_ShowLinkStyleEditor = true;
	}
	else
		ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
	ImGui::Separator();
	if (ImGui::MenuItem("Delete"))
		ed::DeleteLink(contextLinkId);
	ImGui::EndPopup();
}

// 类型编辑器窗口
void MainWindow::ShowLinkStyleEditor() {
	if (!m_ShowLinkStyleEditor) return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Link Sytle Manager", &m_ShowLinkStyleEditor)) {
		LinkStyleManager::Menu();
	}
	ImGui::End();
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
				ImGui::BeginTooltip();
				hoveredPin->Tooltip();
				ImGui::EndTooltip();
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
				ImGui::BeginTooltip();
				hoveredNode->Tooltip();
				ImGui::EndTooltip();
			}
		}
	}
	// 如果既没有悬浮在 Pin 上，也没有悬浮在 Node 上
	else {
		lastHoveredNode = nullptr;
		lastHoveredPin = nullptr;
	}
}

void MainWindow::ShowTypeEnumPopup() {
	if (!m_ShowTypeEnumPopup)
		return;

	if(!ImGui::IsPopupOpen("Type Enum Popup"))
		ImGui::OpenPopup("Type Enum Popup");

	if (ImGui::BeginPopup("Type Enum Popup")) {
		auto& typeSystem = TypeSystem::Get();
		auto& type = m_TypeEnumPopupType;

		for (auto& item : std::get<StringLimit>(type.Data).ValidValues) {
			if (ImGui::MenuItem(item.c_str())) {
				m_TypeEnumPopupPin->SetValue(item);
				m_TypeEnumPopupPin->UpdateOutputLink("");
				m_ShowTypeEnumPopup = false;
				m_TypeEnumPopupType = {};
				m_TypeEnumPopupPin = nullptr;
				break;
			}
		}

		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0)) {
			m_ShowTypeEnumPopup = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}