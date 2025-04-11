#include "Node.h"
#include "Link.h"
#include "MainWindow.h"
#include "TypeLoader.h"
#include "NodeStyle.h"
#include "Utils.h"

std::vector<std::unique_ptr<Node>> Node::Array;

Node::Node(const char* name, int id) :
	Owner(MainWindow::Instance), Name(name, this) {

	if (!id)
		ID = MainWindow::GetNextId();
	else
		ID = MainWindow::GetIdOffset() + id;
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
	ImGui::Text("%s: %p", LOCALE["ID"], ID.AsPointer());
	ImGui::Text("%s: %d", LOCALE["Type"], GetNodeType());
	ImGui::Separator();

	auto& ts = TypeSystem::Get();

	// 类型选择为子菜单结构
	if (ImGui::BeginMenu(LOCALE["Change Type"])) {
		for (const auto& [name, _] : ts.Sections) {
			if (ImGui::MenuItem(name.c_str())) {
				TypeName = name;
			}
		}
		ImGui::EndMenu();
	}

	// 样式选择菜单
	if (ImGui::BeginMenu(LOCALE["Node Style"])) {
		auto& styleMgr = NodeStyleManager::Get();
		for (const auto& style : styleMgr.GetAllTypes()) {
			ImVec4 color = style.Color;
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			bool selected = (Style == style.Identifier);
			if (ImGui::MenuItem(style.DisplayName.c_str(), nullptr, selected)) {
				Style = style.Identifier;
			}
			ImGui::PopStyleColor();
		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	if (ImGui::MenuItem(IsComment ? LOCALE["Uncomment"] : LOCALE["Set Comment"]))
		IsComment = !IsComment;

	if (ImGui::MenuItem(IsFolded ? LOCALE["Unfold"] : LOCALE["Fold"]))
		IsFolded = !IsFolded;
}


void Node::Tooltip() {
	// 类型名称
	ImGui::Text("%s: %s", LOCALE["Type"], TypeName.c_str());

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
}

void Node::HoverMenu(bool isHovered) {
	// 显示类型提示
	static float hoverTime = 0.0f;
	if (isHovered) {
		hoverTime += ImGui::GetIO().DeltaTime;
		if (hoverTime > 0.5f) {
			ImGui::BeginTooltip();
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s: %s",
				LOCALE["Type"], this->TypeName.c_str());

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

bool Node::HasPin(ed::PinId pinId) {
	for (auto pin : GetAllPins()) {
		if (pin->ID == pinId)
			return true;
	}
	return false;
}

Pin* Node::GetPin(ed::PinId pinId) {
	for (auto pin : GetAllPins()) {
		if (pin->ID == pinId)
			return pin;
	}
	return nullptr;
}

std::string Node::GetValue(Pin* from) const {
	return Name;
}

void Node::SaveToJson(json& j) const {
	auto pos = GetPosition();
	// Inputs和Outputs暂时没存，因为目前没有实例
	j["ID"] = ID.Get();
	j["Name"] = Name;
	j["Position"] = { pos.x, pos.y };
	j["Style"] = Style;
	j["Type"] = static_cast<int>(GetNodeType());
	j["TypeName"] = TypeName;
	j["IsFolded"] = IsFolded;
	j["IsComment"] = IsComment;
}

void Node::LoadFromJson(const json& j, bool newId) {
	ID = newId ? MainWindow::GetNextId() : ed::NodeId(j["ID"] + MainWindow::GetIdOffset());
	SetName(j["Name"]);
	this->SetPosition({
		j["Position"][0].get<float>(),
		j["Position"][1].get<float>()
	});
	Style = j["Style"];
	TypeName = j["TypeName"];
	IsFolded = j["IsFolded"];
	IsComment = j["IsComment"];
}
