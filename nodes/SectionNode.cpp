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

void SectionNode::LoadFromJson(const json& j, bool newId) {
	VINode::LoadFromJson(j, newId);

	for (const auto& kvJson : j["KeyValues"])
		AddKeyValue("", "", "", -1)->LoadFromJson(kvJson, newId);
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const Pin& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->ID == key.ID; });
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const std::string& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->Key == key; });
}

void SectionNode::AutoSelectType() {
	auto& typeSystem = TypeSystem::Get();
	int maxMatchCount = 0;
	std::string bestType;

	// 预处理当前节点的key集合
	std::unordered_set<std::string> nodeKeys;
	for (const auto& kv : KeyValues) {
		nodeKeys.insert(kv->Key);
	}

	// 遍历所有预定义Section类型
	for (const auto& [sectionName, section] : typeSystem.Sections) {
		// 获取该Section类型的所有key
		const auto& sectionKeys = section.section; // 假设已预先生成key集合

		// 计算匹配度（考虑双向包含）
		int matchCount = 0;
		for (const auto& key : nodeKeys) {
			if (sectionKeys.count(key)) {
				matchCount++;
			}
		}

		// 优化：优先完全匹配
		bool isFullMatch = (matchCount == sectionKeys.size()) &&
			(nodeKeys.size() == sectionKeys.size());

		// 评分规则：基础分 + 完全匹配奖励分
		int finalScore = matchCount * 100 + (isFullMatch ? 500 : 0);

		// 更新最佳匹配（分数相同则取更具体/更新的类型）
		if (finalScore > maxMatchCount ||
		   (finalScore == maxMatchCount && typeSystem.Sections.contains(bestType) &&
			   section.inheritanceLevel > typeSystem.Sections.at(bestType).inheritanceLevel)) {
			maxMatchCount = finalScore;
			bestType = sectionName;
		}
	}

	// 设置最终类型（至少有1个匹配才生效）
	if (maxMatchCount >= 100) { // 至少匹配1个key
		this->TypeName = bestType;  // 假设SectionNode有Type成员
	}
}

void SectionNode::AddKeyValue() {
	ImVec2 buttonSize(lastMaxSize, 4.0f);
	if (ImGui::Button("##Add Key Value", buttonSize)) {
		AddKeyValue("key", "value");
	}
}

void SectionNode::Menu() {
	VINode::Menu();
	if (ImGui::MenuItem("Auto Select Type"))
		AutoSelectType();
}

std::string SectionNode::GetValue(Pin* from) const {
	if (!from || from == InputPin.get())
		return VINode<KeyValue>::GetValue(from);

	return from->GetValue();
}

std::vector<Pin*> SectionNode::GetAllPins() {
	std::vector<Pin*> pins;
	if (InputPin) pins.push_back(InputPin.get());
	if (OutputPin) pins.push_back(OutputPin.get());

	// 添加KeyValues中的引脚
	for (auto& kv : KeyValues) {
		pins.push_back(&kv->InputPin);
		pins.push_back(kv.get());
	}
	return pins;
}

std::vector<Pin*> SectionNode::GetInputPins() {
	std::vector<Pin*> pins;
	if (InputPin) 
		pins.push_back(InputPin.get());
	for (auto& kv : KeyValues)
		pins.push_back(&kv->InputPin);
	return pins;
}

std::vector<Pin*> SectionNode::GetOutputPins() {
	std::vector<Pin*> pins;
	if (OutputPin) 
		pins.push_back(OutputPin.get());
	for (auto& kv : KeyValues)
		pins.push_back(kv.get());
	return pins;
}

KeyValue* SectionNode::AddKeyValue(const std::string& key, const std::string& value, const std::string& comment, int pinid, bool isInherited, bool isComment, bool isFolded) {
	auto& kv = KeyValues.emplace_back(std::make_unique<KeyValue>(this, key, value, comment, pinid));
	kv->IsInherited = isInherited;
	kv->IsComment = isComment;
	kv->IsFolded = isFolded;

	return kv.get();
}

void SectionNode::UnFoldedKeyValues(KeyValue& kv, int mode) {
	auto builder = BuilderNode::GetBuilder();

	if (mode == 0) {
		builder->Input(kv.InputPin.ID);
		auto& inputPin = kv.InputPin;
		auto alpha = inputPin.GetAlpha();
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		inputPin.DrawPinIcon(inputPin.IsLinked(), (int)(alpha * 255));
		ImGui::Spring(0);
		ImGui::PopStyleVar();
		builder->EndInput();
	}
	else if (mode == 1) {
		builder->Output(kv.ID);
		auto alpha = kv.GetAlpha();
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ImGui::PushID(&kv);

		const bool isDisabled = kv.IsInherited || kv.IsComment || IsComment;
		if (isDisabled) {
			ImGui::TextDisabled("; %s = %s", kv.Key.c_str(), kv.GetValue().c_str());
		}
		else {
			auto w1 = Utils::SetNextInputWidth(kv.Key, 60.f);
			ImGui::InputText("##Key", &kv.Key, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);

			// 获取当前键的类型信息（假设已实现类型查找逻辑）
			auto typeInfo = TypeSystem::Get().GetKeyType(this->TypeName, kv.Key);

			// 根据类型绘制不同控件
			ImGui::PushItemWidth(120);
			auto ms = maxSize;
			auto value = kv.GetValue();
			maxSize = kv.DrawValueWidget(value, typeInfo);
			kv.SetValue(value);
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
}
