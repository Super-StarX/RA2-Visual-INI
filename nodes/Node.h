#pragma once
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
};

class KeyValue;
class MainWindow;
class Node : public Object {
public:
	static std::vector<std::unique_ptr<Node>> Array;

	Node(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
		Owner(owner), ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0) {}

	static Node* Get(ed::NodeId id);
	static std::vector<Node*> GetSelectedNodes();

	virtual void Menu() override;
	virtual void Tooltip() override;
	virtual void HoverMenu(bool isHovered);
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void Update() = 0;
	virtual Pin* GetFirstCompatiblePin(Pin* pin);
	virtual KeyValue* ConvertToKeyValue(Pin* pin);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 pos) const;
	int GetConnectedLinkCount();
	
	MainWindow* Owner = nullptr;

	ed::NodeId ID;
	std::string Name{""};
	std::vector<Pin> Inputs;
	std::vector<Pin> Outputs;
	ImColor Color{};
	NodeType Type{ NodeType::Section };
	ImVec2 Size;
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
