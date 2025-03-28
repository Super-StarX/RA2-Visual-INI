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
	std::unordered_map<int, int> idMap;

	// 创建新节点并建立ID映射
	for (auto& originalNodeData : m_Clipboard.nodes) {
		json nodeData = originalNodeData;
		int oldId = nodeData["ID"];
		int newId = GetNextId();
		idMap[oldId] = newId;
		nodeData["ID"] = newId;

		ImVec2 originalPos(nodeData["Position"][0], nodeData["Position"][1]);
		ImVec2 offset = originalPos - oldCenter;
		ImVec2 newPos = newCenter + offset;
		nodeData["Position"] = { newPos.x, newPos.y };

		if (nodeData.contains("Name")) {
			nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";
		}

		NodeType type = static_cast<NodeType>(nodeData["Type"].get<int>());
		Node* newNode = CreateNodeByType(type);
		if (newNode) {
			newNode->LoadFromJson(nodeData);
			newNodes.push_back(newNode);
		}
	}

	// 重建连线（智能处理新旧节点连接）
	for (auto& linkData : m_Clipboard.links) {
		int oldStartId = linkData["StartID"];
		int oldEndId = linkData["EndID"];

		bool startCopied = idMap.count(oldStartId) > 0;
		bool endCopied = idMap.count(oldEndId) > 0;

		// 核心逻辑：只处理起点被复制的情况
		if (startCopied) {
			ed::PinId newStartId = idMap[oldStartId];
			ed::PinId newEndId = endCopied ? idMap[oldEndId] : oldEndId;

			Pin* startPin = Pin::Get(newStartId);
			Pin* endPin = Pin::Get(newEndId);

			// 确保方向正确：输出Pin连接到输入Pin
			if (startPin && endPin) {
				if (startPin->Kind == PinKind::Output && endPin->Kind == PinKind::Input) {
					startPin->LinkTo(endPin);
				}
				// 忽略反向连接（保持原有单向连接）
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