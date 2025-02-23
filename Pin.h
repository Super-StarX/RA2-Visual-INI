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

struct ComparePinId {
	bool operator()(const ed::PinId& lhs, const ed::PinId& rhs) const {
		return lhs.Get() < rhs.Get();
	}
};

class Node;
class SectionNode;
class Link;
class Pin {
public:
	constexpr static float IconSize = 24.f;
	static std::map<ed::PinId, Pin*, ComparePinId> Array;
	static Pin* Get(ed::PinId id);

	Pin(int id, const char* name, std::string type = "flow", PinKind kind = PinKind::Input);
	virtual ~Pin();

	virtual void SetValue(std::string str) { Name = str; }
	virtual void Tooltip();

	void UpdateOutputLink(std::string value);
	bool CanCreateLink(Pin* b);
	bool IsLinked() const;
	Link* LinkTo(Pin* pin);
	Node* GetLinkedNode() const;
	SectionNode* GetLinkedSection() const;
	ImColor GetIconColor() const;
	std::string GetLinkType() const;
	float GetAlpha();
	void DrawPinIcon(bool connected, int alpha, bool isReverse = false) const;
	void Menu();

	ed::PinId   ID;
	::Node* Node;
	std::map<ed::LinkId, Link*, CompareLinkId> Links;
	std::string Name;
	std::string TypeIdentifier;
	PinKind     Kind;
	float HoverTimer = 0.0f;
};
