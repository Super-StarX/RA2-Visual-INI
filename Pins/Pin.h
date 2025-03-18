#pragma once
#include "Object.h"
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
class Pin : public Object {
public:
	constexpr static float IconSize = 24.f;
	static std::map<ed::PinId, Pin*, ComparePinId> Array;
	static Pin* Get(ed::PinId id);

	Pin(::Node* node, const char* name, PinKind kind = PinKind::Output, int id = 0);
	virtual ~Pin();

	virtual void Tooltip() override;
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void SetValue(const std::string& str);
	virtual std::string GetValue() const;

	void UpdateOutputLink(std::string value);
	bool CanCreateLink(Pin* b);
	bool IsLinked() const;
	Link* LinkTo(Pin* pin);
	Pin* GetLinkedPin() const;
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
	std::string TypeIdentifier = "flow";
	PinKind     Kind;
	float HoverTimer = 0.0f;
};
