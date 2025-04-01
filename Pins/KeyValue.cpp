#include "KeyValue.h"
#include "TypeLoader.h"
#include "Utils.h"

KeyPin::KeyPin(KeyValue* value, const char* name, PinKind kind, int id) :
	Pin(value->Node, name, kind, id), Value(value) {}

std::string KeyPin::GetValue() const {
	return Value->GetValue();
}

KeyValue::KeyValue(::Node* node, const std::string& key, const std::string& value, const std::string& comment, int id) :
	ValuePin(node, value, id),
	Key(key), Comment(comment), InputPin(this, "", PinKind::Input, id == -1 ? -1 : 0) {}

void KeyValue::Tooltip() {
	ValuePin::Tooltip();
	auto type = TypeSystem::Get().GetKeyType(Node->TypeName, Key);
	ImGui::Text("Type: %s", type.TypeName.c_str());
	switch (type.Category) {
	case TypeCategory::NumberLimit:
		ImGui::Text("Range: [%d, %d]",
			std::get<NumberLimit>(type.Data).Min, std::get<NumberLimit>(type.Data).Max);
		break;
	case TypeCategory::StringLimit:
		if (!std::get<StringLimit>(type.Data).ValidValues.empty()) {
			ImGui::Text("Options: %s",
				Utils::JoinStrings(std::get<StringLimit>(type.Data).ValidValues, ", ").c_str());
		}
		break;
	case TypeCategory::List:
		ImGui::Text("Element: %s (%d-%d items)",
			std::get<ListType>(type.Data).ElementType.c_str(),
			std::get<ListType>(type.Data).MinLength,
			std::get<ListType>(type.Data).MaxLength);
		break;
	}
	if (!Comment.empty()) {
		ImGui::Separator();
		ImGui::TextUnformatted(Comment.c_str());
	}
}

void KeyValue::SaveToJson(json& j) const {
	Pin::SaveToJson(j);
	json inputJson;
	InputPin.SaveToJson(inputJson);
	j["InputPin"] = inputJson;
	j["Key"] = Key;
	j["value"] = Value;
	j["IsInherited"] = IsInherited;
	j["IsComment"] = IsComment;
	j["IsFolded"] = IsFolded;
}

void KeyValue::LoadFromJson(const json& j, bool newId) {
	Pin::LoadFromJson(j, newId);
	InputPin.LoadFromJson(j["InputPin"], newId);
	Key = j["Key"];
	Value = j["value"];
	IsInherited = j["IsInherited"];
	IsComment = j["IsComment"];
	IsFolded = j["IsFolded"];
}
