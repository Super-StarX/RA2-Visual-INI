#pragma once
#include <string>

class VIInputText: public std::string {
public:
	VIInputText(const std::string& name, const void* owner) : Owner(owner), std::string(name) {}
	
	void operator=(const std::string& name) { *this = VIInputText(name, Owner); };
	void BeginEditing();
	bool Render();

private:
	const void* Owner{ nullptr };
	bool Editing{ false };
	bool NeedFocus{ false };
	char Buffer[256] = ""; // 临时缓冲区
	std::string Temp;     // 保存编辑前的原始值
};