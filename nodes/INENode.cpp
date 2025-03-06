#include "INENode.h"

INENode::INENode(const char* name, int id): Node(name, id) {
}

Pin* INENode::GetFirstCompatiblePin(Pin* pin) {
	if (pin->Kind == PinKind::Input) {
		for (auto& output : Outputs) {
			if (output.CanCreateLink(pin))
				return &output;
		}
	}
	else {
		for (auto& input : Inputs) {
			if (input.CanCreateLink(pin))
				return &input;
		}
	}
	return nullptr;
}
