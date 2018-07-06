/*
 * ActivityTipsSystem.cpp
 *
 *  Created on: 2014-1-13
 *      Author: root
 */

#include "ActivityTipsSystem.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "MMOActivityTipsSystem.h"
#include "PoolMonitor.h"
#include "MongoDataMap.h"

ActivityTipsSystem::ActivityTipsSystem(void)
{
	// TODO Auto-generated constructor stub
	this->cache_update_ = false;
}

ActivityTipsSystem::~ActivityTipsSystem(void)
{
	// TODO Auto-generated destructor stub
}

ActivityTipsInfoMap &ActivityTipsSystem::tips_info_map(void)
{
	return this->tips_info_map_;
}

ActivityTipsInfo *ActivityTipsSystem::find_activity_tips(int activity_id)
{
    ActivityTipsInfoMap::iterator iter = this->tips_info_map_.find(activity_id);
    if (iter != this->tips_info_map_.end())
    {
        return &(iter->second);
    }
    else
    {
    	return NULL;
    }
}

int ActivityTipsSystem::init(void)
{
    this->load_tips_system();
    this->init_activity_map_from_cfg();
    MSG_USER("ActivityTipsSystem start...");
    return 0;
}

int ActivityTipsSystem::init_activity_map_from_cfg(void)
{
	const Json::Value &activity_list_json = CONFIG_INSTANCE->common_activity_json();
    for (Json::Value::const_iterator json_iter = activity_list_json.begin();
            json_iter != activity_list_json.end(); ++json_iter)
    {
		IntPair pair = GameCommon::to_int_number(json_iter.key().asString());
		JUDGE_CONTINUE(pair.first == true);

	    ActivityTipsInfo *act_info = NULL;
        ActivityTipsInfoMap::iterator act_iter = this->tips_info_map_.find(pair.second);
        if (act_iter == this->tips_info_map_.end())
        {
            act_info = &(this->tips_info_map_[pair.second]);
            act_info->__activity_id = pair.second;
        }
        else
        {
            act_info = &(act_iter->second);
        }

        const Json::Value &activity_json = *json_iter;
        act_info->__limit_level = activity_json["limit_level"].asInt();
        act_info->__time_dynamic = activity_json["time_dynamic"].asInt();
        act_info->__open_day = activity_json["open_day"].asInt();
        act_info->__day_check = activity_json["day_check"].asInt();
        act_info->__open_day_type = activity_json["open_day_type"].asInt();

        ActivityTimeInfo time_info(false);
        GameCommon::cal_activity_info(time_info, activity_json);

        act_info->__active_time = time_info.active_time_;
        act_info->time_info_ = time_info;

        this->refresh_activity_info(act_info, time_info);
        this->cache_update_ = true;
    }

    return 0;
}

int ActivityTipsSystem::upate_activity_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round)
{
    return -1;
}

int ActivityTipsSystem::update_daily_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round)
{
    return 0;
}

int ActivityTipsSystem::update_weekly_round_tick_list(ActivityTipsInfo *act_info, const Json::Value &activity_json, bool force_next_round)
{
    return 0;
}


int ActivityTipsSystem::check_and_broadcast_activity_state(void)
{
    return 0;
}

void ActivityTipsSystem::refresh_activity_info(ActivityTipsInfo *act_info)
{
	switch (act_info->__acti_state)
	{
	case GameEnum::ACTIVITY_STATE_END:
	{
		this->syn_activity_tips_envent_next(act_info->__activity_id, 0);
		break;
	}
	}
}

void ActivityTipsSystem::refresh_activity_info(ActivityTipsInfo *act_info, ActivityTimeInfo& time_info)
{
	act_info->__acti_state = time_info.client_state();
	switch (act_info->__acti_state)
	{
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->syn_activity_tips_envent_ahead(act_info->__activity_id,
				time_info.refresh_time_);
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->syn_activity_tips_envent_start(act_info->__activity_id,
				time_info.refresh_time_);
		break;
	}
	case GameEnum::ACTIVITY_STATE_END:
	{
		this->syn_activity_tips_envent_stop(act_info->__activity_id,
				time_info.fetch_after_end_left_time());
		break;
	}
	}
}

int ActivityTipsSystem::load_tips_system(void)
{
	return 0;
}

int ActivityTipsSystem::save_tips_system(void)
{
    JUDGE_RETURN(this->cache_update_ == true, 0);

    this->cache_update_ = false;
//
//    MongoDataMap *mongo_data = POOL_MONITOR->mongo_data_map_pool()->pop();
//	MMOActivityTipsSystem::save_activity_tips_system(this, mongo_data);
//
//    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_SYSTEM_TIPS, mongo_data) != 0)
//    {
//        POOL_MONITOR->mongo_data_map_pool()->push(mongo_data);
//    }
    return 0;
}

