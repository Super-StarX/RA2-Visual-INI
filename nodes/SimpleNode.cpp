#include "SimpleNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"

void SimpleNode::Update() {
	auto builder = BuilderNode::GetBuilder();

	auto newLinkPin = MainWindow::newLinkPin;

	bool hasOutputDelegates = false;
	for (auto& output : this->Outputs)
		if (output.TypeIdentifier == "delegate")
			hasOutputDelegates = true;

	builder->Begin(this->ID);

	for (auto& input : this->Inputs)
		BuilderNode::UpdateInput(input);

	builder->Middle();

	ImGui::Spring(1, 0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1, 0);

	for (auto& output : this->Outputs)
		BuilderNode::UpdateOutput(output);

	builder->End();
}