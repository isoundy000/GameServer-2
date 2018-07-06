/*
 * LucktTableMonitor.h
 *
 *  Created on: Mar 23, 2016
 *      Author: zhangshaopeng
 */

#ifndef LOGIC_SERVER_LUCKYTABLE_MOUDLE_LUCKTTABLEMONITOR_H_
#define LOGIC_SERVER_LUCKYTABLE_MOUDLE_LUCKTTABLEMONITOR_H_

#include "LogicStruct.h"

typedef std::vector<ItemRate> RateSet;
class BackGameModify;

struct LuckyTableConfig
{
	Int64 activity_key;//活动id
	int open_state;//后台活动开关
	int time_state[2];//当前活动状态
	Int64 start_tick[2];//开始时间
	Int64 end_tick[2];//结束时间
	int base[2];//多少抽一次
	IntSet best_item;//需要广播道具
	int play_time[2];//保底功能次数
	IntVec dis_play_time[2];//反保底功能次数

	std::vector< RateSet > rewrad_table[2];//奖励库，0消费，1充值
	RateSet play_table[2];//保底功能库
	IntVec cost[2];//抽取时消耗到元宝
	IntMap reward_item_set[2];//奖励物品的位置索引,（客户端用）

	LuckyTableConfig();
	void reset();
	LuckyTableConfig& operator=(const LuckyTableConfig& rhs);
};

class LucktTableMonitor
{
public:
	LucktTableMonitor();
	void reset();
	void refresh_from_db(Int64 tick = 0);
public:
	Int64 fetch_left_time(int type);
	void register_role(Int64 role_id, int type);
	void insert_ltable_record(std::vector<BoxRecord>&,int);
	void check_and_reset(Int64 tick);
private:
	void notify_all_table_record(int);
	void notify_table_record(Int64 role_id,int type);
	void notify_all_activity_state(int state1,int state2);

public:
	const LuckyTableConfig& get_ltable_cfg();
	Int64 get_activity_key();
	int open_state();
	int fetch_item_index(int type,int item_id);
	int update_cfg(BackGameModify* game_modify);
private:
	int construct_item_from_json(const Json::Value&,RateSet&);
	void delete_record_list(int type,uint left);
private:
	LuckyTableConfig ltable_cfg_;
	LongSet view_list_[2];
	BoxRecordList record_list_[2];
};

typedef Singleton<LucktTableMonitor> LucktTableSingle;
#define LACKY_TABLE LucktTableSingle::instance()

#endif /* LOGIC_SERVER_LUCKYTABLE_MOUDLE_LUCKTTABLEMONITOR_H_ */
