/*
 * LogicShoper.h
 *
 *  Created on: Nov 1, 2013
 *      Author: peizhibi
 */

#ifndef LOGICSHOPER_H_
#define LOGICSHOPER_H_

#include "BaseLogicPlayer.h"

class LogicShoper : virtual public BaseLogicPlayer
{
public:
	LogicShoper();
	virtual ~LogicShoper();

	void reset();

	/*获取商城界面信息*/
	int request_mall_info(Message* msg);
	int after_request_mall_info(Message* msg);
	int fetch_mall_good_detail(Message* msg);

	/*购买商城物品*/
	int mall_buy_goods_begin(Message* msg);
	int mall_buy_goods_done(Message* msg);

	/*商城赠送物品（不能赠送礼金商城的道具）*/
	int mall_donate_goods_begin(Message* msg);
	int after_load_for_mall_donate(DBShopMode* shop_mode);
	int mall_donate_goods_done(Message* msg);

	int fetch_mall_item_price(Message* msg);
	int fashion_fast_use_begin(Message* msg);

private:
	int judge_mall_item_sex(ShopItem* shop_item);
	int judge_mall_buy_condition(ShopItem* shop_item);
	int judge_mall_item_has_type(ShopItem* shop_item, const int type);
	int judge_mall_item_has_types_one(ShopItem* shop_item, const IntVec& types);

public:
	ShopItem* find_shop_item(int item_id, int shop_type);

	int fetch_left_item(const ShopItem* item);
	int fetch_left_total_item(const ShopItem* item);

	void set_item_buy_limited(ShopItem* item, int buy_amount);
	void set_item_buy_total_limited(ShopItem* item, int buy_amount);
};

#endif /* LOGICSHOPER_H_ */
