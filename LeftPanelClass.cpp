#define IMGUI_DEFINE_MATH_OPERATORS
#include "LeftPanelClass.h"
#include "MainWindow.h"
#include "nodes/SectionNode.h"
#include "utilities/builders.h"
#include "utilities/widgets.h"
#include <ImGui.h>
#include <imgui_internal.h>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <commdlg.h>

LeftPanelClass::LeftPanelClass(MainWindow* owner) :Owner(owner) {
	m_RestoreIcon = Owner->LoadTexture("data/ic_restore_white_24dp.png");
	m_SaveIcon = Owner->LoadTexture("data/ic_save_white_24dp.png");
}

LeftPanelClass::~LeftPanelClass() {
	auto releaseTexture = [this](ImTextureID& id) {
		if (id) {
			Owner->DestroyTexture(id);
			id = nullptr;
		}
	};
	releaseTexture(m_RestoreIcon);
	releaseTexture(m_SaveIcon);
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

void LeftPanelClass::NodesPanel(float paneWidth, std::vector<ed::NodeId>& selectedNodes) {
	auto& io = ImGui::GetIO();

	int saveIconWidth = Owner->GetTextureWidth(m_SaveIcon);
	int saveIconHeight = Owner->GetTextureWidth(m_SaveIcon);
	int restoreIconWidth = Owner->GetTextureWidth(m_RestoreIcon);
	int restoreIconHeight = Owner->GetTextureWidth(m_RestoreIcon);

	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Nodes");
	ImGui::Indent();
	for (auto& node : Owner->GetNodes()) {
		ImGui::PushID(node->ID.AsPointer());
		auto start = ImGui::GetCursorScreenPos();

		if (const auto progress = Owner->GetTouchProgress(node->ID)) {
			ImGui::GetWindowDrawList()->AddLine(
				start + ImVec2(-8, 0),
				start + ImVec2(-8, ImGui::GetTextLineHeight()),
				IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
		}

		bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node->ID) != selectedNodes.end();
# if IMGUI_VERSION_NUM >= 18967
		ImGui::SetNextItemAllowOverlap();
# endif
		if (ImGui::Selectable((node->Name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node->ID.AsPointer()))).c_str(), &isSelected)) {
			if (io.KeyCtrl) {
				if (isSelected)
					ed::SelectNode(node->ID, true);
				else
					ed::DeselectNode(node->ID);
			}
			else
				ed::SelectNode(node->ID, false);

			ed::NavigateToSelection();
		}
		if (ImGui::IsItemHovered() && !node->State.empty())
			ImGui::SetTooltip("State: %s", node->State.c_str());

		auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node->ID.AsPointer())) + ")";
		auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
		auto iconPanelPos = start + ImVec2(
			paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - saveIconWidth - restoreIconWidth - ImGui::GetStyle().ItemInnerSpacing.x * 1,
			(ImGui::GetTextLineHeight() - saveIconHeight) / 2);
		auto textPos = ImVec2(iconPanelPos.x - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y);
		ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

		auto drawList = ImGui::GetWindowDrawList();
		ImGui::SetCursorScreenPos(iconPanelPos);
# if IMGUI_VERSION_NUM < 18967
		ImGui::SetItemAllowOverlap();
# else
		ImGui::SetNextItemAllowOverlap();
# endif
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
# if IMGUI_VERSION_NUM < 18967
		ImGui::SetItemAllowOverlap();
# else
		ImGui::SetNextItemAllowOverlap();
# endif
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
# if IMGUI_VERSION_NUM < 18967
		ImGui::SetItemAllowOverlap();
# endif
		ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));

		ImGui::PopID();
	}
	ImGui::Unindent();
}

