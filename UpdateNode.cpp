#define IMGUI_DEFINE_MATH_OPERATORS
#include "MainWindow.h"
#include "utilities/builders.h"
#include "utilities/widgets.h"

#include <imgui_internal.h>

#include <algorithm>
#include <utility>

static inline ImRect ImGui_GetItemRect() {
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y) {
	auto result = rect;
	result.Min.x -= x;
	result.Min.y -= y;
	result.Max.x += x;
	result.Max.y += y;
	return result;
}

void MainWindow::BlueprintNode(Pin* newLinkPin) {
	ax::NodeEditor::Utilities::BlueprintNodeBuilder builder(m_HeaderBackground, GetTextureWidth(m_HeaderBackground), GetTextureHeight(m_HeaderBackground));
	for (auto& node : m_Nodes) {
		if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
			continue;

		const auto isSimple = node.Type == NodeType::Simple;

		bool hasOutputDelegates = false;
		for (auto& output : node.Outputs)
			if (output.Type == PinType::Delegate)
				hasOutputDelegates = true;

		builder.Begin(node.ID);
		if (!isSimple) {
			builder.Header(node.Color);
			ImGui::Spring(0);
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::Spring(1);
			ImGui::Dummy(ImVec2(0, 28));
			if (hasOutputDelegates) {
				ImGui::BeginVertical("delegates", ImVec2(0, 28));
				ImGui::Spring(1, 0);
				for (auto& output : node.Outputs) {
					if (output.Type != PinType::Delegate)
						continue;

					auto alpha = ImGui::GetStyle().Alpha;
					if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
						alpha = alpha * (48.0f / 255.0f);

					ed::BeginPin(output.ID, ed::PinKind::Output);
					ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
					ed::PinPivotSize(ImVec2(0, 0));
					ImGui::BeginHorizontal(output.ID.AsPointer());
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
					if (!output.Name.empty()) {
						ImGui::TextUnformatted(output.Name.c_str());
						ImGui::Spring(0);
					}
					DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
					ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
					ImGui::EndHorizontal();
					ImGui::PopStyleVar();
					ed::EndPin();

					//DrawItemRect(ImColor(255, 0, 0));
				}
				ImGui::Spring(1, 0);
				ImGui::EndVertical();
				ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
			}
			else
				ImGui::Spring(0);
			builder.EndHeader();
		}

		for (auto& input : node.Inputs) {
			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			builder.Input(input.ID);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
			ImGui::Spring(0);
			if (!input.Name.empty()) {
				ImGui::TextUnformatted(input.Name.c_str());
				ImGui::Spring(0);
			}
			if (input.Type == PinType::Bool) {
				ImGui::Button("Hello");
				ImGui::Spring(0);
			}
			ImGui::PopStyleVar();
			builder.EndInput();
		}

		if (isSimple) {
			builder.Middle();

			ImGui::Spring(1, 0);
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::Spring(1, 0);
		}

		for (auto& output : node.Outputs) {
			if (!isSimple && output.Type == PinType::Delegate)
				continue;

			auto alpha = ImGui::GetStyle().Alpha;
			if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
				alpha = alpha * (48.0f / 255.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			builder.Output(output.ID);
			if (output.Type == PinType::String) {
				static char buffer[128] = "Edit Me\nMultiline!";
				static bool wasActive = false;

				ImGui::PushItemWidth(100.0f);
				ImGui::InputText("##edit", buffer, 127);
				ImGui::PopItemWidth();
				if (ImGui::IsItemActive() && !wasActive) {
					ed::EnableShortcuts(false);
					wasActive = true;
				}
				else if (!ImGui::IsItemActive() && wasActive) {
					ed::EnableShortcuts(true);
					wasActive = false;
				}
				ImGui::Spring(0);
			}
			if (!output.Name.empty()) {
				ImGui::Spring(0);
				ImGui::TextUnformatted(output.Name.c_str());
			}
			ImGui::Spring(0);
			DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
			ImGui::PopStyleVar();
			builder.EndOutput();
		}

		builder.End();
	}
}

void MainWindow::TreeNode(Pin* newLinkPin) {
	for (auto& node : m_Nodes) {
		if (node.Type != NodeType::Tree)
			continue;

		const float rounding = 5.0f;
		const float padding = 12.0f;

		const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

		ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
		ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
		ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
		ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

		ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
		ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
		ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
		ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
		ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
		ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
		ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
		ed::BeginNode(node.ID);

		ImGui::BeginVertical(node.ID.AsPointer());
		ImGui::BeginHorizontal("inputs");
		ImGui::Spring(0, padding * 2);

		ImRect inputsRect;
		int inputAlpha = 200;
		if (!node.Inputs.empty()) {
			auto& pin = node.Inputs[0];
			ImGui::Dummy(ImVec2(0, padding));
			ImGui::Spring(1, 0);
			inputsRect = ImGui_GetItemRect();

			ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
			ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
#if IMGUI_VERSION_NUM > 18101
			ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
#else
			ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
#endif
			ed::BeginPin(pin.ID, ed::PinKind::Input);
			ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
			ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
			ed::EndPin();
			ed::PopStyleVar(3);

			if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
				inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
		}
		else
			ImGui::Dummy(ImVec2(0, padding));

		ImGui::Spring(0, padding * 2);
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal("content_frame");
		ImGui::Spring(1, padding);

		ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
		ImGui::Dummy(ImVec2(160, 0));
		ImGui::Spring(1);
		ImGui::TextUnformatted(node.Name.c_str());
		ImGui::Spring(1);
		ImGui::EndVertical();
		auto contentRect = ImGui_GetItemRect();

		ImGui::Spring(1, padding);
		ImGui::EndHorizontal();

		ImGui::BeginHorizontal("outputs");
		ImGui::Spring(0, padding * 2);

		ImRect outputsRect;
		int outputAlpha = 200;
		if (!node.Outputs.empty()) {
			auto& pin = node.Outputs[0];
			ImGui::Dummy(ImVec2(0, padding));
			ImGui::Spring(1, 0);
			outputsRect = ImGui_GetItemRect();

#if IMGUI_VERSION_NUM > 18101
			ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);
#else
			ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
#endif
			ed::BeginPin(pin.ID, ed::PinKind::Output);
			ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
			ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
			ed::EndPin();
			ed::PopStyleVar();

			if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
				outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
		}
		else
			ImGui::Dummy(ImVec2(0, padding));

		ImGui::Spring(0, padding * 2);
		ImGui::EndHorizontal();

		ImGui::EndVertical();

		ed::EndNode();
		ed::PopStyleVar(7);
		ed::PopStyleColor(4);

		auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

		//const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
		//const auto unitSize    = 1.0f / fringeScale;

		//const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
		//{
		//    if ((col >> 24) == 0)
		//        return;
		//    drawList->PathRect(a, b, rounding, rounding_corners);
		//    drawList->PathStroke(col, true, thickness);
		//};

#if IMGUI_VERSION_NUM > 18101
		const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
		const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;
#else
		const auto    topRoundCornersFlags = 1 | 2;
		const auto bottomRoundCornersFlags = 4 | 8;
#endif

		drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
			IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
		//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
			IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
		//ImGui::PopStyleVar();
		drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
			IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
		//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
			IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
		//ImGui::PopStyleVar();
		drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
		drawList->AddRect(
			contentRect.GetTL(),
			contentRect.GetBR(),
			IM_COL32(48, 128, 255, 100), 0.0f);
		//ImGui::PopStyleVar();
	}
}

