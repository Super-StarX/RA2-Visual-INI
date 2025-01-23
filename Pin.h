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
	constexpr static float IconSize = 24.f;

	Pin(int id, const char* name, PinType type) :
		ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input) {
	}

	static bool CanCreateLink(Pin* a, Pin* b) {
		if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
			return false;

		return true;
	}
	static ImColor GetIconColor(PinType type);

	void DrawPinIcon(bool connected, int alpha) const;
	void Menu() const;

	ed::PinId   ID;
	::Node* Node;
	std::string Name;
	PinType     Type;
	PinKind     Kind;
};
