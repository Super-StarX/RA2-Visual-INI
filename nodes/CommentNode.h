#pragma once
#include "Node.h"

class CommentNode : public Node {
public:
	CommentNode(const char* name = "", int id = 0);
	virtual NodeType GetNodeType() const override { return NodeType::Comment; }
	virtual void Update() override;
	void CommentEditorPopup();

private:
	bool ShowEditPopup = false;
	std::string TempText;
};
