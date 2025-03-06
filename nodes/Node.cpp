#include "Node.h"
#include "Link.h"
#include "MainWindow.h"
#include "TypeLoader.h"
#include "Utils.h"

std::vector<std::unique_ptr<Node>> Node::Array;

Node::Node(const char* name, int id) :
	Owner(MainWindow::Instance), Name(name),
	ID(id ? id : MainWindow::Instance->GetNextId()) {
}

Node* Node::Get(ed::NodeId id) {
	for (const auto& node : Node::Array)
		if (node->ID == id)
			return node.get();

	return nullptr;
}

std::vector<Node*> Node::GetSelectedNodes() {
	std::vector<Node*> ret;
	std::vector<ed::NodeId> selectedNodes;
	selectedNodes.resize(ed::GetSelectedObjectCount(), ed::NodeId(0));
	ed::GetSelectedNodes(selectedNodes.data(), (signed)selectedNodes.size());

	for (auto& id : selectedNodes)
		if (auto node = Node::Get(id))
			ret.push_back(node);

	return ret;
}

void Node::Menu() {
	ImGui::Text("ID: %p", ID.AsPointer());
	ImGui::Text("Type: %s", GetNodeType());
	ImGui::Separator();

	// 类型选择下拉框
	ImGui::Separator();
	if (ImGui::BeginCombo("##NodeType", TypeName.c_str())) {
		auto& ts = TypeSystem::Get();

		// 显示所有已注册类型
		for (auto& [name, _] : ts.Sections)
			if (ImGui::Selectable(name.c_str()))
				TypeName = name;
		for (auto& [name, _] : ts.NumberLimits)
			if (ImGui::Selectable(name.c_str()))
				TypeName = name;
		for (auto& [name, _] : ts.StringLimits)
			if (ImGui::Selectable(name.c_str()))
				TypeName = name;
		for (auto& [name, _] : ts.Lists)
			if (ImGui::Selectable(name.c_str()))
				TypeName = name;
		ImGui::EndCombo();
	}

	if (ImGui::MenuItem(IsComment ? "Uncomment" : "Set Comment"))
		IsComment = !IsComment;
	if (ImGui::MenuItem(IsFolded ? "Unfold" : "Fold"))
		IsFolded = !IsFolded;
}

void Node::Tooltip() {
	ImGui::BeginTooltip();

	// 类型名称
	ImGui::Text("Type:%s", TypeName.c_str());

	// 类型详细信息
	auto typeInfo = TypeSystem::Get().GetTypeInfo(TypeName);
	switch (typeInfo.Category) {
	case TypeCategory::NumberLimit:
		ImGui::Separator();
		ImGui::Text("Value Range:");
		ImGui::BulletText("%d to %d",
			std::get<NumberLimit>(typeInfo.Data).Min,
			std::get<NumberLimit>(typeInfo.Data).Max);
		break;

	case TypeCategory::List:
		ImGui::Separator();
		ImGui::Text("List Properties:");
		ImGui::BulletText("Element: %s", std::get<ListType>(typeInfo.Data).ElementType.c_str());
		ImGui::BulletText("Length: %d-%d",
			std::get<ListType>(typeInfo.Data).MinLength,
			std::get<ListType>(typeInfo.Data).MaxLength);
		break;

	case TypeCategory::StringLimit:
		if (!std::get<StringLimit>(typeInfo.Data).ValidValues.empty()) {
			ImGui::Separator();
			ImGui::Text("Valid Options:");
			for (auto& val : std::get<StringLimit>(typeInfo.Data).ValidValues) {
				ImGui::BulletText("%s", val.c_str());
			}
		}
		break;
	}

	ImGui::EndTooltip();
}

void Node::HoverMenu(bool isHovered) {
	// 显示类型提示
	static float hoverTime = 0.0f;
	if (isHovered) {
		hoverTime += ImGui::GetIO().DeltaTime;
		if (hoverTime > 0.5f) {
			ImGui::BeginTooltip();
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Type: %s",
				this->TypeName.c_str());

			// 添加详细类型信息
			auto typeInfo = TypeSystem::Get().GetTypeInfo(this->TypeName);
			switch (typeInfo.Category) {
			case TypeCategory::NumberLimit:
				ImGui::Text("Range: %d - %d",
					std::get<NumberLimit>(typeInfo.Data).Min,
					std::get<NumberLimit>(typeInfo.Data).Max);
				break;
			case TypeCategory::List:
				ImGui::Text("List of: %s (%d-%d)",
					std::get<ListType>(typeInfo.Data).ElementType.c_str(),
					std::get<ListType>(typeInfo.Data).MinLength,
					std::get<ListType>(typeInfo.Data).MaxLength);
				break;
			case TypeCategory::StringLimit:
				if (!std::get<StringLimit>(typeInfo.Data).ValidValues.empty()) {
					ImGui::Text("Options: %s",
						Utils::JoinStrings(std::get<StringLimit>(typeInfo.Data).ValidValues, ", ").c_str());
				}
				break;
			}

			ImGui::EndTooltip();
		}
	}
	else {
		hoverTime = 0.0f;
	}
}

void Node::SetName(const std::string& str) {
	Name = str;
}

Pin* Node::GetFirstCompatiblePin(Pin* pin) {
	return nullptr;
}

KeyValue* Node::ConvertToKeyValue(Pin* pin) {
	return nullptr;
}

ImVec2 Node::GetNodeSize() const {
	return ed::GetNodeSize(ID);
}

ImVec2 Node::GetPosition() const {
	return ed::GetNodePosition(ID);
}

void Node::SetPosition(ImVec2 pos) const {
	return ed::SetNodePosition(ID, pos);
}

int Node::GetConnectedLinkCount() {
	int count = 0;
	for (auto& link : Link::Array) {
		if (auto pin = Pin::Get(link->StartPinID)) {
			if (pin->Node == this) ++count;
		}
		if (auto pin = Pin::Get(link->EndPinID)) {
			if (pin->Node == this) ++count;
		}
	}
	return count;
}

void Node::SaveToJson(json& j) const {
	auto pos = GetPosition();
	// Inputs和Outputs暂时没存，因为目前没有实例
	j["ID"] = ID.Get();
	j["Section"] = Name;
	j["Position"] = { pos.x, pos.y };
	j["Color"] = { Color.Value.x,Color.Value.y,Color.Value.z };
	j["Type"] = static_cast<int>(GetNodeType());
	j["TypeName"] = TypeName;
	j["IsFolded"] = IsFolded;
	j["IsComment"] = IsComment;
}

void Node::LoadFromJson(const json& j) {
	ID = ed::NodeId(j["ID"]);
	SetName(j["Section"]);
	this->SetPosition({
		j["Position"][0].get<float>(),
		j["Position"][1].get<float>()
	});
	Color = {
		j["Color"][0].get<float>(),
		j["Color"][1].get<float>(),
		j["Color"][2].get<float>(),
		1.0f
	};
	//Type = static_cast<NodeType>(j["Type"]);
	TypeName = j["TypeName"];
	IsFolded = j["IsFolded"];
	IsComment = j["IsComment"];
}
