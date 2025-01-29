#pragma once
#include "Pin.h"
#include "Link.h"
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

struct NodeIdLess {
	bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const {
		return lhs.AsPointer() < rhs.AsPointer();
	}
};

namespace std {
	template<>
	struct hash<ax::NodeEditor::NodeId> {
		size_t operator()(const ax::NodeEditor::NodeId& id) const noexcept {
			return hash<uintptr_t>{}(reinterpret_cast<uintptr_t>(id.AsPointer()));
		}
	};
}

inline bool operator==(const ax::NodeEditor::NodeId& lhs, const ax::NodeEditor::NodeId& rhs) {
	return lhs.AsPointer() == rhs.AsPointer();
}