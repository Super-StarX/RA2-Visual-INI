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
	ImGui::Dummy(ImVec2(0, 28));
	ImGui::Spring(0);
	builder->EndHeader();

	// 渲染键值对
	size_t i = 0;
	while (i < this->KeyValues.size()) {
		if (IsComment)
			break;

		auto& kv = this->KeyValues[i];
		if (!kv.IsFolded) {
			// 展开状态下的渲染
			auto alpha = kv.OutputPin.GetAlpha(Owner->newLinkPin);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::PushID(&kv);
			builder->Output(kv.OutputPin.ID);

			const bool isDisabled = kv.IsInherited || kv.IsComment;
			if (isDisabled)
				ImGui::TextDisabled("; %s = %s", kv.Key.c_str(), kv.Value.c_str());
			else {
				ImGui::SetNextItemWidth(80);
				ImGui::InputText("##Key", &kv.Key, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);

				ImGui::SetNextItemWidth(120);
				ImGui::InputText("##Value", &kv.Value, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);
			}

			ImGui::Spring(0);
			kv.OutputPin.DrawPinIcon(Owner->IsPinLinked(kv.OutputPin.ID), (int)(alpha * 255));

			builder->EndOutput();
			ImGui::PopID();
			ImGui::PopStyleVar();
			i++;
		}
		else {
			// 检测连续折叠的区域
			size_t foldStart = i;
			while (i < this->KeyValues.size() && this->KeyValues[i].IsFolded)
				i++;

			// 绘制合并的折叠线
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImVec2 buttonSize(230, 2.0f);
			ImGui::PushID(static_cast<int>(foldStart)); // 确保唯一ID
			if (ImGui::Button("", buttonSize)) {
				// 展开所有连续折叠的项
				for (size_t j = foldStart; j < i; j++)
					this->KeyValues[j].IsFolded = false;
			}
			ImGui::PopID();
			ImGui::PopStyleVar();
		}
	}

	builder->End();
}