#include "../headers/switcher-data-structs.hpp"
struct RandomSwitch
{
	OBSWeakSource scene;
	OBSWeakSource transition;
	double delay;
	string randomSwitchStr;

	inline RandomSwitch(OBSWeakSource scene_, OBSWeakSource transition_
		, double delay_, string str)
		: scene(scene_)
		, transition(transition_)
		, delay(delay_)
		, randomSwitchStr(str)
	{
	}
};