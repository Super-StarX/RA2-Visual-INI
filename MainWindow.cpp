#define IMGUI_DEFINE_MATH_OPERATORS
#include "MainWindow.h"
#include "LeftPanelClass.h"
#include "Utils.h"
#include "Nodes/SectionNode.h"
#include "Pins/KeyValue.h"
#include "Pins/PinType.h"
#include "Nodes/TagNode.h"

#include <imgui_internal.h>

static ed::EditorContext* m_Editor = nullptr;
MainWindow* MainWindow::Instance = nullptr;
Pin* MainWindow::newLinkPin = nullptr;

float MainWindow::leftPaneWidth = 400.0f;
float MainWindow::rightPaneWidth = 800.0f;

static int m_NextId = 1;

int MainWindow::GetNextId() {
	return m_NextId++;
}

void MainWindow::SetNextId(int id) {
	m_NextId = id;
}

void MainWindow::ClearAll() {
	Node::Array.clear();
	Link::Array.clear();
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

void MainWindow::OnFrameStart() {
	TagNode::UpdateSelectedName();
}

void MainWindow::OnFrameEnd() {
	TagNode::UpdateInputs();
}

void MainWindow::OnStart() {
	ed::Config config;

	config.SettingsFile = "RA2VI.json";

	config.UserPointer = this;

	config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void*) -> size_t {
		auto node = Node::Get(nodeId);
		if (!node)
			return 0;

		if (data != nullptr)
			memcpy(data, node->State.data(), node->State.size());
		return node->State.size();
	};

	config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool {
		auto self = static_cast<MainWindow*>(userPointer);

		auto node = Node::Get(nodeId);
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
	BaseNode::CreateHeader();

	auto node1 = SpawnSectionNode("Section A");
	node1->AddKeyValue("key", "Section B");
	auto node2 = SpawnSectionNode("Section B");
	node2->AddKeyValue("key", "Value");
	auto back = node1->KeyValues.back().get();
	back->LinkTo(node2->InputPin.get());

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
	BaseNode::DestroyHeader();
}

void MainWindow::OnFrame(float deltaTime) {
	OnFrameStart();
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

	for (auto& link : Link::Array)
		link->Draw();

	NodeEditor();

	Menu();
	ed::End();

	m_LeftPanel.ShowOrdinals();

	//ImGui::ShowTestWindow();
	//ImGui::ShowMetricsWindow();
	OnFrameEnd();
}
