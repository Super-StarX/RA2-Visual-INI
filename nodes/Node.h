#pragma once
#include "VIWidget.h"
#include "Link.h"
#include "Object.h"
#include "Pins/Pin.h"
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
	Module,
	IO,
	Registry,

	End,
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

	Node(const char* name = "", int id = 0);
	virtual ~Node() = default;

	static Node* Get(ed::NodeId id);
	static std::vector<Node*> GetSelectedNodes();
	static std::string GetNodeTypeName(NodeType type);
	static std::string GetNodeTypeName(int type);

	template<typename T, typename... Args>
	static T* Create(Args&&... args) {
		static_assert(std::is_base_of<Node, T>::value, "Must be Node derived type");
		auto node = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = node.get();
		Array.push_back(std::move(node));
		return ptr;
	}

	virtual NodeType GetNodeType() const = 0;
	virtual void Menu() override;
	virtual void Tooltip() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j, bool newId = false) override;
	virtual void Update() = 0;
	virtual void SetName(const std::string& str);
	virtual Pin* GetFirstCompatiblePin(Pin* pin);
	virtual std::vector<Pin*> GetAllPins() { return std::vector<Pin*>(); }
	virtual std::vector<Pin*> GetInputPins() { return std::vector<Pin*>(); }
	virtual std::vector<Pin*> GetOutputPins() { return std::vector<Pin*>(); }
	virtual KeyValue* ConvertToKeyValue(Pin* pin);
	virtual std::string GetValue(Pin* from = nullptr) const;
	virtual bool PinNameChangable() const { return true; } // Link是否会影响Pin的名字
	virtual bool PinNameSyncable() const { return false; } // 名字是否会通过Link同步

	ImVec2 GetNodeSize() const;
	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 pos) const;
	int GetConnectedLinkCount();
	Pin* GetPin(ed::PinId pinId);
	bool HasPin(ed::PinId pinId);
	
	MainWindow* Owner = nullptr;

	ed::NodeId ID;
	VIInputText Name;
	int level{};
	bool IsFolded{false};
	bool IsComment{false};
	float HoverTimer = 0.0f;
	std::string TypeName{};
	std::string Style{ "default" };
	std::string State;
	std::string SavedState;
};

struct NodeIdLess {
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const {
		return lhs.AsPointer() < rhs.AsPointer();
	}
};