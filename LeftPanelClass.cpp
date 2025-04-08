#define IMGUI_DEFINE_MATH_OPERATORS
#include "LeftPanelClass.h"
#include "MainWindow.h"
#include "Nodes/SectionNode.h"
#include "utilities/builders.h"
#include "utilities/widgets.h"
#include "Log.h"
#include <ImGui.h>
#include <imgui_internal.h>

LeftPanelClass::LeftPanelClass(MainWindow* owner) :Owner(owner) {
	//m_RestoreIcon = Owner->LoadTexture("data/ic_restore_white_24dp.png");
	//m_SaveIcon = Owner->LoadTexture("data/ic_save_white_24dp.png");
}

LeftPanelClass::~LeftPanelClass() {
	/*
	auto releaseTexture = [this](ImTextureID& id) {
		if (id) {
			Owner->DestroyTexture(id);
			id = nullptr;
		}
	};
	*/
	//releaseTexture(m_RestoreIcon);
	//releaseTexture(m_SaveIcon);
}

void LeftPanelClass::ShowStyleEditor(bool* show) {
	if (!ImGui::Begin("Style", show)) {
		ImGui::End();
		return;
	}

	auto paneWidth = ImGui::GetContentRegionAvail().x;

	auto& editorStyle = ed::GetStyle();
	ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
	ImGui::TextUnformatted("Values");
	ImGui::Spring();
	if (ImGui::Button("Reset to defaults"))
		editorStyle = ed::Style();
	ImGui::EndHorizontal();
	ImGui::Spacing();
	ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
	ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
	ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
	ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
	ImGui::DragFloat("Hovered Node Border Offset", &editorStyle.HoverNodeBorderOffset, 0.1f, -40.0f, 40.0f);
	ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
	ImGui::DragFloat("Selected Node Border Offset", &editorStyle.SelectedNodeBorderOffset, 0.1f, -40.0f, 40.0f);
	ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
	ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
	ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
	//ImVec2  SourceDirection;
	//ImVec2  TargetDirection;
	ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
	ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
	ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
	ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
	//ImVec2  PivotAlignment;
	//ImVec2  PivotSize;
	//ImVec2  PivotScale;
	//float   PinCorners;
	//float   PinRadius;
	//float   PinArrowSize;
	//float   PinArrowWidth;
	ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
	ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

	ImGui::Separator();

	static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
	ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
	ImGui::TextUnformatted("Filter Colors");
	ImGui::Spring();
	ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_DisplayRGB);
	ImGui::Spring(0);
	ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_DisplayHSV);
	ImGui::Spring(0);
	ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_DisplayHex);
	ImGui::EndHorizontal();

	static ImGuiTextFilter filter;
	filter.Draw("##filter", paneWidth);

	ImGui::Spacing();

	ImGui::PushItemWidth(-160);
	for (int i = 0; i < ed::StyleColor_Count; ++i) {
		auto name = ed::GetStyleColorName((ed::StyleColor)i);
		if (filter.PassFilter(name))
			ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
	}
	ImGui::PopItemWidth();

	ImGui::End();
}

