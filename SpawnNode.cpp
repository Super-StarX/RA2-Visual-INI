#include "MainWindow.h"
#include "Pins/KeyValue.h"
#include "Nodes/Node.h"
#include "Nodes/SectionNode.h"
#include "Nodes/TagNode.h"

Node* MainWindow::SpawnNodeFromTemplate(const TemplateSection& templa, ImVec2 position) {
	// 转换屏幕坐标到画布坐标
	const auto canvasPos = ed::ScreenToCanvas(position);

	// 创建节点
	auto* newNode = Node::Create<SectionNode>(templa.Name.c_str());
	ed::SetNodePosition(newNode->ID, canvasPos);
	newNode->TypeName = templa.Type;
	newNode->Color = templa.Color;
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
