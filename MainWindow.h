#pragma once
#include <application.h>
#include <imgui_node_editor.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "nodes/Node.h"
#include "LeftPanelClass.h"
#include "TemplateManager.h"

class SectionNode;

class MainWindow : public Application {
public:
	using Application::Application;
	static int m_NextId;

	static ed::NodeId contextNodeId;
	static ed::LinkId contextLinkId;
	static ed::PinId  contextPinId;
	static bool createNewNode;
	static Pin* newNodeLinkPin;
	static Pin* newLinkPin;

	static float leftPaneWidth;
	static float rightPaneWidth;

	static int GetNextId() { return m_NextId++; };
	static ed::LinkId GetNextLinkId() { return ed::LinkId(GetNextId()); }

	void ClearAll();

	float GetTouchProgress(ed::NodeId id);
	void TouchNode(ed::NodeId id);
	void UpdateTouch();

	Node* FindNode(ed::NodeId id);
	Link* FindLink(ed::LinkId id);
	Pin* FindPin(ed::PinId id);
	bool IsPinLinked(ed::PinId id);
	Node* GetLinkedNode(ed::PinId outputPinId);

	void BuildNode(const std::unique_ptr<Node>& node);
	void BuildNodes();
	SectionNode* SpawnSectionNode(const std::string& section = "");

	void ApplyForceDirectedLayout();
	void NodeEditor();

	void Menu();
	void LayerMenu();
	void NodeMenu();
	void PinMenu();
	void LinkMenu();
	void ShowPinTypeEditor();
	void ShowSectionEditor();
	void CreateNodeFromTemplate(const std::string& sectionName, const std::vector<TemplateSection::KeyValue>& keyValues, ImVec2 position);

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
	TemplateManager m_TemplateManager;
	std::vector<std::unique_ptr<Node>>	m_Nodes;
	std::vector<Link>					m_Links;
	const float							m_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
	bool								m_ShowPinTypeEditor{ false };
	bool                                m_ShowSectionEditor{ false };
	SectionNode*                        m_SectionEditorNode{ nullptr };
};