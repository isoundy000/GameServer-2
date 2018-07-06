/*
 * GameShop.h
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#ifndef GAMESHOP_H_
#define GAMESHOP_H_

#include "LogicStruct.h"

struct GameShop
{
	/*
	 * key : item_id
	 * */
	typedef std::map<int, ShopItem*> ShopItemMap;

	GameShop();
	void reset();
	void sort();

	ShopItem* find_item(int item_id);

	int shop_type_;
	IntVec item_vec_;
	ShopItemMap shop_item_map_;
};

#endif /* GAMESHOP_H_ */
