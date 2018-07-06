/*
 * LogicActivityReminder.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: root
 */

#include "ProtoDefine.h"
#include "LogicActivityReminder.h"
#include "ActivityTipsSystem.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"


LogicActivityReminder::LogicActivityReminder()
{
	// TODO Auto-generated constructor stub
}

LogicActivityReminder::~LogicActivityReminder()
{
	// TODO Auto-generated destructor stub
}

void LogicActivityReminder::reset()
{
	this->activity_info_.clear();
}

void LogicActivityReminder::refresh_activity_record(ActivityTipsInfo *act_tips_info)
{
    ActivityRec *activity_rec = NULL;

    ActivityInfoMap::iterator iter = this->activity_info_.find(act_tips_info->__activity_id);
    if (iter == this->activity_info_.end())
    {
        activity_rec = &(this->activity_info_[act_tips_info->__activity_id]);
        activity_rec->__activity_id = act_tips_info->__activity_id;
        activity_rec->__start_tick = act_tips_info->__start_time;
        activity_rec->__end_tick = act_tips_info->__end_time;
    }
    else
    {
        activity_rec = &(iter->second);
    }
    
    if (activity_rec->__update_tick != act_tips_info->__update_tick)
    {
    	activity_rec->__update_tick = act_tips_info->__update_tick;
        activity_rec->__is_touch = false;
    }
}

ActivityRec *LogicActivityReminder::find_activity_record(const int activity_id)
{
    ActivityInfoMap::iterator iter = this->activity_info_.find(activity_id);
    if (iter != this->activity_info_.end())
    {
        return &(iter->second);
    }
    return NULL;
}

int LogicActivityReminder::validate_activity_condition(const Json::Value& activity_json)
{
	return 0;
}

int LogicActivityReminder::need_notify_activity_tips(ActivityTipsInfo *act_tips)
{
	if (this->role_level() < act_tips->__limit_level)
	{
		return false;
	}

	return true;
}

int LogicActivityReminder::check_and_notify_activity_tips(ActivityTipsInfo *act_tips)
{
	this->refresh_activity_record(act_tips);
	JUDGE_RETURN(this->need_notify_activity_tips(act_tips) == true, -1);

	ActivityRec* activity_record = this->find_activity_record(act_tips->__activity_id);
	JUDGE_RETURN(activity_record->__is_touch == false, -1);
	activity_record->__is_touch = true;

    Proto81401201 respond;
    ProtoActivityInfo *proto_activity = respond.add_activity_info();
    act_tips->serialize(proto_activity);

    int acti_state = this->fetch_acti_state(act_tips);
    proto_activity->set_activity_state(acti_state);

    return this->respond_to_client(ACTIVE_NOTIFY_ACTIVITY_INFO, &respond);
}

int LogicActivityReminder::fetch_all_tips_info(void)
{
	ActivityTipsInfoMap &tips_info_map = ACTIVITY_TIPS_SYSTEM->tips_info_map();

    typedef std::vector<ActivityTipsInfo *> ActSortVec;
    ActSortVec acti_sort_vc;
	acti_sort_vc.reserve(tips_info_map.size());

	for(ActivityTipsInfoMap::iterator iter = tips_info_map.begin(); iter != tips_info_map.end(); ++ iter)
	{
		ActivityTipsInfo &tips_info = iter->second;
		acti_sort_vc.push_back(&tips_info);
	}

//	std::sort(acti_sort_vc.begin(), acti_sort_vc.end(), ActiTimeCmp);

	Proto50100201 activity_info;
	for(ActSortVec::iterator iter = acti_sort_vc.begin(); iter != acti_sort_vc.end(); ++iter)
	{
		ActivityTipsInfo *tips_info = *iter;

        ProtoActivityInfo *proto_activity = activity_info.add_activity_info_list();
        tips_info->serialize(proto_activity);

        int acti_state = this->fetch_acti_state(tips_info);
        proto_activity->set_activity_state(acti_state);
	}

	FINER_PROCESS_RETURN(RETURN_ACTIVITY_TIPS_FETCH_ALL, &activity_info);
}

int LogicActivityReminder::touch_tips_icon(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto10100203*, request, -1);
//
//	int activity_id = request->activity_id();
//    ActivityTipsInfo *act_tips_info = ACTIVITY_TIPS_SYSTEM->find_activity_tips(activity_id);
//    CONDITION_NOTIFY_RETURN(act_tips_info != NULL, RETURN_ACTIVITY_TIPS_TOUCH, ERROR_ACTIVITY_NO_OPEN);
//
//    this->refresh_activity_record(act_tips_info);
//    ActivityRec *act_rec = this->find_activity_record(activity_id);
//
//    act_rec->__is_touch = 1;
//
//	Proto50100203 respond;
//	respond.set_activity_id(activity_id);
//	FINER_PROCESS_RETURN(RETURN_ACTIVITY_TIPS_TOUCH, &respond);
	return 0;
}

int LogicActivityReminder::check_activity_reminder()
{
	ActivityTipsInfoMap &tips_info_map = ACTIVITY_TIPS_SYSTEM->tips_info_map();
	for(ActivityTipsInfoMap::iterator iter = tips_info_map.begin();
			iter != tips_info_map.end(); ++ iter)
	{
		ActivityTipsInfo *act_tips_info = &(iter->second);
        this->check_and_notify_activity_tips(act_tips_info);
	}

	return 0;
}

int LogicActivityReminder::on_join_activity(const int activity_id)
{
//    ActivityTipsInfo *act_tips_info = ACTIVITY_TIPS_SYSTEM->find_activity_tips(activity_id);
//    JUDGE_RETURN(act_tips_info != NULL, -1);
//
//    this->refresh_activity_record(act_tips_info);
//    ActivityRec *activity_record = this->find_activity_record(activity_id);
//    JUDGE_RETURN(activity_record != NULL, -1);
//
//    const Json::Value &activity_json = CONFIG_INSTANCE->common_activity(activity_id);
//
//    if (act_tips_info->__acti_state == GameEnum::ACTIVITY_STATE_START || activity_json["fullday"].asInt() == 1)
//    {
//        ++activity_record->__finish_count;
//        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_ACTIVITY_TIPS);
//    }

	return 0;
}

ActivityInfoMap& LogicActivityReminder::activity_info()
{
	return this->activity_info_;
}

int LogicActivityReminder::fetch_acti_state(ActivityTipsInfo *tips_info)
{
	return tips_info->__acti_state;
}


