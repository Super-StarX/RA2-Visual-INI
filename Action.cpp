#define IMGUI_DEFINE_MATH_OPERATORS
#include "Action.h"
#include "MainWindow.h"
#include "nodes/Node.h"

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
	// 获取选中的节点和链接
	std::vector<ed::NodeId> selectedNodes;
	ed::GetSelectedNodes(selectedNodes.data(), ed::GetSelectedObjectCount());
	/*
	for (auto& nodeId : selectedNodes) {
		if (auto node = Node::Get(nodeId)) {
			// 复制节点
			auto newNode = SpawnNodeFromTemplate(node->Name, {});
			ed::SelectNode(newNode->ID);
		}
	}*/
}