int ActivityTipsSystem::syn_activity_tips_envent_no_start(int event_id)
{
//    ActivityTipsInfo *act_tips_info =this->find_activity_tips(event_id);
//    JUDGE_RETURN(act_tips_info != NULL, -1);
//
//    act_tips_info->__acti_state = GameEnum::ACTIVITY_STATE_NO_START;
//    this->update_activity_tick(act_tips_info);
//
//    this->cache_update_ = true;
//    this->notify_activity_info(act_tips_info);
	return 0;
}

int ActivityTipsSystem::syn_activity_tips_envent_ahead(int event_id, int ahead_time)
{
    ActivityTipsInfo *act_tips_info = this->find_activity_tips(event_id);
    JUDGE_RETURN(act_tips_info != NULL, -1);

    Int64 now_time = ::time(0);
    act_tips_info->__acti_state = GameEnum::ACTIVITY_STATE_AHEAD;
	act_tips_info->__start_time = now_time + ahead_time;
	act_tips_info->__end_time = act_tips_info->__start_time
			+ act_tips_info->__active_time;
	act_tips_info->__update_tick = now_time;

    this->cache_update_ = true;
    this->notify_activity_info(act_tips_info);
	return 0;
}

int ActivityTipsSystem::syn_activity_tips_envent_start(int event_id, int end_time)
{
    ActivityTipsInfo *act_tips_info =this->find_activity_tips(event_id);
    JUDGE_RETURN(act_tips_info != NULL, -1);

    if (act_tips_info->__acti_state != GameEnum::ACTIVITY_STATE_START)
    {
        const Json::Value &activity_json = CONFIG_INSTANCE->common_activity(act_tips_info->__activity_id);
        int start_brocast_id = activity_json["start_brocast"].asInt();
        if (start_brocast_id > 0)
        {
            GameCommon::announce(start_brocast_id);
        }
    }

    Int64 now_time = ::time(0);
    act_tips_info->__acti_state = GameEnum::ACTIVITY_STATE_START;
	act_tips_info->__start_time = now_time;
	act_tips_info->__end_time = now_time + end_time;
	act_tips_info->__update_tick = now_time;

    this->cache_update_ = true;
    this->notify_activity_info(act_tips_info);

	return 0;
}

int ActivityTipsSystem::syn_activity_tips_envent_stop(int event_id, int left_time/*=0*/)
{
    ActivityTipsInfo *act_tips_info =this->find_activity_tips(event_id);
    JUDGE_RETURN(act_tips_info != NULL, -1);

    if (act_tips_info->__acti_state != GameEnum::ACTIVITY_STATE_END)
    {
        const Json::Value &activity_json = CONFIG_INSTANCE->common_activity(act_tips_info->__activity_id);
        int end_brocast_id = activity_json["end_brocast"].asInt();
        if (end_brocast_id > 0)
        {
            GameCommon::announce(end_brocast_id);
        }
    }

    Int64 now_time = ::time(0);
    act_tips_info->__acti_state = GameEnum::ACTIVITY_STATE_END;
    act_tips_info->__end_time = now_time;
    act_tips_info->__update_tick = now_time;
    act_tips_info->__reset_time = now_time + left_time;

    this->cache_update_ = true;
    this->notify_activity_info(act_tips_info);
	return 0;
}

int ActivityTipsSystem::syn_activity_tips_envent_next(int event_id, int ahead_time)
{
    ActivityTipsInfo *act_tips_info = this->find_activity_tips(event_id);
    JUDGE_RETURN(act_tips_info != NULL, -1);

    Int64 now_time = ::time(0);
    act_tips_info->__acti_state = GameEnum::ACTIVITY_STATE_NO_START;
    act_tips_info->__update_tick = now_time;

    this->notify_activity_info(act_tips_info);
	return 0;
}

int ActivityTipsSystem::remove_activity_buff(int activity_id)
{
	JUDGE_RETURN(activity_id == GameEnum::QUINTUPLE_ACTIVITY_ID, -1);

	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
//			JUDGE_CONTINUE(player != NULL);
		Proto30102004 buff_info;
		buff_info.set_status(true);
		LOGIC_MONITOR->dispatch_to_scene(player, &buff_info);
	}
	return 0;
}

int ActivityTipsSystem::sync_activity_buff_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30102001*, request, -1);
	JUDGE_RETURN(request->activity_id() == GameEnum::QUINTUPLE_ACTIVITY_ID, -1);

	if (request->status() && request->activity_id() == GameEnum::QUINTUPLE_ACTIVITY_ID)
	{
		LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
		for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
				iter != player_map.end(); ++iter)
		{
			LogicPlayer* player = iter->second;
//			JUDGE_CONTINUE(player != NULL);
			Proto30102002 buff_info;
			buff_info.set_status(true);
			buff_info.set_buff_time(request->buff_time());
			LOGIC_MONITOR->dispatch_to_scene(player, &buff_info);
		}
	}
	return 0;
}

