#include <QFileDialog>
#include <QTextStream>
#include <obs.hpp>

#include "headers/advanced-scene-switcher.hpp"

void SceneSwitcher::on_sceneRoundTripAdd_clicked()
{
	QString scene1Name = ui->sceneRoundTripFromScenes->currentText();
	QString scene2Name = ui->sceneRoundTripToScenes->currentText();
	QString transitionName = ui->sceneRoundTripTransitions->currentText();

	if (scene1Name.isEmpty() || scene2Name.isEmpty())
		return;

	double delay = ui->sceneRoundTripSpinBox->value();

	if (scene1Name == scene2Name)
		return;

	OBSWeakSource source1 = GetWeakSourceByQString(scene1Name);
	OBSWeakSource source2 = GetWeakSourceByQString(scene2Name);
	OBSWeakSource transition = GetWeakTransitionByQString(transitionName);

	QString text = MakeSceneRoundTripSwitchName(scene1Name, scene2Name, transitionName, delay);
	QVariant v = QVariant::fromValue(text);

	int idx = SceneRoundTripFindByData(scene1Name);

	if (idx == -1)
	{
		QListWidgetItem* item = new QListWidgetItem(text, ui->sceneRoundTripSwitchesList);
		item->setData(Qt::UserRole, v);
		//string ugg = "ugg";
		lock_guard<mutex> lock(switcher->m);
		switcher->sceneRoundTripSwitches.emplace_back(
			source1, source2, transition, int(delay * 1000), (scene2Name == QString(PREVIOUS_SCENE_NAME)), text.toUtf8().constData());
		//switcher->sceneRoundTripSwitches.emplace_back(
		//	source1, source2, ugg, transition, false, false, 0, 0,0,0,
		//	text.toUtf8().constData(), "");
		//(scene2Name == QString(PREVIOUS_SCENE_NAME)) int(delay * 1000)
	}
	else
	{
		QListWidgetItem* item = ui->sceneRoundTripSwitchesList->item(idx);
		item->setText(text);

		{
			lock_guard<mutex> lock(switcher->m);
			for (auto& s : switcher->sceneRoundTripSwitches)
			{
				if (s.scene == source1)
				{
					s.scene2 = source2;
					s.delay = int(delay * 1000);
					s.transition = transition;
					s.usePreviousScene = (scene2Name == QString(PREVIOUS_SCENE_NAME));
					s.sceneRoundTripStr = text.toUtf8().constData();
					break;
				}
			}
		}

		ui->sceneRoundTripSwitchesList->sortItems();
	}
}

void SceneSwitcher::on_sceneRoundTripRemove_clicked()
{
	QListWidgetItem* item = ui->sceneRoundTripSwitchesList->currentItem();
	if (!item)
		return;

	string text = item->data(Qt::UserRole).toString().toUtf8().constData();

	{
		lock_guard<mutex> lock(switcher->m);
		auto& switches = switcher->sceneRoundTripSwitches;

		for (auto it = switches.begin(); it != switches.end(); ++it)
		{
			auto& s = *it;

			if (s.sceneRoundTripStr == text)
			{
				switches.erase(it);
				break;
			}
		}
	}

	delete item;
}

void SceneSwitcher::on_autoStopSceneCheckBox_stateChanged(int state)
{
	if (loading)
		return;

	lock_guard<mutex> lock(switcher->m);
	if (!state)
	{
		ui->autoStopScenes->setDisabled(true);
		switcher->autoStopEnable = false;
	}
	else
	{
		ui->autoStopScenes->setDisabled(false);
		switcher->autoStopEnable = true;
	}
}

void SceneSwitcher::UpdateAutoStopScene(const QString& name)
{
	obs_source_t* scene = obs_get_source_by_name(name.toUtf8().constData());
	obs_weak_source_t* ws = obs_source_get_weak_source(scene);

	switcher->autoStopScene = ws;

	obs_weak_source_release(ws);
	obs_source_release(scene);
}

void SceneSwitcher::on_autoStopScenes_currentTextChanged(const QString& text)
{
	if (loading)
		return;

	lock_guard<mutex> lock(switcher->m);
	UpdateAutoStopScene(text);
}

