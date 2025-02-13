#pragma once
#include "utilities/builders.h"

#include <string>
#include <map>

namespace ed = ax::NodeEditor;
enum class PinKind {
	Output,
	Input
};

struct CompareLinkId {
	bool operator()(const ed::LinkId& lhs, const ed::LinkId& rhs) const {
		return lhs.Get() < rhs.Get();
	}
};

class Node;
class Link;
class Pin {
public:
	constexpr static float IconSize = 24.f;

	Pin(int id, const char* name, std::string type = "flow", PinKind kind = PinKind::Input) :
		ID(id), Node(nullptr), Name(name), TypeIdentifier(type), Kind(kind) {
	}

	static bool CanCreateLink(Pin* a, Pin* b);

	ImColor GetIconColor() const;
	std::string GetLinkType() const;
	float GetAlpha(Pin* newLinkPin);
	void DrawPinIcon(bool connected, int alpha) const;
	void Menu();

	ed::PinId   ID;
	::Node* Node;
	std::map<ed::LinkId, Link*, CompareLinkId> Links;
	std::string Name;
	std::string TypeIdentifier;
	PinKind     Kind;
};
