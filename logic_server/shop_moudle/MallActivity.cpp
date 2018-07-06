/*
 * MallActivity.cpp
 *
 *  Created on: 2013-12-23
 *      Author: louis
 */

#include "MallActivity.h"

MallActivity::MallActivity() {
	// TODO Auto-generated constructor stub

}

MallActivity::~MallActivity() {
	// TODO Auto-generated destructor stub
}

MallActivityDetail& MallActivity::mall_activity_detail(void)
{
	return this->mall_activity_detail_;
}

int MallActivity::mall_activity_refresh(void)
{
	int now_tick = Time_Value::gettimeofday().sec();
	if(this->mall_activity_detail_.__refresh_tick < now_tick)
	{
		MallActivityDetail::MallBuyRecord::iterator it = this->mall_activity_detail_.__player_record.begin();
		for(; it != this->mall_activity_detail_.__player_record.end(); ++it)
		{
			it->second.clear();
		}
		this->mall_activity_detail_.__player_record.clear();
		this->mall_activity_detail_.__server_buy_map.clear();
	}
	return 0;
}

int MallActivity::modify_buy_mall_activity_goods(Int64 role_id, int shop_type, int goods_id, int goods_amount)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	JUDGE_RETURN(shop_type == GameEnum::MALL_TYPE_ACTIVITY, 0);
	switch(this->mall_activity_detail_.__limit_type)
	{
	{
		case GameEnum::MALL_BUY_TOTAL_LIMIT:
			JUDGE_RETURN(this->calc_server_buy_left_amount(goods_id) >=
					goods_amount, ERROR_MALL_ACTIVITY_TOTAL_BUY_LIMIT);
			break;
	}
	{
		case GameEnum::MALL_BUY_SINGLE_LIMIT:
			JUDGE_RETURN(this->calc_single_buy_left_amount(role_id, goods_id) >=
					goods_amount, ERROR_MALL_ACTIVITY_SINGLE_BUY_LIMIT);
			break;
	}
	{
		case GameEnum::MALL_BUY_BOTH_LIMIT:
			JUDGE_RETURN(this->calc_server_buy_left_amount(goods_id) >=
					goods_amount, ERROR_MALL_ACTIVITY_TOTAL_BUY_LIMIT);
			JUDGE_RETURN(this->calc_single_buy_left_amount(role_id, goods_id) >=
					goods_amount, ERROR_MALL_ACTIVITY_SINGLE_BUY_LIMIT);
			break;
	}
	default:
		break;
	}
	return 0;
}

int MallActivity::update_buy_record(Int64 role_id, int goods_id, int goods_amount)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	switch(this->mall_activity_detail_.__limit_type)
	{
	{
		case GameEnum::MALL_BUY_TOTAL_LIMIT:
			this->update_server_buy_record(goods_id, goods_amount);
			break;
	}
	{
		case GameEnum::MALL_BUY_SINGLE_LIMIT:
			this->update_single_buy_record(role_id, goods_id, goods_amount);
			break;
	}
	{
		case GameEnum::MALL_BUY_BOTH_LIMIT:
			this->update_server_buy_record(goods_id, goods_amount);
			this->update_single_buy_record(role_id, goods_id, goods_amount);
			break;
	}
	default:
		break;
	}
	return 0;
}

int MallActivity::update_server_buy_record(int goods_id, int goods_amount)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	JUDGE_RETURN(this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_TOTAL_LIMIT ||
			this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_BOTH_LIMIT, 0);

	if(this->mall_activity_detail_.__server_buy_map.count(goods_id) > 0)
		this->mall_activity_detail_.__server_buy_map[goods_id] += goods_amount;
	else
		this->mall_activity_detail_.__server_buy_map[goods_id] = goods_amount;

	this->mall_activity_detail_.__data_change = true;
	return 0;
}

int MallActivity::update_single_buy_record(Int64 role_id, int goods_id, int goods_amount)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	JUDGE_RETURN(this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_SINGLE_LIMIT ||
			this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_BOTH_LIMIT, 0);

	if(this->mall_activity_detail_.__player_record.count(role_id) == 0)
	{
		IntMap goods_map;
		goods_map[goods_id] = goods_amount;
		this->mall_activity_detail_.__player_record[role_id] = goods_map;
	}
	else
	{
		IntMap& goods_map = this->mall_activity_detail_.__player_record[role_id];
		if(goods_map.count(goods_id) > 0)
			goods_map[goods_id] += goods_amount;
		else
			goods_map[goods_id] = goods_amount;
	}

	this->mall_activity_detail_.__data_change = true;
	return 0;
}


int MallActivity::calc_server_buy_left_amount(int goods_id)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	JUDGE_RETURN(this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_TOTAL_LIMIT ||
			this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_BOTH_LIMIT, 0);

	//calc buy amount
	int buy_amount = 0;
	if(this->mall_activity_detail_.__server_buy_map.count(goods_id) > 0)
		buy_amount = this->mall_activity_detail_.__server_buy_map[goods_id];

	int left_amount = this->mall_activity_detail_.__server_limit_amount - buy_amount;
	left_amount = left_amount > 0 ? left_amount : 0;
	return left_amount;
}

int MallActivity::calc_single_buy_left_amount(Int64 role_id, int goods_id)
{
	JUDGE_RETURN(this->is_in_mall_activity_period(), 0);
	JUDGE_RETURN(this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_SINGLE_LIMIT ||
			this->mall_activity_detail_.__limit_type == GameEnum::MALL_BUY_BOTH_LIMIT, 0);

	//calc buy amount
	int buy_amount = 0;
	{
		MallActivityDetail::MallBuyRecord::iterator it = this->mall_activity_detail_.__player_record.find(role_id);
		if(it != this->mall_activity_detail_.__player_record.end())
		{
			IntMap goods_set = it->second;
			if(goods_set.count(goods_id) > 0)
				buy_amount = goods_set[goods_id];
		}
	}
	int left_amount = this->mall_activity_detail_.__single_limit_amount - buy_amount;
	left_amount = left_amount > 0 ? left_amount : 0;
	return left_amount;
}

int MallActivity::calc_mall_visible_left_time(void)
{
	return 0;
}

int MallActivity::calc_mall_activity_left_time(void)
{
	return 0;
}

bool MallActivity::is_in_mall_activity_period(void)
{
	return false;
}

