#include "headers/advanced-scene-switcher.hpp"

int SceneSwitcher::executableFindByData(const QString& exe)
{
	int count = ui->executableSwitchesList->count();
	
	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = ui->executableSwitchesList->item(i);
		QString itemExe = item->data(Qt::UserRole).toString();

		if (itemExe == exe)
			return i;
	}

	return -1;
}

void SceneSwitcher::on_executables_currentRowChanged(int idx)
{
	if (loading)
		return;
	if (idx == -1)
		return;

	QListWidgetItem* item = ui->executableSwitchesList->item(idx);

	QString exec = item->data(Qt::UserRole).toString();

	lock_guard<mutex> lock(switcher->m);
	for (auto& s : switcher->executableSwitches)
	{
		if (exec.compare(s.mExe) == 0)
		{
			QString sceneName = GetWeakSourceName(s.mScene).c_str();
			QString transitionName = GetWeakSourceName(s.mTransition).c_str();
			ui->executableScenes->setCurrentText(sceneName);
			ui->executableExecutables->setCurrentText(exec);
			ui->executableTransitions->setCurrentText(transitionName);
			ui->requiresFocusCheckBox->setChecked(s.mInFocus);
			break;
		}
	}
}

void SceneSwitcher::on_executableAdd_clicked()
{
	QString sceneName = ui->executableScenes->currentText();
	QString exeName = ui->executableExecutables->currentText();
	QString transitionName = ui->executableTransitions->currentText();
	bool inFocus = ui->requiresFocusCheckBox->isChecked();

	if (exeName.isEmpty() || sceneName.isEmpty())
		return;

	OBSWeakSource source = GetWeakSourceByQString(sceneName);
	OBSWeakSource transition = GetWeakTransitionByQString(transitionName);
	QVariant v = QVariant::fromValue(exeName);

	QString text = MakeSwitchNameExecutable(sceneName, exeName, transitionName, inFocus);

	int idx = executableFindByData(exeName);

	if (idx == -1)
	{
		lock_guard<mutex> lock(switcher->m);
		switcher->executableSwitches.emplace_back(
			source, transition, exeName.toUtf8().constData(), inFocus);

		QListWidgetItem* item = new QListWidgetItem(text, ui->executableSwitchesList);
		item->setData(Qt::UserRole, v);
	}
	else
	{
		QListWidgetItem* item = ui->executableSwitchesList->item(idx);
		item->setText(text);

		{
			lock_guard<mutex> lock(switcher->m);
			for (auto& s : switcher->executableSwitches)
			{
				if (s.mExe == exeName)
				{
					s.mScene = source;
					s.mTransition = transition;
					s.mInFocus = inFocus;
					break;
				}
			}
		}

		ui->executableSwitchesList->sortItems();
	}
}

void SceneSwitcher::on_executableRemove_clicked()
{
	QListWidgetItem* item = ui->executableSwitchesList->currentItem();
	if (!item)
		return;

	QString exe = item->data(Qt::UserRole).toString();

	{
		lock_guard<mutex> lock(switcher->m);
		auto& switches = switcher->executableSwitches;

		for (auto it = switches.begin(); it != switches.end(); ++it)
		{
			auto& s = *it;

			if (s.mExe == exe)
			{
				switches.erase(it);
				break;
			}
		}
	}

	delete item;
}

void SwitcherData::checkExeSwitch(bool& match, OBSWeakSource& scene, OBSWeakSource& transition)
{
	QStringList runningProcesses;
	GetProcessList(runningProcesses);
	for (ExecutableSceneSwitch& s : executableSwitches)
	{
		if (runningProcesses.contains(s.mExe))
		{
			scene = s.mScene;
			transition = s.mTransition;
			match = !s.mInFocus || (s.mInFocus && isInFocus(s.mExe));
			break;
		}
	}
}

void SaveExecutableSwitcher(obs_data_array_t*& array) {
	for (ExecutableSceneSwitch& s : switcher->executableSwitches)
	{
		obs_data_t* array_obj = obs_data_create();

		obs_source_t* source = obs_weak_source_get_source(s.mScene);
		obs_source_t* transition = obs_weak_source_get_source(s.mTransition);

		if (source && transition)
		{
			const char* sceneName = obs_source_get_name(source);
			const char* transitionName = obs_source_get_name(transition);
			obs_data_set_string(array_obj, "scene", sceneName);
			obs_data_set_string(array_obj, "transition", transitionName);
			obs_data_set_string(array_obj, "exefile", s.mExe.toUtf8());
			obs_data_set_bool(array_obj, "infocus", s.mInFocus);
			obs_data_array_push_back(array, array_obj);
			obs_source_release(source);
			obs_source_release(transition);
		}

		obs_data_release(array_obj);
	}
}

void LoadExecutableSwitcher(obs_data_array_t*& array) {
	switcher->executableSwitches.clear();
	size_t count = obs_data_array_count(array);

	for (size_t i = 0; i < count; i++)
	{
		obs_data_t* array_obj = obs_data_array_item(array, i);

		const char* scene = obs_data_get_string(array_obj, "scene");
		const char* transition = obs_data_get_string(array_obj, "transition");
		const char* exe = obs_data_get_string(array_obj, "exefile");
		bool infocus = obs_data_get_bool(array_obj, "infocus");

		switcher->executableSwitches.emplace_back(
			GetWeakSourceByName(scene), GetWeakTransitionByName(transition), exe, infocus);

		obs_data_release(array_obj);
	}
}
