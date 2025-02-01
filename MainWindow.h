#pragma once
#include <application.h>
#include <imgui_node_editor.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "nodes/Node.h"
#include "LeftPanelClass.h"

class SectionNode;

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

	int GetNextId() { return m_NextId++; };
	ed::LinkId GetNextLinkId() { return ed::LinkId(GetNextId()); }
	float GetTouchProgress(ed::NodeId id);
	std::vector<Link>& GetLinks() { return m_Links; };
	std::vector<std::unique_ptr<Node>>& GetNodes() { return m_Nodes; };
	Node* FindNode(ed::NodeId id);
	Link* FindLink(ed::LinkId id);
	Pin* FindPin(ed::PinId id);
	bool IsPinLinked(ed::PinId id);

	void BuildNode(const std::unique_ptr<Node>& node);
	void BuildNodes();
	void ClearAll();
	void CreateLinkFromReference(Pin* outputPin, const std::string& targetSection);
	void TouchNode(ed::NodeId id) { m_NodeTouchTime[id] = m_TouchTime; };
	void UpdateTouch();
	void ApplyForceDirectedLayout();

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
	SectionNode* SpawnSectionNode(const std::string& section);
	void NodeEditor();
	void NodeMenu();
	void PinMenu();
	void LinkMenu();
	void CreateInitNodes();
	void CreateNewNode();
	void ShowStyleEditor(bool* show = nullptr);
	void LoadProject(const std::string& path);
	void SaveProject(const std::string& path);
	void ImportINI(const std::string& path);
	void ExportINI(const std::string& path);

	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void OnFrame(float deltaTime) override;


	std::unordered_map<std::string, SectionNode*> m_SectionMap;
	std::unordered_map<ed::NodeId, std::string, std::hash<ed::NodeId>, std::equal_to<ed::NodeId>> m_NodeSections;
	LeftPanelClass m_LeftPanel;
	int									m_NextId = 1;
	std::vector<std::unique_ptr<Node>>	m_Nodes;
	std::vector<Link>					m_Links;
	const float							m_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
};