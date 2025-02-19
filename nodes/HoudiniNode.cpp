#include "HoudiniNode.h"
#include "MainWindow.h"
#include <imgui_node_editor_internal.h>

void HoudiniNode::Update() {
	const float rounding = 10.0f;
	const float padding = 12.0f;


	ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(229, 229, 229, 200));
	ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(125, 125, 125, 200));
	ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(229, 229, 229, 60));
	ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(125, 125, 125, 60));

	const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

	ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
	ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
	ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
	ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
	ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
	ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
	ed::PushStyleVar(ed::StyleVar_PinRadius, 6.0f);
	ed::BeginNode(this->ID);

	ImGui::BeginVertical(this->ID.AsPointer());
	if (!this->Inputs.empty()) {
		ImGui::BeginHorizontal("inputs");
		ImGui::Spring(1, 0);

		ImRect inputsRect;
		int inputAlpha = 200;
		for (auto& pin : this->Inputs) {
			ImGui::Dummy(ImVec2(padding, padding));
			inputsRect = ed::Detail::ImGui_GetItemRect();
			ImGui::Spring(1, 0);
			inputsRect.Min.y -= padding;
			inputsRect.Max.y -= padding;

#if IMGUI_VERSION_NUM > 18101
			const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
#else
			const auto allRoundCornersFlags = 15;
#endif
			//ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
			//ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
			ed::PushStyleVar(ed::StyleVar_PinCorners, allRoundCornersFlags);

			ed::BeginPin(pin.ID, ed::PinKind::Input);
			ed::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
			ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
			ed::EndPin();
			//ed::PopStyleVar(3);
			ed::PopStyleVar(1);

			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(inputsRect.GetTL(), inputsRect.GetBR(),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);
			drawList->AddRect(inputsRect.GetTL(), inputsRect.GetBR(),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);

			inputAlpha = pin.GetAlpha();
		}

		//ImGui::Spring(1, 0);
		ImGui::EndHorizontal();
	}

	ImGui::BeginHorizontal("content_frame");
	ImGui::Spring(1, padding);

	ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
	ImGui::Dummy(ImVec2(160, 0));
	ImGui::Spring(1);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::PopStyleColor();
	ImGui::Spring(1);
	ImGui::EndVertical();
	auto contentRect = ed::Detail::ImGui_GetItemRect();

	ImGui::Spring(1, padding);
	ImGui::EndHorizontal();

	if (!this->Outputs.empty()) {
		ImGui::BeginHorizontal("outputs");
		ImGui::Spring(1, 0);

		ImRect outputsRect;
		int outputAlpha = 200;
		for (auto& pin : this->Outputs) {
			ImGui::Dummy(ImVec2(padding, padding));
			outputsRect = ed::Detail::ImGui_GetItemRect();
			ImGui::Spring(1, 0);
			outputsRect.Min.y += padding;
			outputsRect.Max.y += padding;

#if IMGUI_VERSION_NUM > 18101
			const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
			const auto topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
#else
			const auto allRoundCornersFlags = 15;
			const auto topRoundCornersFlags = 3;
#endif

			ed::PushStyleVar(ed::StyleVar_PinCorners, topRoundCornersFlags);
			ed::BeginPin(pin.ID, ed::PinKind::Output);
			ed::PinPivotRect(outputsRect.GetCenter(), outputsRect.GetCenter());
			ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
			ed::EndPin();
			ed::PopStyleVar();


			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR(),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);
			drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR(),
				IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);

			outputAlpha = pin.GetAlpha();
		}

		ImGui::EndHorizontal();
	}

	ImGui::EndVertical();

	ed::EndNode();
	ed::PopStyleVar(7);
	ed::PopStyleColor(4);

	// auto drawList = ed::GetNodeBackgroundDrawList(this->ID);

	//const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
	//const auto unitSize    = 1.0f / fringeScale;

	//const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
	//{
	//    if ((col >> 24) == 0)
	//        return;
	//    drawList->PathRect(a, b, rounding, rounding_corners);
	//    drawList->PathStroke(col, true, thickness);
	//};

	//drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
	//    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
	//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	//drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
	//    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
	//ImGui::PopStyleVar();
	//drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
	//    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
	////ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	//drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
	//    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
	////ImGui::PopStyleVar();
	//drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
	//drawList->AddRect(
	//    contentRect.GetTL(),
	//    contentRect.GetBR(),
	//    IM_COL32(48, 128, 255, 100), 0.0f);
	//ImGui::PopStyleVar();
}
