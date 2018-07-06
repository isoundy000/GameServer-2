/*
 * MLTinyer.h
 *
 *  Created on: Nov 22, 2013
 *      Author: peizhibi
 */

#ifndef MLTINYER_H_
#define MLTINYER_H_

#include "MLPacker.h"

class MLTinyer : virtual public MLPacker
{
public:
	MLTinyer();
	virtual ~MLTinyer();

	void reset_found();
	void reset_everyday();
	void notify_client_tick();

	TinyDetail* tiny_detail();

	/*
	 * none
	 * */
	int check_and_level_up_player(const Json::Value &effect);
    int process_item_transfer(const Json::Value &effect);
    int process_item_transfer_random(const Json::Value &effect);
	int check_and_transfer_player(const Json::Value& effect,
			PackageItem* pack_item);
    int check_and_transfer_random_player(const Json::Value &effect, PackageItem *pack_item);
	int request_notice_add_goods(Message* msg);

    /*
     * about market
     * */
    int request_add_onsell_goods(Message* msg);
    int market_buy_goods(Message* msg);

    /*
     * about arena
     * */
    int request_buy_arena_times(Message* msg);
    int request_buy_clear_cool(Message* msg);

    /*
     * about guide
     * */
    int fetch_guide_info();
    int save_guide_info(Message* msg);

	/*
	 * transfer
	 */
	int sync_transfer_tiny(int scene_id);
	int read_transfer_tiny(Message* msg);

	int get_total_recharge();
	Int64 get_last_recharge_time();
	void set_total_recharge(int gold);
	void set_last_recharge_time(Int64 time);

	int lucky_table_cost(Message *msg);

private:
    TinyDetail tiny_detail_;	//基金信息
    int daily_total_gold_;	//当天累计充值元宝数（每天0点重置）
    Int64 last_recharge_time_;	//最近一次充值时间
};

#endif /* MLTINYER_H_ */
