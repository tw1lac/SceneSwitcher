#include "../headers/switcher-data-structs.hpp"
struct ScreenRegionSwitch
{
	OBSWeakSource scene;
	OBSWeakSource transition;
	int minX, minY, maxX, maxY;
	string regionStr;

	inline ScreenRegionSwitch(OBSWeakSource scene_, OBSWeakSource transition_, int minX_, int minY_,
		int maxX_, int maxY_, string regionStr_)
		: scene(scene_)
		, transition(transition_)
		, minX(minX_)
		, minY(minY_)
		, maxX(maxX_)
		, maxY(maxY_)
		, regionStr(regionStr_)
	{
	}
};