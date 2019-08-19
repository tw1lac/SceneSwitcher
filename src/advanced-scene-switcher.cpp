#pragma once

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>
#include <util/util.hpp>

#include <QMainWindow>
#include <QMessageBox>
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QTimer>
#include <QRegularExpression>

#include <condition_variable>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <regex>
#include <mutex>
#include <fstream>

#include "headers/switcher-data-structs.hpp"
#include "headers/utility.hpp"
#include "headers/advanced-scene-switcher.hpp"

#include "save-and-load.cpp"
//#include "ui-creator.cpp"


SwitcherData* switcher = nullptr;


/********************************************************************************
 * Main switcher thread
 ********************************************************************************/
void SwitcherData::Thread()
{

	//to avoid scene duplication when rapidly switching scene collection
	this_thread::sleep_for(chrono::seconds(2));

	int sleep = 0;

	while (true)
	{
	startLoop:
		unique_lock<mutex> lock(m);
		bool match = false;
		OBSWeakSource scene;
		OBSWeakSource transition;
		chrono::milliseconds duration;
		if (sleep > interval)
			duration = chrono::milliseconds(sleep);
		else
			duration = chrono::milliseconds(interval);
		sleep = 0;
		switcher->Prune();
		writeSceneInfoToFile();
		//sleep for a bit
		cv.wait_for(lock, duration);
		if (switcher->stop)
		{
			break;
		}
		setDefaultSceneTransitions(lock);
		if (autoStopEnable)
		{
			autoStopStreamAndRecording();
		}
		if (checkPause())
		{
			continue;
		}

		//UGGUGG
		for (int switchFuncName : functionNamesByPriority)
		{
			switch (switchFuncName) {
			case READ_FILE_FUNC:
				checkSwitchInfoFromFile(match, scene, transition);
				checkFileContent(match, scene, transition);
				break;
			case IDLE_FUNC:
				checkIdleSwitch(match, scene, transition);
				break;
			case EXE_FUNC:
				checkExeSwitch(match, scene, transition);
				break;
			case PIXEL_COLOR_FUNC:
				checkPixelSwitch(match, scene, transition);
				break;
			case SCREEN_REGION_FUNC:
				checkScreenRegionSwitch(match, scene, transition);
				break;
			case WINDOW_TITLE_FUNC:
				checkWindowTitleSwitch(match, scene, transition);
				break;
			case ROUND_TRIP_FUNC:
				checkSceneRoundTrip(match, scene, transition, lock);
				if (sceneChangedDuringWait()) //scene might have changed during the sleep
				{
					goto startLoop;
				}
			}
			if (switcher->stop)
			{
				goto endLoop;
			}
			if (match)
			{
				break;
			}
		}

		if (!match && switchIfNotMatching == SWITCH && nonMatchingScene)
		{
			match = true;
			scene = nonMatchingScene;
			transition = nullptr;
		}
		if (!match && switchIfNotMatching == RANDOM_SWITCH)
		{
			checkRandom(match, scene, transition, sleep);
		}
		if (match)
		{
			switchScene(scene, transition);
		}
	}
endLoop:
	;
}

void switchScene(OBSWeakSource& scene, OBSWeakSource& transition)
{
	obs_source_t* source = obs_weak_source_get_source(scene);
	obs_source_t* currentSource = obs_frontend_get_current_scene();

	if (source && source != currentSource)
	{
		obs_weak_source_t*  currentScene = obs_source_get_weak_source(currentSource);
		obs_weak_source_t*  nextTransitionWs = getNextTransition(currentScene, scene);
		obs_weak_source_release(currentScene);

		if (nextTransitionWs)
		{
			obs_source_t* nextTransition = obs_weak_source_get_source(nextTransitionWs);
			//lock.unlock();
			//transitionCv.wait(transitionLock, transitionActiveCheck);
			//lock.lock();
			obs_frontend_set_current_transition(nextTransition);
			obs_source_release(nextTransition);
		}
		else if (transition)
		{
			obs_source_t* nextTransition = obs_weak_source_get_source(transition);
			//lock.unlock();
			//transitionCv.wait(transitionLock, transitionActiveCheck);
			//lock.lock();
			obs_frontend_set_current_transition(nextTransition);
			obs_source_release(nextTransition);
		}
		obs_frontend_set_current_scene(source);
		obs_weak_source_release(nextTransitionWs);
	}
	obs_source_release(currentSource);
	obs_source_release(source);
}

bool SwitcherData::sceneChangedDuringWait() {
	bool r = false;
	obs_source_t* currentSource = obs_frontend_get_current_scene();
	if (!currentSource)
		return true;
	obs_source_release(currentSource);
	if (waitScene && currentSource != waitScene)
		r = true;
	waitScene = NULL;
	return r;
}

void SwitcherData::Start()
{
	if (!th.joinable())
	{
		stop = false;
		switcher->th = thread([]()
		{
			switcher->Thread();
		});
	}
}

void SwitcherData::Stop()
{
	if (th.joinable())
	{
		switcher->stop = true;
		transitionCv.notify_one();
		cv.notify_one();
		th.join();
	}
}


/********************************************************************************
 * OBS module setup
 ********************************************************************************/
extern "C" void FreeSceneSwitcher()
{
	delete switcher;
	switcher = nullptr;
}

static void OBSEvent(enum obs_frontend_event event, void* switcher)
{
	switch (event){
	case OBS_FRONTEND_EVENT_EXIT:
		FreeSceneSwitcher();
		break;

	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
	{
		SwitcherData* s = (SwitcherData*)switcher;
		lock_guard<mutex> lock(s->m);

		//stop waiting if scene was manually changed
		if (s->sceneChangedDuringWait())
			s->cv.notify_one();

		//set previous scene
		obs_source_t* source = obs_frontend_get_current_scene();
		obs_weak_source_t* ws = obs_source_get_weak_source(source);
		obs_source_release(source);
		obs_weak_source_release(ws);
		if (source && s->PreviousScene2 != ws)
		{
			s->previousScene = s->PreviousScene2;
			s->PreviousScene2 = ws;
		}


		break;
	}
	default:
		break;
	}
}

extern "C" void InitSceneSwitcher()
{
	QAction* action
		= (QAction*)obs_frontend_add_tools_menu_qaction("Advanced Scene Switcher");

	switcher = new SwitcherData;

	auto cb = []()
	{
		QMainWindow* window = (QMainWindow*)obs_frontend_get_main_window();

		SceneSwitcher ss(window);
		ss.exec();
	};

	obs_frontend_add_save_callback(SaveSceneSwitcher, nullptr);
	obs_frontend_add_event_callback(OBSEvent, switcher);

	action->connect(action, &QAction::triggered, cb);

	char* path = obs_module_config_path("");
	QDir dir(path);
	if (!dir.exists())
	{
		dir.mkpath(".");
	}
	bfree(path);

	obs_hotkey_id pauseHotkeyId = obs_hotkey_register_frontend("startStopSwitcherHotkey",
		"Toggle Start/Stop for the Advanced Scene Switcher", startStopHotkeyFunc, NULL);
	loadKeybinding(pauseHotkeyId);
}
