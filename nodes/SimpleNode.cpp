#include "SimpleNode.h"
#include "MainWindow.h"

ImTextureID SimpleNode::m_HeaderBackground = nullptr;

SimpleNode::SimpleNode(MainWindow* owner, int id, const char* name, ImColor color) :
	Node(owner, id, name, color) {
	m_HeaderBackground = owner->LoadTexture("data/BlueprintBackground.png");
}

SimpleNode::~SimpleNode() {
	auto releaseTexture = [this](ImTextureID& id) {
		if (id) {
			Owner->DestroyTexture(id);
			id = nullptr;
		}
	};

	releaseTexture(m_HeaderBackground);
}
void SimpleNode::Update() {
	using namespace ax::NodeEditor::Utilities;
	static BlueprintNodeBuilder builder = BlueprintNodeBuilder(m_HeaderBackground,
			Owner->GetTextureWidth(m_HeaderBackground), Owner->GetTextureHeight(m_HeaderBackground));

	auto newLinkPin = Owner->newLinkPin;

	bool hasOutputDelegates = false;
	for (auto& output : this->Outputs)
		if (output.Type == PinType::Delegate)
			hasOutputDelegates = true;

	builder.Begin(this->ID);

	for (auto& input : this->Inputs) {
		auto alpha = ImGui::GetStyle().Alpha;
		if (newLinkPin && !Pin::CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
			alpha = alpha * (48.0f / 255.0f);

		builder.Input(input.ID);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		input.DrawPinIcon(Owner->IsPinLinked(input.ID), (int)(alpha * 255));
		ImGui::Spring(0);
		if (!input.Name.empty()) {
			ImGui::TextUnformatted(input.Name.c_str());
			ImGui::Spring(0);
		}
		if (input.Type == PinType::Bool) {
			ImGui::Button("Hello");
			ImGui::Spring(0);
		}
		ImGui::PopStyleVar();
		builder.EndInput();
	}

	builder.Middle();

	ImGui::Spring(1, 0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1, 0);

	builder.End();
}