#include "SectionNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include "Pins/KeyValue.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

std::unordered_map<std::string, SectionNode*> SectionNode::Map;

void SectionNode::Update() {
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
			UnFoldedKeyValues(*kv, builder); // 展开状态下的渲染
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
		AddKeyValue("key", "value");
	}
	ImGui::PopID();

	builder->End();
}

Pin* SectionNode::GetFirstCompatiblePin(Pin* pin) {
	return pin->Kind == PinKind::Input ? OutputPin.get() : InputPin.get();
}

void SectionNode::SaveToJson(json& j) const {
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

void SectionNode::LoadFromJson(const json& j) {
	Node::LoadFromJson(j);

	InputPin = std::make_unique<Pin>(-1, "input");
	InputPin->Node = this;
	InputPin->LoadFromJson(j["Input"]);

	OutputPin = std::make_unique<Pin>(-1, "output");
	OutputPin->Node = this;
	OutputPin->LoadFromJson(j["Output"]);

	for (const auto& kvJson : j["KeyValues"])
		AddKeyValue("", "", -1)->LoadFromJson(kvJson);
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const Pin& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->ID == key.ID; });
}

std::vector<std::unique_ptr<KeyValue>>::iterator SectionNode::FindPin(const std::string& key) {
	return std::find_if(KeyValues.begin(), KeyValues.end(), [&key](const std::unique_ptr<KeyValue>& kv) { return kv->Key == key; });
}

KeyValue* SectionNode::AddKeyValue(const std::string& key, const std::string& value, int pinid, bool isInherited, bool isComment, bool isFolded) {
	if (!pinid)
		pinid = MainWindow::GetNextId();

	auto& kv = KeyValues.emplace_back(std::make_unique<KeyValue>(this, key, value, pinid));
	kv->IsInherited = isInherited;
	kv->IsComment = isComment;
	kv->IsFolded = isFolded;

	return kv.get();
}

void SectionNode::UnFoldedKeyValues(KeyValue& kv, ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder) {
	auto alpha = kv.GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ImGui::PushID(&kv);
	builder->Output(kv.ID);

	const bool isDisabled = kv.IsInherited || kv.IsComment || IsComment;
	if (isDisabled) {
		ImGui::TextDisabled("; %s = %s", kv.Key.c_str(), kv.Value.c_str());
	}
	else {
		auto w1 = Utils::SetNextInputWidth(kv.Key, 60.f);
		ImGui::InputText("##Key", &kv.Key, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);

		// 获取当前键的类型信息（假设已实现类型查找逻辑）
		auto typeInfo = GetKeyTypeInfo(this->TypeName, kv.Key);

		// 根据类型绘制不同控件
		ImGui::PushItemWidth(120);
		auto ms = maxSize;
		DrawValueWidget(kv, typeInfo);
		// 这里的逻辑是，利用maxsize暂存value的长度，因此把原maxSize的值存到ms里
		// 所以比较的长度是key的长度（w1）和value的长度（maxSize）之和与原maxSize（ms）
		maxSize = std::max(maxSize + w1, ms);
		ImGui::PopItemWidth();
	}

	ImGui::Spring(0);
	kv.DrawPinIcon(kv.IsLinked(), (int)(alpha * 255));

	builder->EndOutput();
	ImGui::PopID();
	ImGui::PopStyleVar();
}

void SectionNode::FoldedKeyValues(size_t& i) {
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
	ImGui::Dummy(ImVec2(0, 0.15));
	ImGui::PopID();
	ImGui::PopStyleVar();
}

