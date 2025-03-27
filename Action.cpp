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
	clipboard.links.clear();
	clipboard.nodes.clear();

	// 收集选中元素
	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;
	ed::GetSelectedNodes(selectedNodes.data(), ed::GetSelectedObjectCount());
	ed::GetSelectedLinks(selectedLinks.data(), ed::GetSelectedObjectCount());
	/*
	// 深拷贝节点
	for (auto& nodeId : selectedNodes) {
		auto node = Node::Get(nodeId);
		auto clonedNode = node->Clone(); // 实现深拷贝方法
		clipboard.nodes.push_back(clonedNode);
	}

	// 深拷贝关联连接
	for (auto& linkId : selectedLinks) {
		auto link = Link::Get(linkId);
		auto clonedLink = link->Clone();
		clipboard.links.push_back(clonedLink);
	}*/
}

void MainWindow::Paste() {
	if (clipboard.nodes.empty())
		return;
	// 计算粘贴偏移量
	static ImVec2 s_PasteOffset = ImVec2(20, 20);
	const ImVec2 mousePos = ed::ScreenToCanvas(ImGui::GetMousePos());

	// 生成新ID并建立映射
	/*std::unordered_map<ed::NodeId, ed::NodeId> idMap;
	for (auto& node : clipboard.nodes) {
		auto originalId = node->ID;
		node->ID = ed::NodeId::Generate(); // 生成新ID
		idMap[originalId] = node->ID;

		// 设置新位置
		node->Position = mousePos + s_PasteOffset;
		AddNode(node);

		s_PasteOffset += ImVec2(20, 20); // 下次粘贴偏移
	}

	// 重新映射连接
	for (auto& link : clipboard.links) {
		link->ID = ed::LinkId::Generate();
		link->StartPin.ID = idMap[link->StartPin.NodeId].Get()
			? context.FindPin(idMap[link->StartPin.NodeId], link->StartPin.Name)
			: ed::PinId::Invalid;
		link->EndPin.ID = idMap[link->EndPin.NodeId].Get()
			? context.FindPin(idMap[link->EndPin.NodeId], link->EndPin.Name)
			: ed::PinId::Invalid;

		if (link->StartPin.ID && link->EndPin.ID)
			context.AddLink(link);
	}*/

	ed::NavigateToSelection(); // 聚焦到新粘贴内容
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