void SceneSwitcher::on_sceneRoundTripSave_clicked()
{
	QString directory = QFileDialog::getSaveFileName(
		this, tr("Save Scene Round Trip to file ..."), QDir::currentPath(), tr("Text files (*.txt)"));
	if (!directory.isEmpty())
	{
		QFile file(directory);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;
		QTextStream out(&file);
		//for (SceneRoundTripSwitch s : switcher->sceneRoundTripSwitches)
		for (SceneRoundTripSwitch s : switcher->sceneRoundTripSwitches)
		{
			out << QString::fromStdString(GetWeakSourceName(s.scene)) << "\n";
			if (s.usePreviousScene)
				out << (PREVIOUS_SCENE_NAME) << "\n";
			else
				out << QString::fromStdString(GetWeakSourceName(s.scene2)) << "\n";
			out << s.delay << "\n";
			out << QString::fromStdString(s.sceneRoundTripStr) << "\n";
			out << QString::fromStdString(GetWeakSourceName(s.transition)) << "\n";
		}
	}
}

void SceneSwitcher::on_sceneRoundTripLoad_clicked()
{
	lock_guard<mutex> lock(switcher->m);

	QString directory = QFileDialog::getOpenFileName(
		this, tr("Select a file to read Scene Round Trip from ..."), QDir::currentPath(), tr("Text files (*.txt)"));
	if (!directory.isEmpty())
	{
		QFile file(directory);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;

		QTextStream in(&file);
		vector<QString> lines;

		vector<SceneRoundTripSwitch> newSceneRoundTripSwitch;

		while (!in.atEnd())
		{
			QString line = in.readLine();
			lines.push_back(line);
			if (lines.size() == 5)
			{
				OBSWeakSource scene1 = GetWeakSourceByQString(lines[0]);
				OBSWeakSource scene2 = GetWeakSourceByQString(lines[1]);
				OBSWeakSource transition = GetWeakTransitionByQString(lines[4]);

				if (WeakSourceValid(scene1) && ((lines[1] == QString(PREVIOUS_SCENE_NAME)) || (WeakSourceValid(scene2)))
					&& WeakSourceValid(transition))
				{
					newSceneRoundTripSwitch.emplace_back(SceneRoundTripSwitch(
						GetWeakSourceByQString(lines[0]),
						GetWeakSourceByQString(lines[1]),
						GetWeakTransitionByQString(lines[4]),
						lines[2].toInt(),
						(lines[1] == QString(PREVIOUS_SCENE_NAME)),
						lines[3].toStdString()));
				}
				lines.clear();
			}
		}

		if (lines.size() != 0 || newSceneRoundTripSwitch.size() == 0)
			return;

		switcher->sceneRoundTripSwitches.clear();
		ui->sceneRoundTripSwitchesList->clear();
		switcher->sceneRoundTripSwitches = newSceneRoundTripSwitch;
		for (SceneRoundTripSwitch s : switcher->sceneRoundTripSwitches)
		{
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(s.sceneRoundTripStr), ui->sceneRoundTripSwitchesList);
			item->setData(Qt::UserRole, QString::fromStdString(s.sceneRoundTripStr));
		}
	}
}

void SwitcherData::checkSceneRoundTrip(bool& match, OBSWeakSource& scene, OBSWeakSource& transition, unique_lock<mutex>& lock)
{
	bool sceneRoundTripActive = false;
	obs_source_t* currentSource = obs_frontend_get_current_scene();
	obs_weak_source_t* ws = obs_source_get_weak_source(currentSource);

	for (SceneRoundTripSwitch& s : sceneRoundTripSwitches)
	{
		if (s.scene == ws)
		{
			sceneRoundTripActive = true;
			int dur = s.delay - interval;
			if (dur > 0)
			{
				waitScene = currentSource;
				cv.wait_for(lock, chrono::milliseconds(dur));
			}
			obs_source_t* currentSource2 = obs_frontend_get_current_scene();

			// only switch if user hasn't changed scene manually
			if (currentSource == currentSource2)
			{
				match = true;
				scene = (s.usePreviousScene) ? previousScene : s.scene2;
				transition = s.transition;
			}
			obs_source_release(currentSource2);
			break;
		}
	}
	obs_source_release(currentSource);
	obs_weak_source_release(ws);
}

void SceneSwitcher::on_sceneRoundTrips_currentRowChanged(int idx)
{
	if (loading)
		return;
	if (idx == -1)
		return;

	QListWidgetItem* item = ui->sceneRoundTripSwitchesList->item(idx);

	QString sceneRoundTrip = item->text();

	lock_guard<mutex> lock(switcher->m);
	for (auto& s : switcher->sceneRoundTripSwitches)
	{
		if (sceneRoundTrip.compare(s.sceneRoundTripStr.c_str()) == 0)
		{
			string scene1 = GetWeakSourceName(s.scene);
			string scene2 = GetWeakSourceName(s.scene2);
			string transitionName = GetWeakSourceName(s.transition);
			int delay = s.delay;
			ui->sceneRoundTripFromScenes->setCurrentText(scene1.c_str());
			ui->sceneRoundTripToScenes->setCurrentText(scene2.c_str());
			ui->sceneRoundTripTransitions->setCurrentText(transitionName.c_str());
			ui->sceneRoundTripSpinBox->setValue((double)delay/1000);
			break;
		}
	}
}

