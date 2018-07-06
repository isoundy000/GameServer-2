/*
 * ArrTimeManager.cpp
 *
 *  Created on: Nov 8, 2013
 *      Author: peizhibi
 */

#include "ArrTimeManager.h"

ArrTimeManager::ArrTimeManager(HandleTimeOut handle_timeout)
{
	// TODO Auto-generated constructor stub
	this->handle_timeout_ = handle_timeout;
}

ArrTimeManager::~ArrTimeManager()
{
	// TODO Auto-generated destructor stub
}


void ArrTimeManager::add_item(ArrTimeItem* arr_item)
{
	ArrItemList::iterator iter = this->arr_item_list_.begin();
	for (; iter != this->arr_item_list_.end(); ++iter)
	{
		ArrTimeItem* cur_item = *iter;
		JUDGE_BREAK(arr_item->arr_tick_ >= cur_item->arr_tick_);
	}

	this->arr_item_list_.insert(iter, arr_item);
}

void ArrTimeManager::remove_item(ArrTimeItem* arr_item)
{
	JUDGE_RETURN(arr_item != NULL, ;);

	ArrItemList::iterator iter = this->arr_item_list_.begin();
	for (; iter != this->arr_item_list_.end(); ++iter)
	{
		ArrTimeItem* cur_item = *iter;
		JUDGE_CONTINUE(cur_item == arr_item);

		this->arr_item_list_.erase(iter);
		break;
	}
}

void ArrTimeManager::handle_time_out(Int64 now_tick)
{
	ArrItemList arr_item_list(this->arr_item_list_.begin(),
			this->arr_item_list_.end());

	ArrItemList::iterator iter = arr_item_list.begin();
	while (iter != arr_item_list.end())
	{
		ArrTimeItem* cur_item = *iter;
		if (cur_item->arr_tick_ <= now_tick)
		{
			this->handle_timeout_(cur_item);
			this->remove_item(cur_item);
		}
		else
		{
			break;
		}

		++iter;
	}
}
