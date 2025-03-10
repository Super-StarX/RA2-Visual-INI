#include "KeyValue.h"
#include "TypeLoader.h"
#include "Utils.h"

KeyValue::KeyValue(::Node* node, const std::string& key, const std::string& value, const std::string& comment, int id) :
	ValuePin(node, value, id),
	Key(key), Comment(comment) {}

void KeyValue::Tooltip() {
	if (!Node) return;

	if (auto pKv = dynamic_cast<KeyValue*>(this)) {
		ImGui::BeginTooltip();

		auto type = TypeSystem::Get().GetKeyType(Node->TypeName, pKv->Key);
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
		ImGui::EndTooltip();
	}
}

void KeyValue::SaveToJson(json& j) const {
	Pin::SaveToJson(j);
	j["Key"] = Key;
	j["value"] = Value;
	j["IsInherited"] = IsInherited;
	j["IsComment"] = IsComment;
	j["IsFolded"] = IsFolded;
}

void KeyValue::LoadFromJson(const json& j) {
	Pin::LoadFromJson(j);
	Key = j["Key"];
	Value = j["value"];
	IsInherited = j["IsInherited"];
	IsComment = j["IsComment"];
	IsFolded = j["IsFolded"];
}
