#include "../switcher-data-structs.hpp"

struct WindowSceneSwitch
{
	OBSWeakSource scene;
	string window;
	OBSWeakSource transition;
	bool fullscreen;
	bool checkBackground;

	inline WindowSceneSwitch(
		OBSWeakSource scene_, const char* window_, OBSWeakSource transition_, bool fullscreen_, bool checkBackground_)
		: scene(scene_)
		, window(window_)
		, transition(transition_)
		, fullscreen(fullscreen_)
		, checkBackground(checkBackground_)
	{
	}
};