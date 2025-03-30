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
void RebuildLink(std::unordered_map<uintptr_t, Node*>& nodeMap, uintptr_t& oldStartPinId, uintptr_t& oldEndPinId, std::unordered_map<uintptr_t, uintptr_t>& pinMap) {
	bool startCopied = nodeMap.contains(Pin::Get(oldStartPinId)->Node->ID.Get());
	bool endCopied = nodeMap.contains(Pin::Get(oldEndPinId)->Node->ID.Get());

	if (startCopied) {
		// 两侧都被复制：新(起点)连新(终点)
		// 仅起点被复制：新(起点)连旧(终点)
		// 仅终点被复制：不管
		Pin* startPin = Pin::Get(pinMap[oldStartPinId]);
		Pin* endPin = Pin::Get(endCopied ? pinMap[oldEndPinId] : oldEndPinId);

		startPin->LinkTo(endPin);
	}
}

void CollectPinMap(Node* original, Node* newNode, std::unordered_map<uintptr_t, uintptr_t>& pinMap) {
	// 建立Pin ID映射（通过名称匹配）
	auto oldPins = original->GetAllPins();
	auto newPins = newNode->GetAllPins();
	for (size_t i = 0; i < oldPins.size(); ++i) {
		if (i < newPins.size()) {
			pinMap[oldPins[i]->ID.Get()] = newPins[i]->ID.Get();
		}
	}
}

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
	}

	// 收集相关连线（包含至少一个端点在选中节点中的连线）
	for (Node* node : selectedNodes) {
		for (auto& pin : node->GetAllPins()) {
			for (auto& [_, link] : pin->Links) {
				// 收集所有与选中节点有关联的连线（任意一端在选中节点中）
				if (std::any_of(selectedNodes.begin(), selectedNodes.end(), [&](Node* n) {
					return n->HasPin(link->StartPinID) || n->HasPin(link->EndPinID);
				})) {
					json linkData;
					link->SaveToJson(linkData);
					m_Clipboard.links.push_back(linkData);
				}
			}
		}
	}

	// 去重连线
	std::sort(m_Clipboard.links.begin(), m_Clipboard.links.end());
	m_Clipboard.links.erase(std::unique(m_Clipboard.links.begin(), m_Clipboard.links.end()), m_Clipboard.links.end());

	m_Clipboard.hasData = true;
}

void MainWindow::Paste() {
	if (!m_Clipboard.hasData)
		return;

	ImVec2 newCenter = ed::ScreenToCanvas(ImGui::GetMousePos());
	ImVec2 oldCenter = m_Clipboard.copyCenter;

	std::vector<Node*> newNodes;
	std::unordered_map<uintptr_t, Node*> idMap;
	std::unordered_map<uintptr_t, uintptr_t> pinMap;    // 旧Pin ID → 新Pin ID

	// 创建新节点并建立ID映射
	for (auto& originalNodeData : m_Clipboard.nodes) {
		json nodeData = originalNodeData;

		ImVec2 originalPos(nodeData["Position"][0], nodeData["Position"][1]);
		ImVec2 offset = originalPos - oldCenter;
		ImVec2 newPos = newCenter + offset;
		nodeData["Position"] = { newPos.x, newPos.y };
		nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";

		NodeType type = static_cast<NodeType>(nodeData["Type"].get<int>());
		Node* newNode = CreateNodeByType(type);
		newNode->LoadFromJson(nodeData, true);
		newNodes.push_back(newNode);
		idMap[nodeData["ID"]] = newNode;

		CollectPinMap(Node::Get(nodeData["ID"].get<int>()), newNode, pinMap);
	}

	// 重建连线（智能处理新旧节点连接）
	for (auto& linkData : m_Clipboard.links) {
		uintptr_t oldStartPinId = linkData["StartID"];
		uintptr_t oldEndPinId = linkData["EndID"];

		RebuildLink(idMap, oldStartPinId, oldEndPinId, pinMap);
	}

	// 更新选择状态
	ed::ClearSelection();
	for (Node* node : newNodes)
		ed::SelectNode(node->ID, true);
}

void MainWindow::Duplicate() {
	auto selectedNodes = Node::GetSelectedNodes();
	if (selectedNodes.empty())
		return;

	std::vector<Node*> newNodes;
	std::unordered_map<uintptr_t, Node*> nodeMap;    // 旧Node ID → 新Node
	std::unordered_map<uintptr_t, uintptr_t> pinMap;       // 旧Pin ID → 新Pin ID
	ImVec2 mousePos = ed::ScreenToCanvas(ImGui::GetMousePos());

	// 计算原始几何中心
	ImVec2 originalCenter(0, 0);
	for (Node* node : selectedNodes)
		originalCenter += node->GetPosition();
	originalCenter /= static_cast<float>(selectedNodes.size());

	// 收集需要复制的连线
	std::vector<Link*> linksToCopy;
	for (Node* node : selectedNodes)
		for (Pin* pin : node->GetAllPins())
			for (auto& [_, link] : pin->Links)
				if (std::find(linksToCopy.begin(), linksToCopy.end(), link) == linksToCopy.end()) // 只处理未处理过的连线
					linksToCopy.push_back(link);

	// 第一阶段：复制节点并建立映射
	for (Node* original : selectedNodes) {
		json nodeData;
		original->SaveToJson(nodeData);

		// 计算新位置
		ImVec2 originalPos = original->GetPosition();
		ImVec2 offset = originalPos - originalCenter;
		ImVec2 newPos = mousePos + offset;
		nodeData["Position"] = { newPos.x, newPos.y };
		nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";

		Node* newNode = CreateNodeByType(original->GetNodeType());
		newNode->LoadFromJson(nodeData, true);
		newNodes.push_back(newNode);
		nodeMap[original->ID.Get()] = newNode;

		CollectPinMap(original, newNode, pinMap);
	}

	// 第二阶段：重建连线
	for (Link* originalLink : linksToCopy) {
		uintptr_t oldStartPinId = originalLink->StartPinID.Get();
		uintptr_t oldEndPinId = originalLink->EndPinID.Get();

		RebuildLink(nodeMap, oldStartPinId, oldEndPinId, pinMap);
	}

	// 更新编辑器状态
	ed::ClearSelection();
	for (Node* node : newNodes)
		ed::SelectNode(node->ID, true);
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