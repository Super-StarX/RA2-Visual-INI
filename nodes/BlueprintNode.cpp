#include "BlueprintNode.h"
#include "MainWindow.h"

ImTextureID BlueprintNode::m_HeaderBackground = nullptr;
BlueprintNode::BlueprintNode(MainWindow* owner, int id, const char* name, ImColor color) :
	Node(owner, id, name, color) {
	m_HeaderBackground = owner->LoadTexture("data/BlueprintBackground.png");
}

BlueprintNode::~BlueprintNode() {
	auto releaseTexture = [this](ImTextureID& id) {
		if (id) {
			Owner->DestroyTexture(id);
			id = nullptr;
		}
	};

	releaseTexture(m_HeaderBackground);
}

void BlueprintNode::Update() {
	using namespace ax::NodeEditor::Utilities;
	static BlueprintNodeBuilder builder = BlueprintNodeBuilder(m_HeaderBackground, 
			Owner->GetTextureWidth(m_HeaderBackground), Owner->GetTextureHeight(m_HeaderBackground));

	auto newLinkPin = Owner->newLinkPin;

	bool hasOutputDelegates = false;
	for (auto& output : this->Outputs)
		if (output.Type == PinType::Delegate)
			hasOutputDelegates = true;

	builder.Begin(this->ID);
	builder.Header(this->Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	if (hasOutputDelegates) {
		ImGui::BeginVertical("delegates", ImVec2(0, 28));
		ImGui::Spring(1, 0);
		for (auto& output : this->Outputs) {
			if (output.Type != PinType::Delegate)
				continue;

			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !Pin::CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				alpha *= 48.0f / 255.0f;

			ed::BeginPin(output.ID, ed::PinKind::Output);
			ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
			ed::PinPivotSize(ImVec2(0, 0));
			ImGui::BeginHorizontal(output.ID.AsPointer());
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			if (!output.Name.empty()) {
				ImGui::TextUnformatted(output.Name.c_str());
				ImGui::Spring(0);
			}
			output.DrawPinIcon(Owner->IsPinLinked(output.ID), (int)(alpha * 255));
			ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
			ImGui::EndHorizontal();
			ImGui::PopStyleVar();
			ed::EndPin();

			//DrawItemRect(ImColor(255, 0, 0));
		}
		ImGui::Spring(1, 0);
		ImGui::EndVertical();
		ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
	}
	else
		ImGui::Spring(0);
	builder.EndHeader();

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

	for (auto& output : this->Outputs) {
		if (output.Type == PinType::Delegate)
			continue;

		auto alpha = ImGui::GetStyle().Alpha;
		if (newLinkPin && !Pin::CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
			alpha = alpha * (48.0f / 255.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		builder.Output(output.ID);
		if (output.Type == PinType::String) {
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
		builder.EndOutput();
	}

	builder.End();
}