int ActivityTipsSystem::scene_sync_activity_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100601*, request, -1);

	int activity_id = request->activity_id();
	int activity_state = request->activity_state();

    ActivityTipsInfo *act_tips_info = this->find_activity_tips(activity_id);
    JUDGE_RETURN(this->validate_open_days_activity(act_tips_info) == true, -1);

    act_tips_info->__sub_value = request->sub_value();
	switch(activity_state)
	{
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		return this->syn_activity_tips_envent_ahead(activity_id, request->ahead_time());
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		return this->syn_activity_tips_envent_start(activity_id, request->end_time());
	}
	case GameEnum::ACTIVITY_STATE_END:
	{
		return this->syn_activity_tips_envent_stop(activity_id, request->left_times());
	}
	}

	return -1;
}

int ActivityTipsSystem::validate_open_days_activity(ActivityTipsInfo *act_tips_info)
{
	JUDGE_RETURN(act_tips_info != NULL, false);

	//没有限制
	if (act_tips_info->__open_day <= 0 && act_tips_info->__day_check <= 0)
	{
		return true;
	}

	//限制类型一：开服天数活动
	if (act_tips_info->__open_day > 0)
	{
#ifdef LOCAL_DEBUG
		return true;
#else
		return CONFIG_INSTANCE->open_server_days() >= act_tips_info->__open_day;
#endif
	}

	//限制类型二:几天后检测
	int open_days = 0;
	if (act_tips_info->__open_day_type == 1)
	{
		open_days = CONFIG_INSTANCE->client_open_days();
	}
	else
	{
		open_days = CONFIG_INSTANCE->open_server_days();
	}

	if (open_days < act_tips_info->__day_check)
	{
		return true;
	}
	else
	{
#ifdef LOCAL_DEBUG
		return true;
#else
		return act_tips_info->time_info_.today_activity();
#endif
	}
}

int ActivityTipsSystem::notify_activity_info(ActivityTipsInfo *act_tips_info)
{
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
        player->check_and_notify_activity_tips(act_tips_info);
	}

	return 0;
}

int ActivityTipsSystem::trvl_sync_announce(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100607*, request, -1);

	Proto30200123 brocast_info;
	brocast_info.ParseFromString(request->brocast_info());
	return LOGIC_MONITOR->dispatch_to_chat(SCENE_CHAT, &brocast_info);
}

int ActivityTipsSystem::handle_tips_timeout(void)
{
	this->check_and_broadcast_activity_state();
	this->save_tips_system();
	this->check_monster_tower_start();
    return 0;
}

int ActivityTipsSystem::handle_tips_midnight_timeout()
{
	for (ActivityTipsInfoMap::iterator iter = this->tips_info_map_.begin();
			iter != this->tips_info_map_.end(); ++iter)
	{
		this->refresh_activity_info(&iter->second);
	}

	return 0;
}

int ActivityTipsSystem::sync_single_player_join_activity(int activity_id, int64_t role_id)
{
	LogicPlayer* player = NULL;
	if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
	{
		player->on_join_activity(activity_id);
	}

	return 0;
}

int ActivityTipsSystem::sync_single_player_join_activity(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100602*, request, -1);
	int64_t role_id = request->role_id();
	int activity_id = request->activity_id();

	return this->sync_single_player_join_activity(activity_id, role_id);
}

int ActivityTipsSystem::sync_batch_player_join_activity(int activity_id, const LongVec &role_id_set)
{
	JUDGE_RETURN(role_id_set.size() > 0, -1);

	for(LongVec::const_iterator iter = role_id_set.begin(); iter != role_id_set.end(); ++iter)
	{
		int64_t role_id = *iter;
		LogicPlayer* player = NULL;
		if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
		{
			player->on_join_activity(activity_id);
		}
	}

	return 0;
}

int ActivityTipsSystem::sync_batch_player_join_activity(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100603*, request, -1);
	int activity_id = request->activity_id();

	LongVec role_id_set;
	for (int i = 0; i < request->role_id_size(); ++i)
	{
		role_id_set.push_back(request->role_id(i));
	}

	return sync_batch_player_join_activity(activity_id, role_id_set);
}

