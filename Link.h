#pragma once
#include "Object.h"
#include "utilities/builders.h"
#include <string>
#include <vector>
#include <memory>

namespace ed = ax::NodeEditor;
class Link : public Object {
public:
	static std::vector<std::unique_ptr<Link>> Array;

	Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
		ID(id), StartPinID(startPinId), EndPinID(endPinId) {}
	~Link();

	static Link* Get(ed::LinkId id);

	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual void Tooltip() override;

	void Draw() const;
	void Menu();

	ed::LinkId ID;

	ed::PinId StartPinID;
	ed::PinId EndPinID;

	std::string TypeIdentifier = "default";
};
