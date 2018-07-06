/*
 * MarketStruct.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: peizhibi
 */

#include "MarketStruct.h"
#include "MarketSystem.h"

MarketItem::MarketItem(void)
{
	this->market_index_ = GlobalIndex::global_market_index_++;
	MarketItem::reset();
}

void MarketItem::reset(void)
{
	this->role_id_ = 0;
	this->onsale_tick_ = 0;
	this->item_obj_.reset();

	this->main_type_ = 0;
	this->sub_type_ = 0;

	this->money_type_ = 0;
	this->price_ = 0;

	this->undo_amount_ = 0;
	this->item_back_ = false;
}

int MarketItem::set_item_id(int item_id)
{
	this->item_obj_.__id = item_id;

	this->main_type_ = MARKET_SYS->fetch_main_type(item_id);
	this->sub_type_ = MARKET_SYS->fetch_sub_type(item_id);

	return 0;
}

int MarketItem::left_tick(Int64 now_tick)
{
	return std::max<int>(this->arr_tick_ - now_tick, 0);
}

MarketItemType::MarketItemType(void) :
    market_type_(0)
{
	this->item_set_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
}

void MarketItemType::reset(void)
{
	this->market_type_ = 0;
	this->item_set_.clear();
}

void MarketItemType::fetch_item_sort(Int64 req_role, ThreeObjVec& obj_vec,
		int sub_type, int sort_type)
{
	for (MarketItemSet::iterator iter = this->item_set_.begin();
			iter != this->item_set_.end(); ++iter)
	{
		MarketItem* market_item = *iter;
		JUDGE_CONTINUE(market_item != NULL);

		if (sub_type > 0)
		{
			JUDGE_CONTINUE(market_item->sub_type_ == sub_type);
		}

//		JUDGE_CONTINUE(market_item->role_id_ != req_role_id);

		ThreeObj obj;
		obj.id_ = market_item->market_index_;
		obj.tick_ = market_item->onsale_tick_;

		if (sort_type == 1)
		{
			obj.value_ = market_item->item_obj_.__amount;
		}
		else if (sort_type == 2)
		{
			obj.value_ = market_item->price_;
		}
		else if (sort_type == 3)
		{
			obj.value_ = market_item->price_ * market_item->item_obj_.__amount;
		}

		obj_vec.push_back(obj);
	}
}

MarketItem* MarketItemType::find_market_item(int market_index)
{
	for (MarketItemSet::iterator iter = this->item_set_.begin();
			iter != this->item_set_.end(); ++iter)
	{
		MarketItem* market_item = *iter;
		JUDGE_CONTINUE(market_item->market_index_ == market_index);

		return market_item;
	}

	return NULL;
}

bool MarketItemType::erase_market_item(MarketItem* market_item)
{
	JUDGE_RETURN(market_item != NULL, false);

	for (MarketItemSet::iterator iter = this->item_set_.begin();
			iter != this->item_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter == market_item);
		market_item->item_back_ = false;
		this->item_set_.erase(iter);
		return true;
	}

	return false;
}

MarketerInfo::MarketerInfo(void)
{
	MarketerInfo::reset();
}

void MarketerInfo::reset(void)
{
	this->role_index_ = 0;
	this->sell_log_.clear();
	this->onsell_map_.clear();
}

void MarketerInfo::add_sell_log(int item_id, int money, Int64 tick)
{
	ThreeObj obj;
	obj.id_ = item_id;
	obj.value_ = money;
	obj.tick_ = ::time(NULL);
	this->sell_log_.push_front(obj);

	static uint MAX_SELL_LOG = 20;
	JUDGE_RETURN(this->sell_log_.size() > MAX_SELL_LOG, ;);

	this->sell_log_.pop_back();
}