void SectionNode::DrawValueWidget(KeyValue& kv, const TypeInfo& type) {
	std::string& value = kv.Value;
	//const float itemWidth = ImGui::GetContentRegionAvail().x * 0.6f;
	//ImGui::PushItemWidth(itemWidth);
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 3));
	float width = 120.f;

	switch (type.Category) {
	case TypeCategory::NumberLimit: {
		int numValue = atoi(value.c_str());
		bool modified = ImGui::DragInt("##num", &numValue, 1,
			std::get<NumberLimit>(type.Data).Min, std::get<NumberLimit>(type.Data).Max, "%d",
			ImGuiSliderFlags_AlwaysClamp);
		if (modified || numValue < std::get<NumberLimit>(type.Data).Min ||
			numValue > std::get<NumberLimit>(type.Data).Max) {
			numValue = std::clamp(numValue,
				std::get<NumberLimit>(type.Data).Min, std::get<NumberLimit>(type.Data).Max);
			value = std::to_string(numValue);
		}
		break;
	}
	case TypeCategory::StringLimit: {
		if (!std::get<StringLimit>(type.Data).ValidValues.empty()) {
			int current = Utils::GetComboIndex(value, std::get<StringLimit>(type.Data).ValidValues);
			if (ImGui::Combo("##str", &current,
				Utils::GetComboItems(std::get<StringLimit>(type.Data).ValidValues))) {
				value = std::get<StringLimit>(type.Data).ValidValues[current];
			}
		}
		else {
			auto w = Utils::SetNextInputWidth(value, 100.f);
			width = std::max(width, w);
			ImGui::InputText("##str", &value);
		}
		break;
	}
	case TypeCategory::List: {
		DrawListInput(value, std::get<ListType>(type.Data));
		break;
	}
	case TypeCategory::Bool: {
		// 解析字符串为布尔值
		bool boolValue = (value == "true");
		if (ImGui::Checkbox("##bool", &boolValue))
			value = boolValue ? "true" : "false"; // 更新字符串值
		break;
	}
	case TypeCategory::Color: {
		// 解析字符串为颜色值
		ImVec4 color;
		int iR, iG, iB;
		if (sscanf_s(value.c_str(), "%d,%d,%d", &iR, &iG, &iB) == 3) {
			color = ImVec4(iR / 255.f, iG / 255.f, iB / 255.f, 1.0f);
		}
		else {
			iR = iG = iB = 255;
			color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色
		}

		std::string R = std::to_string(iR);
		std::string G = std::to_string(iG);
		std::string B = std::to_string(iB);

		bool valueChanged = false;
		ImGui::PushItemWidth(40);
		if (ImGui::InputText("##ColorR", &R))
			valueChanged = true;
		if (ImGui::InputText("##ColorG", &G))
			valueChanged = true;
		if (ImGui::InputText("##ColorB", &B))
			valueChanged = true;

		ImGui::PopItemWidth();

		if (ImGui::ColorButton("##ColorButton", color, ImGuiColorEditFlags_NoAlpha)) {
			//ImGui::OpenPopup("KVColorPicker");
		}
		/*
		if (ImGui::BeginPopup("KVColorPicker")) {
			if (ImGui::ColorPicker3("##picker", &color.x))
				;

			ImGui::EndPopup();
		}
		*/

		if (valueChanged) {	// 将颜色值保存回字符串
			char buffer[32];
			auto convert = [&](const std::string& s) {
				try {
					int i = std::stoi(s);
					return std::clamp(i, 0, 255);
				}
				catch (std::exception&) {
					return 255;
				}
			};

			iR = convert(R);
			iG = convert(G);
			iB = convert(B);

			snprintf(buffer, sizeof(buffer), "%d,%d,%d", iR, iG, iB);
			value = buffer;
		}
		break;
	}
	default: {
		auto w = Utils::SetNextInputWidth(kv.Value, 100.f);
		width = std::max(width, w);
		if (ImGui::InputText("##value", &value))
			kv.UpdateOutputLink(kv.Value);
		break;
	}
	}

	maxSize = width;
	//ImGui::PopStyleVar();
	//ImGui::PopItemWidth();
}

