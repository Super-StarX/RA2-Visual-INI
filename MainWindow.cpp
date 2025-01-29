#define IMGUI_DEFINE_MATH_OPERATORS
#include "MainWindow.h"
#include "LeftPanelClass.h"
#include "Utils.h"
#include "nodes/BlueprintNode.h"

#include <imgui_internal.h>

static ed::EditorContext* m_Editor = nullptr;
ed::NodeId MainWindow::contextNodeId = 0;
ed::LinkId MainWindow::contextLinkId = 0;
ed::PinId  MainWindow::contextPinId = 0;
bool MainWindow::createNewNode = false;
Pin* MainWindow::newNodeLinkPin = nullptr;
Pin* MainWindow::newLinkPin = nullptr;

float MainWindow::leftPaneWidth = 400.0f;
float MainWindow::rightPaneWidth = 800.0f;

float MainWindow::GetTouchProgress(ed::NodeId id) {
	auto it = m_NodeTouchTime.find(id);
	if (it != m_NodeTouchTime.end() && it->second > 0.0f)
		return (m_TouchTime - it->second) / m_TouchTime;
	else
		return 0.0f;
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

//void MainWindow::DrawItemRect(ImColor color, float expand = 0.0f)
//{
//    ImGui::GetWindowDrawList()->AddRect(
//        ImGui::GetItemRectMin() - ImVec2(expand, expand),
//        ImGui::GetItemRectMax() + ImVec2(expand, expand),
//        color);
//};

//void MainWindow::FillItemRect(ImColor color, float expand = 0.0f, float rounding = 0.0f)
//{
//    ImGui::GetWindowDrawList()->AddRectFilled(
//        ImGui::GetItemRectMin() - ImVec2(expand, expand),
//        ImGui::GetItemRectMax() + ImVec2(expand, expand),
//        color, rounding);
//};

void MainWindow::BuildNodes() {
	for (const auto& node : m_Nodes)
		BuildNode(node);
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

	m_Editor = ed::CreateEditor(&config);
	ed::SetCurrentEditor(m_Editor);

	CreateInitNodes();
	ed::NavigateToContent();

	BuildNodes();

	m_LeftPanel = LeftPanelClass(this);
	//auto& io = ImGui::GetIO();
}

void MainWindow::OnStop() {
	if (m_Editor) {
		ed::DestroyEditor(m_Editor);
		m_Editor = nullptr;
	}
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
	if (pin)
		pin->Menu();
	else
		ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

	ImGui::EndPopup();
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

void MainWindow::NodeEditor() {
	auto cursorTopLeft = ImGui::GetCursorScreenPos();

	for (auto& node : m_Nodes)
		node->Update();

	for (auto& link : m_Links)
		ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

	if (!createNewNode) {
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
					else if (endPin->Type != startPin->Type) {
						showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
						ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
					}
					else {
						showLabel("+ Create Link", ImColor(32, 45, 32, 180));
						if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {
							m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
							m_Links.back().Color = Pin::GetIconColor(startPin->Type);
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
	}

	ImGui::SetCursorScreenPos(cursorTopLeft);
}

void MainWindow::OnFrame(float deltaTime) {
	UpdateTouch();

	auto& io = ImGui::GetIO();

	ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

	ed::SetCurrentEditor(m_Editor);

	//auto& style = ImGui::GetStyle();

# if 0
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

	ed::Begin("Node editor");
	NodeEditor();

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
		CreateNewNode();
	else
		createNewNode = false;
	ImGui::PopStyleVar();
	ed::Resume();

	/*
		cubic_bezier_t c;
		c.p0 = pointf(100, 600);
		c.p1 = pointf(300, 1200);
		c.p2 = pointf(500, 100);
		c.p3 = pointf(900, 600);

		auto drawList = ImGui::GetWindowDrawList();
		auto offset_radius = 15.0f;
		auto acceptPoint = [drawList, offset_radius](const bezier_subdivide_result_t& r)
		{
			drawList->AddCircle(to_imvec(r.point), 4.0f, IM_COL32(255, 0, 255, 255));

			auto nt = r.tangent.normalized();
			nt = pointf(-nt.y, nt.x);

			drawList->AddLine(to_imvec(r.point), to_imvec(r.point + nt * offset_radius), IM_COL32(255, 0, 0, 255), 1.0f);
		};

		drawList->AddBezierCurve(to_imvec(c.p0), to_imvec(c.p1), to_imvec(c.p2), to_imvec(c.p3), IM_COL32(255, 255, 255, 255), 1.0f);
		cubic_bezier_subdivide(acceptPoint, c);
	*/

	ed::End();

	m_LeftPanel.ShowOrdinals();

	//ImGui::ShowTestWindow();
	//ImGui::ShowMetricsWindow();
}

void MainWindow::ClearAll() {
	m_Nodes.clear();
	m_Links.clear();
	m_SectionMap.clear();
	m_NodeSections.clear();
}

Node* MainWindow::SpawnSectionNode(const std::string& section) {
	m_Nodes.emplace_back(std::make_unique<BlueprintNode>(this, GetNextId(), section.c_str()));
	auto node = m_Nodes.back().get();
	m_SectionMap[section] = node;
	m_NodeSections[node->ID] = section;
	return node;
}

void MainWindow::CreateLinkFromReference(Pin* outputPin, const std::string& targetSection) {
	if (m_SectionMap.find(targetSection) != m_SectionMap.end()) {
		auto targetNode = m_SectionMap[targetSection];
		for (auto& input : targetNode->Inputs) {
			if (Pin::CanCreateLink(outputPin, &input)) {
				m_Links.emplace_back(Link(GetNextId(), outputPin->ID, input.ID));
				m_Links.back().Color = Pin::GetIconColor(outputPin->Type);
				break;
			}
		}
	}
}
