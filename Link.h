#pragma once
#include "utilities/builders.h"
#include <string>
#include <vector>
#include <memory>

namespace ed = ax::NodeEditor;
class Link {
public:
	static std::vector<std::unique_ptr<Link>> Array;

	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId) {}
	~Link();

	static Link* FindLink(ed::LinkId id);

	void Draw() const;
	void Menu();

	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	std::string TypeIdentifier = "default";
};