// 列表控件绘制
void SectionNode::DrawListInput(std::string& listValue, const ListType& listType) {
	std::vector<std::string> elements = Utils::SplitString(listValue, ',');

	// 自动调整元素数量
	elements.resize(std::clamp(
		int(elements.size()),
		listType.MinLength,
		listType.MaxLength
	));

	// 特殊处理多选类型
	if (auto elemType = GetTypeInfo(listType.ElementType);
		elemType.Category == TypeCategory::StringLimit &&
		!std::get<StringLimit>(elemType.Data).ValidValues.empty()) {
		// 多选控件
		std::unordered_set<std::string> selected(elements.begin(), elements.end());
		if (ImGui::BeginCombo("##multi", "")) {
			for (auto& option : std::get<StringLimit>(elemType.Data).ValidValues) {
				bool isSelected = selected.count(option);
				if (ImGui::Selectable(option.c_str(), isSelected)) {
					if (isSelected) selected.erase(option);
					else selected.insert(option);
				}
			}
			ImGui::EndCombo();

			// 更新元素列表
			elements.assign(selected.begin(), selected.end());
		}
	}
	else if (listType.MaxLength <= 3) { // 短列表展开显示
		for (size_t i = 0; i < elements.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			if (i > 0) ImGui::SameLine();

			// 递归绘制元素控件
			TypeInfo elemType = GetTypeInfo(listType.ElementType);
			//DrawValueWidget(elements[i], elemType);

			ImGui::PopID();
		}
	}
	else { // 长列表折叠显示
		if (ImGui::Button("Edit List")) {
			// 打开列表编辑窗口（需实现弹出窗口逻辑）
			OpenListEditor(listValue, listType);
		}
	}

	listValue = Utils::JoinStrings(elements, ",");
}

// 列表编辑窗口实现
void SectionNode::OpenListEditor(std::string& listValue, const ListType& listType) {
	static std::string editBuffer;
	static ListType editType;

	if (!ImGui::IsPopupOpen("List Editor")) {
		editBuffer = listValue;
		editType = listType;
		ImGui::OpenPopup("List Editor");
	}

	if (ImGui::BeginPopupModal("List Editor", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		std::vector<std::string> elements = Utils::SplitString(editBuffer, ',');

		// 自动填充/截断
		if (elements.size() < editType.MinLength)
			elements.resize(editType.MinLength);
		else if (elements.size() > editType.MaxLength)
			elements.resize(editType.MaxLength);

		// 元素类型信息
		TypeInfo elemType = TypeSystem::Get().GetTypeInfo(editType.ElementType);

		// 动态绘制元素
		bool modified = false;
		for (size_t i = 0; i < elements.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			ImGui::Text("Item %d:", static_cast<int>(i + 1));
			ImGui::SameLine();

			std::string elemValue = elements[i];
			if (DrawElementEditor(elemValue, elemType)) {
				elements[i] = elemValue;
				modified = true;
			}
			ImGui::PopID();
		}

		// 长度控制按钮
		if (elements.size() < editType.MaxLength) {
			if (ImGui::Button("+ Add Item")) {
				elements.emplace_back("");
				modified = true;
			}
		}
		if (elements.size() > editType.MinLength) {
			ImGui::SameLine();
			if (ImGui::Button("- Remove Last")) {
				elements.pop_back();
				modified = true;
			}
		}

		// 确认操作
		ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			listValue = Utils::JoinStrings(elements, ",");
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

		if (modified)
			editBuffer = Utils::JoinStrings(elements, ",");

		ImGui::EndPopup();
	}
}

// 元素级编辑器
bool SectionNode::DrawElementEditor(std::string& value, const TypeInfo& type) {
	const float width = ImGui::GetContentRegionAvail().x * 0.7f;
	ImGui::PushItemWidth(width);

	bool modified = false;
	std::string original = value;

	switch (type.Category) {
	case TypeCategory::NumberLimit: {
		int numValue = atoi(value.c_str());
		if (ImGui::DragInt("##elem", &numValue, 1,
			std::get<NumberLimit>(type.Data).Min, std::get<NumberLimit>(type.Data).Max)) {
			value = std::to_string(numValue);
			modified = true;
		}
		break;
	}
	case TypeCategory::StringLimit: {
		if (!std::get<StringLimit>(type.Data).ValidValues.empty()) {
			int current = Utils::GetComboIndex(value, std::get<StringLimit>(type.Data).ValidValues);
			if (ImGui::Combo("##elem", &current,
				Utils::GetComboItems(std::get<StringLimit>(type.Data).ValidValues))) {
				value = std::get<StringLimit>(type.Data).ValidValues[current];
				modified = true;
			}
		}
		else if (ImGui::InputText("##elem", &value))
			modified = true;
		break;
	}
	default: {
		if (ImGui::InputText("##elem", &value))
			modified = true;
	}
	}

	ImGui::PopItemWidth();
	return modified || (original != value);
}
