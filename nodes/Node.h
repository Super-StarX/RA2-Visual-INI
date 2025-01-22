#pragma once
#include "utilities/builders.h"
#include "utilities/widgets.h"

#include <imgui_internal.h>
#include <string>
#include <vector>

namespace ed = ax::NodeEditor;
enum class PinType {
	Flow,
	Bool,
	Int,
	Float,
	String,
	Object,
	Function,
	Delegate,
};

enum class PinKind {
	Output,
	Input
};

enum class NodeType {
	Blueprint,
	Simple,
	Tree,
	Comment,
	Houdini

};

class Node;

class Pin {
public:
	Pin(int id, const char* name, PinType type) :
		ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input) {
	}

	static bool CanCreateLink(Pin* a, Pin* b) {
		if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
			return false;

		return true;
	}

	ed::PinId   ID;
	::Node* Node;
	std::string Name;
	PinType     Type;
	PinKind     Kind;
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
