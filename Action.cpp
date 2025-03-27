#define IMGUI_DEFINE_MATH_OPERATORS
#include "Action.h"
#include "MainWindow.h"
#include "Pins/KeyValue.h"
#include "Nodes/Node.h"
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
#include <imgui_internal.h>

void MainWindow::Copy() {
	auto selectedNodes = Node::GetSelectedNodes();
	if (selectedNodes.empty())
		return;

	m_Clipboard = {}; // 清空剪贴板

	// 计算几何中心
	ImVec2 center(0, 0);
	for (Node* node : selectedNodes) {
		center += node->GetPosition();
	}
	center.x /= selectedNodes.size();
	center.y /= selectedNodes.size();
	m_Clipboard.copyCenter = center;

	// 序列化节点
	for (Node* node : selectedNodes) {
		json nodeData;
		node->SaveToJson(nodeData);
		m_Clipboard.nodes.push_back(nodeData);

		// 收集相关连线
		/*for (auto& pin : node->GetAllPins()) {
			for (auto& [_, link] : pin->Links) {
				// 确保只保存两端都在选中节点中的连线
				if (std::find_if(selectedNodes.begin(), selectedNodes.end(),
					[&](Node* n) { return n->HasPin(link->StartPinID) || n->HasPin(link->EndPinID); }) != selectedNodes.end()) {
					json linkData;
					link->SaveToJson(linkData);
					m_Clipboard.links.push_back(linkData);
				}
			}
		}*/
	}
	// 去重连线
	std::sort(m_Clipboard.links.begin(), m_Clipboard.links.end());
	m_Clipboard.links.erase(std::unique(m_Clipboard.links.begin(), m_Clipboard.links.end()), m_Clipboard.links.end());

	m_Clipboard.hasData = true;
}

void MainWindow::Paste() {
	if (!m_Clipboard.hasData)
		return;

	const ImVec2 pastePos = ed::ScreenToCanvas(ImGui::GetMousePos());
	std::vector<Node*> newNodes;
	std::unordered_map<int, int> idMap; // 旧ID到新ID的映射

	// 第一阶段：创建所有新节点
	for (auto& nodeData : m_Clipboard.nodes) {
		// 创建新ID并建立映射
		const int oldId = nodeData["ID"];
		const int newId = GetNextId();
		idMap[oldId] = newId;
		nodeData["ID"] = newId;

		// 计算新位置
		ImVec2 originalPos(nodeData["Position"][0], nodeData["Position"][1]);
		ImVec2 offset = originalPos - m_Clipboard.copyCenter;
		ImVec2 newPos = pastePos + offset;
		nodeData["Position"] = { newPos.x, newPos.y };

		// 修改名称避免冲突
		if (nodeData.contains("Name")) {
			nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";
		}

		// 创建节点
		NodeType type = static_cast<NodeType>(nodeData["Type"].get<int>());
		Node* newNode = CreateNodeByType(type);
		if (newNode) {
			newNode->LoadFromJson(nodeData);
			newNodes.push_back(newNode);
		}
	}

	// 第二阶段：重建连线
	for (auto& linkData : m_Clipboard.links) {
		int oldStartId = linkData["StartPin"];
		int oldEndId = linkData["EndPin"];

		// 查找映射后的新ID
		auto startIt = idMap.find(oldStartId);
		auto endIt = idMap.find(oldEndId);

		if (startIt != idMap.end() && endIt != idMap.end()) {
			Pin* startPin = Pin::Get(startIt->second);
			Pin* endPin = Pin::Get(endIt->second);

			if (startPin && endPin) {
				startPin->LinkTo(endPin);
			}
		}
	}

	// 更新选择状态
	ed::ClearSelection();
	for (Node* node : newNodes) {
		ed::SelectNode(node->ID, true);
	}
}

void MainWindow::Duplicate() {
	auto selectedNodes = Node::GetSelectedNodes();
	if (selectedNodes.empty())
		return;

	std::vector<Node*> newNodes;
	const ImVec2 mousePos = ed::ScreenToCanvas(ImGui::GetMousePos());

	// 第一阶段：计算原始几何中心
	ImVec2 originalCenter(0, 0);
	for (Node* node : selectedNodes) {
		originalCenter = originalCenter + node->GetPosition();
	}
	originalCenter.x /= selectedNodes.size();
	originalCenter.y /= selectedNodes.size();

	// 第二阶段：复制节点并计算新位置
	for (Node* original : selectedNodes) {
		json nodeData;
		original->SaveToJson(nodeData);

		// 计算相对位置偏移
		ImVec2 originalPos = original->GetPosition();
		ImVec2 relativeOffset = originalPos - originalCenter;

		// 设置新位置（鼠标位置+相对偏移）
		ImVec2 newPos = mousePos + relativeOffset;
		nodeData["Position"] = { newPos.x, newPos.y };
		nodeData["ID"] = GetNextId();
		nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";

		// 创建新节点（优化后的工厂方法）
		Node* newNode = CreateNodeByType(original->GetNodeType());
		if (!newNode)
			continue;

		// 特殊类型处理
		if (original->GetNodeType() == NodeType::Section) {
			auto& sectionNode = static_cast<SectionNode&>(*original);
			auto& keyValues = nodeData["KeyValues"];
			keyValues = json::array();
			for (auto& kv : sectionNode.KeyValues) {
				json kvData;
				kv->SaveToJson(kvData);
				kvData["ID"] = GetNextId();
				keyValues.push_back(kvData);
			}
		}

		newNode->LoadFromJson(nodeData);
		newNodes.push_back(newNode);
	}

	// 更新编辑器状态
	ed::ClearSelection();
	for (Node* node : newNodes) {
		ed::SelectNode(node->ID, true);
	}
}

Node* MainWindow::CreateNodeByType(NodeType type) {
	switch (type) {
	case NodeType::Tag:      return Node::Create<TagNode>(true);
	case NodeType::Group:    return Node::Create<GroupNode>();
	case NodeType::Section:  return Node::Create<SectionNode>();
	case NodeType::Comment:  return Node::Create<CommentNode>();
	case NodeType::List:     return Node::Create<ListNode>();
	case NodeType::Module:   return Node::Create<ModuleNode>();
	case NodeType::IO:      return Node::Create<IONode>(PinKind::Input);
	default:                return nullptr;
	}
}