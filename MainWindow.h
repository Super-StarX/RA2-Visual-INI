#pragma once
#include <application.h>
#include <imgui_node_editor.h>

#include <string>
#include <vector>
#include <map>

#include "nodes/Node.h"
#include "LeftPanelClass.h"

class MainWindow : public Application {
public:
	using Application::Application;

	static ed::NodeId contextNodeId;
	static ed::LinkId contextLinkId;
	static ed::PinId  contextPinId;
	static bool createNewNode;
	static Pin* newNodeLinkPin;
	static Pin* newLinkPin;

	static float leftPaneWidth;
	static float rightPaneWidth;
private:
	int GetNextId();
	ed::LinkId GetNextLinkId();
	void TouchNode(ed::NodeId id);
	void UpdateTouch();
	Node* FindNode(ed::NodeId id);
	Link* FindLink(ed::LinkId id);
	Pin* FindPin(ed::PinId id);
	bool CanCreateLink(Pin* a, Pin* b);
	void BuildNode(const std::unique_ptr<Node>& node);
	Node* SpawnInputActionNode();
	Node* SpawnBranchNode();
	Node* SpawnDoNNode();
	Node* SpawnOutputActionNode();
	Node* SpawnPrintStringNode();
	Node* SpawnMessageNode();
	Node* SpawnSetTimerNode();
	Node* SpawnLessNode();
	Node* SpawnWeirdNode();
	Node* SpawnTraceByChannelNode();
	Node* SpawnTreeSequenceNode();
	Node* SpawnTreeTaskNode();
	Node* SpawnTreeTask2Node();
	Node* SpawnComment();
	Node* SpawnHoudiniTransformNode();
	Node* SpawnHoudiniGroupNode();
	void BuildNodes();
	void NodeEditor();
	void NodeMenu();
	void PinMenu();
	void LinkMenu();
	void CreateNewNode(ImVec2 openPopupPosition);

	ImColor GetIconColor(PinType type);
	void ShowStyleEditor(bool* show = nullptr);

public:
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void OnFrame(float deltaTime) override;

	bool IsPinLinked(ed::PinId id);
	std::vector<std::unique_ptr<Node>>& GetNodes() { return m_Nodes; };
	std::vector<Link>& GetLinks() { return m_Links; };
	float GetTouchProgress(ed::NodeId id);
	void DrawPinIcon(const Pin& pin, bool connected, int alpha);
private:
	LeftPanelClass m_LeftPanel;
	int									m_NextId = 1;
	const int							m_PinIconSize = 24;
	std::vector<std::unique_ptr<Node>>	m_Nodes;
	std::vector<Link>					m_Links;
	ImTextureID							m_HeaderBackground = nullptr;
	const float							m_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
};