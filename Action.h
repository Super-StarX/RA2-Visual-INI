#pragma once
#include "nodes/Node.h"
#include "utilities/builders.h"
#include <vector>

class Action {
};

namespace ed = ax::NodeEditor;
struct ClipboardData {
	struct NodeData {
		ed::NodeId id;
		std::string name;
		NodeType type;
		ImVec2 position;
		ImColor color;
		// 其他必要属性...
	};

	struct LinkData {
		ed::PinId startPinId;
		ed::PinId endPinId;
	};

	std::vector<NodeData> nodes;
	std::vector<LinkData> links;
};