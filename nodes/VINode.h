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
	virtual void SaveToJson(json& j) const override;
	virtual void LoadFromJson(const json& j) override;
	virtual Pin* GetFirstCompatiblePin(Pin* pin) override;
	virtual void AddKeyValue() = 0;
	virtual void UnFoldedKeyValues(T& kv, bool isDisabled) = 0;
	virtual void FoldedKeyValues(size_t& i);

	vector KeyValues;
	std::unique_ptr<Pin> InputPin;
	std::unique_ptr<Pin> OutputPin;
	float maxSize{ 0 };
	float lastMaxSize{ 0 };
};

