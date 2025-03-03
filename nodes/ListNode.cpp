#include "ListNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

std::unordered_map<std::string, ListNode*> ListNode::Map;

void ListNode::Update() {
	auto builder = GetBuilder();

	builder->Begin(ID);
	builder->Header(Color);
	ImGui::Spring(0);

	{
		auto alpha = InputPin->GetAlpha();
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ed::BeginPin(InputPin->ID, ed::PinKind::Input);
		InputPin->DrawPinIcon(InputPin->IsLinked(), (int)(alpha * 255));
		ed::EndPin();
		ImGui::PopStyleVar();
	}

	ImGui::PushID(this);
	ImGui::SetNextItemWidth(150);
	if (ImGui::InputText("##SectionName", &Name)) {
		for (const auto& [_, pLink] : InputPin->Links) {
			auto pPin = Pin::Get(pLink->StartPinID);
			if (auto kv = dynamic_cast<KeyValue*>(pPin))
				kv->SetValue(Name);
		}
	}
	ImGui::PopID();

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
	lastMaxSize = maxSize + 40.f;
	maxSize = 0;
	while (i < this->KeyValues.size()) {
		if (IsFolded)
			break;

		auto& kv = this->KeyValues[i];
		if (!kv->IsFolded) {
			builder->Output(kv->ID);
			UnFoldedKeyValues(*kv, builder); // 展开状态下的渲染
			builder->EndOutput();
			i++;
		}
		else
			FoldedKeyValues(i); // 检测连续折叠的区域
	}

	ImGui::PushID(this);
	ImGui::Spring(1);
	ImGui::Dummy(ImVec2(0, 1));
	ImGui::Spring(0);
	ImVec2 buttonSize(lastMaxSize, 4.0f);
	if (ImGui::Button("##AddKeyValue", buttonSize)) {
		AddKeyValue("value");
	}
	ImGui::PopID();

	builder->End();
}

void ListNode::SaveToJson(json& j) const {
	Node::SaveToJson(j);

	json inputJson;
	InputPin->SaveToJson(inputJson);
	j["Input"] = inputJson;

	json outputJson;
	OutputPin->SaveToJson(outputJson);
	j["Output"] = outputJson;

	json keyValuesJson;
	for (const auto& kv : KeyValues) {
		json kvJson;
		kv->SaveToJson(kvJson);
		keyValuesJson.push_back(kvJson);
	}
	j["KeyValues"] = keyValuesJson;
}

void ListNode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);

	InputPin = std::make_unique<Pin>(-1, "input");
	InputPin->Node = this;
	InputPin->LoadFromJson(j["Input"]);

	OutputPin = std::make_unique<Pin>(-1, "output");
	OutputPin->Node = this;
	OutputPin->LoadFromJson(j["Output"]);

	for (const auto& kvJson : j["KeyValues"])
		AddKeyValue("", -1)->LoadFromJson(kvJson);
}

ValuePin* ListNode::AddKeyValue(const std::string& value, int pinid, bool isInherited, bool isComment, bool isFolded) {
	if (!pinid)
		pinid = MainWindow::GetNextId();

	auto& kv = KeyValues.emplace_back(std::make_unique<ValuePin>(this, value, pinid));
	kv->IsInherited = isInherited;
	kv->IsComment = isComment;
	kv->IsFolded = isFolded;

	return kv.get();
}

void ListNode::UnFoldedKeyValues(ValuePin& kv, ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder) {
	auto alpha = kv.GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ImGui::PushID(&kv);

	const bool isDisabled = kv.IsInherited || kv.IsComment || IsComment;
	if (isDisabled) {
		ImGui::TextDisabled("; %s", kv.Value.c_str());
	}
	else {
		// 获取当前键的类型信息（假设已实现类型查找逻辑）
		auto typeInfo = TypeSystem::Get().GetTypeInfo(this->TypeName);

		// 根据类型绘制不同控件
		ImGui::PushItemWidth(120);
		auto ms = maxSize;
		maxSize = kv.DrawValueWidget(kv.Value, typeInfo);
		// 这里的逻辑是，利用maxsize暂存value的长度，因此把原maxSize的值存到ms里
		// 所以比较的长度是key的长度（w1）和value的长度（maxSize）之和与原maxSize（ms）
		maxSize = std::max(maxSize, ms);
		ImGui::PopItemWidth();
	}

	ImGui::Spring(0);
	kv.DrawPinIcon(kv.IsLinked(), (int)(alpha * 255));

	ImGui::PopID();
	ImGui::PopStyleVar();
}

void ListNode::FoldedKeyValues(size_t& i) {
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
