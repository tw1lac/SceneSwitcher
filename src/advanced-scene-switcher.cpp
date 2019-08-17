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


SwitcherData* switcher = nullptr;


/********************************************************************************
 * Create the Advanced Scene Switcher settings window
 ********************************************************************************/
SceneSwitcher::SceneSwitcher(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui_SceneSwitcher)
{
	ui->setupUi(this);

	lock_guard<mutex> lock(switcher->m);

	switcher->Prune();

	const QRegularExpression scenesRegex(".*[Ss]cenes?\\d?");
	QList<QComboBox*> sceneBoxes = ui->tabWidget->findChildren<QComboBox*>(scenesRegex);

	//Adds all scenes to the scene-list
	BPtr<char*> scenes = obs_frontend_get_scene_names();
	char** temp = scenes;
	while (*temp)
	{
		const char* name = *temp;
		/*ui->scenes->addItem(name);
		ui->noMatchSwitchScene->addItem(name);
		ui->screenRegionScenes->addItem(name);
		ui->pixelScenes->addItem(name);
		ui->pauseScenesScenes->addItem(name);
		ui->sceneRoundTripScenes1->addItem(name);
		ui->sceneRoundTripScenes2->addItem(name);
		ui->autoStopScenes->addItem(name);
		ui->transitionsScene1->addItem(name);
		ui->transitionsScene2->addItem(name);
		ui->defaultTransitionsScene->addItem(name);
		ui->executableScenes->addItem(name);
		ui->idleScenes->addItem(name);
		ui->randomScenes->addItem(name);
		ui->fileScenes->addItem(name);*/
		for (auto& s : sceneBoxes) {
			s->addItem(name);
		}
		temp++;
	}

	ui->sceneRoundTripToScenes->addItem(PREVIOUS_SCENE_NAME);
	ui->idleScenes->addItem(PREVIOUS_SCENE_NAME);


	//adds all transitions to the transition-lists
	obs_frontend_source_list* transitions = new obs_frontend_source_list();
	obs_frontend_get_transitions(transitions);

	const QRegularExpression transitionRegex(".*ransitions");
	QList<QComboBox*> transitionBoxes = ui->tabWidget->findChildren<QComboBox*>(transitionRegex);
	//ui->debugTextTwilac->setText(to_string(transitionBoxes.length()).c_str());

	for (size_t i = 0; i < transitions->sources.num; i++)
	{
		const char* name = obs_source_get_name(transitions->sources.array[i]);
		/*ui->transitions->addItem(name);
		ui->screenRegionsTransitions->addItem(name);
		ui->pixelTransitions->addItem(name);
		ui->sceneRoundTripTransitions->addItem(name);
		ui->transitionsTransitions->addItem(name);
		ui->defaultTransitionsTransitions->addItem(name);
		ui->executableTransitions->addItem(name);
		ui->idleTransitions->addItem(name);
		ui->randomTransitions->addItem(name);
		ui->fileTransitions->addItem(name);*/

		
		//for (int i = 0; i <transitionBoxes.length(); i++) {
		for (auto& s : transitionBoxes){
			s->addItem(name);
		}
	}

	obs_frontend_source_list_free(transitions);

	if (switcher->switchIfNotMatching == SWITCH)
	{
		ui->noMatchSwitch->setChecked(true);
		ui->noMatchSwitchScenes->setEnabled(true);
	}
	else if (switcher->switchIfNotMatching == NO_SWITCH)
	{
		ui->noMatchDontSwitch->setChecked(true);
		ui->noMatchSwitchScenes->setEnabled(false);
	}
	else
	{
		ui->noMatchRandomSwitch->setChecked(true);
		ui->noMatchSwitchScenes->setEnabled(false);
	}
	ui->noMatchSwitchScenes->setCurrentText(GetWeakSourceName(switcher->nonMatchingScene).c_str());
	ui->checkInterval->setValue(switcher->interval);


	//adds current opened windows to the windows-lists
	vector<string> windows;
	GetWindowList(windows);

	for (string& window : windows)
	{
		ui->windowTitleWindows->addItem(window.c_str());
		ui->ignoreWindowsWindows->addItem(window.c_str());
		ui->pauseWindowsWindows->addItem(window.c_str());
		ui->ignoreIdleWindowsWindows->addItem(window.c_str());
	}


	//adds current running processes to the processes-lists
	QStringList processes;
	GetProcessList(processes);
	for (QString& process : processes)
	{
		ui->executable->addItem(process);
	}
		

	for (auto& s : switcher->executableSwitches)
	{
		string sceneName = GetWeakSourceName(s.mScene);
		string transitionName = GetWeakSourceName(s.mTransition);
		QString text = MakeSwitchNameExecutable(
			sceneName.c_str(), s.mExe, transitionName.c_str(), s.mInFocus);

		QListWidgetItem* item = new QListWidgetItem(text, ui->executableSwitchesList);
		item->setData(Qt::UserRole, s.mExe);
	}

	for (auto& s : switcher->windowSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeSwitchName(
			sceneName.c_str(), s.window.c_str(), transitionName.c_str(), s.fullscreen, s.checkBackground);

		QListWidgetItem* item = new QListWidgetItem(text, ui->windowTitleSwitchesList);
		item->setData(Qt::UserRole, s.window.c_str());
	}

	for (auto& s : switcher->screenRegionSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeScreenRegionSwitchName(
			sceneName.c_str(), transitionName.c_str(), s.minX, s.minY, s.maxX, s.maxY);

		QListWidgetItem* item = new QListWidgetItem(text, ui->screenRegionSwitchesList);
		item->setData(Qt::UserRole, s.regionStr.c_str());
	}

	for (auto& s : switcher->pixelColorSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakePixelSwitchName(
			sceneName.c_str(), transitionName.c_str(), s.pxX, s.pxY, s.colorsStr.c_str());

		QListWidgetItem* item = new QListWidgetItem(text, ui->pixelColorSwitchesList);
		item->setData(Qt::UserRole, s.pixelStr.c_str());
	}

	ui->autoStopSceneCheckBox->setChecked(switcher->autoStopEnable);
	ui->autoStopScenes->setCurrentText(GetWeakSourceName(switcher->autoStopScene).c_str());

	if (ui->autoStopSceneCheckBox->checkState())
	{
		ui->autoStopScenes->setDisabled(false);
	}
	else
	{
		ui->autoStopScenes->setDisabled(true);
	}

	for (auto& scene : switcher->pauseScenesSwitches)
	{
		string sceneName = GetWeakSourceName(scene);
		QString text = QString::fromStdString(sceneName);

		QListWidgetItem* item = new QListWidgetItem(text, ui->pauseScenesList);
		item->setData(Qt::UserRole, text);
	}

	for (auto& window : switcher->pauseWindowsSwitches)
	{
		QString text = QString::fromStdString(window);

		QListWidgetItem* item = new QListWidgetItem(text, ui->pauseWindowsList);
		item->setData(Qt::UserRole, text);
	}

	for (auto& window : switcher->ignoreWindowsSwitches)
	{
		QString text = QString::fromStdString(window);

		QListWidgetItem* item = new QListWidgetItem(text, ui->ignoreWindowsList);
		item->setData(Qt::UserRole, text);
	}

	int smallestDelay = switcher->interval;
	for (auto& s : switcher->sceneRoundTripSwitches)
	{
		string sceneName1 = GetWeakSourceName(s.scene1); 
		string sceneName2 = (s.usePreviousScene) ? PREVIOUS_SCENE_NAME : GetWeakSourceName(s.scene2);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeSceneRoundTripSwitchName(
			sceneName1.c_str(), sceneName2.c_str(), transitionName.c_str(), (double)s.delay / 1000);

		QListWidgetItem* item = new QListWidgetItem(text, ui->sceneRoundTripSwitchesList);
		item->setData(Qt::UserRole, text);

		if (s.delay < smallestDelay)
			smallestDelay = s.delay;
	}
	(smallestDelay < switcher->interval) ? ui->intervalWarning->setVisible(true) : ui->intervalWarning->setVisible(false);

	for (auto& s : switcher->sceneTransitions)
	{
		string sceneName1 = GetWeakSourceName(s.scene1);
		string sceneName2 = GetWeakSourceName(s.scene2);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeSceneTransitionName(
			sceneName1.c_str(), sceneName2.c_str(), transitionName.c_str());

		QListWidgetItem* item = new QListWidgetItem(text, ui->sceneTransitionsList);
		item->setData(Qt::UserRole, text);
	}
	//(transitionDurationLongerThanInterval(switcher->interval)) ? ui->transitionWarning->setVisible(true) : ui->transitionWarning->setVisible(false);

	for (auto& s : switcher->defaultSceneTransitions)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeDefaultSceneTransitionName(
			sceneName.c_str(), transitionName.c_str());

		QListWidgetItem* item = new QListWidgetItem(text, ui->defaultTransitionsList);
		item->setData(Qt::UserRole, text);
	}

	for (auto& window : switcher->ignoreIdleWindows)
	{
		QString text = QString::fromStdString(window);

		QListWidgetItem* item = new QListWidgetItem(text, ui->ignoreIdleWindowsList);
		item->setData(Qt::UserRole, text);
	}

	for (auto& s : switcher->randomSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakeRandomSwitchName(
			sceneName.c_str(), transitionName.c_str(), s.delay);

		QListWidgetItem* item = new QListWidgetItem(text, ui->randomScenesList);
		item->setData(Qt::UserRole, text);
	}

	for (auto& s : switcher->fileSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString listText = MakeFileSwitchName(
			sceneName.c_str(), transitionName.c_str(), s.file.c_str(), s.text.c_str(), s.useRegex, s.useTime);

		QListWidgetItem* item = new QListWidgetItem(listText, ui->fileSwitchesList);
		item->setData(Qt::UserRole, listText);
	}

	ui->idleCheckBox->setChecked(switcher->idleData.idleEnable);
	ui->idleScenes->setCurrentText(
		switcher->idleData.usePreviousScene ? PREVIOUS_SCENE_NAME : GetWeakSourceName(switcher->idleData.scene).c_str());
	ui->idleTransitions->setCurrentText(GetWeakSourceName(switcher->idleData.transition).c_str());
	ui->idleSpinBox->setValue(switcher->idleData.time);

	if (ui->idleCheckBox->checkState())
	{
		ui->idleScenes->setDisabled(false);
		ui->idleSpinBox->setDisabled(false);
		ui->idleTransitions->setDisabled(false);
	}
	else
	{
		ui->idleScenes->setDisabled(true);
		ui->idleSpinBox->setDisabled(true);
		ui->idleTransitions->setDisabled(true);
	}

	ui->readPathLineEdit->setText(QString::fromStdString(switcher->fileIO.readPath.c_str()));
	ui->readFileCheckBox->setChecked(switcher->fileIO.readEnabled);
	ui->writePathLineEdit->setText(QString::fromStdString(switcher->fileIO.writePath.c_str()));

	if (ui->readFileCheckBox->checkState())
	{
		ui->browseButton_2->setDisabled(false);
		ui->readPathLineEdit->setDisabled(false);
	}
	else
	{
		ui->browseButton_2->setDisabled(true);
		ui->readPathLineEdit->setDisabled(true);
	}

	if (switcher->th.joinable())
		SetStarted();
	else
		SetStopped();

	loading = false;

	// screen region cursor position
	QTimer* screenRegionTimer = new QTimer(this);
	connect(screenRegionTimer, SIGNAL(timeout()), this, SLOT(updateScreenRegionCursorPos()));
	screenRegionTimer->start(1000);

	for (int p : switcher->functionNamesByPriority)
	{
		string s = "";
		switch (p) {
		case READ_FILE_FUNC:
			s = "File Content";
			break;
		case ROUND_TRIP_FUNC:
			s = "Scene Sequence";
			break;
		case IDLE_FUNC:
			s = "Idle Detection";
			break;
		case EXE_FUNC:
			s = "Executable";
			break;
		case SCREEN_REGION_FUNC:
			s = "Screen Region";
			break;
		case WINDOW_TITLE_FUNC:
			s = "Window Title";
			break;
		case PIXEL_COLOR_FUNC:
			s = "Pixel Color";
		}
		QString text(s.c_str());
		QListWidgetItem* item = new QListWidgetItem(text, ui->priorityList);
		item->setData(Qt::UserRole, text);
	}

}


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


		
	/*	for (int i = 0; i < switcher->functionNamesByPriority.size(); i++) {
			obs_data_set_default_int(obj, "priority" + i, switcher->functionNamesByPriority[i]);
			switcher->functionNamesByPriority[i] = (obs_data_get_int(obj, "priority" +i));
		}*/

		/*if (!switcher->prioFuncsValid())
		{
			switcher->functionNamesByPriority = vector<int>{ DEFAULT_PRIORITY };
		}*/

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
