/*
 * TaskTrialImp.cpp
 *
 * Created on: 2015-07-23 14:36
 *     Author: lyz
 */

#include "TaskTrialImp.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"
#include "MapTaskStruct.h"

TaskTrialImp::~TaskTrialImp(void)
{ /*NULL*/ }

void TaskTrialImp::recycle_self(void)
{
    MAP_MONITOR->task_trial_imp_pool()->push(this);
}

int TaskTrialImp::process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);

 	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;
		if (task_cond->is_lvl_monster() == false &&
                task_cond->is_kill_boss() == false)
		{
			if (task_cond->__current_value < task_cond->__final_value)
            {
				is_all_pass = false;
                break;
            }
			continue;
		}

        const Json::Value &condition_json = task_json["exec"]["condition"][task_cond->__cond_index];
        if (task_cond->is_lvl_monster())
        {
            const Json::Value &monster_json = CONFIG_INSTANCE->monster(sort);
            if (monster_json["lvl"].asInt() < condition_json["level_above"].asInt())
            {
    			if (task_cond->__current_value < task_cond->__final_value)
                {
    				is_all_pass = false;
    				continue;
                }
            }
        }
        if (task_cond->is_kill_boss())
        {
            if (GameCommon::is_boss(sort) == false)
            {
            	if (task_cond->__current_value < task_cond->__final_value)
            	{
            		is_all_pass = false;
            	}
                continue;
            }
        }

        const Json::Value &scene_list_json = condition_json["scene_list"];
        for (uint i = 0; i < scene_list_json.size(); ++i)
        {
            if (this->tasker()->task_player()->scene_id() == scene_list_json[i].asInt())
            {
                task_cond->__current_value += num;
                break;
            }
        }

		if (task_cond->__current_value >= task_cond->__final_value)
		{
			task_cond->__current_value = task_cond->__final_value;
		}
		else
		{
			is_all_pass = false;
			this->tasker()->notify_task_update(task_info);
		}
    }

    if (is_all_pass == true)
        this->process_finish(task_info);
    return 0;
}

int TaskTrialImp::process_listen_collect_item(TaskInfo *task_info, const int id, const int num, const int bind)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);

 	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;
		if (task_cond->is_collect_any() == false)
		{
			if (task_cond->__current_value < task_cond->__final_value)
            {
				is_all_pass = false;
                break;
            }
			continue;
		}

        const Json::Value &condition_json = task_json["exec"]["condition"][task_cond->__cond_index];
        const Json::Value &scene_list_json = condition_json["scene_list"];
        for (uint i = 0; i < scene_list_json.size(); ++i)
        {
            if (this->tasker()->task_player()->scene_id() == scene_list_json[i].asInt())
            {
                task_cond->__current_value += num;
                break;
            }
        }

		if (task_cond->__current_value >= task_cond->__final_value)
		{
			task_cond->__current_value = task_cond->__final_value;
		}
		else
		{
			is_all_pass = false;
			this->tasker()->notify_task_update(task_info);
		}
    }

    if (is_all_pass == true)
        this->process_finish(task_info);
    return 0;
}

int TaskTrialImp::process_finish(TaskInfo *task_info)
{
    int ret = TaskRoutineImp::process_finish(task_info);
    return ret;
}

int TaskTrialImp::process_submit(TaskInfo *task_info, const int is_double)
{
//	MapLogicTasker *tasker = this->tasker();
//
//    JUDGE_RETURN(tasker->left_trial_task_times() > 0, ERROR_TASK_FINISH_TIMES_LIMIT);
//
//    int rate = 100;
//    if (task_info->__fast_finish_rate > 0)
//        rate += task_info->__fast_finish_rate;
//
//    // 奖励倍数
//    int ret = this->process_task_level_reward(task_info, rate);
//    JUDGE_RETURN(ret == 0, ret);
//
//    tasker->refresh_trial_task(false);
//    tasker->inc_used_trial_times();
//
//    tasker->trial_task_set().erase(task_info->__task_id);
//
//    this->remove_task_listen(task_info);
//    this->remove_task_info(task_info, true);
//
//    MapLogicPlayer *player = tasker->task_player();
//    player->monitor()->inner_notify_event_for_liveness(player, GameEnum::DL_COND_ID_TRIAL_TASK, LivenessFinishType::FINISH, 1);

    return 0;
}

int TaskTrialImp::process_abandon(TaskInfo *task_info)
{
	return TaskImplement::process_abandon(task_info);
}
