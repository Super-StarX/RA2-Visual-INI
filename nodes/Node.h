#pragma once
#include "Pin.h"
#include "utilities/builders.h"

#include <vector>

namespace ed = ax::NodeEditor;
enum class NodeType {
	Blueprint,
	Simple,
	Tree,
	Comment,
	Houdini

};

class MainWindow;
class Node {
public:
	Node(MainWindow* owner, int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
		Owner(owner), ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0) {
	}

	virtual void Update() = 0;

	MainWindow* Owner = nullptr;

	ed::NodeId ID;
	std::string Name;
	std::vector<Pin> Inputs;
	std::vector<Pin> Outputs;
	ImColor Color;
	NodeType Type;
	ImVec2 Size;

	std::string State;
	std::string SavedState;
};

class Link {
public:
	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255) {
	}

	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	ImColor Color;
};

struct NodeIdLess {
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const {
		return lhs.AsPointer() < rhs.AsPointer();
	}
};
