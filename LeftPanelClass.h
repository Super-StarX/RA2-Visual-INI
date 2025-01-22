#pragma once
#include <imgui.h>

class MainWindow;
class LeftPanelClass {
public:
	LeftPanelClass(MainWindow* owner);
	~LeftPanelClass();
	void ShowStyleEditor(bool* show);
	void ShowLeftPane(float paneWidth);
	bool                 m_ShowOrdinals = false;
private:
	MainWindow* Owner;
	ImTextureID          m_RestoreIcon = nullptr;
	ImTextureID          m_SaveIcon = nullptr;
};

