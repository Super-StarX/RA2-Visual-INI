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

Node* MainWindow::SpawnNodeFromTemplate(const TemplateSection& templa, ImVec2 position) {
	// 转换屏幕坐标到画布坐标
	const auto canvasPos = ed::ScreenToCanvas(position);

	// 创建节点
	auto* newNode = Node::Create<SectionNode>(templa.Name.c_str());
	ed::SetNodePosition(newNode->ID, canvasPos);
	newNode->TypeName = templa.Type;
	newNode->Style = templa.Style;
	newNode->IsFolded = templa.IsFolded;
	newNode->IsComment = templa.IsComment;

	// 填充键值对
	for (const auto& kv : templa.KeyValues) {
		auto newkv = newNode->AddKeyValue(kv.Key, kv.Value, "", GetNextId(), kv.IsInherited, kv.IsComment, kv.IsFolded);

		// 如果场内有对应的section就连上Link
		if (SectionNode::Map.contains(kv.Value)) {
			auto targetNode = SectionNode::Map[kv.Value];
			if (targetNode->InputPin->CanCreateLink(newkv))
				newkv->LinkTo(targetNode->InputPin.get())->TypeIdentifier = newkv->GetLinkType();
		}
	}

	// 添加动画效果
	ed::SelectNode(newNode->ID);
	ed::NavigateToSelection(true);

	return newNode;
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

Node* MainWindow::CreateNodeByName() {
	if (ImGui::MenuItem(LOCALE["SectionNode"]))
		return Node::Create<SectionNode>();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["SectionNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["ListNode"]))
		return Node::Create<ListNode>();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["ListNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["InputTagNode"]))
		return Node::Create<TagNode>(true);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["InputTagNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["OutputTagNode"]))
		return Node::Create<TagNode>(false);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["OutputTagNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["ModuleNode"]))
		return Node::Create<ModuleNode>();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["ModuleNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["InputIONode"]))
		return Node::Create<IONode>(PinKind::Input);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["InputIONodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["OutputIONode"]))
		return Node::Create<IONode>(PinKind::Output);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["OutputIONodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["CommentNode"]))
		return Node::Create<CommentNode>();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["CommentNodeTooltip"]);

	if (ImGui::MenuItem(LOCALE["GroupNode"]))
		return Node::Create<GroupNode>();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(LOCALE["GroupNodeTooltip"]);
	return nullptr;
}
