/*
 * MallActivity.h
 *
 *  Created on: 2013-12-23
 *      Author: louis
 */

#ifndef MALLACTIVITY_H_
#define MALLACTIVITY_H_

#include "PubStruct.h"

class MallActivity
{
public:
	MallActivity();
	virtual ~MallActivity();

	MallActivityDetail& mall_activity_detail(void);

	int mall_activity_refresh(void);
	int modify_buy_mall_activity_goods(Int64 role_id, int shop_type, int goods_id, int goods_amount);
	int update_buy_record(Int64 role_id,int goods_id, int goods_amount);

	int calc_mall_visible_left_time(void);
	int calc_mall_activity_left_time(void);
	bool is_in_mall_activity_period(void);

private:
	int calc_server_buy_left_amount(int goods_id);
	int calc_single_buy_left_amount(Int64 role_id, const int goods_id);

	int update_server_buy_record(int goods_id, int goods_amount);
	int update_single_buy_record(Int64 role_id, int goods_id, int goods_amount);

private:
	MallActivityDetail mall_activity_detail_;
};

#endif /* MALLACTIVITY_H_ */
