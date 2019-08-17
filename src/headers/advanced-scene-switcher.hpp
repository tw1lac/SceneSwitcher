#pragma once

#include <QDialog>
#include <memory>
#include <vector>
#include <string>
#include "../../forms/ui_advanced-scene-switcher.h"
#include "switcher-data-structs.hpp"

class QCloseEvent;


/*******************************************************************************
 * Advanced Scene Switcher window
 *******************************************************************************/
class SceneSwitcher : public QDialog {
	Q_OBJECT

public:
	std::unique_ptr<Ui_SceneSwitcher> ui;
	bool loading = true;

	SceneSwitcher(QWidget *parent);

	void closeEvent(QCloseEvent *event) override;

	void SetStarted();
	void SetStopped();

	int FindByData(const QString &window);
	int ScreenRegionFindByData(const QString& region);
	int PixelFindByData(const QString& pixel);
	int PauseScenesFindByData(const QString &scene);
	int PauseWindowsFindByData(const QString &window);
	int IgnoreWindowsFindByData(const QString &window);
	int SceneRoundTripFindByData(const QString &scene1);
	int SceneTransitionsFindByData(const QString &scene1, const QString &scene2);
	int DefaultTransitionsFindByData(const QString &scene);
	int executableFindByData(const QString &exe);
	int IgnoreIdleWindowsFindByData(const QString& window);
	int randomFindByData(const QString& scene);

	void UpdateNonMatchingScene(const QString &name);
	void UpdateAutoStopScene(const QString &name);
	void UpdateIdleDataTransition(const QString& name);
	void UpdateIdleDataScene(const QString& name);

public slots:
	void on_switches_currentRowChanged(int idx);
	void on_add_clicked();
	void on_remove_clicked();
	void on_noMatchDontSwitch_clicked();
	void on_noMatchSwitch_clicked();
	void on_noMatchRandomSwitch_clicked();
	void on_startAtLaunch_toggled(bool value);
	void on_noMatchSwitchScene_currentTextChanged(const QString &text);
	void on_checkInterval_valueChanged(int value);
	void on_toggleStartButton_clicked();

	void on_pixelSwitches_currentRowChanged(int idx);
	void on_pixelSwitchesAdd_clicked();
	void on_pixelSwitchesRemove_clicked();

	void on_screenRegions_currentRowChanged(int idx);
	void on_screenRegionAdd_clicked();
	void on_screenRegionRemove_clicked();

	void on_pauseScenes_currentRowChanged(int idx);
	void on_pauseScenesAdd_clicked();
	void on_pauseScenesRemove_clicked();

	void on_pauseWindows_currentRowChanged(int idx);
	void on_pauseWindowsAdd_clicked();
	void on_pauseWindowsRemove_clicked();

	void on_ignoreWindows_currentRowChanged(int idx);
	void on_ignoreWindowsAdd_clicked();
	void on_ignoreWindowsRemove_clicked();

	void on_sceneRoundTrips_currentRowChanged(int idx);
	void on_sceneRoundTripAdd_clicked();
	void on_sceneRoundTripRemove_clicked();
	void on_sceneRoundTripSave_clicked();
	void on_sceneRoundTripLoad_clicked();

	void on_autoStopSceneCheckBox_stateChanged(int state);
	void on_autoStopScenes_currentTextChanged(const QString &text);

	void on_sceneTransitions_currentRowChanged(int idx);
	void on_transitionsAdd_clicked();
	void on_transitionsRemove_clicked();
	void on_defaultTransitions_currentRowChanged(int idx);
	void on_defaultTransitionsAdd_clicked();
	void on_defaultTransitionsRemove_clicked();

	void on_browseButton_clicked();
	void on_readFileCheckBox_stateChanged(int state);
	void on_readPathLineEdit_textChanged(const QString & text);
	void on_writePathLineEdit_textChanged(const QString & text);
	void on_browseButton_2_clicked();

	void on_executableAdd_clicked();
	void on_executableRemove_clicked();
	void on_executables_currentRowChanged(int idx);

