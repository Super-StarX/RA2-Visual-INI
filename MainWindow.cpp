#define IMGUI_DEFINE_MATH_OPERATORS
#include "MainWindow.h"
#include "LeftPanelClass.h"
#include "PinType.h"
#include "Utils.h"
#include "nodes/SectionNode.h"

#include <imgui_internal.h>

static ed::EditorContext* m_Editor = nullptr;
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
	m_Nodes.clear();
	m_Links.clear();
	m_SectionMap.clear();
	m_NodeSections.clear();
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
	for (const auto& node : m_Nodes)
		if (node->ID == id)
			return node.get();

	return nullptr;
}

Link* MainWindow::FindLink(ed::LinkId id) {
	for (auto& link : m_Links)
		if (link.ID == id)
			return &link;

	return nullptr;
}

Pin* MainWindow::FindPin(ed::PinId id) {
	if (!id)
		return nullptr;

	for (auto& node : m_Nodes) {
		for (auto& pin : node->Inputs)
			if (pin.ID == id)
				return &pin;

		for (auto& pin : node->Outputs)
			if (pin.ID == id)
				return &pin;

		if (node->Type == NodeType::Section) {
			auto sectionNode = dynamic_cast<SectionNode*>(node.get());
			for (auto& [_, __, pin] : sectionNode->KeyValues)
				if (pin.ID == id)
					return &pin;

			if (sectionNode->OutputPin->ID == id)
				return sectionNode->OutputPin.get();

			if (sectionNode->InputPin->ID == id)
				return sectionNode->InputPin.get();
		}
	}

	return nullptr;
}

bool MainWindow::IsPinLinked(ed::PinId id) {
	if (!id)
		return false;

	for (auto& link : m_Links)
		if (link.StartPinID == id || link.EndPinID == id)
			return true;

	return false;
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
	PinTypeManager::Get().InitializeDefaults();
	PinTypeManager::Get().LoadFromFile("custom_types.json");
	m_Editor = ed::CreateEditor(&config);
	ed::SetCurrentEditor(m_Editor);

	SpawnSectionNode();

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
					showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
				//else if (endPin->Node == startPin->Node)
				//{
				//    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
				//    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
				//}
				else if (endPin->TypeIdentifier != startPin->TypeIdentifier) {
					showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
					ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
				}
				else {
					showLabel("+ Create Link", ImColor(32, 45, 32, 180));
					if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
						m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
						m_Links.back().Color = startPin->GetIconColor();
					}
				}
			}
		}

		ed::PinId pinId = 0;
		if (ed::QueryNewNode(&pinId)) {
			newLinkPin = FindPin(pinId);
			if (newLinkPin)
				showLabel("+ Create Node", ImColor(32, 45, 32, 180));

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
				auto id = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](auto& node) { return node->ID == nodeId; });
				if (id != m_Nodes.end())
					m_Nodes.erase(id);
			}
		}

		ed::LinkId linkId = 0;
		while (ed::QueryDeletedLink(&linkId)) {
			if (ed::AcceptDeletedItem()) {
				auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
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

	Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

	m_LeftPanel.ShowLeftPanel(leftPaneWidth - 4.0f);

	ImGui::SameLine(0.0f, 12.0f);

	ShowPinTypeEditor();

	ed::Begin("Node editor");
	for (auto& node : m_Nodes)
		node->Update();

	for (auto& link : m_Links)
		ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

	NodeEditor();
	Menu();
	ed::End();

	m_LeftPanel.ShowOrdinals();

	//ImGui::ShowTestWindow();
	//ImGui::ShowMetricsWindow();
}