void LeftPanelClass::DrawIcon(ImDrawList* drawList, ImTextureID* icon) {
	if (ImGui::IsItemActive())
		drawList->AddImage(icon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
	else if (ImGui::IsItemHovered())
		drawList->AddImage(icon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
	else
		drawList->AddImage(icon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
}

std::array<bool, static_cast<size_t>(NodeType::IO) + 1> m_TypeFilters;
Node* m_EditingNode = nullptr;    // 当前正在编辑的节点
char m_NodeNameBuffer[256] = "";  // 编辑时的缓冲区
void LeftPanelClass::NodesPanel(float paneWidth, std::vector<ed::NodeId>& selectedNodes) {
	auto& io = ImGui::GetIO();

	// 绘制标题栏
	int saveIconWidth = Owner->GetTextureWidth(m_SaveIcon);
	int saveIconHeight = Owner->GetTextureWidth(m_SaveIcon);
	int restoreIconWidth = Owner->GetTextureWidth(m_RestoreIcon);
	int restoreIconHeight = 9; // Owner->GetTextureWidth(m_RestoreIcon);

	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted(LOCALE["Sections"]);
	ImGui::Indent();

	// 搜索框逻辑
	static char searchText[256] = ""; // 用于存储用户输入的搜索文本
	ImGui::SetNextItemWidth(-ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetFontSize() * 2); // 为筛选按钮预留空间
	ImGui::InputTextWithHint("##search", LOCALE["Search sections..."], searchText, IM_ARRAYSIZE(searchText));

	// 筛选按钮
	ImGui::SameLine();
	if (ImGui::Button("...")) {
		ImGui::OpenPopup("type_filter_popup");
	}

	// 动态检测当前场景中存在哪些节点类型
	std::array<bool, static_cast<size_t>(NodeType::IO) + 1> existingTypes{};
	for (auto& node : Node::Array) {
		NodeType nodeType = node->GetNodeType();
		existingTypes[static_cast<size_t>(nodeType)] = true;
	}

	// 类型筛选弹出菜单
	if (ImGui::BeginPopup("type_filter_popup")) {
		const char* NodeTypeNames[] = {
			"Blueprint", "Simple", "Tag", "Tree", "Group", "Houdini",
			"Section", "Comment", "List", "Module", "IO"
		};

		// 只显示当前场景中存在的节点类型
		for (int i = 0; i <= static_cast<int>(NodeType::IO); ++i) {
			if (existingTypes[i]) { // 如果该类型在场景中存在
				ImGui::Checkbox(NodeTypeNames[i], &m_TypeFilters[i]);
			}
		}
		ImGui::EndPopup();
	}

	// 筛选节点列表
	std::vector<Node*> filteredNodes;
	for (auto& node : Node::Array) {
		bool nameMatches = (strstr(node->Name.c_str(), searchText) != nullptr || strlen(searchText) == 0);
		NodeType nodeType = node->GetNodeType();
		bool typeMatches = m_TypeFilters[static_cast<size_t>(nodeType)];
		bool anyTypeSelected = std::any_of(m_TypeFilters.begin(), m_TypeFilters.end(), [](bool b) { return b; });

		if ((!anyTypeSelected || typeMatches) && nameMatches) {
			filteredNodes.push_back(node.get());
		}
	}

	// 绘制筛选后的节点列表（原有代码保持不变）
	for (auto& node : filteredNodes) {
		ImGui::PushID(node->ID.AsPointer());
		auto start = ImGui::GetCursorScreenPos();

		// 如果当前节点处于编辑模式
		if (m_EditingNode == node) {
			// 创建独立的编辑框，占据一整行
			ImGui::SetNextItemWidth(paneWidth - ImGui::GetStyle().FramePadding.x * 4);
			if (ImGui::InputText("##edit_node_name", m_NodeNameBuffer, IM_ARRAYSIZE(m_NodeNameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
				node->Name = m_NodeNameBuffer;
				m_EditingNode = nullptr;
			}
			ImGui::PopID();
			continue;  // 跳过后续绘制逻辑，避免重复绘制按钮和图标
		}

		// 正常显示可选择的节点名称
		bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node->ID) != selectedNodes.end();
		if (ImGui::Selectable((node->Name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node->ID.AsPointer()))).c_str(), &isSelected)) {
			if (io.KeyCtrl) {
				if (isSelected)
					ed::SelectNode(node->ID, true);
				else
					ed::DeselectNode(node->ID);
			}
			else {
				ed::SelectNode(node->ID, false);
			}
			ed::NavigateToSelection();
		}

		// 右键点击进入编辑模式
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			m_EditingNode = node;
			strcpy_s(m_NodeNameBuffer, node->Name.c_str());  // 初始化缓冲区
			ImGui::SetKeyboardFocusHere(-1);  // 自动聚焦到输入框
		}

		// 原有的图标和ID绘制逻辑保持不变
		auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node->ID.AsPointer())) + ")";
		auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
		float iconPanelX = paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - saveIconWidth - restoreIconWidth - ImGui::GetStyle().ItemInnerSpacing.x * 1;
		auto iconPanelPos = start + ImVec2(iconPanelX, (ImGui::GetTextLineHeight() - saveIconHeight) / 2);
		auto textPos = ImVec2(iconPanelX - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y);
		ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

		/*
		auto drawList = ImGui::GetWindowDrawList();
		ImGui::SetCursorScreenPos(iconPanelPos);
		ImGui::SetItemAllowOverlap();
		if (node->SavedState.empty()) {
			if (ImGui::InvisibleButton("save", ImVec2((float)saveIconWidth, (float)saveIconHeight)))
				node->SavedState = node->State;

			DrawIcon(drawList, &m_SaveIcon);
		}
		else {
			ImGui::Dummy(ImVec2((float)saveIconWidth, (float)saveIconHeight));
			drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
		}

		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::SetItemAllowOverlap();
		if (!node->SavedState.empty()) {
			if (ImGui::InvisibleButton("restore", ImVec2((float)restoreIconWidth, (float)restoreIconHeight))) {
				node->State = node->SavedState;
				ed::RestoreNodeState(node->ID);
				node->SavedState.clear();
			}

			DrawIcon(drawList, &m_RestoreIcon);
		}
		else {
			ImGui::Dummy(ImVec2((float)restoreIconWidth, (float)restoreIconHeight));
			drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
		}

		ImGui::SameLine(0, 0);
		ImGui::SetItemAllowOverlap();
		ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));
		*/
		ImGui::PopID();
	}
	ImGui::Unindent();
}

