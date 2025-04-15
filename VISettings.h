#pragma once
#include "thirdparty/Settings.h"

class VISettings : public Settings {
public:
	DECLARE_SETTING(int, Load_CreateTagThreshold, 30);
};