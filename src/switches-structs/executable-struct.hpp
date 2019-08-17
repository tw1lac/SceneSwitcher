#include "../headers/switcher-data-structs.hpp"
struct ExecutableSceneSwitch
{
	OBSWeakSource mScene;
	OBSWeakSource mTransition;
	QString mExe;
	bool mInFocus;

	inline ExecutableSceneSwitch(
		OBSWeakSource scene, OBSWeakSource transition, const QString& exe, bool inFocus)
		: mScene(scene)
		, mTransition(transition)
		, mExe(exe)
		, mInFocus(inFocus)
	{
	}
};