	void on_idleCheckBox_stateChanged(int state);
	void on_idleTransitions_currentTextChanged(const QString& text);
	void on_idleScenes_currentTextChanged(const QString& text);
	void on_idleSpinBox_valueChanged(int i);
	void on_ignoreIdleWindows_currentRowChanged(int idx);
	void on_ignoreIdleAdd_clicked();
	void on_ignoreIdleRemove_clicked();

	void on_randomAdd_clicked();
	void on_randomRemove_clicked();
	void on_randomScenesList_currentRowChanged(int idx);

	void on_fileAdd_clicked();
	void on_fileRemove_clicked();
	void on_fileScenesList_currentRowChanged(int idx);
	void on_browseButton_3_clicked();

	void on_priorityUp_clicked();
	void on_priorityDown_clicked();

	void updateScreenRegionCursorPos();

	void on_close_clicked();
};


/********************************************************************************
 * Window Title helper
 ********************************************************************************/
void GetWindowList(std::vector<std::string> &windows);
void GetCurrentWindowTitle(std::string &title);
bool isFullscreen();
bool existsInWindowList(const std::string& title);
void SaveWindowSwitcher(obs_data_array_t*& array);
void LoadWindowSwitcher(obs_data_array_t*& array);
void SaveIgnoreWindowSwitcher(obs_data_array_t*& array);
void LoadIgnoreWindowSwitcher(obs_data_array_t*& array);


/********************************************************************************
 * Pixel Color helper
 ********************************************************************************/
void getPixelColor(HWND window, int pxX, int pxY, string& currPixelColor);
void SavePixelSwitcher(obs_data_array_t*& array);
void LoadPixelSwitcher(obs_data_array_t*& array);


/********************************************************************************
 * Screen Region helper
 ********************************************************************************/
pair<int, int> getCursorPos();
void SaveScreenRegionSwitcher(obs_data_array_t*& array);
void LoadScreenRegionSwitcher(obs_data_array_t*& array);


/********************************************************************************
 * Idle Detection helper
 ********************************************************************************/
int secondsSinceLastInput();


/********************************************************************************
 * Executable helper
 ********************************************************************************/
void GetProcessList(QStringList &processes);
bool isInFocus(const QString &exeToCheck);
void SaveExecutableSwitcher(obs_data_array_t*& array);
void LoadExecutableSwitcher(obs_data_array_t*& array);

/********************************************************************************
 * Sceneswitch helper
 ********************************************************************************/
struct obs_weak_source;
typedef struct obs_weak_source obs_weak_source_t;

obs_weak_source_t* getNextTransition(obs_weak_source_t* scene1, obs_weak_source_t* scene2);
void switchScene(OBSWeakSource& scene, OBSWeakSource& transition);


/********************************************************************************
 * Hotkey helper
 ********************************************************************************/
void startStopHotkeyFunc(void* data, obs_hotkey_id id, obs_hotkey_t* hotkey, bool pressed);
void loadKeybinding(obs_hotkey_id hotkeyId);


/********************************************************************************
 * Main SwitcherData
 ********************************************************************************/
struct SwitcherData;
extern SwitcherData* switcher;

//OTHER
void SavePauseSwitcher(obs_data_array_t*& array);
void LoadPauseSwitcher(obs_data_array_t*& array);
void SavePauseWindowSwitcher(obs_data_array_t*& array);
void LoadPauseWindowSwitcher(obs_data_array_t*& array);

void SaveScreenRoundTripSwitcher(obs_data_array_t*& array);
void LoadScreenRoundTripSwitcher(obs_data_array_t*& array);

void SaveScreenTransitions(obs_data_array_t*& array);
void LoadScreenTransitions(obs_data_array_t*& array);
void SaveDefaultScreenTransitions(obs_data_array_t*& array);
void LoadDefaultScreenTransitions(obs_data_array_t*& array);

void SaveRandomSwitcher(obs_data_array_t*& array);
void LoadRandomSwitcher(obs_data_array_t*& array);

void SaveFileSwitcher(obs_data_array_t*& array);
void LoadFileSwitcher(obs_data_array_t*& array);

void SaveIgnoreIdleWindows(obs_data_array_t*& array);
void LoadIgnoreIdleWindows(obs_data_array_t*& array);

void SavePriorityOrder(obs_data_t*& obj);
void LoadPriorityOrder(obs_data_t*& obj);