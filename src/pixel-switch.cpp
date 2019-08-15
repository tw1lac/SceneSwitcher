#include "headers/advanced-scene-switcher.hpp"

void SwitcherData::checkPixelSwitch(bool& match, OBSWeakSource& scene, OBSWeakSource& transition)
{
	string currPixelColor;


	for (auto& s : pixels)
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
	QString sceneName = ui->pixelScenes->currentText();
	QString transitionName = ui->pixelTransitions->currentText();

	if (sceneName.isEmpty()) { 
		return;
	}

	int pxX = ui->pixelX->value();
	int pxY = ui->pixelY->value();
	QString colors = ui->pixelColors->text();

	string pixelStr =  to_string(pxX) + ", " + to_string(pxY) ;
	QString pixel = QString::fromStdString(pixelStr) + colors;

	OBSWeakSource source = GetWeakSourceByQString(sceneName);
	OBSWeakSource transition = GetWeakTransitionByQString(transitionName);
	QVariant v = QVariant::fromValue(pixel);

	QString text = MakePixelSwitchName(sceneName, transitionName, pxX, pxY, colors);

	int idx = PixelFindByData(pixel);

	if (idx == -1)
	{
		QListWidgetItem* item = new QListWidgetItem(text, ui->pixelSwitches);
		item->setData(Qt::UserRole, v);

		lock_guard<mutex> lock(switcher->m);
		switcher->pixels.emplace_back(
			source, transition, pxX, pxY, colors.toUtf8().constData(), pixelStr);
	}
	else
	{
		QListWidgetItem* item = ui->pixelSwitches->item(idx);
		item->setText(text);

		string curPixel = colors.toUtf8().constData();

		{
			lock_guard<mutex> lock(switcher->m);
			for (auto& s : switcher->pixels)
			{
				if (s.colorsStr == curPixel)
				{
					s.scene = source;
					s.transition = transition;
					break;
				}
			}
		}

		ui->pixelSwitches->sortItems();
	}
}

void SceneSwitcher::on_pixelSwitchesRemove_clicked()
{
	QListWidgetItem* item = ui->pixelSwitches->currentItem();
	if (!item)
		return;

	string pixel = item->data(Qt::UserRole).toString().toUtf8().constData();

	{
		lock_guard<mutex> lock(switcher->m);
		auto& switches = switcher->pixels;

		for (auto it = switches.begin(); it != switches.end(); ++it)
		{
			auto& s = *it;

			if (s.pixelStr == pixel)
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

	QListWidgetItem* item = ui->pixelSwitches->item(idx);

	QString pixel = item->data(Qt::UserRole).toString();

	lock_guard<mutex> lock(switcher->m);
	for (auto& s : switcher->pixels)
	{
		if (pixel.compare(s.pixelStr.c_str()) == 0)
		{
			string name = GetWeakSourceName(s.scene);
			string transitionName = GetWeakSourceName(s.transition);
			//QString colorsText = s.colorsStr.c_str();
			ui->pixelScenes->setCurrentText(name.c_str());
			ui->pixelTransitions->setCurrentText(transitionName.c_str());
			ui->pixelX->setValue(s.pxX);
			ui->pixelY->setValue(s.pxY);
			ui->pixelColors->setText(s.colorsStr.c_str());
			break;
		}
	}
}

int SceneSwitcher::PixelFindByData(const QString& pixel)
{
	int count = ui->pixelSwitches->count();
	int idx = -1;

	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = ui->pixelSwitches->item(i);
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
