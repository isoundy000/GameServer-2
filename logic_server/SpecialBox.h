/*
 * SpecialBox.h
 *
 *  Created on: May 24, 2017
 *      Author: sai
 */

#include "BaseLogicPlayer.h"
#include "ProtoDefine.h"

#ifndef SPECIALBOX_H_
#define SPECIALBOX_H_

class SpecialBox : virtual public BaseLogicPlayer
{
public:
	enum
	{
		BUY_KEY,
		COST_ITEM,
		COST_MONEY,
		CHANGE_ITEM,
	};
public:
	SpecialBox();
	virtual ~SpecialBox();
	void init();
	void reset_every_day();
public:
	//神秘宝箱
	int fetch_special_box_info(Message *msg);
	int fetch_special_box_buy_key_begin(Message *msg);
	int fetch_special_box_buy_key_end(Message *msg);

	//抽奖
	int fetch_special_box_reward_check_money(Message *msg);
	int fetch_special_box_reward_check_item(Message *msg);
	int fetch_special_box_reward_end(Message *msg);

	//兑换
	int fetch_special_box_change_info(Message *msg);
	int fetch_special_box_change_reward_begin(Message *msg);
	int fetch_special_box_change_reward_end(Message *msg);

	void clean_special_box_info();

	void serial_draw_serialize(ProtoSerialObj* obj, int type);
	int cost_item_or_money_return(Message* msg);

	int special_box_get_buy_times();
	int special_box_get_score();
	int special_box_get_refresh_times(int slot_id);
	IntMap& special_box_get_refresh_times_map();

	void special_box_set_buy_times(int times);
	void special_box_set_score(int score);
	void special_box_set_refresh_times(int slot_id, int times);
protected:
	SpecialBoxItemMap& fetch_special_box_item_by_day(int day);
	ChangeItemMap& fetch_special_box_change_item_by_day(int day);
	void test_print_refresh_list();
public:
	int buy_times_;
	int score_;
	IntMap refresh_times_;
	DayBoxItemMap day_item_;
	DayChangeItemMap day_change_item_;
};

#endif /* SPECIALBOX_H_ */
