
#include "IONode.h"

IONode::IONode(const char* name, IOMode mode, int id)
	: Node(name, id), mode(mode) {
	Color = (mode == IOMode::Input) ? ImColor(0, 255, 0) : ImColor(255, 0, 0);

	if (mode == IOMode::Input) {
		IOPin = std::make_unique<Pin>(this, "input", PinKind::Input);
	}
	else {
		IOPin = std::make_unique<Pin>(this, "output", PinKind::Output);
	}
}

void IONode::SaveToJson(json& j) const {
	Node::SaveToJson(j);
	j["Mode"] = (mode == IOMode::Input) ? "Input" : "Output";
}

void IONode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);
	mode = (j["Mode"] == "Input") ? IOMode::Input : IOMode::Output;
}