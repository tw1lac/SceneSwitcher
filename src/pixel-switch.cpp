#include "headers/advanced-scene-switcher.hpp"

void SwitcherData::checkPixelSwitch(bool& match, OBSWeakSource& scene, OBSWeakSource& transition)
{
	string currPixelColor;


	for (auto& s : pixelColorSwitches)
	{
		getPixelColor(NULL, s.pxX, s.pxY, currPixelColor);
		if (s.colorsStr.find(currPixelColor))
		{
			match = true;
			scene = s.scene;
			transition = s.transition;
			return;
		}
	}
}

//void SceneSwitcher::updateScreenRegionCursorPos()
//{
//	/*pair<int, int> position = getCursorPos();
//	ui->cursorXPosition->setText(QString::number(position.first));
//	;
//	ui->cursorYPosition->setText(QString::number(position.second));*/
//}

void SceneSwitcher::on_pixelSwitchesAdd_clicked()
{
	QString sceneName = ui->pixelColorScenes->currentText();
	QString transitionName = ui->pixelColorTransitions->currentText();

	if (sceneName.isEmpty()) { 
		return;
	}

	int pxX = ui->pixelX->value();
	int pxY = ui->pixelY->value();
	QString colors = ui->pixelColorsLineEdit->text();

	string pixelStr =  to_string(pxX) + ", " + to_string(pxY) ;
	QString pixel = QString::fromStdString(pixelStr) + colors;

	OBSWeakSource source = GetWeakSourceByQString(sceneName);
	OBSWeakSource transition = GetWeakTransitionByQString(transitionName);
	QVariant v = QVariant::fromValue(pixel);

	QString text = MakePixelSwitchName(sceneName, transitionName, pxX, pxY, colors);

	int idx = PixelFindByData(pixel);

	if (idx == -1)
	{
		QListWidgetItem* item = new QListWidgetItem(text, ui->pixelColorSwitchesList);
		item->setData(Qt::UserRole, v);
		lock_guard<mutex> lock(switcher->m);
		switcher->pixelColorSwitches.emplace_back(
			source, source, "", transition, false,false, pxX, pxY, 0, 0, colors.toUtf8().constData(), pixelStr);
	}
	else
	{
		QListWidgetItem* item = ui->pixelColorSwitchesList->item(idx);
		item->setText(text);

		string curPixel = colors.toUtf8().constData();

		{
			lock_guard<mutex> lock(switcher->m);
			for (auto& s : switcher->pixelColorSwitches)
			{
				if (s.colorsStr == curPixel)
				{
					s.scene = source;
					s.transition = transition;
					break;
				}
			}
		}

		ui->pixelColorSwitchesList->sortItems();
	}
}

void SceneSwitcher::on_pixelSwitchesRemove_clicked()
{
	QListWidgetItem* item = ui->pixelColorSwitchesList->currentItem();
	if (!item)
		return;

	string pixel = item->data(Qt::UserRole).toString().toUtf8().constData();

	{
		lock_guard<mutex> lock(switcher->m);
		auto& switches = switcher->pixelColorSwitches;

		for (auto it = switches.begin(); it != switches.end(); ++it)
		{
			auto& s = *it;

			if (s.uiDisplayStr == pixel)
			{
				switches.erase(it);
				break;
			}
		}
	}

	delete item;
}

// Keeps UI updated
void SceneSwitcher::on_pixelSwitches_currentRowChanged(int idx)
{
	if (loading)
		return;
	if (idx == -1)
		return;

	QListWidgetItem* item = ui->pixelColorSwitchesList->item(idx);

	QString pixel = item->data(Qt::UserRole).toString();

	lock_guard<mutex> lock(switcher->m);
	for (auto& s : switcher->pixelColorSwitches)
	{
		if (pixel.compare(s.uiDisplayStr.c_str()) == 0)
		{
			string name = GetWeakSourceName(s.scene);
			string transitionName = GetWeakSourceName(s.transition);
			//QString colorsText = s.colorsStr.c_str();
			ui->pixelColorScenes->setCurrentText(name.c_str());
			ui->pixelColorTransitions->setCurrentText(transitionName.c_str());
			ui->pixelX->setValue(s.pxX);
			ui->pixelY->setValue(s.pxY);
			ui->pixelColorsLineEdit->setText(s.colorsStr.c_str());
			break;
		}
	}
}

int SceneSwitcher::PixelFindByData(const QString& pixel)
{
	int count = ui->pixelColorSwitchesList->count();
	int idx = -1;

	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = ui->pixelColorSwitchesList->item(i);
		QString itemPixel = item->data(Qt::UserRole).toString();

		if (itemPixel == pixel)
		{
			idx = i;
			break;
		}
	}

	return idx;
	//return 0;
}

void SavePixelSwitcher(obs_data_array_t*& array) {
	//for (PixelSwitch& s : switcher->pixelColorSwitches)
	for (StructSwitch& s : switcher->pixelColorSwitches)
	{
		obs_data_t* array_obj = obs_data_create();

		obs_source_t* source = obs_weak_source_get_source(s.scene);
		obs_source_t* transition = obs_weak_source_get_source(s.transition);
		if (source && transition)
		{
			const char* sceneName = obs_source_get_name(source);
			const char* transitionName = obs_source_get_name(transition);
			obs_data_set_string(array_obj, "pixelColorScene", sceneName);
			obs_data_set_string(array_obj, "transition", transitionName);
			obs_data_set_int(array_obj, "pxX", s.pxX);
			obs_data_set_int(array_obj, "pxY", s.pxY);
			obs_data_set_string(array_obj, "colorsStr", s.colorsStr.c_str());
			obs_data_set_string(array_obj, "pixelStr", s.uiDisplayStr.c_str());
			obs_data_array_push_back(array, array_obj);
			obs_source_release(source);
			obs_source_release(transition);
		}

		obs_data_release(array_obj);
	}
}

void LoadPixelSwitcher(obs_data_array_t*& array) {
	switcher->pixelColorSwitches.clear();
	size_t count = obs_data_array_count(array);

	for (size_t i = 0; i < count; i++)
	{
		obs_data_t* array_obj = obs_data_array_item(array, i);

		const char* scene = obs_data_get_string(array_obj, "pixelScene");
		const char* transition = obs_data_get_string(array_obj, "transition");
		int pxX = obs_data_get_int(array_obj, "pxX");
		int pxY = obs_data_get_int(array_obj, "pxY");
		string colorsStr = obs_data_get_string(array_obj, "colorsStr");
		string pixelStr = obs_data_get_string(array_obj, "pixelStr");

		switcher->pixelColorSwitches.emplace_back(GetWeakSourceByName(scene), GetWeakSourceByName(scene),
			"", GetWeakTransitionByName(transition), false, false, pxX, pxY, 0, 0, colorsStr, pixelStr);

		obs_data_release(array_obj);
	}
}