void MainWindow::HoudiniNode(Pin* newLinkPin) {
	for (auto& node : m_Nodes) {
		if (node.Type != NodeType::Houdini)
			continue;

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
		ed::BeginNode(node.ID);

		ImGui::BeginVertical(node.ID.AsPointer());
		if (!node.Inputs.empty()) {
			ImGui::BeginHorizontal("inputs");
			ImGui::Spring(1, 0);

			ImRect inputsRect;
			int inputAlpha = 200;
			for (auto& pin : node.Inputs) {
				ImGui::Dummy(ImVec2(padding, padding));
				inputsRect = ImGui_GetItemRect();
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

				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
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
		ImGui::TextUnformatted(node.Name.c_str());
		ImGui::PopStyleColor();
		ImGui::Spring(1);
		ImGui::EndVertical();
		auto contentRect = ImGui_GetItemRect();

		ImGui::Spring(1, padding);
		ImGui::EndHorizontal();

		if (!node.Outputs.empty()) {
			ImGui::BeginHorizontal("outputs");
			ImGui::Spring(1, 0);

			ImRect outputsRect;
			int outputAlpha = 200;
			for (auto& pin : node.Outputs) {
				ImGui::Dummy(ImVec2(padding, padding));
				outputsRect = ImGui_GetItemRect();
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


				if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
					outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
			}

			ImGui::EndHorizontal();
		}

		ImGui::EndVertical();

		ed::EndNode();
		ed::PopStyleVar(7);
		ed::PopStyleColor(4);

		// auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

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
}

void MainWindow::CommentNode() {
	for (auto& node : m_Nodes) {
		if (node.Type != NodeType::Comment)
			continue;

		const float commentAlpha = 0.75f;

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
		ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
		ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
		ed::BeginNode(node.ID);
		ImGui::PushID(node.ID.AsPointer());
		ImGui::BeginVertical("content");
		ImGui::BeginHorizontal("horizontal");
		ImGui::Spring(1);
		ImGui::TextUnformatted(node.Name.c_str());
		ImGui::Spring(1);
		ImGui::EndHorizontal();
		ed::Group(node.Size);
		ImGui::EndVertical();
		ImGui::PopID();
		ed::EndNode();
		ed::PopStyleColor(2);
		ImGui::PopStyleVar();

		if (ed::BeginGroupHint(node.ID)) {
			//auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
			auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

			//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

			auto min = ed::GetGroupMin();
			//auto max = ed::GetGroupMax();

			ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
			ImGui::BeginGroup();
			ImGui::TextUnformatted(node.Name.c_str());
			ImGui::EndGroup();

			auto drawList = ed::GetHintBackgroundDrawList();

			auto hintBounds = ImGui_GetItemRect();
			auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

			drawList->AddRectFilled(
				hintFrameBounds.GetTL(),
				hintFrameBounds.GetBR(),
				IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

			drawList->AddRect(
				hintFrameBounds.GetTL(),
				hintFrameBounds.GetBR(),
				IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

			//ImGui::PopStyleVar();
		}
		ed::EndGroupHint();
	}
}
