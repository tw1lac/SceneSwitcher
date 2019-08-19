#include <obs-module.h>
#include "headers/advanced-scene-switcher.hpp"


void SceneSwitcher::addStuff(string switchName)
{
//	QString sceneName = ui->windowTitleScenes->currentText();
//	QString windowName = ui->windowTitleWindows->currentText();
//	QString transitionName = ui->windowTitleTransitions->currentText();
//	bool fullscreen = ui->fullscreenCheckBox->isChecked();
//	bool checkBackground = ui->checkBackgroundCheckBox->isChecked();
//
//	if (windowName.isEmpty() || sceneName.isEmpty())
//		return;
//
//	OBSWeakSource source = GetWeakSourceByQString(sceneName);
//	OBSWeakSource transition = GetWeakTransitionByQString(transitionName);
//	QVariant v = QVariant::fromValue(windowName);
//
//	QString text = MakeSwitchName(sceneName, windowName, transitionName, fullscreen, checkBackground);
//
//	int idx = FindByData(windowName);
//
//	if (idx == -1)
//	{
//		lock_guard<mutex> lock(switcher->m);
//		switcher->windowSwitches.emplace_back(
//			source, NULL, windowName.toUtf8().constData(), transition, fullscreen, checkBackground, NULL, NULL, NULL, NULL);
//
//		QListWidgetItem* item = new QListWidgetItem(text, ui->windowTitleSwitchesList);
//		item->setData(Qt::UserRole, v);
//	}
//	else
//	{
//		QListWidgetItem* item = ui->windowTitleSwitchesList->item(idx);
//		item->setText(text);
//
//		string window = windowName.toUtf8().constData();
//
//		{
//			lock_guard<mutex> lock(switcher->m);
//			for (auto& s : switcher->windowSwitches)
//			{
//				if (s.window == window)
//				{
//					s.scene = source;
//					s.transition = transition;
//					s.fullscreen = fullscreen;
//					break;
//				}
//			}
//		}
//
//		ui->windowTitleSwitchesList->sortItems();
//	}
}

//// remove window title switch rule
//void SceneSwitcher::on_remove_clicked()
//{
//	QListWidgetItem* item = ui->windowTitleSwitchesList->currentItem();
//	if (!item)
//		return;
//
//	string window = item->data(Qt::UserRole).toString().toUtf8().constData();
//
//	{
//		lock_guard<mutex> lock(switcher->m);
//		auto& switches = switcher->windowSwitches;
//
//		for (auto it = switches.begin(); it != switches.end(); ++it)
//		{
//			auto& s = *it;
//
//			if (s.window == window)
//			{
//				switches.erase(it);
//				break;
//			}
//		}
//	}
//
//	delete item;
//}

// remove switch rule
void SceneSwitcher::removeSwitch(string switchName, vector<StructSwitch>& switchVecktor)
{
	//QListWidgetItem* item = ui->windowTitleSwitchesList->currentItem();
	string listWidgetName = switchName + "SwitchesList";
	const QRegularExpression listRegex(switchName.c_str());
	QListWidget* switchList = ui->tabWidget->findChildren<QListWidget*>(listRegex)[0];
	QListWidgetItem* item = switchList->currentItem();

	if (!item)
		return;

	string itemSwitch = item->data(Qt::UserRole).toString().toUtf8().constData();

	{
		lock_guard<mutex> lock(switcher->m);
		auto& switches = switchVecktor;

		for (auto it = switches.begin(); it != switches.end(); ++it)
		{
			auto& s = *it;

			if (s.window == itemSwitch)
			{
				switches.erase(it);
				break;
			}
		}
	}

	delete item;
}

// add window title stay rule
//void SceneSwitcher::on_ignoreWindowsAdd_clicked()
//{
//	QString windowName = ui->ignoreWindowsWindows->currentText();
//
//	if (windowName.isEmpty())
//		return;
//
//	QVariant v = QVariant::fromValue(windowName);
//
//	QList<QListWidgetItem*> items = ui->ignoreWindowsList->findItems(windowName, Qt::MatchExactly);
//
//	if (items.size() == 0)
//	{
//		QListWidgetItem* item = new QListWidgetItem(windowName, ui->ignoreWindowsList);
//		item->setData(Qt::UserRole, v);
//
//		lock_guard<mutex> lock(switcher->m);
//		switcher->ignoreWindowsSwitches.emplace_back(windowName.toUtf8().constData());
//		ui->ignoreWindowsList->sortItems();
//	}
//}

