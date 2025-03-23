#include "SectionNode.h"
#include "BuilderNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

std::unordered_map<std::string, SectionNode*> SectionNode::Map;
std::vector<SectionNode*> SectionNode::Array;

SectionNode::SectionNode(const char* name, int id) :
	VINode(name, id) {
	Map[name] = this;
	Array.push_back(this);
}

SectionNode::~SectionNode() {
	Map.erase(Name);
	Array.erase(std::find(Array.begin(), Array.end(), this));
}

void SectionNode::SaveToJson(json& j) const {
	VINode::SaveToJson(j);

	json keyValuesJson;
	for (const auto& kv : KeyValues) {
		json kvJson;
		kv->SaveToJson(kvJson);
		keyValuesJson.push_back(kvJson);
	}
	j["KeyValues"] = keyValuesJson;
}

void SectionNode::LoadFromJson(const json& j) {
	VINode::LoadFromJson(j);

	for (const auto& kvJson : j["KeyValues"])
		AddKeyValue("", "", "", -1)->LoadFromJson(kvJson);
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const Pin& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->ID == key.ID; });
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const std::string& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->Key == key; });
}

void SectionNode::AddKeyValue() {
	ImVec2 buttonSize(lastMaxSize, 4.0f);
	if (ImGui::Button("##Add Key Value", buttonSize)) {
		AddKeyValue("key", "value");
	}
}

KeyValue* SectionNode::AddKeyValue(const std::string& key, const std::string& value, const std::string& comment, int pinid, bool isInherited, bool isComment, bool isFolded) {
	auto& kv = KeyValues.emplace_back(std::make_unique<KeyValue>(this, key, value, comment, pinid));
	kv->IsInherited = isInherited;
	kv->IsComment = isComment;
	kv->IsFolded = isFolded;

	return kv.get();
}

void SectionNode::UnFoldedKeyValues(KeyValue& kv) {
	auto builder = BuilderNode::GetBuilder();

	builder->Input(kv.InputPin.ID);
	auto& inputPin = kv.InputPin;
	inputPin.DrawPinIcon(inputPin.IsLinked(), (int)(inputPin.GetAlpha() * 255));
	builder->EndInput();

	builder->Output(kv.ID);
	auto alpha = kv.GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ImGui::PushID(&kv);

	const bool isDisabled = kv.IsInherited || kv.IsComment || IsComment;
	if (isDisabled) {
		ImGui::TextDisabled("; %s = %s", kv.Key.c_str(), kv.Value.c_str());
	}
	else {
		auto w1 = Utils::SetNextInputWidth(kv.Key, 60.f);
		ImGui::InputText("##Key", &kv.Key, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);

		// 获取当前键的类型信息（假设已实现类型查找逻辑）
		auto typeInfo = TypeSystem::Get().GetKeyType(this->TypeName, kv.Key);

		// 根据类型绘制不同控件
		ImGui::PushItemWidth(120);
		auto ms = maxSize;
		maxSize = kv.DrawValueWidget(kv.Value, typeInfo);
		// 这里的逻辑是，利用maxsize暂存value的长度，因此把原maxSize的值存到ms里
		// 所以比较的长度是key的长度（w1）和value的长度（maxSize）之和与原maxSize（ms）
		maxSize = std::max(maxSize + w1, ms);
		ImGui::PopItemWidth();
	}

	ImGui::Spring(0);
	kv.DrawPinIcon(kv.IsLinked(), (int)(alpha * 255));

	ImGui::PopID();
	ImGui::PopStyleVar();
	builder->EndOutput();
}
