#include "Node.h"
#include "Link.h"
#include "MainWindow.h"
#include "TypeLoader.h"
#include "Utils.h"

std::vector<std::unique_ptr<Node>> Node::Array;

Node* Node::FindNode(ed::NodeId id) {
	for (const auto& node : Node::Array)
		if (node->ID == id)
			return node.get();

	return nullptr;
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
		if (auto pin = Pin::FindPin(link->StartPinID)) {
			if (pin->Node == this) ++count;
		}
		if (auto pin = Pin::FindPin(link->EndPinID)) {
			if (pin->Node == this) ++count;
		}
	}
	return count;
}

void Node::Menu() {
	ImGui::Text("ID: %p", ID.AsPointer());
	ImGui::Text("Type: %s", Type == NodeType::Section ? "Section" : "Unexcepted");
	ImGui::Text("Inputs: %d", (int)Inputs.size());
	ImGui::Text("Outputs: %d", (int)Outputs.size());
	ImGui::Separator();

	// 类型选择下拉框
	ImGui::Separator();
	ImGui::Text("NodeType:", (int)Outputs.size());
	if (ImGui::BeginCombo("##NodeType", TypeName.c_str())) {
		auto& ts = TypeSystem::Get();

		// 显示所有已注册类型
		for (auto& [name, _] : ts.Sections) {
			if (ImGui::Selectable(name.c_str())) {
				TypeName = name;
			}
		}
		for (auto& [name, _] : ts.NumberLimits) {
			if (ImGui::Selectable(name.c_str())) {
				TypeName = name;
			}
		}
		for (auto& [name, _] : ts.Lists) {
			if (ImGui::Selectable(name.c_str())) {
				TypeName = name;
			}
		}
		ImGui::EndCombo();
	}

	if (Type == NodeType::Section) {
		if (ImGui::MenuItem(IsComment ? "Uncomment" : "Set Comment"))
			IsComment = !IsComment;
		if (ImGui::MenuItem(IsFolded ? "Unfold" : "Fold"))
			IsFolded = !IsFolded;
	}
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
