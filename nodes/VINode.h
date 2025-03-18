#pragma once
#include "Node.h"
#include <memory>
#include <string>

template <typename T>
class VINode : public Node {
public:
	using vector = std::vector<std::unique_ptr<T>>;
	VINode(const char* name = "", int id = 0);
	virtual void Update() override;
	void DrawHeader();
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	virtual void AddKeyValue() = 0;
	virtual void UnFoldedKeyValues(T& kv, bool isDisabled) = 0;
	virtual void FoldedKeyValues(size_t& i);

	vector KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;
	bool isEditing{ false };
	char inputBuffer[256] = ""; // 临时缓冲区
	std::string pendingName;     // 保存编辑前的原始值
	float maxSize{ 0 };
	float lastMaxSize{ 0 };
};

