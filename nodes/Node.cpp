#include "Node.h"

ImVec2 Node::GetPosition() {
	return ed::GetNodePosition(ID);
}

void Node::SetPosition(ImVec2 pos) {
	return ed::SetNodePosition(ID, pos);
}
