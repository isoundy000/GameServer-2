/*
 * BaseShop.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: root
 */

#include "BaseShop.h"

BaseShop::BaseShop()
{
	// TODO Auto-generated constructor stub

}

BaseShop::~BaseShop()
{
	// TODO Auto-generated destructor stub
}

BaseShop::GameShopMap* BaseShop::shop_map(void)
{
	return &(this->shop_map_);
}

GameShop* BaseShop::find_shop(int shop_type)
{
	JUDGE_RETURN(this->shop_map_.count(shop_type) > 0, NULL);
	return this->shop_map_[shop_type];
}

GameShop* BaseShop::find_and_pop(GameShopMap& shop_map, int shop_type)
{
	GameShopMap::iterator iter = shop_map.find(shop_type);
	JUDGE_RETURN(iter == shop_map.end(), iter->second);

	GameShop* game_shop = this->pop_game_shop();
	JUDGE_RETURN(game_shop != NULL, NULL);

	game_shop->shop_type_ = shop_type;
	shop_map[shop_type] = game_shop;

	return game_shop;
}

GameShop* BaseShop::pop_game_shop()
{
	return this->game_shop_pool_.pop();
}

void BaseShop::push_game_shop(GameShop* game_shop)
{
	this->game_shop_pool_.push(game_shop);
}

ShopItem* BaseShop::pop_shop_item()
{
	return this->shop_item_pool_.pop();
}

void BaseShop::push_shop_item(ShopItem* shop_item)
{
	this->shop_item_pool_.push(shop_item);
}

int BaseShop::add_shop_item(GameShopMap& shop_map, ShopItem* shop_item)
{
	JUDGE_RETURN(shop_item != NULL, -1);
	JUDGE_RETURN(shop_item->shop_type_ > 0, -1);

	GameShop* game_shop = this->find_and_pop(shop_map, shop_item->shop_type_);

	GameShop::ShopItemMap::iterator iter = game_shop->shop_item_map_.find(shop_item->item_id_);
	if (iter != game_shop->shop_item_map_.end())
	{
		this->push_shop_item(iter->second);
		iter->second = shop_item;
	}
	else
	{
		game_shop->shop_item_map_[shop_item->item_id_] = shop_item;
	}

	return 0;
}

int BaseShop::recycle_game_shop(GameShop* game_shop)
{
	GameShop::ShopItemMap::iterator iter = game_shop->shop_item_map_.begin();
	for (; iter != game_shop->shop_item_map_.end(); ++iter)
	{
		this->push_shop_item(iter->second);
	}

	game_shop->shop_item_map_.clear();
	this->push_game_shop(game_shop);
	return 0;
}

ShopItem* BaseShop::fetch_shop_item(const int shop_type, const int item_id)
{
	JUDGE_RETURN(this->shop_map_.count(shop_type) > 0, NULL);

	GameShop* game_shop = this->shop_map_[shop_type];
	JUDGE_RETURN(NULL != game_shop, NULL);

	return game_shop->find_item(item_id);
}
