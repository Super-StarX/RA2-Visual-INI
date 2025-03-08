#include "ListNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

std::unordered_map<std::string, ListNode*> ListNode::Map;

void ListNode::SaveToJson(json& j) const {
	VINode::SaveToJson(j);

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

	InputPin = std::make_unique<Pin>(this, "input", PinKind::Input, -1);
	InputPin->LoadFromJson(j["Input"]);

	OutputPin = std::make_unique<Pin>(this, "output", PinKind::Input, -1);
	OutputPin->LoadFromJson(j["Output"]);

	for (const auto& kvJson : j["KeyValues"])
		AddKeyValue("", -1)->LoadFromJson(kvJson);
}

void ListNode::AddKeyValue() {
	ImVec2 buttonSize(lastMaxSize, 4.0f);
	if (ImGui::Button("##Add Value", buttonSize))
		AddKeyValue("value");
}

ValuePin* ListNode::AddKeyValue(const std::string& value, int pinid, bool isInherited, bool isComment, bool isFolded) {
	auto& kv = KeyValues.emplace_back(std::make_unique<ValuePin>(this, value, pinid));
	kv->IsInherited = isInherited;
	kv->IsComment = isComment;
	kv->IsFolded = isFolded;

	return kv.get();
}

void ListNode::UnFoldedKeyValues(ValuePin& kv, bool isDisabled) {
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
}
