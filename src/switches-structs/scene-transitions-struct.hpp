#include "../headers/switcher-data-structs.hpp"
struct SceneTransition
{
	OBSWeakSource scene1;
	OBSWeakSource scene2;
	OBSWeakSource transition;
	string sceneTransitionStr;

	inline SceneTransition(OBSWeakSource scene1_, OBSWeakSource scene2_, OBSWeakSource transition_,
		string sceneTransitionStr_)
		: scene1(scene1_)
		, scene2(scene2_)
		, transition(transition_)
		, sceneTransitionStr(sceneTransitionStr_)
	{
	}
};