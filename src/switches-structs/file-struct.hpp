#include "../headers/switcher-data-structs.hpp"
struct FileSwitch
{
	OBSWeakSource scene;
	OBSWeakSource transition;
	string file;
	string text;
	bool useRegex = false;
	bool useTime = false;
	QDateTime lastMod;

	inline FileSwitch(
		OBSWeakSource scene_, OBSWeakSource transition_, const char* file_, const char* text_, bool useRegex_, bool useTime_)
		: scene(scene_)
		, transition(transition_)
		, file(file_)
		, text(text_)
		, useRegex(useRegex_)
		, useTime(useTime_)
		, lastMod()
	{
	}
};

struct FileIOData
{
	bool readEnabled = false;
	string readPath;
	bool writeEnabled = false;
	string writePath;
};