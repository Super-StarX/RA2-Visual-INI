#include "SectionNode.h"
#include "MainWindow.h"
#include <misc/cpp/imgui_stdlib.h>

void SectionNode::Update() {
	auto builder = GetBuilder();

	builder->Begin(this->ID);
	builder->Header(this->Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	ImGui::Spring(0);
	builder->EndHeader();

	// 渲染键值对
	for (auto& kv : this->KeyValues) {
		auto alpha = kv.OutputPin.GetAlpha(Owner->newLinkPin);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ImGui::PushID(&kv);
		builder->Output(kv.OutputPin.ID);

		ImGui::SetNextItemWidth(80);
		ImGui::InputText("##Key", &kv.Key);

		ImGui::SetNextItemWidth(120);
		ImGui::InputText("##Value", &kv.Value, ImGuiInputTextFlags_ReadOnly);

		ImGui::Spring(0);
		kv.OutputPin.DrawPinIcon(Owner->IsPinLinked(kv.OutputPin.ID), (int)(alpha * 255));

		builder->EndOutput();
		ImGui::PopID();
		ImGui::PopStyleVar();
	}

	builder->End();
}