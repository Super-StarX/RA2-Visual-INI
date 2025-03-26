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
	// 改进后的节点复制逻辑
	auto selectedNodes = Node::GetSelectedNodes();
	std::vector<Node*> newNodes;

	// 分两阶段处理保证引用完整性
	for (size_t i = 0; i < selectedNodes.size(); ++i) {
		Node* original = selectedNodes[i];
		json nodeData;
		original->SaveToJson(nodeData);

		// 生成新ID并更新数据
		const auto pos = ImGui::GetMousePos();
		nodeData["ID"] = GetNextId();
		nodeData["Name"] = nodeData["Name"].get<std::string>() + "_copy";
		nodeData["Position"] = { pos.x,pos.y };

		// 创建新节点
		Node* newNode = nullptr;
		switch (original->GetNodeType()) {
		case NodeType::Blueprint:
		case NodeType::Tree:
		case NodeType::Houdini:
		case NodeType::Simple:
			break;
		case NodeType::Tag:
			newNode = Node::Create<TagNode>(true);
			break;
		case NodeType::Group:
			newNode = Node::Create<GroupNode>();
			break;
		case NodeType::Section:
		{
			// 递归复制键值对
			auto& sectionNode = static_cast<SectionNode&>(*original);
			auto& keyValues = nodeData["KeyValues"];
			keyValues = json::array();
			for (auto& kv : sectionNode.KeyValues) {
				json kvData;
				kv->SaveToJson(kvData);
				kvData["ID"] = GetNextId(); // 为每个KeyValue生成新ID
				keyValues.push_back(kvData);
			}
			newNode = Node::Create<SectionNode>();
			break;
		}
		case NodeType::Comment:
			newNode = Node::Create<CommentNode>();
			break;
		case NodeType::List:
			newNode = Node::Create<ListNode>();
			break;
		case NodeType::Module:
			newNode = Node::Create<ModuleNode>();
			break;
		case NodeType::IO:
			newNode = Node::Create<IONode>(PinKind::Input);
			break;
		default:
			break;
		}
		if (newNode) {
			newNode->LoadFromJson(nodeData);
			newNodes.push_back(newNode);
		}
	}

	// 更新编辑器状态
	ed::ClearSelection();
	for (auto node : newNodes) {
		ed::SelectNode(node->ID, true);
	}
}
