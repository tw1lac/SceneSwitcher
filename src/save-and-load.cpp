
//#include "headers/switcher-data-structs.hpp"
//#include "headers/utility.hpp"
#include "headers/advanced-scene-switcher.hpp"

/********************************************************************************
 * Saving and loading
 ********************************************************************************/
static void SaveSceneSwitcher(obs_data_t* save_data, bool saving, void*)
{
	if (saving)
	{
		lock_guard<mutex> lock(switcher->m);
		obs_data_t* obj = obs_data_create();
		obs_data_array_t* windowTitleArray = obs_data_array_create();
		obs_data_array_t* screenRegionArray = obs_data_array_create();
		obs_data_array_t* pixelColorArray = obs_data_array_create();
		obs_data_array_t* pauseScenesArray = obs_data_array_create();
		obs_data_array_t* pauseWindowsArray = obs_data_array_create();
		obs_data_array_t* ignoreWindowsArray = obs_data_array_create();
		obs_data_array_t* sceneRoundTripArray = obs_data_array_create();
		obs_data_array_t* sceneTransitionsArray = obs_data_array_create();
		obs_data_array_t* defaultTransitionsArray = obs_data_array_create();
		obs_data_array_t* ignoreIdleWindowsArray = obs_data_array_create();
		obs_data_array_t* executableArray = obs_data_array_create();
		obs_data_array_t* randomArray = obs_data_array_create();
		obs_data_array_t* fileArray = obs_data_array_create();

		switcher->Prune();

		SaveWindowSwitcher(windowTitleArray);

		SaveScreenRegionSwitcher(screenRegionArray);

		SaveWindowSwitcher(pixelColorArray);

		SavePauseSwitcher(pauseScenesArray);

		SavePauseWindowSwitcher(pauseWindowsArray);

		SaveIgnoreWindowSwitcher(ignoreWindowsArray);

		SaveScreenRoundTripSwitcher(sceneRoundTripArray);

		SaveScreenTransitions(sceneTransitionsArray);

		SaveDefaultScreenTransitions(defaultTransitionsArray);

		SaveExecutableSwitcher(executableArray);

		SaveFileSwitcher(fileArray);

		SaveRandomSwitcher(randomArray);

		SaveIgnoreIdleWindows(ignoreIdleWindowsArray);


		string nonMatchingSceneName = GetWeakSourceName(switcher->nonMatchingScene);

		obs_data_set_int(obj, "interval", switcher->interval);
		obs_data_set_string(obj, "non_matching_scene", nonMatchingSceneName.c_str());
		obs_data_set_int(obj, "switch_if_not_matching", switcher->switchIfNotMatching);
		obs_data_set_bool(obj, "active", !switcher->stop);

		obs_data_set_array(obj, "switches", windowTitleArray);
		obs_data_set_array(obj, "screenRegion", screenRegionArray);
		obs_data_set_array(obj, "pixelColor", pixelColorArray);
		obs_data_set_array(obj, "pauseScenes", pauseScenesArray);
		obs_data_set_array(obj, "pauseWindows", pauseWindowsArray);
		obs_data_set_array(obj, "ignoreWindows", ignoreWindowsArray);
		obs_data_set_array(obj, "sceneRoundTrip", sceneRoundTripArray);
		obs_data_set_array(obj, "sceneTransitions", sceneTransitionsArray);
		obs_data_set_array(obj, "defaultTransitions", defaultTransitionsArray);
		obs_data_set_array(obj, "executableSwitches", executableArray);
		obs_data_set_array(obj, "ignoreIdleWindows", ignoreIdleWindowsArray);
		obs_data_set_array(obj, "randomSwitches", randomArray);
		obs_data_set_array(obj, "fileSwitches", fileArray);


		string autoStopSceneName = GetWeakSourceName(switcher->autoStopScene);
		obs_data_set_bool(obj, "autoStopEnable", switcher->autoStopEnable);
		obs_data_set_string(obj, "autoStopSceneName", autoStopSceneName.c_str());

		string idleSceneName = GetWeakSourceName(switcher->idleData.scene);
		string idleTransitionName = GetWeakSourceName(switcher->idleData.transition);
		obs_data_set_bool(obj, "idleEnable", switcher->idleData.idleEnable);
		obs_data_set_string(obj, "idleSceneName",
			switcher->idleData.usePreviousScene ? PREVIOUS_SCENE_NAME : idleSceneName.c_str());
		obs_data_set_string(obj, "idleTransitionName", idleTransitionName.c_str());
		obs_data_set_int(obj, "idleTime", switcher->idleData.time);

		obs_data_set_bool(obj, "readEnabled", switcher->fileIO.readEnabled);
		obs_data_set_string(obj, "readPath", switcher->fileIO.readPath.c_str());
		obs_data_set_bool(obj, "writeEnabled", switcher->fileIO.writeEnabled);
		obs_data_set_string(obj, "writePath", switcher->fileIO.writePath.c_str());

		SavePriorityOrder(obj);

		/*for (int i = 0; i < switcher->functionNamesByPriority.size(); i++) {
			obs_data_set_int(obj, "priority" + i, switcher->functionNamesByPriority[i]);
		}*/

		obs_data_set_obj(save_data, "advanced-scene-switcher", obj);

		obs_data_array_release(windowTitleArray);
		obs_data_array_release(screenRegionArray);
		obs_data_array_release(pixelColorArray);
		obs_data_array_release(pauseScenesArray);
		obs_data_array_release(pauseWindowsArray);
		obs_data_array_release(ignoreWindowsArray);
		obs_data_array_release(sceneRoundTripArray);
		obs_data_array_release(sceneTransitionsArray);
		obs_data_array_release(defaultTransitionsArray);
		obs_data_array_release(executableArray);
		obs_data_array_release(ignoreIdleWindowsArray);
		obs_data_array_release(randomArray);
		obs_data_array_release(fileArray);

		obs_data_release(obj);
	}
	else
	{
		switcher->m.lock();

		obs_data_t* obj = obs_data_get_obj(save_data, "advanced-scene-switcher");
		obs_data_array_t* windowTitleArray = obs_data_get_array(obj, "switches");
		obs_data_array_t* screenRegionArray = obs_data_get_array(obj, "screenRegion");
		obs_data_array_t* pixelColorArray = obs_data_get_array(obj, "pixelColor");
		obs_data_array_t* pauseScenesArray = obs_data_get_array(obj, "pauseScenes");
		obs_data_array_t* pauseWindowsArray = obs_data_get_array(obj, "pauseWindows");
		obs_data_array_t* ignoreWindowsArray = obs_data_get_array(obj, "ignoreWindows");
		obs_data_array_t* sceneRoundTripArray = obs_data_get_array(obj, "sceneRoundTrip");
		obs_data_array_t* sceneTransitionsArray = obs_data_get_array(obj, "sceneTransitions");
		obs_data_array_t* defaultTransitionsArray = obs_data_get_array(obj, "defaultTransitions");
		obs_data_array_t* executableArray = obs_data_get_array(obj, "executableSwitches");
		obs_data_array_t* ignoreIdleWindowsArray = obs_data_get_array(obj, "ignoreIdleWindows");
		obs_data_array_t* randomArray = obs_data_get_array(obj, "randomSwitches");
		obs_data_array_t* fileArray = obs_data_get_array(obj, "fileSwitches");

		if (!obj)
			obj = obs_data_create();

		obs_data_set_default_int(obj, "interval", DEFAULT_INTERVAL);

		switcher->interval = obs_data_get_int(obj, "interval");
		obs_data_set_default_int(obj, "switch_if_not_matching", NO_SWITCH);
		switcher->switchIfNotMatching = (NoMatch)obs_data_get_int(obj, "switch_if_not_matching");
		string nonMatchingScene = obs_data_get_string(obj, "non_matching_scene");
		bool active = obs_data_get_bool(obj, "active");

		switcher->nonMatchingScene = GetWeakSourceByName(nonMatchingScene.c_str());

		//size_t count; //ta bort senare och lägg i loadfunktionerna

		LoadWindowSwitcher(windowTitleArray);

		LoadScreenRegionSwitcher(screenRegionArray);

		LoadWindowSwitcher(pixelColorArray);

		LoadPauseSwitcher(pauseScenesArray);

		LoadPauseWindowSwitcher(pauseWindowsArray);

		LoadIgnoreWindowSwitcher(ignoreWindowsArray);

		LoadScreenRoundTripSwitcher(sceneRoundTripArray);

		LoadScreenTransitions(sceneTransitionsArray);

		LoadDefaultScreenTransitions(defaultTransitionsArray);

		LoadExecutableSwitcher(executableArray);

		LoadIgnoreIdleWindows(ignoreIdleWindowsArray);

		LoadRandomSwitcher(randomArray);

		LoadFileSwitcher(fileArray);


		string autoStopScene = obs_data_get_string(obj, "autoStopSceneName");
		switcher->autoStopEnable = obs_data_get_bool(obj, "autoStopEnable");
		switcher->autoStopScene = GetWeakSourceByName(autoStopScene.c_str());

		string idleSceneName = obs_data_get_string(obj, "idleSceneName");
		string idleTransitionName = obs_data_get_string(obj, "idleTransitionName");
		switcher->idleData.scene = GetWeakSourceByName(idleSceneName.c_str());
		switcher->idleData.transition = GetWeakTransitionByName(idleTransitionName.c_str());
		obs_data_set_default_bool(obj, "idleEnable", false);
		switcher->idleData.idleEnable = obs_data_get_bool(obj, "idleEnable");
		obs_data_set_default_int(obj, "idleTime", DEFAULT_IDLE_TIME);
		switcher->idleData.time = obs_data_get_int(obj, "idleTime");
		switcher->idleData.usePreviousScene = (idleSceneName == PREVIOUS_SCENE_NAME);

		obs_data_set_default_bool(obj, "readEnabled", false);
		switcher->fileIO.readEnabled = obs_data_get_bool(obj, "readEnabled");
		switcher->fileIO.readPath = obs_data_get_string(obj, "readPath");
		obs_data_set_default_bool(obj, "writeEnabled", false);
		switcher->fileIO.writeEnabled = obs_data_get_bool(obj, "writeEnabled");
		switcher->fileIO.writePath = obs_data_get_string(obj, "writePath");

		LoadPriorityOrder(obj);



			for (int i = 0; i < switcher->functionNamesByPriority.size(); i++) {
				obs_data_set_default_int(obj, "priority" + i, switcher->functionNamesByPriority[i]);
				switcher->functionNamesByPriority[i] = (obs_data_get_int(obj, "priority" +i));
			}

			if (!switcher->prioFuncsValid())
			{
				switcher->functionNamesByPriority = vector<int>{ DEFAULT_PRIORITY };
			}

		obs_data_array_release(windowTitleArray);
		obs_data_array_release(screenRegionArray);
		obs_data_array_release(pixelColorArray);
		obs_data_array_release(pauseScenesArray);
		obs_data_array_release(ignoreWindowsArray);
		obs_data_array_release(pauseWindowsArray);
		obs_data_array_release(sceneRoundTripArray);
		obs_data_array_release(sceneTransitionsArray);
		obs_data_array_release(defaultTransitionsArray);
		obs_data_array_release(executableArray);
		obs_data_array_release(ignoreIdleWindowsArray);
		obs_data_array_release(randomArray);
		obs_data_array_release(fileArray);

		obs_data_release(obj);

		switcher->m.unlock();

		if (active)
			switcher->Start();
		else
			switcher->Stop();
	}
}