void LeftPanelClass::SelectionPanel(float paneWidth, int nodeCount, std::vector<ed::NodeId>& selectedNodes, int linkCount, std::vector<ed::LinkId>& selectedLinks) {
	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted(LOCALE["Selection"]);

	ImGui::Indent();
	for (int i = 0; i < nodeCount; ++i) ImGui::Text("%s (%p)", LOCALE["Node"], selectedNodes[i].AsPointer());
	for (int i = 0; i < linkCount; ++i) ImGui::Text("%s (%p)", LOCALE["Link"], selectedLinks[i].AsPointer());
	ImGui::Unindent();

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F)))
		for (auto& link : Link::Array)
			ed::Flow(link->ID);
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)))
		Owner->Copy();
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)))
		Owner->Paste();
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
		Owner->Duplicate();

	ImGui::EndChild();
}

void LeftPanelClass::OnFrame(float paneWidth) {
	ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

	paneWidth = ImGui::GetContentRegionAvail().x;

	static bool showStyleEditor = false;
	ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
	ImGui::Spring(0.0f, 0.0f);
	if (ImGui::Button(LOCALE["Zoom to Content"])) {
		LOG_INFO("居中缩放");
		ed::NavigateToContent();
	}
	ImGui::Spring(0.0f);
	if (ImGui::Button(LOCALE["Show Flow"])) {
		LOG_INFO("显示数据流");
		for (auto& link : Link::Array)
			ed::Flow(link->ID);
	}
	ImGui::Spring();
	if (ImGui::Button(LOCALE["Edit Style"])) {
		LOG_INFO("打开样式编辑器");
		showStyleEditor = true;
	}
	ImGui::EndHorizontal();
	if (ImGui::Checkbox(LOCALE["Show Ordinals"], &m_ShowOrdinals)) {
		LOG_INFO("显示序号");
	}

	if (showStyleEditor)
		ShowStyleEditor(&showStyleEditor);

	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;
	selectedNodes.resize(ed::GetSelectedObjectCount());
	selectedLinks.resize(ed::GetSelectedObjectCount());

	int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
	int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

	selectedNodes.resize(nodeCount);
	selectedLinks.resize(linkCount);

	ImGui::BeginHorizontal("File Operations", ImVec2(paneWidth, 0));
	if (ImGui::Button(LOCALE["Load INI"]))
		ShowINIFileDialog(false);
	if (ImGui::Button(LOCALE["Save INI"]))
		ShowINIFileDialog(true);
	ImGui::EndHorizontal();

	ImGui::BeginHorizontal("Project Operations", ImVec2(paneWidth, 0));
	if (ImGui::Button(LOCALE["Load Project"]))
		ShowProjFileDialog(false);
	if (ImGui::Button(LOCALE["Save Project"]))
		ShowProjFileDialog(true);
	ImGui::EndHorizontal();

	if (ImGui::Button(LOCALE["Rebuild Layout"])) {
		LOG_INFO("自动重设布局");
		Owner->EnableApplyForceDirectedLayout();
	}

	NodesPanel(paneWidth, selectedNodes);
	SelectionPanel(paneWidth, nodeCount, selectedNodes, linkCount, selectedLinks);
}

void LeftPanelClass::ShowOrdinals() const {
	auto editorMin = ImGui::GetItemRectMin();
	auto editorMax = ImGui::GetItemRectMax();
	if (m_ShowOrdinals) {
		int nodeCount = ed::GetNodeCount();
		std::vector<ed::NodeId> orderedNodeIds;
		orderedNodeIds.resize(static_cast<size_t>(nodeCount));
		ed::GetOrderedNodeIds(orderedNodeIds.data(), nodeCount);


		auto drawList = ImGui::GetWindowDrawList();
		drawList->PushClipRect(editorMin, editorMax);

		int ordinal = 0;
		for (auto& nodeId : orderedNodeIds) {
			auto p0 = ed::GetNodePosition(nodeId);
			auto p1 = p0 + ed::GetNodeSize(nodeId);
			p0 = ed::CanvasToScreen(p0);
			p1 = ed::CanvasToScreen(p1);


			ImGuiTextBuffer builder;
			builder.appendf("#%d", ordinal++);

			auto textSize = ImGui::CalcTextSize(builder.c_str());
			auto padding = ImVec2(2.0f, 2.0f);
			auto widgetSize = textSize + padding * 2;

			auto widgetPosition = ImVec2(p1.x, p0.y) + ImVec2(0.0f, -widgetSize.y);

			drawList->AddRectFilled(widgetPosition, widgetPosition + widgetSize, IM_COL32(100, 80, 80, 190), 3.0f, ImDrawFlags_RoundCornersAll);
			drawList->AddRect(widgetPosition, widgetPosition + widgetSize, IM_COL32(200, 160, 160, 190), 3.0f, ImDrawFlags_RoundCornersAll);
			drawList->AddText(widgetPosition + padding, IM_COL32(255, 255, 255, 255), builder.c_str());
		}

		drawList->PopClipRect();
	}
}
