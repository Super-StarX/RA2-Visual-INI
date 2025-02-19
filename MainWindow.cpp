#define IMGUI_DEFINE_MATH_OPERATORS
#include "MainWindow.h"
#include "LeftPanelClass.h"
#include "PinType.h"
#include "Utils.h"
#include "nodes/SectionNode.h"

#include <imgui_internal.h>

static ed::EditorContext* m_Editor = nullptr;
MainWindow* MainWindow::Instance = nullptr;
int MainWindow::m_NextId = 1;
ed::NodeId MainWindow::contextNodeId = 0;
ed::LinkId MainWindow::contextLinkId = 0;
ed::PinId  MainWindow::contextPinId = 0;
bool MainWindow::createNewNode = false;
Pin* MainWindow::newNodeLinkPin = nullptr;
Pin* MainWindow::newLinkPin = nullptr;

float MainWindow::leftPaneWidth = 400.0f;
float MainWindow::rightPaneWidth = 800.0f;

void MainWindow::ClearAll() {
	Node::Array.clear();
	m_Links.clear();
	SectionNode::Map.clear();
}

float MainWindow::GetTouchProgress(ed::NodeId id) {
	auto it = m_NodeTouchTime.find(id);
	if (it != m_NodeTouchTime.end() && it->second > 0.0f)
		return (m_TouchTime - it->second) / m_TouchTime;
	else
		return 0.0f;
}

void MainWindow::TouchNode(ed::NodeId id) {
	m_NodeTouchTime[id] = m_TouchTime; 
}

void MainWindow::UpdateTouch() {
	const auto deltaTime = ImGui::GetIO().DeltaTime;
	for (auto& entry : m_NodeTouchTime) {
		if (entry.second > 0.0f)
			entry.second -= deltaTime;
	}
}

Node* MainWindow::FindNode(ed::NodeId id) {
	for (const auto& node : Node::Array)
		if (node->ID == id)
			return node.get();

	return nullptr;
}

Link* MainWindow::FindLink(ed::LinkId id) {
	for (auto& link : m_Links)
		if (link->ID == id)
			return link.get();

	return nullptr;
}

Pin* MainWindow::FindPin(ed::PinId id) {
	if (!id)
		return nullptr;

	return m_Pins.count(id) ? m_Pins.at(id) : nullptr;
}

bool MainWindow::IsPinLinked(ed::PinId id) {
	if (!id)
		return false;

	if (auto pin = FindPin(id))
		return !pin->Links.empty();

	return false;
}

Node* MainWindow::GetLinkedNode(ed::PinId outputPinId) {
	if (!outputPinId)
		return nullptr;

	auto pin = FindPin(outputPinId);
	if(!pin || pin->Links.empty())
		return nullptr;

	auto begin = pin->Links.begin();
	if (auto endpin = FindPin(begin->second->EndPinID))
		return endpin->Node;

	return nullptr;
}

Node* MainWindow::GetHoverNode() {
	return FindNode(ed::GetHoveredNode());
}

Link* MainWindow::CreateLink(Pin* startPin, Pin* endPin) {
	auto& link = m_Links.emplace_back(
		std::make_unique<Link>(GetNextId(), startPin->ID, endPin->ID));
	startPin->Links[link->ID] = link.get();
	endPin->Links[link->ID] = link.get();
	return link.get();
}

void MainWindow::OnStart() {
	ed::Config config;

	config.SettingsFile = "RA2VI.json";

	config.UserPointer = this;

	config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t {
		auto self = static_cast<MainWindow*>(userPointer);

		auto node = self->FindNode(nodeId);
		if (!node)
			return 0;

		if (data != nullptr)
			memcpy(data, node->State.data(), node->State.size());
		return node->State.size();
	};

	config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool {
		auto self = static_cast<MainWindow*>(userPointer);

		auto node = self->FindNode(nodeId);
		if (!node)
			return false;

		node->State.assign(data, size);

		self->TouchNode(nodeId);

		return true;
	};
	PinTypeManager::Get().LoadFromFile("custom_types.json");
	TypeSystem::Get().LoadFromINI("INICodingCheck.ini");
	m_TemplateManager.LoadTemplates("templates.ini");
	m_Editor = ed::CreateEditor(&config);
	ed::SetCurrentEditor(m_Editor);

	auto node1 = SpawnSectionNode("Section A");
	node1->AddKeyValue("key", "Section B");
	auto node2 = SpawnSectionNode("Section B");
	node2->AddKeyValue("key", "Value");
	CreateLink(node1->KeyValues.back().OutputPin.get(), node2->InputPin.get());

	ed::NavigateToContent();

	BuildNodes();

	m_LeftPanel = LeftPanelClass(this);
	//auto& io = ImGui::GetIO();
}

void MainWindow::OnStop() {
	PinTypeManager::Get().SaveToFile("custom_types.json");
	if (m_Editor) {
		ed::DestroyEditor(m_Editor);
		m_Editor = nullptr;
	}
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
			auto startPin = FindPin(startPinId);
			auto endPin = FindPin(endPinId);

			newLinkPin = startPin ? startPin : endPin;

			if (startPin->Kind == PinKind::Input) {
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
					if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
						CreateLink(startPin, endPin)->TypeIdentifier = startPin->GetLinkType();
					}
				}
			}
		}

		ed::PinId pinId = 0;
		if (ed::QueryNewNode(&pinId)) {
			newLinkPin = FindPin(pinId);
			if (newLinkPin)
				Utils::DrawTextOnCursor("+ Create Node", ImColor(32, 45, 32, 180));

			if (ed::AcceptNewItem()) {
				createNewNode = true;
				newNodeLinkPin = FindPin(pinId);
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
				auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto& link) { return link->ID == linkId; });
				if (id != m_Links.end())
					m_Links.erase(id);
			}
		}
	}
	ed::EndDelete();
	ImGui::SetCursorScreenPos(cursorTopLeft);
}

void MainWindow::OnFrame(float deltaTime) {
	UpdateTouch();

	auto& io = ImGui::GetIO();

	ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

	ed::SetCurrentEditor(m_Editor);

	//auto& style = ImGui::GetStyle();

# if 0
	// 在窗口上画等距斜线
	{
		for (auto x = -io.DisplaySize.y; x < io.DisplaySize.x; x += 10.0f) {
			ImGui::GetWindowDrawList()->AddLine(ImVec2(x, 0), ImVec2(x + io.DisplaySize.y, io.DisplaySize.y),
				IM_COL32(255, 255, 0, 255));
		}
	}
# endif

	Utils::Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

	m_LeftPanel.ShowLeftPanel(leftPaneWidth - 4.0f);

	ImGui::SameLine(0.0f, 12.0f);

	ShowPinTypeEditor();
	ShowSectionEditor();

	ed::Begin("Node editor");
	for (auto& node : Node::Array)
		node->Update();

	for (auto& link : m_Links)
		link->Draw();

	NodeEditor();

	Menu();
	ed::End();

	m_LeftPanel.ShowOrdinals();

	//ImGui::ShowTestWindow();
	//ImGui::ShowMetricsWindow();
}
