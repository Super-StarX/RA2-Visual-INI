﻿#pragma once
#include "Pins/Pin.h"
#include "Link.h"
#include "Object.h"
#include "utilities/builders.h"

#include <vector>
#include <memory>

namespace ed = ax::NodeEditor;
enum class NodeType {
	Blueprint,
	Simple,
	Tag,
	Tree,
	Group,
	Houdini,
	Section,
	Comment,
	List,
};
/*
Node->INENode->HeaderNode->BlueprintNode
Node->INENode->HeaderNode->SimpleNode
Node->INENode->HoudiniNode
Node->INENode->TreeNode
Node->VINode->SectionNode
Node->VINode->ListNode
Node->GroupNode
Node->TagNode
*/
class KeyValue;
class MainWindow;
class Node : public Object {
public:
	static std::vector<std::unique_ptr<Node>> Array;

	Node(const char* name, int id = 0);
	virtual ~Node() = default;

	static Node* Get(ed::NodeId id);
	static std::vector<Node*> GetSelectedNodes();

	virtual NodeType GetNodeType() const = 0;
	virtual void Menu() override;
	virtual void Tooltip() override;
	virtual void HoverMenu(bool isHovered);
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void Update() = 0;
	virtual void SetName(const std::string& str);
	virtual Pin* GetFirstCompatiblePin(Pin* pin);
	virtual KeyValue* ConvertToKeyValue(Pin* pin);

	ImVec2 GetNodeSize() const;
	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 pos) const;
	int GetConnectedLinkCount();
	
	MainWindow* Owner = nullptr;

	ed::NodeId ID;
	std::string Name{""};
	ImColor Color{};
	int level;
	bool IsFolded{false};
	bool IsComment{false};
	float HoverTimer = 0.0f;
	std::string TypeName{};
	std::string State;
	std::string SavedState;
};

struct NodeIdLess {
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const {
		return lhs.AsPointer() < rhs.AsPointer();
	}
};
