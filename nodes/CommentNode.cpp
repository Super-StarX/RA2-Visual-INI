#define IMGUI_DEFINE_MATH_OPERATORS
#include "CommentNode.h"
#include "BuilderNode.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

CommentNode::CommentNode(const char* name, int id) :
	Node(name, id) {
	Name.Multiline = true;
}

void CommentNode::Update() {
	auto builder = BuilderNode::GetBuilder();
	builder->Begin(ID);

	ImGui::PushID(this);
	ImGui::BeginGroup();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

	Name.Render();

	ImGui::PopStyleVar();
	ImGui::EndGroup();
	ImGui::PopID();

	builder->End();
}