//// remove window title stay rule
//void SceneSwitcher::on_ignoreWindowsRemove_clicked()
//{
//	QListWidgetItem* item = ui->ignoreWindowsList->currentItem();
//	if (!item)
//		return;
//
//	QString windowName = item->data(Qt::UserRole).toString();
//
//	{
//		lock_guard<mutex> lock(switcher->m);
//		auto& switches = switcher->ignoreWindowsSwitches;
//
//		for (auto it = switches.begin(); it != switches.end(); ++it)
//		{
//			auto& s = *it;
//
//			if (s == windowName.toUtf8().constData())
//			{
//				switches.erase(it);
//				break;
//			}
//		}
//	}
//
//	delete item;
//}

int SceneSwitcher::FindByDataUgg(string switchName, const QString& dataString)
{
	//int count = ui->windowTitleSwitchesList->count();
	string listWidgetName = switchName + "SwitchesList";
	const QRegularExpression listRegex(switchName.c_str());
	//QListWidget* switchList = ui->tabWidget->findChild<QListWidget*>(listRegex)->count();
	QListWidget* switchList = ui->tabWidget->findChildren<QListWidget*>(listRegex)[0];
	int count = ui->tabWidget->findChildren<QListWidget*>(listRegex)[0]->count();
	int idx = -1;

	for (int i = 0; i < count; i++)
	{
		//QListWidgetItem* item = ui->windowTitleSwitchesList->item(i);
		QListWidgetItem* item = switchList->item(i);
		QString itemSwitch = item->data(Qt::UserRole).toString();

		if (itemSwitch == dataString)
		{
			idx = i;
			break;
		}
	}

	return idx;
}

//int SceneSwitcher::IgnoreWindowsFindByData(const QString& window)
//{
//	int count = ui->ignoreWindowsList->count();
//	int idx = -1;
//
//	for (int i = 0; i < count; i++)
//	{
//		QListWidgetItem* item = ui->ignoreWindowsList->item(i);
//		QString itemRegion = item->data(Qt::UserRole).toString();
//
//		if (itemRegion == window)
//		{
//			idx = i;
//			break;
//		}
//	}
//
//	return idx;
//}

//void SceneSwitcher::on_switches_currentRowChanged(int idx)
//{
//	if (loading)
//		return;
//	if (idx == -1)
//		return;
//
//	QListWidgetItem* item = ui->windowTitleSwitchesList->item(idx);
//
//	QString window = item->data(Qt::UserRole).toString();
//
//	lock_guard<mutex> lock(switcher->m);
//	for (auto& s : switcher->windowSwitches)
//	{
//		if (window.compare(s.window.c_str()) == 0)
//		{
//			string name = GetWeakSourceName(s.scene);
//			string transitionName = GetWeakSourceName(s.transition);
//			ui->windowTitleScenes->setCurrentText(name.c_str());
//			ui->windowTitleWindows->setCurrentText(window);
//			ui->windowTitleTransitions->setCurrentText(transitionName.c_str());
//			ui->fullscreenCheckBox->setChecked(s.fullscreen);
//			ui->checkBackgroundCheckBox->setChecked(s.checkBackground);
//
//			break;
//		}
//	}
//}

//void SceneSwitcher::on_ignoreWindows_currentRowChanged(int idx)
//{
//	if (loading)
//		return;
//	if (idx == -1)
//		return;
//
//	QListWidgetItem* item = ui->ignoreWindowsList->item(idx);
//
//	QString window = item->data(Qt::UserRole).toString();
//
//	lock_guard<mutex> lock(switcher->m);
//	for (auto& s : switcher->ignoreWindowsSwitches)
//	{
//		if (window.compare(s.c_str()) == 0)
//		{
//			ui->ignoreWindowsWindows->setCurrentText(s.c_str());
//			break;
//		}
//	}
//}

