/*
 * ActivityTipsSystem.h
 *
 *  Created on: 2014-1-13
 *      Author: root
 */

#ifndef ACTIVITYTIPSSYSTEM_H_
#define ACTIVITYTIPSSYSTEM_H_

#include "LogicStruct.h"

typedef std::map<int, ActivityTipsInfo> ActivityTipsInfoMap;

class ActivityTipsSystem
{
public:
	ActivityTipsSystem(void);
	virtual ~ActivityTipsSystem(void);

	ActivityTipsInfoMap &tips_info_map(void);
    ActivityTipsInfo *find_activity_tips(int activity_id);

    int init(void);
    int init_activity_map_from_cfg(void);
    int upate_activity_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round);
    int update_daily_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round);
    int update_weekly_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round);
	int check_and_broadcast_activity_state(void);

	void refresh_activity_info(ActivityTipsInfo *act_info);
	void refresh_activity_info(ActivityTipsInfo *act_info, ActivityTimeInfo& time_info);

	int load_tips_system(void);
	int save_tips_system(void);

	int syn_activity_tips_envent_no_start(int event_id);// 接口:活动未开启
	int syn_activity_tips_envent_ahead(int event_id, int ahead_time);// 接口:活动提前提醒 ahead_time 倒计时
	int syn_activity_tips_envent_start(int event_id, int end_time);// 接口:活动开始 end_time 默认活动结束时间
	int syn_activity_tips_envent_stop(int event_id, int left_time = 0);// 接口:活动提前结束 left_time=1表示结束后不再有活动 客户端会显示“已结束”
	int syn_activity_tips_envent_next(int event_id, int ahead_time);

	int scene_sync_activity_state(Message *msg);// 活动提前结束
	int sync_activity_buff_state(Message *msg); //活动额外BUFF
	int remove_activity_buff(int activity_id);
	int validate_open_days_activity(ActivityTipsInfo *act_tips_info);
	int notify_activity_info(ActivityTipsInfo *act_tips_info);//通知所有玩家活动信息

	int trvl_sync_announce(Message *msg);	//跨服广播处理

	int sync_single_player_join_activity(int activity_id, int64_t role_id);
	int sync_single_player_join_activity(Message *msg);

	int sync_batch_player_join_activity(int activity_id, const LongVec &role_id_set);
	int sync_batch_player_join_activity(Message *msg);

	int handle_tips_timeout(void);
	int handle_tips_midnight_timeout();

	int update_activity_tick(ActivityTipsInfo *act_tips_info);
	int check_monster_tower_start();

	int update_activity_buff(Int64 role_id);
private:
	void handle_activity_ahead(const ActivityTipsInfo &tips_info);
	void handle_activity_start(const ActivityTipsInfo &tips_info);
	void handle_activity_next(const ActivityTipsInfo &tips_info);
	void handle_activity_end(const ActivityTipsInfo &tips_info);

private:
	ActivityTipsInfoMap tips_info_map_;
    bool cache_update_;
};

typedef Singleton<ActivityTipsSystem> ActivityTipsSystemSingle;
#define ACTIVITY_TIPS_SYSTEM ActivityTipsSystemSingle::instance()

#endif /* ACTIVITYTIPSSYSTEM_H_ */
