#pragma once
#include <application.h>
#include <imgui_node_editor.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "Action.h"
#include "nodes/Node.h"
#include "LeftPanelClass.h"
#include "TemplateManager.h"

class SectionNode;
class TagNode;
class GroupNode;
class CommentNode;

class MainWindow : public Application {
public:
	using Application::Application;
	static MainWindow* Instance;
	
	static bool createNewNode;
	static Pin* newNodeLinkPin;
	static Pin* newLinkPin;

	static float leftPaneWidth;
	static float rightPaneWidth;

	static int GetNextId();
	static void ClearAll();

	float GetTouchProgress(ed::NodeId id);
	void TouchNode(ed::NodeId id);
	void UpdateTouch();

	void BuildNode(const std::unique_ptr<Node>& node);
	void BuildNodes();
	Node* SpawnNodeFromTemplate(const std::string& sectionName, const std::vector<TemplateSection::KeyValue>& keyValues, ImVec2 position);
	SectionNode* SpawnSectionNode(const std::string& section = "");
	GroupNode* SpawnGroupNode(const std::string& section = "");
	CommentNode* SpawnCommentNode(const std::string& section = "");
	TagNode* SpawnTagNode(bool input = true, const std::string& section = "");

	void ApplyForceDirectedLayout();
	void NodeEditor();

	void Menu();
	void ToolTip();
	void LayerMenu();
	void NodeMenu();
	void PinMenu();
	void LinkMenu();
	void ShowPinTypeEditor();
	void ShowSectionEditor();

	void Copy();
	void Paste();
	void Duplicate();
	
	void LoadProject(const std::string& path);
	void SaveProject(const std::string& path);
	void ImportINI(const std::string& path);
	void ExportINI(const std::string& path);

	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void OnFrame(float deltaTime) override;

	ClipboardData clipboard;
	LeftPanelClass m_LeftPanel;
	TemplateManager m_TemplateManager;
	const float							m_TouchTime = 1.0f;
	std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
	bool								m_ShowPinTypeEditor{ false };
	bool                                m_ShowSectionEditor{ false };
	SectionNode*                        m_SectionEditorNode{ nullptr };
};