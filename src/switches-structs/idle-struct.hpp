#include "../headers/switcher-data-structs.hpp"
struct IdleData
{
	bool idleEnable = false;
	int time = DEFAULT_IDLE_TIME;
	OBSWeakSource scene;
	OBSWeakSource transition;
	bool usePreviousScene;
	bool alreadySwitched = false;
};