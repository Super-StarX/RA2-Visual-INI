#define IMGUI_DEFINE_MATH_OPERATORS
#include "CommentNode.h"
#include "BuilderNode.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>


void CommentNode::Update() {
	const float frameRounding = 6.0f;       // 圆角半径
	const ImVec4 bgColor(1.0f, 1.0f, 1.0f, 0.2f); // RGBA背景色
	const ImVec4 borderColor(1.0f, 1.0f, 1.0f, 0.4f); // 边框颜色
	auto builder = BuilderNode::GetBuilder();
	builder->Begin(this->ID);

	// 设置节点样式
	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(bgColor));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(borderColor));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));

	{
		ImGui::PushID(ID.AsPointer());

		// 自动计算初始尺寸
		if (Size.x <= 0 || Size.y <= 0)
			Size = ImVec2(200, 120);

		// 标题栏区域
		if (ShowTitle) {
			ImGui::BeginHorizontal("TitleBar");
			{
				// 可编辑标题
				ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
				ImGui::SetNextItemWidth(-1);

				ImGui::InputText("##Title", &Name);

				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
			}
			ImGui::EndHorizontal();
			ImGui::Spacing();
		}

		// 内容编辑区域
		ImGui::BeginVertical("ContentRegion", Size);
		{
			// 带自适应尺寸的多行文本框
			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 30));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

			const ImVec2 contentSize(
				ImGui::GetContentRegionAvail().x,
				std::max(20.0f, Size.y - (ShowTitle ? 30.0f : 0.0f))
			);

			if (ImGui::InputTextMultiline("##Content", &Content, contentSize,
				ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine)) {
				// 内容变化处理
			}

			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();

			// 自动更新节点尺寸
			//const ImVec2 newSize = ImGui::GetItemRectSize() + ImVec2(16, 16);
			//if (!ImGui::IsMouseDragging(0)) // 避免拖动时冲突
			//	Size = newSize;
		}
		ImGui::EndVertical();

		// 应用尺寸到节点
		ed::Group(Size);

		ImGui::PopID();
	}

	// 恢复样式
	ImGui::PopStyleVar(2);
	ed::PopStyleColor(2);

	builder->End();

	// 绘制调整手柄
	if (ed::IsNodeSelected(ID)) {
		const ImVec2 handleSize(8, 8);
		const ImVec2 nodeMax = ed::GetNodePosition(ID) + Size;

		ed::Suspend();
		ImGui::SetCursorScreenPos(nodeMax - handleSize);
		if (ImGui::InvisibleButton("Resize", handleSize)) {
			// 处理调整尺寸逻辑
			if (ImGui::IsMouseDragging(0)) {
				const ImVec2 delta = ImGui::GetMouseDragDelta(0);
				Size += delta;
				ImGui::ResetMouseDragDelta();
			}
		}
		ed::Resume();
	}
}