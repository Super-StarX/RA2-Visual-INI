#include "BaseNode.h"
#include "MainWindow.h"

ImTextureID BaseNode::m_HeaderBackground = nullptr;
BaseNode::BaseNode(MainWindow* owner, int id, const char* name, ImColor color) :
	Node(owner, id, name, color) {
	m_HeaderBackground = owner->LoadTexture("data/BlueprintBackground.png");
}

BaseNode::~BaseNode() {
	auto releaseTexture = [this](ImTextureID& id) {
		if (id) {
			Owner->DestroyTexture(id);
			id = nullptr;
		}
	};

	releaseTexture(m_HeaderBackground);
}

void BaseNode::UpdateInput(Pin& input) {
	auto builder = GetBuilder();
	float alpha = input.GetAlpha(Owner->newLinkPin);
	builder->Input(input.ID);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	input.DrawPinIcon(Owner->IsPinLinked(input.ID), (int)(alpha * 255));
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

void BaseNode::UpdateOutput(Pin& output) {
	auto builder = GetBuilder();
	auto newLinkPin = Owner->newLinkPin;
	auto alpha = ImGui::GetStyle().Alpha;
	if (newLinkPin && !Pin::CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
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
	output.DrawPinIcon(Owner->IsPinLinked(output.ID), (int)(alpha * 255));
	ImGui::PopStyleVar();
	builder->EndOutput();
}

BlueprintNodeBuilder* BaseNode::GetBuilder() {
	static BlueprintNodeBuilder builder = BlueprintNodeBuilder(m_HeaderBackground,
			Owner->GetTextureWidth(m_HeaderBackground), Owner->GetTextureHeight(m_HeaderBackground));
	
	return &builder;
}