#include "BlueprintNode.h"
#include "MainWindow.h"

void BlueprintNode::Update() {
	auto builder = GetBuilder();
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

	for (auto& input : this->Inputs)
		UpdateInput(input);

	for (auto& output : this->Outputs)
		if (output.Type != PinType::Delegate)
			UpdateOutput(output);

	if (ImGui::Button("+")) {
		this->Inputs.emplace_back(Owner->GetNextId(), "NewAddInput", PinType::Delegate);
		this->Outputs.emplace_back(Owner->GetNextId(), "NewAddOutput", PinType::Bool);
	}

	builder.End();
}