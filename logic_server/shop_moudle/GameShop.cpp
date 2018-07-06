/*
 * GameShop.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#include "GameShop.h"
#include "ShopMonitor.h"

GameShop::GameShop() : shop_type_(0)
{ /*NULL*/ }

void GameShop::reset()
{
    for (ShopItemMap::iterator iter = this->shop_item_map_.begin();
    		iter != this->shop_item_map_.end(); ++iter)
    {
        SHOP_MONITOR->push_shop_item(iter->second);
    }

    this->shop_type_ = 0;
    this->item_vec_.clear();
    this->shop_item_map_.clear();
}

void GameShop::sort()
{
	PairObjVec rank_vec;
	rank_vec.reserve(this->shop_item_map_.size());

	for (ShopItemMap::iterator iter = this->shop_item_map_.begin();
			iter != this->shop_item_map_.end(); ++iter)
	{
		PairObj obj;
		obj.id_ = iter->second->item_id_;
		obj.value_ = iter->second->item_pos_;
		rank_vec.push_back(obj);
	}

	std::sort(rank_vec.begin(), rank_vec.end(), GameCommon::pair_comp_by_asc);

	this->item_vec_.clear();
	this->item_vec_.reserve(rank_vec.size());

	for (PairObjVec::iterator iter = rank_vec.begin();
			iter != rank_vec.end(); ++iter)
	{
		this->item_vec_.push_back(iter->id_);
	}
}

ShopItem* GameShop::find_item(int item_id)
{
	JUDGE_RETURN(this->shop_item_map_.count(item_id) > 0, NULL);
	return this->shop_item_map_[item_id];
}

