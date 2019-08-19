#pragma once

#include <util/util.hpp>
#include <QTimer>
#include "headers/advanced-scene-switcher.hpp"

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

	const QRegularExpression scenesRegex(".*Scenes");
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

	const QRegularExpression transitionRegex(".*Transitions");
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
		for (auto& s : transitionBoxes) {
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

	const QRegularExpression windowsRegex(".*Windows");
	QList<QComboBox*> windowBoxes = ui->tabWidget->findChildren<QComboBox*>(windowsRegex);

	for (string& window : windows)
	{
		//ui->windowTitleWindows->addItem(window.c_str());
		//ui->ignoreWindowsWindows->addItem(window.c_str());
		//ui->pauseWindowsWindows->addItem(window.c_str());
		//ui->ignoreIdleWindowsWindows->addItem(window.c_str());

		for (auto& s : windowBoxes) {
			s->addItem(window.c_str());
		}
	}



	//adds current running processes to the processes-lists
	QStringList processes;
	GetProcessList(processes);

	const QRegularExpression executableRegex(".*Executables");
	QList<QComboBox*> executableBoxes = ui->tabWidget->findChildren<QComboBox*>(executableRegex);

	for (QString& process : processes)
	{
		//ui->executable->addItem(process);
		for (auto& s : executableBoxes) {
			s->addItem(process);
		}
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
		item->setData(Qt::UserRole, s.uiDisplayStr.c_str());
	}

	for (auto& s : switcher->pixelColorSwitches)
	{
		string sceneName = GetWeakSourceName(s.scene);
		string transitionName = GetWeakSourceName(s.transition);
		QString text = MakePixelSwitchName(
			sceneName.c_str(), transitionName.c_str(), s.pxX, s.pxY, s.colorsStr.c_str());

		QListWidgetItem* item = new QListWidgetItem(text, ui->pixelColorSwitchesList);
		item->setData(Qt::UserRole, s.uiDisplayStr.c_str());
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
		string sceneName1 = GetWeakSourceName(s.scene);
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
		string sceneName1 = GetWeakSourceName(s.scene);
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

	string priorityNames[] = PRIORITY_LIST_NAMES;
	for (int p : switcher->functionNamesByPriority)
	{
		string s = priorityNames[p];
		//string s = "";
		//switch (p) {
		//case READ_FILE_FUNC:
		//	s = "File Content";
		//	break;
		//case ROUND_TRIP_FUNC:
		//	s = "Scene Sequence";
		//	break;
		//case IDLE_FUNC:
		//	s = "Idle Detection";
		//	break;
		//case EXE_FUNC:
		//	s = "Executable";
		//	break;
		//case SCREEN_REGION_FUNC:
		//	s = "Screen Region";
		//	break;
		//case WINDOW_TITLE_FUNC:
		//	s = "Window Title";
		//	break;
		//case PIXEL_COLOR_FUNC:
		//	s = "Pixel Color";
		//}
		QString text(s.c_str());
		QListWidgetItem* item = new QListWidgetItem(text, ui->priorityList);
		item->setData(Qt::UserRole, text);
	}

}