int SceneSwitcher::SceneRoundTripFindByData(const QString& scene1)
{
	QRegExp rx(scene1 + " ->.*");
	int count = ui->sceneRoundTripSwitchesList->count();
	int idx = -1;

	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = ui->sceneRoundTripSwitchesList->item(i);
		QString itemString = item->data(Qt::UserRole).toString();

		if (rx.exactMatch(itemString))
		{
			idx = i;
			break;
		}
	}

	return idx;
}

void SwitcherData::autoStopStreamAndRecording()
{
	obs_source_t* currentSource = obs_frontend_get_current_scene();
	obs_weak_source_t* ws = obs_source_get_weak_source(currentSource);

	if (ws && autoStopScene == ws)
	{
		if (obs_frontend_streaming_active())
			obs_frontend_streaming_stop();
		if (obs_frontend_recording_active())
			obs_frontend_recording_stop();
	}
	obs_source_release(currentSource);
	obs_weak_source_release(ws);
}

void SaveScreenRoundTripSwitcher(obs_data_array_t*& array) {
	for (SceneRoundTripSwitch& s : switcher->sceneRoundTripSwitches)
	//for (StructSwitch& s : switcher->sceneRoundTripSwitches)
	{
		obs_data_t* array_obj = obs_data_create();

		obs_source_t* source1 = obs_weak_source_get_source(s.scene);
		obs_source_t* source2 = obs_weak_source_get_source(s.scene2);
		obs_source_t* transition = obs_weak_source_get_source(s.transition);
		if (source1 && (s.usePreviousScene || source2) && transition)
		{
			const char* sceneName1 = obs_source_get_name(source1);
			const char* sceneName2 = obs_source_get_name(source2);
			const char* transitionName = obs_source_get_name(transition);
			obs_data_set_string(array_obj, "sceneRoundTripScene1", sceneName1);
			obs_data_set_string(array_obj, "sceneRoundTripScene2",
				s.usePreviousScene ? PREVIOUS_SCENE_NAME : sceneName2);
			obs_data_set_string(array_obj, "transition", transitionName);
			obs_data_set_int(array_obj, "sceneRoundTripDelay", s.delay / 1000);	//delay stored in two separate values
			obs_data_set_int(array_obj, "sceneRoundTripDelayMs", s.delay % 1000);	//to be compatible with older versions
			obs_data_set_string(array_obj, "sceneRoundTripStr", s.sceneRoundTripStr.c_str());
			obs_data_array_push_back(array, array_obj);
			obs_source_release(source1);
			obs_source_release(source2);
			obs_source_release(transition);
		}

		obs_data_release(array_obj);
	}
}

void LoadScreenRoundTripSwitcher(obs_data_array_t*& array) {
	switcher->sceneRoundTripSwitches.clear();
	size_t count = obs_data_array_count(array);

	for (size_t i = 0; i < count; i++)
	{
		obs_data_t* array_obj = obs_data_array_item(array, i);

		const char* scene1 = obs_data_get_string(array_obj, "sceneRoundTripScene1");
		const char* scene2 = obs_data_get_string(array_obj, "sceneRoundTripScene2");
		const char* transition = obs_data_get_string(array_obj, "transition");
		int delay = obs_data_get_int(array_obj, "sceneRoundTripDelay");			//delay stored in two separate values
		delay = delay * 1000 + obs_data_get_int(array_obj, "sceneRoundTripDelayMs"); 	//to be compatible with older versions
		string str = MakeSceneRoundTripSwitchName(scene1, scene2, transition, ((double)delay) / 1000.0).toUtf8().constData();
		const char* sceneRoundTripStr = str.c_str();

		switcher->sceneRoundTripSwitches.emplace_back(GetWeakSourceByName(scene1),
			GetWeakSourceByName(scene2), GetWeakTransitionByName(transition), delay,
			(strcmp(scene2, PREVIOUS_SCENE_NAME) == 0), sceneRoundTripStr);

		obs_data_release(array_obj);
	}
}
