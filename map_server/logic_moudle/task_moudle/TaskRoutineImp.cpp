/*
 * TaskRoutineImp.cpp
 *
 * Created on: 2014-09-02 10:07
 *     Author: lyz
 */

#include "GameHeader.h"
#include "TaskRoutineImp.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"
#include "MapTaskStruct.h"

TaskRoutineImp::TaskRoutineImp(void)
{ /*NULL*/ }

TaskRoutineImp::~TaskRoutineImp(void)
{ /*NULL*/ }

void TaskRoutineImp::reset(void)
{
    TaskImplement::reset();
}

void TaskRoutineImp::recycle_self(void)
{
    MAP_MONITOR->task_routine_imp_pool()->push(this);
}

int TaskRoutineImp::process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num)
{
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;
		if (task_cond->is_kill_monster() == false || (task_cond->__cond_id != sort && task_cond->__kill_type != 2)
				|| (task_cond->__kill_type == 2 && this->tasker()->fetch_routine_monster_id(
						task_info->__game_type, task_cond->__range_level, sort) == 0))
		{
			if (task_cond->__current_value < task_cond->__final_value)
				is_all_pass = false;
			continue;
		}

		task_cond->__current_value += num;
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

int TaskRoutineImp::process_finish(TaskInfo *task_info)
{
	MapLogicPlayer *player = this->tasker()->task_player();

	int type = task_info->__game_type;

    this->remove_task_listen(task_info);
    task_info->set_task_status(TaskInfo::TASK_STATUS_FINISH);

    player->record_serial(task_info->__task_id, TASK_SERIAL_FINISH);
    player->check_current_version_open_ui(task_info->__task_id, GameEnum::CHECK_UIT_FINISH_TASK);
    player->cache_tick().update_cache(MapLogicPlayer::CACHE_TASK, true);

    this->new_process_task_reward(task_info);

    this->remove_task_listen(task_info);
    this->remove_task_info(task_info, true);

    if (type == TaskInfo::TASK_GTYPE_ROUTINE
    	|| type == TaskInfo::TASK_GTYPE_OFFER_ROUTINE
    	|| type == TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE)
    {
    	player->generate_next_routine_task(type);
    	player->update_sword_pool_info(1, type);
    	player->update_cornucopia_task_info(1, type);
    }

    return 0;
}

int TaskRoutineImp::process_submit(TaskInfo *task_info, const int is_double)
{
    MapLogicPlayer *player = this->tasker()->task_player();
    int rate = 100;
//    if (is_double == 1)
//    {
//    	int cost_gold = CONFIG_INSTANCE->task_setting()["routine_double_award_money"].asInt();
//    	JUDGE_RETURN(cost_gold > 0, ERROR_CLIENT_OPERATE);
//
//    	Money cost(cost_gold);
//    	JUDGE_RETURN(player->validate_money(cost), ERROR_PACKAGE_GOLD_AMOUNT);
//    	player->pack_money_sub(cost, SerialObj(SUB_MONEY_TASK));
//
//    	rate = 200;
//    }

    // 奖励倍数
    int ret = this->process_task_level_reward(task_info,  rate);
    JUDGE_RETURN(ret == 0, ret);

//    player->monitor()->inner_notify_event_for_liveness(player, GameEnum::DL_COND_ID_RING_TASK, LivenessFinishType::FINISH, 1);

    this->remove_task_listen(task_info);
    this->remove_task_info(task_info, true);

   player->generate_next_routine_task();

   return 0;
}


int TaskRoutineImp::new_process_task_reward(TaskInfo *task_info)
{
    MapLogicPlayer *player = this->tasker()->task_player();
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
    int award_id = task_json["award"]["award_id"].asInt();
    int award_exp;
    int ext_award;
    int ext_exp;
    int serial = 0;
    switch(task_info->__game_type)
    {
    	case TaskInfo::TASK_GTYPE_ROUTINE:
    	{
    		award_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["daliy_routine_exp"].asInt();
    		ext_award = CONFIG_INSTANCE->role_level(0, player->role_level())["routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["routine_ext_exp"].asInt();
    		serial = ADD_FROM_DAILY_ROUTINE_TASK;
    		break;
    	}
    	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
    	{
    		award_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["offer_routine_exp"].asInt();
    		ext_award = CONFIG_INSTANCE->role_level(0, player->role_level())["offer_routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["offer_routine_ext_exp"].asInt();
    		serial = ADD_FROM_OFFER_ROUTINE_TASK;
    		break;
    	}
    	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
    	{
    		award_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["league_routine_exp"].asInt();
    		ext_award = CONFIG_INSTANCE->role_level(0, player->role_level())["league_routine_ext_award"].asInt();
    		ext_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["league_routine_ext_exp"].asInt();
    		serial = ADD_FROM_LEAGUE_ROUTINE_TASK;
    		break;
    	}
    }

    if (this->tasker()->routine_task_index(task_info->__game_type) + 1 >=
    		this->tasker()->routine_total_num(task_info->__game_type))
    {
    	player->add_reward(ext_award, SerialObj(serial, task_info->__task_id));
    	player->request_add_exp(ext_exp, SerialObj(EXP_TASK, task_info->__task_id));
    }
//    int award_exp = 100; //test
    Money award_money; //test

    player->add_reward(award_id, SerialObj(serial, task_info->__task_id));
    player->pack_money_add(award_money, SerialObj(ADD_FROM_TASK_MONEY, task_info->__task_id));
    player->request_add_exp(award_exp, SerialObj(EXP_TASK, task_info->__task_id));

    return 0;
}

int TaskRoutineImp::process_task_level_reward(TaskInfo *task_info, const int rate)
{
    MapLogicPlayer *player = this->tasker()->task_player();
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
    const Json::Value &level_award_json = task_json["level_award"];
    const Json::Value &level_json = level_award_json["level"];
    int level = player->role_level() % 1000, level_index = 0;
    for (level_index = 0; level_index < int(level_json.size()); ++level_index)
    {
        if (level_json[level_index].asInt() == level)
            break;
    }
    if (level_index >= int(level_json.size()))
        level_index = level_json.size() - 1;
    JUDGE_RETURN(level_index >= 0, ERROR_CONFIG_ERROR);

    // 双倍奖励
    double real_rate = 1.0;
    if (rate > 0)
    	real_rate = rate / 100.0;

    // 星级奖励
    double star_rate = player->calc_star_to_rate(task_info->__task_star, task_json);
    if (star_rate > 0.000001)
        real_rate *= star_rate;

    int award_exp = 0;
    Money award_money;
    std::map<int, ItemObj> item_map;
    player->calc_routine_task_award(&award_exp, award_money, item_map, task_json, real_rate);

    // 判断如果完成今天的所有环式任务，则有额外奖励
    if (player->is_last_routine_task() && task_info->is_routine_task())
    {
        player->calc_routine_extra_award(&award_exp, award_money, item_map);
    }

    if (item_map.size() > 0)
    {
    	JUDGE_RETURN(int(item_map.size()) <= player->pack_left_capacity(), ERROR_PACKAGE_NO_CAPACITY);
    }

    for (std::map<int, ItemObj>::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
    {
        ItemObj &item_obj = iter->second;
        player->pack_insert(SerialObj(ADD_RROM_TASK_ITEM), item_obj);
    }

    player->pack_money_add(award_money, SerialObj(ADD_FROM_TASK_MONEY, task_info->__task_id));
    player->request_add_exp(award_exp, SerialObj(EXP_TASK, task_info->__task_id));

    return 0;
}

