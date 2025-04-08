#pragma once
#include "Nodes/Node.h"
#include "utilities/builders.h"
#include <vector>

class Action {
};

namespace ed = ax::NodeEditor;

// 剪贴板数据
struct ClipboardData {
	std::vector<json> nodes;
	std::vector<json> links;
	ImVec2 copyCenter;
	bool hasData = false;
	int clipboardSequence;
};