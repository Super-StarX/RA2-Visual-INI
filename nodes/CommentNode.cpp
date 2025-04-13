#define IMGUI_DEFINE_MATH_OPERATORS
#include "CommentNode.h"
#include "BuilderNode.h"
#include "Utils.h"
#include "MainWindow.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

CommentNode::CommentNode(const char* name, int id) :
	Node(name, id) {
}

void CommentNode::Update() {
	auto builder = BuilderNode::GetBuilder();
	builder->Begin(ID);

	ImGui::PushID(this);
	ImGui::BeginGroup();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

	ImGui::TextWrapped("%s", Name.c_str());

	if (ed::GetHoveredNode() == ID && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
		MainWindow::Instance->m_ShowCommentEditor = this;
		ShowEditPopup = true;
	}

	ImGui::PopStyleVar();
	ImGui::EndGroup();
	ImGui::PopID();

	builder->End();
}

void CommentNode::CommentEditorPopup() {
	if (!ShowEditPopup)
		return;

	ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
	ImGui::Begin(("编辑注释##" + std::to_string(ID.Get())).c_str(), &ShowEditPopup,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

	// 初始化文本一次
	static ed::NodeId lastEditId = 0;
	if (lastEditId != ID) {
		TempText = Name;
		lastEditId = ID;
	}

	// 自适应高度、最长一行宽度
	const float MaxWidth = 500.0f;
	const float MaxHeight = 300.0f;
	const float Padding = 20.0f;

	std::vector<std::string> lines = Utils::SplitString(TempText, '\n');
	ImFont* font = ImGui::GetFont();
	float maxLineWidth = 0.0f;
	for (const auto& line : lines) {
		float w = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, line.c_str()).x;
		maxLineWidth = std::max(maxLineWidth, w);
	}
	float width = std::clamp(maxLineWidth + Padding, 200.0f, MaxWidth);
	float height = std::clamp((float)(lines.size() + 1) * font->FontSize + Padding, 100.0f, MaxHeight);

	// 渲染文本框
	ImGui::InputTextMultiline("##CommentEditBox", &TempText, ImVec2(width, height));

	ImGui::Separator();
	if (ImGui::Button("确定", ImVec2(120, 0))) {
		Name = TempText;
		MainWindow::Instance->m_ShowCommentEditor = nullptr;
	}
	ImGui::SameLine();
	if (ImGui::Button("取消", ImVec2(120, 0))) {
		MainWindow::Instance->m_ShowCommentEditor = nullptr;
	}

	ImGui::End();
}


