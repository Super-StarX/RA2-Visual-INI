#include "SectionNode.h"
#include "MainWindow.h"
#include "Utils.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>

std::unordered_map<std::string, SectionNode*> SectionNode::Map;
void SectionNode::Update() {
	auto builder = GetBuilder();

	builder->Begin(this->ID);
	builder->Header(this->Color);
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
	if (ImGui::InputText("##SectionName", &this->Name)) {
		for (const auto& [_, pLink] : InputPin->Links) {
			auto pPin = Pin::Get(pLink->StartPinID);
			auto pNode = pPin->Node;
			if (pNode->Type != NodeType::Section)
				continue;
			auto pSNode = reinterpret_cast<SectionNode*>(pNode);
			auto it = std::find_if(pSNode->KeyValues.begin(), pSNode->KeyValues.end(),
				[pPin](const KeyValue& kv) { return kv.OutputPin.get() == pPin; });

			if (it != pSNode->KeyValues.end())
				it->Value = this->Name;
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
	while (i < this->KeyValues.size()) {
		if (IsFolded)
			break;

		auto& kv = this->KeyValues[i];
		if (!kv.IsFolded) {
			UnFoldedKeyValues(kv, builder); // 展开状态下的渲染
			i++;
		}
		else
			FoldedKeyValues(i); // 检测连续折叠的区域
	}
	ImVec2 buttonSize(230, 2.0f);
	if (ImGui::Button("", buttonSize)) {
		AddKeyValue("key", "value");
	}

	builder->End();
}

void SectionNode::UnFoldedKeyValues(KeyValue& kv, ax::NodeEditor::Utilities::BlueprintNodeBuilder* builder) {
	auto alpha = kv.OutputPin->GetAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
	ImGui::PushID(&kv);
	builder->Output(kv.OutputPin->ID);

	const bool isDisabled = kv.IsInherited || kv.IsComment || IsComment;
	if (isDisabled) {
		ImGui::TextDisabled("; %s = %s", kv.Key.c_str(), kv.Value.c_str());
	}
	else {
		ImGui::SetNextItemWidth(80);
		ImGui::InputText("##Key", &kv.Key, kv.IsInherited ? ImGuiInputTextFlags_ReadOnly : 0);

		// 获取当前键的类型信息（假设已实现类型查找逻辑）
		auto typeInfo = GetKeyTypeInfo(this->TypeName, kv.Key);

		// 根据类型绘制不同控件
		ImGui::SetNextItemWidth(120);
		DrawValueWidget(kv.Value, typeInfo);
	}

	ImGui::Spring(0);
	kv.OutputPin->DrawPinIcon(kv.OutputPin->IsLinked(), (int)(alpha * 255));

	builder->EndOutput();
	ImGui::PopID();
	ImGui::PopStyleVar();
}

void SectionNode::FoldedKeyValues(size_t& i) {
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

KeyValue& SectionNode::AddKeyValue(const std::string& key, const std::string& value, int pinid, bool isInherited, bool isComment, bool isFolded) {
	if (!pinid)
		pinid = MainWindow::GetNextId();

	auto& kv = KeyValues.emplace_back(KeyValue{ key, value, std::make_unique<Pin>(pinid, key.c_str()), isInherited, isComment, isFolded });
	kv.OutputPin->Node = this;
	kv.OutputPin->Kind = PinKind::Output;

	return kv;
}

void SectionNode::DrawValueWidget(std::string& value, const TypeInfo& type) {
	const float itemWidth = ImGui::GetContentRegionAvail().x * 0.6f;
	ImGui::PushItemWidth(itemWidth);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 3));

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
		else
			ImGui::InputText("##str", &value);
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
		if (sscanf_s(value.c_str(), "%f,%f,%f", &color.x, &color.y, &color.z) != 3) {
			color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色
		}

		if (ImGui::ColorEdit3("##color", &color.x)) {
			// 将颜色值保存回字符串
			char buffer[64];
			snprintf(buffer, sizeof(buffer), "%.3f,%.3f,%.3f", color.x, color.y, color.z);
			value = buffer;
		}
		break;
	}
	default: {
		ImGui::SetNextItemWidth(value.size() * 10.f);
		ImGui::InputText("##value", &value);
		break;
	}
	}

	ImGui::PopStyleVar();
	ImGui::PopItemWidth();
}

// 列表控件绘制
void SectionNode::DrawListInput(std::string & listValue, const ListType & listType) {
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
			DrawValueWidget(elements[i], elemType);

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
