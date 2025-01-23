#pragma once
#include "utilities/builders.h"

#include <string>

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
