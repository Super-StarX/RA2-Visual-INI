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
	builder->EndHeader();

	// 渲染键值对
	for (auto& kv : this->KeyValues) {
		ImGui::PushID(&kv);

		ImGui::BeginGroup();
		ImGui::SetNextItemWidth(80);
		ImGui::InputText("##Key", &kv.Key);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		ImGui::SetNextItemWidth(120);
		ImGui::InputText("##Value", &kv.Value, ImGuiInputTextFlags_ReadOnly);
		ImGui::EndGroup();

		ImGui::BeginGroup();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);

		float alpha = kv.OutputPin.GetAlpha(Owner->newLinkPin);
		ed::BeginPin(kv.OutputPin.ID, ed::PinKind::Output);
		kv.OutputPin.DrawPinIcon(Owner->IsPinLinked(kv.OutputPin.ID), (int)(alpha * 255));
		ed::EndPin();
		ImGui::EndGroup();

		ImGui::PopID();
		ImGui::Dummy(ImVec2(0, 28));
	}

	builder->End();
}