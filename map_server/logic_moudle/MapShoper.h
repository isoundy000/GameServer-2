/*
 * MapShoper.h
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#ifndef MAPSHOPER_H_
#define MAPSHOPER_H_

#include "MLPacker.h"

class MapShoper : virtual public MLPacker
{
public:
	enum NPC_SHOP_PAY_TYPE
	{
		NS_PAYTYPE_MONEY = 1,	// 金钱购买
		NS_PAYTYPE_ITEM = 2,	// 道具购买
		NS_PAYTYPE_MONEY_AND_ITEM = 3,	// 金钱和道具都需要
		NS_PAYTYPE_SCORE = 4,	// 活动积分购买
        NS_PAYTYPE_EXPLOIT = 5, // 功勋兑换
        NS_PAYTYPE_REPUTATION = 6, // 声望兑换
        NS_PAYTYPE_SECRET_TREASIRE = 7, // 秘藏兑换
        NS_PAYTYPE_HONOUR = 8,	//荣誉兑换
		NS_PAYTYPE_END
	};
public:
	MapShoper();
	virtual ~MapShoper();

	void reset();

	int sell_goods(Message* msg);
	int sell_out_info();
	int buy_goods_back(Message* msg);

	Money fetch_buy_money(int money_item, int money_amount,
			int buy_amount = 1);
	int fetch_need_item(IntMap& need_map, const IntMap& item_map,
			int buy_count);

	/*
	 * mall
	 * */
	int get_own_goods(Message* msg);
	int buy_mall_goods(Message* msg);
	int buy_mall_goods_use_money(Message* msg);
	int buy_mall_goods_use_resource(Message* msg);
	int buy_mall_goods_use_goods(Message* msg);
	int map_process_donate_mall_goods(Message* msg);

	int batch_sell_goods(Message* msg);
	int lrf_buy_hickty(Message* msg);

private:
	SellOutDetail sellout_detial_;
};

#endif /* MAPSHOPER_H_ */
