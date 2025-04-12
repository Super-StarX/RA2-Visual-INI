#include "IONode.h"
#include "BuilderNode.h"
#include "Utils.h"
#include "ModuleNode.h"
#include <misc/cpp/imgui_stdlib.h>

IONode::IONode(PinKind mode, const char* name, int id, ModuleNode* parent)
	: Node(name, id), Mode(mode), Parent(parent) {
	Style = (mode == PinKind::Input) ? "green" : "red";

	if (mode == PinKind::Input) {
		IOPin = std::make_unique<Pin>(this, name, PinKind::Input);
	}
	else {
		IOPin = std::make_unique<Pin>(this, name, PinKind::Output);
	}
}

void IONode::SaveToJson(json& j) const {
	Node::SaveToJson(j);
	IOPin->SaveToJson(j);
	j["Mode"] = static_cast<int>(Mode);
}

void IONode::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);
	IOPin->LoadFromJson(j, newId);
	Mode = static_cast<PinKind>(j["Mode"]);
}

void IONode::Update() {
	auto builder = BuilderNode::GetBuilder();

	builder->Begin(this->ID);
	auto alpha = IOPin->GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	if (Mode == PinKind::Input) {
		ed::BeginPin(IOPin->ID, ed::PinKind::Input);
		IOPin->DrawPinIcon(IOPin->IsLinked(), (int)(alpha * 255));
		ed::EndPin();
		ImGui::SameLine();
	}
	Utils::SetNextInputWidth(Name, 120.f);
	if(ImGui::InputText("##Name", &Name))
		IOPin->SetValue(Name);

	if (Mode == PinKind::Output) {
		ImGui::SameLine();
		ed::BeginPin(IOPin->ID, ed::PinKind::Output);
		IOPin->DrawPinIcon(IOPin->IsLinked(), (int)(alpha * 255));
		ed::EndPin();
	}
	ImGui::PopStyleVar();
	builder->End();
}

std::string IONode::GetValue(Pin* from) const {
	if (GetMode() == PinKind::Output) {
		return GetPin()->GetLinkedNode()->GetValue();
	}
	else {
		if(Parent)
			return Parent->GetValue(from);

		return Node::GetValue();
	}
}
