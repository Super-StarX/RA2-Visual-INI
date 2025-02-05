#pragma once
#include "utilities/builders.h"

#include <string>

namespace ed = ax::NodeEditor;
enum class PinKind {
	Output,
	Input
};

class Node;
class Pin {
public:
	constexpr static float IconSize = 24.f;

	Pin(int id, const char* name, std::string type = "flow") :
		ID(id), Node(nullptr), Name(name), TypeIdentifier(type), Kind(PinKind::Input) {
	}

	static bool CanCreateLink(Pin* a, Pin* b);

	ImColor GetIconColor() const;
	std::string GetLinkType() const;
	float GetAlpha(Pin* newLinkPin);
	void DrawPinIcon(bool connected, int alpha) const;
	void Menu();

	ed::PinId   ID;
	::Node* Node;
	std::string Name;
	std::string TypeIdentifier;
	PinKind     Kind;
};