int ActivityTipsSystem::update_activity_buff(Int64 role_id)
{
	LogicPlayer* player = NULL;
	if(LOGIC_MONITOR->find_player(role_id, player) == 0 && player != NULL)
	{
		ActivityTipsInfo *activity_info = this->find_activity_tips(GameEnum::QUINTUPLE_ACTIVITY_ID);
		if (activity_info != NULL && activity_info->__acti_state == GameEnum::ACTIVITY_STATE_START)
		{
			Int64 now_time = ::time(0);
			Proto30102002 buff_info;
			buff_info.set_status(true);
			buff_info.set_buff_time(activity_info->__end_time - now_time);
			LOGIC_MONITOR->dispatch_to_scene(player, &buff_info);
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

int ActivityTipsSystem::update_activity_tick(ActivityTipsInfo *act_tips_info)
{
//	const Json::Value &activity_json = CONFIG_INSTANCE->common_activity(act_tips_info->__activity_id);
//	int start_time = 0, end_time = 0;
//	if (act_tips_info->__round_tick_list.size() < 2)
//	{
//		this->upate_activity_round_tick_list(act_tips_info, activity_json, true);
//	}
//	int nowtime_t = ::time(NULL);
//	while (act_tips_info->__round_tick_list.size() > 0)
//	{
//		start_time = act_tips_info->__round_tick_list.front();
//		act_tips_info->__round_tick_list.erase(act_tips_info->__round_tick_list.begin());
//		if (act_tips_info->__round_tick_list.size() <= 0)
//			break;
//		end_time = act_tips_info->__round_tick_list.front();
//		act_tips_info->__round_tick_list.erase(act_tips_info->__round_tick_list.begin());
//
//		if (nowtime_t < end_time)
//			break;
//		if(act_tips_info->__round_tick_list.size() > 0)
//			act_tips_info->__round_tick_list.erase(act_tips_info->__round_tick_list.begin());
//		if(act_tips_info->__round_tick_list.size() > 0)
//			act_tips_info->__round_tick_list.erase(act_tips_info->__round_tick_list.begin());
//		start_time = 0;
//		end_time = 0;
//	}
//	JUDGE_RETURN(start_time > 0 && end_time > 0 && nowtime_t  < end_time, 0);
//
//	if (activity_json.isMember("ahead_notify_time"))
//	{
//		act_tips_info->__ahead_time = start_time - activity_json["ahead_notify_time"].asInt();
//	}
//	act_tips_info->__start_time = start_time;
//	act_tips_info->__end_time = end_time;

	return 0;
}


int ActivityTipsSystem::check_monster_tower_start()
{
//	tm tm1;
//	time_t t1 = ::time(NULL);
//	localtime_r(&t1, &tm1);
//
//	const Json::Value &seven_script_json = CONFIG_INSTANCE->script_monster_tower();
//	int start_hour = seven_script_json["start_time"][0u].asInt();
//	int start_minu = seven_script_json["start_time"][1u].asInt();
//
//	if (tm1.tm_hour == start_hour && tm1.tm_min == start_minu)
//	{
//        BrocastParaVec para_vec;
//		LOGIC_MONITOR->announce_world(SHOUT_ALL_MONSTER_TOWER_START, para_vec);
//	}
	return 0;
}


void ActivityTipsSystem::handle_activity_ahead(const ActivityTipsInfo &tips_info)
{
//	 switch(tips_info.__activity_id)
//	 {
//	 case GameEnum::ACTIVITY_WILDRAID:
//	 {
//		break;
//	 }
//	 }
}

void ActivityTipsSystem::handle_activity_start(const ActivityTipsInfo &tips_info)
{
//	switch(tips_info.__activity_id)
//	{
//	case GameEnum::ACTIVITY_GONGCHENG:
//	{
//		BrocastParaVec para_vec;
//		LOGIC_MONITOR->announce_world(SHOUT_ALL_SM_ATTACK,para_vec);
//		break;
//	}
//	case GameEnum::ACTIVITY_WILDRAID:
//	{
//		break;
//	}
//	}
}

void ActivityTipsSystem::handle_activity_next(const ActivityTipsInfo &tips_info)
{
//	switch(tips_info.__activity_id)
//	{
//	case GameEnum::ACTIVITY_WILDRAID:
//	{
//		break;
//	}
//	}
}

void ActivityTipsSystem::handle_activity_end(const ActivityTipsInfo &tips_info)
{
//	switch(tips_info.__activity_id)
//	{
//	case GameEnum::ACTIVITY_WILDRAID:
//	{
//		LOGIC_MONITOR->rand_scene_.clear();
//		break;
//	}
//	case GameEnum::ACTIVITY_GONGCHENG:
//	{
//		BrocastParaVec para_vec;
//		LOGIC_MONITOR->announce_world(SHOUT_ALL_SM_ATTACK_END,para_vec);
//		break;
//	}
//	}
}

