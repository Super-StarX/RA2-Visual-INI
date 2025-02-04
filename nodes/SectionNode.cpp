#include "SectionNode.h"
#include "MainWindow.h"
#include <misc/cpp/imgui_stdlib.h>

void SectionNode::Update() {
	auto builder = GetBuilder();

	builder->Begin(this->ID);
	builder->Header(this->Color);
	ImGui::Spring(0);
	
	{
		auto alpha = InputPin->GetAlpha(Owner->newLinkPin);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ed::BeginPin(InputPin->ID, ed::PinKind::Input);
		InputPin->DrawPinIcon(Owner->IsPinLinked(InputPin->ID), (int)(alpha * 255));
		ed::EndPin();
		ImGui::PopStyleVar();
	}

	//ImGui::TextUnformatted(this->Name.c_str());
	ImGui::PushID(this);
	ImGui::SetNextItemWidth(150);
	ImGui::InputText("##SectionName", &this->Name);
	ImGui::PopID();

	{
		auto alpha = OutputPin->GetAlpha(Owner->newLinkPin);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ed::BeginPin(OutputPin->ID, ed::PinKind::Output);
		OutputPin->DrawPinIcon(Owner->IsPinLinked(OutputPin->ID), (int)(alpha * 255));
		ed::EndPin();
		ImGui::PopStyleVar();
	}

	ImGui::Spring(1);
	//UpdateOutput(*OutputPin.get());
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
		ImGui::InputText("##Value", &kv.Value);

		ImGui::Spring(0);
		kv.OutputPin.DrawPinIcon(Owner->IsPinLinked(kv.OutputPin.ID), (int)(alpha * 255));

		builder->EndOutput();
		ImGui::PopID();
		ImGui::PopStyleVar();
	}

	builder->End();
}