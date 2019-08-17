#include <algorithm>
#include "headers/advanced-scene-switcher.hpp"

void SceneSwitcher::on_priorityUp_clicked()
{
	int currentIndex = ui->priorityList->currentRow();
	if (currentIndex != -1 && currentIndex != 0)
	{
		ui->priorityList->insertItem(currentIndex - 1 ,ui->priorityList->takeItem(currentIndex));
		ui->priorityList->setCurrentRow(currentIndex -1);
		lock_guard<mutex> lock(switcher->m);

		iter_swap(switcher->functionNamesByPriority.begin() + currentIndex, switcher->functionNamesByPriority.begin() + currentIndex - 1);
	}
}

void SceneSwitcher::on_priorityDown_clicked()
{
	int currentIndex = ui->priorityList->currentRow();
	if (currentIndex != -1 && currentIndex != ui->priorityList->count() - 1)
	{
		ui->priorityList->insertItem(currentIndex + 1, ui->priorityList->takeItem(currentIndex));
		ui->priorityList->setCurrentRow(currentIndex + 1);
		lock_guard<mutex> lock(switcher->m);

		iter_swap(switcher->functionNamesByPriority.begin() + currentIndex, switcher->functionNamesByPriority.begin() + currentIndex + 1);
	}
}

bool SwitcherData::prioFuncsValid()
{
	auto it = std::unique(functionNamesByPriority.begin(), functionNamesByPriority.end());
	bool wasUnique = (it == functionNamesByPriority.end());
	if (!wasUnique)
		return false;

	for (int p : functionNamesByPriority)
	{
		if (p < 0 || p > 5)
			return false;
	}
	return true;
}

void SavePriorityOrder(obs_data_t*& obj) {
	for (int i = 0; i < switcher->functionNamesByPriority.size(); i++) {
		obs_data_set_int(obj, "priority" + i, switcher->functionNamesByPriority[i]);
	}
}

void LoadPriorityOrder(obs_data_t*& obj) {
	for (int i = 0; i < switcher->functionNamesByPriority.size(); i++) {
		obs_data_set_default_int(obj, "priority" + i, switcher->functionNamesByPriority[i]);
		switcher->functionNamesByPriority[i] = (obs_data_get_int(obj, "priority" + i));
	}

	if (!switcher->prioFuncsValid())
	{
		switcher->functionNamesByPriority = vector<int>{ DEFAULT_PRIORITY };
	}
}