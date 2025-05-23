﻿#include "BlueprintNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include "NodeStyle.h"

void BlueprintNode::Update() {
	auto builder = BuilderNode::GetBuilder();

	bool hasOutputDelegates = false;
	for (auto& output : this->Outputs)
		if (output.TypeIdentifier == "delegate")
			hasOutputDelegates = true;

	builder->Begin(this->ID);

	auto* typeInfo = NodeStyleManager::Get().FindType(Style);
	if (!typeInfo) return; 
	builder->Header(typeInfo->Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	if (hasOutputDelegates) {
		ImGui::BeginVertical("delegates", ImVec2(0, 28));
		ImGui::Spring(1, 0);
		for (auto& output : this->Outputs) {
			if (output.TypeIdentifier != "delegate")
				continue;

			float alpha = output.GetAlpha();
			ed::BeginPin(output.ID, ed::PinKind::Output);
			ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
			ed::PinPivotSize(ImVec2(0, 0));
			ImGui::BeginHorizontal(output.ID.AsPointer());
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			if (!output.Name.empty()) {
				ImGui::TextUnformatted(output.Name.c_str());
				ImGui::Spring(0);
			}
			output.DrawPinIcon(output.IsLinked(), (int)(alpha * 255));
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
	builder->EndHeader();

	for (auto& input : this->Inputs)
		BuilderNode::UpdateInput(input);

	for (auto& output : this->Outputs)
		if (output.TypeIdentifier != "delegate")
			BuilderNode::UpdateOutput(output);

	if (ImGui::Button("+")) {
		this->Inputs.emplace_back(this, "NewAddInput", PinKind::Input);
		this->Outputs.emplace_back(this, "NewAddOutput", PinKind::Output);
	}

	builder->End();
}