//void SwitcherData::checkWindowTitleSwitch(bool& match, OBSWeakSource& scene, OBSWeakSource& transition)
//{
//	//check if title should be ignored
//	string title;
//
//	GetCurrentWindowTitle(title);
//	for (auto& window : ignoreWindowsSwitches)
//	{
//		try
//		{
//			bool matches = regex_match(title, regex(window));
//			if (matches)
//			{
//				title = lastTitle;
//				break;
//			}
//		}
//		catch (const regex_error&)
//		{
//		}
//	}
//	lastTitle = title;
//
//	//direct match
//	//for (WindowSceneSwitch& s : windowSwitches)
//	for (StructSwitch& s : windowSwitches)
//	{
//		if (s.window == title)
//		{
//			match = !s.fullscreen || (s.fullscreen && isFullscreen());
//			scene = s.scene;
//			transition = s.transition;
//			return;
//		}
//		if (s.checkBackground && existsInWindowList(s.window)) {
//			match = !s.fullscreen || (s.fullscreen && isFullscreen());
//			scene = s.scene;
//			transition = s.transition;
//			return;
//		}
//	}
//	//regex match
//	//for (WindowSceneSwitch& s : windowSwitches)
//	for (StructSwitch& s : windowSwitches)
//	{
//		try
//		{
//			bool matches = regex_match(title, regex(s.window));
//			if (matches)
//			{
//				match = !s.fullscreen || (s.fullscreen && isFullscreen());
//				scene = s.scene;
//				transition = s.transition;
//			}
//		}
//		catch (const regex_error&)
//		{
//		}
//	}
//
//}

//void SaveWindowSwitcher(obs_data_array_t*& array) {
//	//for (WindowSceneSwitch& s : switcher->windowSwitches)
//	for (StructSwitch& s : switcher->windowSwitches)
//	{
//		obs_data_t* array_obj = obs_data_create();
//
//		obs_source_t* source = obs_weak_source_get_source(s.scene);
//		obs_source_t* transition = obs_weak_source_get_source(s.transition);
//		if (source && transition)
//		{
//			const char* sceneName = obs_source_get_name(source);
//			const char* transitionName = obs_source_get_name(transition);
//			obs_data_set_string(array_obj, "scene", sceneName);
//			obs_data_set_string(array_obj, "transition", transitionName);
//			obs_data_set_string(array_obj, "window_title", s.window.c_str());
//			obs_data_set_bool(array_obj, "fullscreen", s.fullscreen);
//			obs_data_set_bool(array_obj, "checkBackground", s.checkBackground);
//			obs_data_array_push_back(array, array_obj);
//			obs_source_release(source);
//			obs_source_release(transition);
//		}
//
//		obs_data_release(array_obj);
//	}
//}

//void LoadWindowSwitcher(obs_data_array_t*& array) {
//	switcher->windowSwitches.clear();
//	size_t count = obs_data_array_count(array);
//
//	for (size_t i = 0; i < count; i++)
//	{
//		obs_data_t* array_obj = obs_data_array_item(array, i);
//
//		const char* scene = obs_data_get_string(array_obj, "scene");
//		const char* transition = obs_data_get_string(array_obj, "transition");
//		const char* window = obs_data_get_string(array_obj, "window_title");
//		bool fullscreen = obs_data_get_bool(array_obj, "fullscreen");
//		bool checkBackground = obs_data_get_bool(array_obj, "checkBackground");
//
//		switcher->windowSwitches.emplace_back(GetWeakSourceByName(scene), window,
//			GetWeakTransitionByName(transition), fullscreen, checkBackground);
//
//		obs_data_release(array_obj);
//	}
//}

//void SaveIgnoreWindowSwitcher(obs_data_array_t*& array) {
//	for (string& window : switcher->ignoreWindowsSwitches)
//	{
//		obs_data_t* array_obj = obs_data_create();
//		obs_data_set_string(array_obj, "ignoreWindow", window.c_str());
//		obs_data_array_push_back(array, array_obj);
//		obs_data_release(array_obj);
//	}
//}

//void LoadIgnoreWindowSwitcher(obs_data_array_t*& array) {
//	switcher->ignoreWindowsSwitches.clear();
//	size_t count = obs_data_array_count(array);
//
//	for (size_t i = 0; i < count; i++)
//	{
//		obs_data_t* array_obj = obs_data_array_item(array, i);
//
//		const char* window = obs_data_get_string(array_obj, "ignoreWindow");
//
//		switcher->ignoreWindowsSwitches.emplace_back(window);
//
//		obs_data_release(array_obj);
//	}
//}