#include "SimpleNode.h"
#include "MainWindow.h"

void SimpleNode::Update() {
	auto builder = GetBuilder();

	auto newLinkPin = MainWindow::newLinkPin;

	bool hasOutputDelegates = false;
	for (auto& output : this->Outputs)
		if (output.TypeIdentifier == "delegate")
			hasOutputDelegates = true;

	builder->Begin(this->ID);

	for (auto& input : this->Inputs)
		UpdateInput(input);

	builder->Middle();

	ImGui::Spring(1, 0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1, 0);

	for (auto& output : this->Outputs)
		UpdateOutput(output);

	builder->End();
}