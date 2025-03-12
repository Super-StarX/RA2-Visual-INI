#include "INENode.h"
#include "Pins/Pin.h"
#include "Pins/ValuePin.h"

template<typename T>
INENode<T>::INENode(const char* name, int id): Node(name, id) {
}

template<typename T>
T* INENode<T>::GetFirstCompatiblePin(Pin* pin) {
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

template class INENode<Pin>;
template class INENode<ValuePin>;
