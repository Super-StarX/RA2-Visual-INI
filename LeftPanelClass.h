#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include "utilities/builders.h"

namespace ed = ax::NodeEditor;

class MainWindow;
class LeftPanelClass {
public:
	LeftPanelClass() {};
	LeftPanelClass(MainWindow* owner);
	~LeftPanelClass();
	void ShowStyleEditor(bool* show);
	void DrawIcon(ImDrawList* drawList, ImTextureID* icon);
	void SelectionPanel(float paneWidth, int nodeCount, std::vector<ed::NodeId>& selectedNodes, int linkCount, std::vector<ed::LinkId>& selectedLinks);
	void NodesPanel(float paneWidth, std::vector<ed::NodeId>& selectedNodes);
	void ShowLeftPanel(float paneWidth);
	void ShowOrdinals() const;
	void ShowFileDialog(bool isSaving);
	bool OpenFileDialog(char* path, int maxPath, bool isSaving);
	void LoadINI(const std::string& path);
	void SaveINI(const std::string& path);
	bool                 m_ShowOrdinals = false;
private:
	MainWindow*			 Owner = nullptr;
	ImTextureID          m_RestoreIcon = nullptr;
	ImTextureID          m_SaveIcon = nullptr;
	bool                m_IsSaving = false;
};

