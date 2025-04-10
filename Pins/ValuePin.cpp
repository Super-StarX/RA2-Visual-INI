#include "ValuePin.h"
#include "Utils.h"
#include "MainWindow.h"
#include <misc/cpp/imgui_stdlib.h>

std::string ValuePin::EditBuffer;
ListType ValuePin::EditType;
ValuePin* ValuePin::EditPin = nullptr;

ValuePin::ValuePin(::Node* node, std::string value, int id) :
	Pin(node, value.c_str(), PinKind::Output, id),
	Value(value) {
}

void ValuePin::SetValue(const std::string& str) { 
	Value = str; 
}

std::string ValuePin::GetValue() const {
	if (auto pNode = this->GetLinkedNode())
		return pNode->GetValue(GetLinkedPin());

	return Value;
}

float ValuePin::DrawValueWidget(std::string& value, const TypeInfo& type) {
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
			ImGui::BeginHorizontal("#combo_str");
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			auto w = Utils::SetNextInputWidth(value, 80.f);
			width = std::max(width, w);
			ImGui::InputText("##str", &value);
			if (ImGui::Button("...")) {
				MainWindow::Instance->m_ShowTypeEnumPopup = true;
				MainWindow::Instance->m_TypeEnumPopupType = type;
				MainWindow::Instance->m_TypeEnumPopupPin = this;
			}
			ImGui::PopStyleVar();
			ImGui::EndHorizontal();
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
		auto w = Utils::SetNextInputWidth(value, 100.f);
		width = std::max(width, w);
		if (ImGui::InputText("##value", &value))
			UpdateOutputLink(value);
		break;
	}
	}
	return width;
}

// 列表控件绘制
void ValuePin::DrawListInput(std::string& listValue, const ListType& listType) {
	std::vector<std::string> elements = Utils::SplitString(listValue, ',');
	bool changedDirectly = false;
	// 自动调整元素数量
	elements.resize(std::clamp(
		int(elements.size()),
		listType.MinLength,
		listType.MaxLength
	));

	// 特殊处理多选类型
	if (auto elemType = TypeSystem::Get().GetTypeInfo(listType.ElementType);
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
			TypeInfo elemType = TypeSystem::Get().GetTypeInfo(listType.ElementType);
			DrawValueWidget(elements[i], elemType);

			ImGui::PopID();
		}
	}
	else { // 长列表折叠显示
		ImGui::PushID(this->ID.AsPointer());
		ImGui::BeginHorizontal("##listedit");
		Utils::SetNextInputWidth(listValue, 100.f);
		if (ImGui::InputText("##list", &listValue))
			changedDirectly = true;
		
		if (ImGui::Button("Edit")) {
			// 打开列表编辑窗口
			EditPin = this;
			EditBuffer = listValue;
			EditType = listType;
			MainWindow::Instance->m_ShowListEditor = true;
		}
		ImGui::EndHorizontal();
		ImGui::PopID();
	}

	if (!changedDirectly)
		listValue = Utils::JoinStrings(elements, ",");
}

// 元素级编辑器
bool ValuePin::DrawElementEditor(std::string& value, const TypeInfo& type) {
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
