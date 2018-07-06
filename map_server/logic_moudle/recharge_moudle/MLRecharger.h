/*
 * MLRecharger.h
 *
 *  Created on: Aug 25, 2014
 *      Author: louis
 */

#ifndef MLRECHARGER_H_
#define MLRECHARGER_H_

#include "MLPacker.h"

class MLRecharger  : virtual public MLPacker
{
public:
	MLRecharger();
	virtual ~MLRecharger();
	virtual int total_recharge_gold();

	int reset();
	int player_recharge_timeup(void);
	int after_load_recharge_order(Transaction* trans);

	int recharger_check_notify(void);
	int notify_recharge_awards(void);
	int gain_multiple_money(int gold, const Json::Value &awards_json);
	int record_recharge_awards(int gold, int serial);

	int update_recharge_rebate(int gold = 0);

	int fetch_recharge_awards_info(void);
	int notify_recharge_activity(void);
	int fetch_recharge_awards(void);
	int fetch_recharge_info(Message* msg);

	int sync_transfer_recharge_rewards(int scene_id);
	int read_transfer_recharge_rewards(Message *msg);

	RechargeDetail &recharge_detail(void);
	bool is_on_recharge_activity(void);

	int use_recharge_item(const Json::Value& effect, int num);
	void play800_income_log(mongo::BSONObj& res,int recharge_ret);

private:
	int request_load_recharge_order(void);
	int request_handle_recharge_order(mongo::BSONObj& res, bool is_use_item=false);
	int request_sync_recharge_info_to_other_role(const std::string& account,
			const int gold, const Int64 exclusive_id = 0);
	int request_update_recharge_order_flag(const int order_id);
	void first_recharge_annouce_world(ItemObjVec &shout_item_set);

	int refresh_order_record_id(void);

public:
	RechargeDetail recharge_detail_;
};

#endif /* MLRECHARGER_H_ */



