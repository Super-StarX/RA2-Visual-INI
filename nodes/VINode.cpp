﻿#include "VINode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "NodeStyle.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

template<typename T>
VINode<T>::VINode(const char* name, int id) :
	Node(name, id) {
	InputPin = std::make_unique<Pin>(this, "input", PinKind::Input);
	OutputPin = std::make_unique<Pin>(this, "output", PinKind::Output);
}

template<typename T>
void VINode<T>::Update() {
	auto builder = BuilderNode::GetBuilder();

	builder->Begin(ID);
	auto* typeInfo = NodeStyleManager::Get().FindType(Style);
	if (!typeInfo) return;
	builder->Header(typeInfo->Color);
	ImGui::Spring(0);

	{
		auto alpha = InputPin->GetAlpha();
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ed::BeginPin(InputPin->ID, ed::PinKind::Input);
		InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
		ed::EndPin();
		ImGui::PopStyleVar();
	}

	if (Name.Render() && this->PinNameSyncable())
		for (const auto& [_, pLink] : InputPin->Links)
			if (auto pPin = Pin::Get(pLink->StartPinID))
				if (pPin->Node->PinNameChangable())
					pPin->SetValue(Name);

	{
		auto alpha = OutputPin->GetAlpha();
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ed::BeginPin(OutputPin->ID, ed::PinKind::Output);
		OutputPin->DrawPinIcon(OutputPin->IsLinked(), (int)(alpha * 255));
		ed::EndPin();
		ImGui::PopStyleVar();
	}

	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 28));
	ImGui::Spring(0);
	builder->EndHeader();

	// 渲染键值对
	size_t i = 0;
	lastMaxSize = std::max(maxSize + 8.f, 130.f);
	maxSize = 0;

	if (!IsFolded) {
		while (i < this->KeyValues.size()) {
			auto kv = this->KeyValues[i].get();
			if (!kv->IsFolded) {
				UnFoldedKeyValues(kv, 0); // 展开状态下的渲染
				i++;
			}
			else {
				while (i < this->KeyValues.size() && this->KeyValues[i]->IsFolded)
					i++;
				ImGui::Dummy(ImVec2(0, 2.4f));
			}
		}

		i = 0;
		while (i < this->KeyValues.size()) {
			auto kv = this->KeyValues[i].get();
			if (!kv->IsFolded) {
				UnFoldedKeyValues(kv, 1); // 展开状态下的渲染
				i++;
			}
			else
				FoldedKeyValues(i); // 检测连续折叠的区域
		}
	}

	ImGui::PushID(this);
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 1));
	ImGui::Spring(0);
	AddKeyValue();
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(24, 1));
	ImGui::PopID();

	builder->End();

	// 注释节点绘制斜线
	if (this->IsComment) {
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(min.x + 5, min.y + 5),
			ImVec2(max.x - 5, max.y - 5),
			IM_COL32(255, 0, 0, 255),
			2.0f
		);
	}
}

template<typename T>
Pin* VINode<T>::GetFirstCompatiblePin(Pin* pin) {
	return pin->Kind == PinKind::Input ? OutputPin.get() : InputPin.get();
}

template<typename T>
void VINode<T>::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	json inputJson;
	InputPin->SaveToJson(inputJson);
	j["Input"] = inputJson;

	json outputJson;
	OutputPin->SaveToJson(outputJson);
	j["Output"] = outputJson;
}

template<typename T>
void VINode<T>::LoadFromJson(const json& j, bool newId) {
	Node::LoadFromJson(j, newId);

	InputPin = std::make_unique<Pin>(this, "input", PinKind::Input, -1);
	InputPin->LoadFromJson(j["Input"], newId);

	OutputPin = std::make_unique<Pin>(this, "output", PinKind::Output, -1);
	OutputPin->LoadFromJson(j["Output"], newId);
}

template<typename T>
void VINode<T>::FoldedKeyValues(size_t& i) {
	size_t foldStart = i;
	while (i < this->KeyValues.size() && this->KeyValues[i]->IsFolded)
		i++;

	// 绘制合并的折叠线
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImVec2 buttonSize(lastMaxSize, 2.0f);
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 0.25));
	ImGui::Spring(0);
	ImGui::PushID(static_cast<int>(foldStart)); // 确保唯一ID
	// 展开所有连续折叠的项
	if (ImGui::Button("", buttonSize))
		for (size_t j = foldStart; j < i; j++)
			this->KeyValues[j]->IsFolded = false;
	ImGui::Dummy(ImVec2(0, 0.15f));
	ImGui::PopID();
	ImGui::PopStyleVar();
}

template class VINode<ValuePin>;
template class VINode<KeyValue>;