void LeftPanelClass::SelectionPanel(float paneWidth, int nodeCount, std::vector<ed::NodeId>& selectedNodes, int linkCount, std::vector<ed::LinkId>& selectedLinks) {
	static int changeCount = 0;

	ImGui::GetWindowDrawList()->AddRectFilled(
		ImGui::GetCursorScreenPos(),
		ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
	ImGui::Spacing(); ImGui::SameLine();
	ImGui::TextUnformatted("Selection");

	ImGui::BeginHorizontal("Selection Stats", ImVec2(paneWidth, 0));
	ImGui::Text("Changed %d time%s", changeCount, changeCount > 1 ? "s" : "");
	ImGui::Spring();
	if (ImGui::Button("Deselect All"))
		ed::ClearSelection();
	ImGui::EndHorizontal();
	ImGui::Indent();
	for (int i = 0; i < nodeCount; ++i) ImGui::Text("Node (%p)", selectedNodes[i].AsPointer());
	for (int i = 0; i < linkCount; ++i) ImGui::Text("Link (%p)", selectedLinks[i].AsPointer());
	ImGui::Unindent();

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
		for (auto& link : Owner->GetLinks())
			ed::Flow(link.ID);

	if (ed::HasSelectionChanged())
		++changeCount;

	ImGui::EndChild();
}

void LeftPanelClass::ShowFileDialog(bool isSaving) {
	char path[MAX_PATH] = { 0 };
	if (OpenFileDialog(path, MAX_PATH, isSaving)) {
		if (isSaving)
			SaveINI(path);
		else
			LoadINI(path);
	}
}

bool LeftPanelClass::OpenFileDialog(char* path, int maxPath, bool isSaving) {
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "INI Files (*.ini)\0*.ini\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (isSaving) {
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		if (GetSaveFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	else {
		if (GetOpenFileNameA(&ofn) == TRUE) {
			strcpy_s(path, maxPath, szFile);
			return true;
		}
	}
	return false;
}

void LeftPanelClass::ShowLeftPanel(float paneWidth) {

	ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

	paneWidth = ImGui::GetContentRegionAvail().x;

	static bool showStyleEditor = false;
	ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
	ImGui::Spring(0.0f, 0.0f);
	if (ImGui::Button("Zoom to Content"))
		ed::NavigateToContent();
	ImGui::Spring(0.0f);
	if (ImGui::Button("Show Flow")) {
		for (auto& link : Owner->GetLinks())
			ed::Flow(link.ID);
	}
	ImGui::Spring();
	if (ImGui::Button("Edit Style"))
		showStyleEditor = true;
	ImGui::EndHorizontal();
	ImGui::Checkbox("Show Ordinals", &m_ShowOrdinals);

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
	if (ImGui::Button("Load INI"))
		ShowFileDialog(false);
	ImGui::SameLine();
	if (ImGui::Button("Save INI"))
		ShowFileDialog(true);
	ImGui::EndHorizontal();

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

// LeftPanelClass.cpp
void LeftPanelClass::LoadINI(const std::string& path) {
    Owner->ClearAll();
    std::ifstream file(path);
    std::string line, currentSection;

    // 第一阶段：创建所有Section节点
    while (std::getline(file, line)) {
        if (line.find('[') != std::string::npos) {
            currentSection = line.substr(1, line.find(']') - 1);
            Owner->SpawnSectionNode(currentSection);
        }
    }

    // 重置文件指针并重新解析
    file.clear();
    file.seekg(0);

    // 第二阶段：处理Key-Value和连接
    while (std::getline(file, line)) {
        if (line.find('[') != std::string::npos) {
            currentSection = line.substr(1, line.find(']') - 1);
        } else if (!currentSection.empty() && line.find('=') != std::string::npos) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                // 创建Key-Value节点
                auto node = std::make_unique<SectionNode>(Owner, Owner->GetNextId(), "Key-Value");
                node->Outputs.emplace_back(Owner->GetNextId(), "Value", PinType::String);
				// 给pin增加key和value的文本框
                Owner->m_Nodes.push_back(std::move(node));

                // 查找目标Section节点并创建连接
                if (Owner->m_SectionMap.find(value) != Owner->m_SectionMap.end()) {
                    auto targetNode = Owner->m_SectionMap[value];
                    Owner->m_Links.emplace_back(
                        Owner->GetNextLinkId(),
                        node->Outputs[0].ID,
                        targetNode->Inputs[0].ID
                    );
                }
            }
        }
    }
}

void LeftPanelClass::SaveINI(const std::string& path) {
	std::ofstream file(path);

	for (auto& [section, node] : Owner->m_SectionMap) {
		file << "[" << section << "]\n";

		// 收集输出引脚对应的连接
		std::unordered_map<std::string, std::string> keyValues;
		for (auto& output : node->Outputs) {
			for (auto& link : Owner->m_Links) {
				if (link.StartPinID == output.ID) {
					auto targetNode = Owner->FindNode(
						Owner->FindPin(link.EndPinID)->Node->ID);
					keyValues[output.Name] = Owner->m_NodeSections[targetNode->ID];
				}
			}
		}

		// 写入键值对
		for (auto& [key, value] : keyValues) {
			file << key << "=" << value << "\n";
		}

		file << "\n";
	}
}