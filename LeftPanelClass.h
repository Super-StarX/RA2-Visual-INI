#pragma once
#include <imgui.h>

class MainWindow;
class LeftPanelClass {
public:
	LeftPanelClass() {};
	LeftPanelClass(MainWindow* owner);
	~LeftPanelClass();
	void ShowStyleEditor(bool* show);
	void ShowLeftPanel(float paneWidth);
	void ShowOrdinals() const;
	bool                 m_ShowOrdinals = false;
private:
	MainWindow*			 Owner = nullptr;
	ImTextureID          m_RestoreIcon = nullptr;
	ImTextureID          m_SaveIcon = nullptr;
};

