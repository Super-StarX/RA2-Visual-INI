#pragma once
#include <application.h>
#include <imgui_node_editor.h>

#include <string>
#include <vector>
#include <map>

#include "Node.h"

struct MainWindow : public Application {
	using Application::Application;

private:
	int GetNextId();
	ed::LinkId GetNextLinkId();
	void TouchNode(ed::NodeId id);
	float GetTouchProgress(ed::NodeId id);
	void UpdateTouch();
	Node* FindNode(ed::NodeId id);
	Link* FindLink(ed::LinkId id);
	Pin* FindPin(ed::PinId id);
	bool IsPinLinked(ed::PinId id);
	bool CanCreateLink(Pin* a, Pin* b);
	void BuildNode(Node* node);
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

	ImColor GetIconColor(PinType type);
	void DrawPinIcon(const Pin& pin, bool connected, int alpha);
	void ShowStyleEditor(bool* show = nullptr);
	void ShowLeftPane(float paneWidth);

public:
	void OnStart() override;
	void OnStop() override;
	void OnFrame(float deltaTime) override;

private:
	int                  m_NextId = 1;
	const int            m_PinIconSize = 24;
	std::vector<Node>    m_Nodes;
	std::vector<Link>    m_Links;
	ImTextureID          m_HeaderBackground = nullptr;
	ImTextureID          m_SaveIcon = nullptr;
	ImTextureID          m_RestoreIcon = nullptr;
	const float          m_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
	bool                 m_ShowOrdinals = false;
};