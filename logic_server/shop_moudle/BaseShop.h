/*
 * BaseShop.h
 *
 *  Created on: Sep 29, 2014
 *      Author: root
 */

#ifndef BASESHOP_H_
#define BASESHOP_H_

#include "GameShop.h"

class BaseShop {

public:
	BaseShop();
	virtual ~BaseShop();

public:
	/*
	 * key : shop_type,value : game_shop
	 * */
	typedef std::map<int, GameShop*> GameShopMap;

	GameShopMap* shop_map(void);

	GameShop* find_shop(int shop_type);
	GameShop* find_and_pop(GameShopMap& shop_map, int shop_type);

	GameShop* pop_game_shop();
	void push_game_shop(GameShop* game_shop);

	ShopItem* pop_shop_item();
	void push_shop_item(ShopItem* shop_item);

	int add_shop_item(GameShopMap& shop_map, ShopItem* shop_item);/// 会取代已存在的shop_item
	int recycle_game_shop(GameShop* game_shop);

protected:
	ShopItem* fetch_shop_item(const int shop_type, const int item_id);

protected:
	GameShopMap shop_map_;
	ObjectPoolEx<GameShop> game_shop_pool_;
	ObjectPoolEx<ShopItem> shop_item_pool_;
};

#endif /* BASESHOP_H_ */
