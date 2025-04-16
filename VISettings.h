#pragma once
#include "thirdparty/Settings.h"

class ViewSettings : public Settings {
public:
	DECLARE_CLASS_SETTINGS("View")
	DECLARE_SETTING(int, Load_CreateTagThreshold, 30);
};