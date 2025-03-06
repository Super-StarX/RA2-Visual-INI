#include "BuilderNode.h"
#include "MainWindow.h"

ImTextureID BuilderNode::m_HeaderBackground = nullptr;
void BuilderNode::CreateHeader() {
	m_HeaderBackground = MainWindow::Instance->LoadTexture("data/BlueprintBackground.png");
}
void BuilderNode::DestroyHeader() {
	if (m_HeaderBackground) {
		MainWindow::Instance->DestroyTexture(m_HeaderBackground);
		m_HeaderBackground = nullptr;
	}
}

void BuilderNode::UpdateInput(Pin& input) {
	auto builder = GetBuilder();
	float alpha = input.GetAlpha();
	builder->Input(input.ID);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	input.DrawPinIcon(input.IsLinked(), (int)(alpha * 255));
	ImGui::Spring(0);
	if (!input.Name.empty()) {
		ImGui::TextUnformatted(input.Name.c_str());
		ImGui::Spring(0);
	}
	if (input.TypeIdentifier == "bool") {
		ImGui::Button("Hello");
		ImGui::Spring(0);
	}
	ImGui::PopStyleVar();
	builder->EndInput();
}

void BuilderNode::UpdateOutput(Pin& output) {
	auto builder = GetBuilder();
	auto newLinkPin = MainWindow::newLinkPin;
	auto alpha = ImGui::GetStyle().Alpha;
	if (newLinkPin && !newLinkPin->CanCreateLink(&output) && &output != newLinkPin)
		alpha = alpha * (48.0f / 255.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	builder->Output(output.ID);
	if (output.TypeIdentifier == "string") {
		static char buffer[128] = "Edit Me\nMultiline!";
		static bool wasActive = false;

		ImGui::PushItemWidth(100.0f);
		ImGui::InputText("##edit", buffer, 127);
		ImGui::PopItemWidth();
		if (ImGui::IsItemActive() && !wasActive) {
			ed::EnableShortcuts(false);
			wasActive = true;
		}
		else if (!ImGui::IsItemActive() && wasActive) {
			ed::EnableShortcuts(true);
			wasActive = false;
		}
		ImGui::Spring(0);
	}
	if (!output.Name.empty()) {
		ImGui::Spring(0);
		ImGui::TextUnformatted(output.Name.c_str());
	}
	ImGui::Spring(0);
	output.DrawPinIcon(output.IsLinked(), (int)(alpha * 255));
	ImGui::PopStyleVar();
	builder->EndOutput();
}

BlueprintNodeBuilder* BuilderNode::GetBuilder() {
	static BlueprintNodeBuilder builder = BlueprintNodeBuilder(
		m_HeaderBackground, 
		MainWindow::Instance->GetTextureWidth(m_HeaderBackground),
		MainWindow::Instance->GetTextureHeight(m_HeaderBackground)
	);
	
	return &builder;
}