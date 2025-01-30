#include "SectionNode.h"
#include "MainWindow.h"

void SectionNode::Update() {
	auto builder = GetBuilder();

	builder->Begin(this->ID);
	builder->Header(this->Color);
	ImGui::Spring(0);
	ImGui::TextUnformatted(this->Name.c_str());
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	builder->EndHeader();

	// 渲染键值对
	for (auto& kv : this->KeyValues) {
		ImGui::PushID(&kv);

		// Key输入框
		ImGui::SetNextItemWidth(80);
		if (ImGui::InputText("##Key", &kv.Key)) {
			// 处理key修改
		}

		ImGui::SameLine();

		// Value输入框
		ImGui::SetNextItemWidth(120);
		if (kv.HasConnection) {
			ImGui::BeginDisabled();
			ImGui::InputText("##Value", &kv.Value, ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();
		}
		else {
			if (ImGui::InputText("##Value", &kv.Value)) {
				// 处理value修改
			}
		}

		ImGui::SameLine();

		// 输出引脚
		float alpha = kv.OutputPin.GetAlpha(Owner->newLinkPin);
		ed::BeginPin(kv.OutputPin.ID, ed::PinKind::Output);
		ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
		kv.OutputPin.DrawPinIcon(Owner->IsPinLinked(kv.OutputPin.ID), (int)(alpha * 255));
		ed::EndPin();

		ImGui::PopID();
	}

	// 添加新键值对的按钮
	if (ImGui::Button("+ Add Key")) {
		auto newID = Owner->GetNextId();
		this->KeyValues.emplace_back(KeyValuePair{
			"", "",
			Pin(newID, "", PinType::String),
			false
		});
	}

	builder->End();
}