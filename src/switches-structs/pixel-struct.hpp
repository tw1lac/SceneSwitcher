#include "../headers/switcher-data-structs.hpp"
struct PixelSwitch
{
	OBSWeakSource scene;
	OBSWeakSource transition;
	int pxX, pxY;
	string colorsStr;
	string pixelStr;

	inline PixelSwitch(OBSWeakSource scene_, OBSWeakSource transition_, int pxX_, int pxY_,
		string colorsStr_, string pixelStr_)
		: scene(scene_)
		, transition(transition_)
		, pxX(pxX_)
		, pxY(pxY_)
		, colorsStr(colorsStr_)
		, pixelStr(pixelStr_)
	{
	}
};