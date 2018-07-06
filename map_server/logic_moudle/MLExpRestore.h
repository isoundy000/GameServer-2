/*
 * MLExpRestore.h
 *
 *  Created on: 2014-2-17
 *      Author: lijin
 */

#ifndef MLEXPRESTORE_H_
#define MLEXPRESTORE_H_

#include "MLPacker.h"

class MLExpRestore : virtual public MLPacker
{
public:
	MLExpRestore();
	virtual ~MLExpRestore();
	void reset();

	int check_restore_info(RewardInfo& reward_info_total, int act_type, int& exp, int& reputation, int& honour,
			int& exploit, int& league_contri, int& reiki, int& gold, int& bind_gold,
			IntVec& free_list, IntVec& money_list, int check_type = 0, int get_type = 1, int times = 1);
// check_type 0查看  1 查看并获得 get_type 获得类型 1免费 2 元宝

	int insert_restore_good(int& exp, int& reputation, int& honour, int& exploit,
			int& league_contri, int& reiki, int& gold, int& bind_gold, RewardInfo& reward_info_total);

	int fetch_restore_goods_info(Message* msg);
	int fetch_diff_day(const Time_Value &check_day, const Time_Value &nowtime);
	void reinit_restore_exp_map(void);
	int check_and_update_exp_restore_info();
	int sync_transfer_exp_restore(int scene_id);
	int read_transfer_exp_restore(Message* msg);

	int fetch_exp_restore_info();
	int exp_restore_single(Message* msg);
	int exp_restore_all(Message* msg);
	int exp_restore_event_done(Message *msg);
	int sync_storage_stage_info(Message *msg);
	int refresh_restore_info(int event_id, int value, int times);

	int exp_restore_lvl_up(int level);
	void exp_restore_vip_rec(int vip_type, int expired_time);
	int exp_restore_notify_reflash();

	int restore_jump_next_day(const int jump_days);
	int exp_restore_max_day();
	int set_notify_msg_exp_restore_info(Message *msg);
	int storage_finish_times_reduce(int storage_id, const Time_Value &timev);

	ExpRestoreDetail &exp_restore_detail();
	StorageRecordSet &storage_record_set();
	LongMap &stamp_level_map();
	LongMap &stamp_vip_map();

private:
	int count_exp_storage_num(const Time_Value &timev);
	int mark_storage_record_invalid(int storage_id, const Time_Value &timesv);
	int exp_restore_cost_money(int exp_value);
	int cal_exp_storage(int storage_id, const Time_Value &timev, int &count, int &exp, int &money);
	int storage_exp_value(int storage_id, int64_t time_stamp=0);// 每次参与对应的经验
	int storage_max_finish_times(int storage_id, int vip_type, int64_t time_stamp=0);// 活动每天可参加次数
	int vip_extra_finish_times(int storage_id, int vip_type);

	int get_storage_id_set(IntVec &id_set);

	int level_with_timestamp(int64_t timestamp);
	int vip_type_with_timestamp(int64_t timestamp);

	void cal_restore_record(const Time_Value &timev);
	void cal_stamp_level_map(const Time_Value &timev);
	void cal_stamp_vip_map(const Time_Value &timev);

	int trans_storage_id(int event_id);
	int cal_storage_stage_value(int event_id, int stage);
	int upsert_storage_stage_info(const Time_Value &timev, int event_id, int stage);
	int query_storage_stage_info(const Time_Value &timev, int storage_id);


	int insert_exp_storage_rec(StorageRecord &record);
	void test_output_storage_info();

private:
	ExpRestoreDetail exp_restore_detail_;
};

#endif /* MLEXPRESTORE_H_ */
