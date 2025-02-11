#pragma once
#include "utilities/builders.h"
#include <string>

namespace ed = ax::NodeEditor;
class Link {
public:
	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId) {
	}

	void Draw() const;
	void Menu();

	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	std::string TypeIdentifier = "default";
};
