#include "../headers/switcher-data-structs.hpp"
struct SceneRoundTripSwitch
{
	OBSWeakSource scene1;
	OBSWeakSource scene2;
	OBSWeakSource transition;
	int delay;
	bool usePreviousScene;
	string sceneRoundTripStr;

	inline SceneRoundTripSwitch(OBSWeakSource scene1_, OBSWeakSource scene2_,
		OBSWeakSource transition_, int delay_, bool usePreviousScene_, string str)
		: scene1(scene1_)
		, scene2(scene2_)
		, transition(transition_)
		, delay(delay_)
		, usePreviousScene(usePreviousScene_)
		, sceneRoundTripStr(str)
	{
	}
};