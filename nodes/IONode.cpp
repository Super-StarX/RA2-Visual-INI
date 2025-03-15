#include "IONode.h"
#include "BuilderNode.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>

IONode::IONode(PinKind mode, const char* name, int id)
	: Node(name, id), mode(mode) {
	Color = (mode == PinKind::Input) ? ImColor(0, 255, 0) : ImColor(255, 0, 0);

	if (mode == PinKind::Input) {
		IOPin = std::make_unique<Pin>(this, "input", PinKind::Input);
	}
	else {
		IOPin = std::make_unique<Pin>(this, "output", PinKind::Output);
	}
}

void IONode::SaveToJson(json& j) const {
	Node::SaveToJson(j);
	IOPin->SaveToJson(j);
}

void IONode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);
	IOPin->LoadFromJson(j);
}

void IONode::Update() {
	auto builder = BuilderNode::GetBuilder();

	builder->Begin(this->ID);
	Utils::SetNextInputWidth(Name, 120.f);
	ImGui::InputText("##Name", &Name);

	ImGui::SameLine();
	auto alpha = IOPin->GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ed::BeginPin(IOPin->ID, ed::PinKind::Output);
	IOPin->DrawPinIcon(IOPin->IsLinked(), (int)(alpha * 255));
	ed::EndPin();
	ImGui::PopStyleVar();
	builder->End();
}
