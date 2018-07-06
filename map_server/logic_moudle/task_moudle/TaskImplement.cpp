/*
 * TaskImplement.cpp
 *
 * Created on: 2013-11-07 16:06
 *     Author: lyz
 */

#include "MapTaskStruct.h"
#include "TaskImplement.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "SerialRecord.h"

#include "GameConfig.h"
#include "ProtoDefine.h"

TaskImplement::TaskImplement(void) : tasker_(0)
{ /*NULL*/ }

TaskImplement::~TaskImplement(void)
{ /*NULL*/ }

void TaskImplement::reset(void)
{
    this->tasker_ = 0;
}

MapLogicTasker *TaskImplement::tasker(void)
{
    return this->tasker_;
}

void TaskImplement::set_tasker(MapLogicTasker *tasker)
{
    this->tasker_ = tasker;
}

int TaskImplement::process_accept(TaskInfo *task_info, const bool is_init, const bool is_notify)
{
    MapLogicPlayer* player = this->tasker()->task_player();
    task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTED);
    player->check_current_version_open_ui(task_info->__task_id, GameEnum::CHECK_UIT_ACCEPTED_TASK);


    if (task_info->is_logic_level_up())
	{
    	player->task_accepted_lvl_set().insert(task_info);
    	player->task_listen_lvl_up(player->role_level());
	}
    else if (task_info->is_logic_kill_monster() || task_info->is_logic_lvl_monster()
    		|| task_info->is_logic_kill_boss())
    {
    	player->task_accepted_monster_set().insert(task_info);
    }
    else if (task_info->is_logic_collect_item() || task_info->is_logic_collect_any())
    {
    	player->task_accepted_collect_set().insert(task_info);
    }
    else if (task_info->is_logic_attend())
    {
    	player->task_accepted_attend_set().insert(task_info);
    }
    else if (task_info->is_logic_scene())
    {
    	player->task_accepted_scene_set().insert(task_info);
    }
    else if (task_info->is_logic_npc_dialogue())
    {
    	task_info->set_task_status(TaskInfo::TASK_STATUS_FINISH);
    	player->check_current_version_open_ui(task_info->__task_id, GameEnum::CHECK_UIT_FINISH_TASK);
    }
    else if (task_info->is_logic_package())
    {
    	player->init_task_listen_item(task_info);
    	player->task_accepted_package_set().insert(task_info);
    }
    else if (task_info->is_logic_script_wave())
    {
    	player->start_script_wave_task_listen(player->scene_id());
    }
    else if (task_info->is_logic_branch())
    {
    	player->task_accepted_branch_set().insert(task_info);
    }

    if (is_notify == true)
    {
		if (is_init == false)
			player->notify_task_update(task_info);
		else
			player->notify_task_add(task_info);
    }

    if (task_info->is_logic_attend())
    {
    	player->refresh_daily_league_contri();
    }

    return 0;
}

int TaskImplement::process_abandon(TaskInfo *task_info)
{
	this->remove_task_listen(task_info);
	this->remove_task_info(task_info, true);

    return 0;
}

int TaskImplement::process_submit(TaskInfo *task_info, const int is_double)
{
    task_info->set_task_status(TaskInfo::TASK_STATUS_SUBMIT);

    MapLogicPlayer *player = this->tasker()->task_player();
    player->check_current_version_open_ui(task_info->__task_id,
    		GameEnum::CHECK_UIT_SUBMIT_TASK);

    const Json::Value &task_json = task_info->conf();
    this->new_process_task_reward(task_info, task_json["award_id"].asInt());

    int post_task_id = task_info->__post_task;
    player->notify_task_add(task_info);
    player->task_submited_once_set().insert(task_info->__task_id);

    if (task_info->__task_id == CONFIG_INSTANCE->const_set("new_main_task_id"))
    {
    	player->task_construct_branch(true);
    	player->new_task_construct_routine(true);
    	player->offer_task_construct_routine(true);
    	player->league_task_construct_routine(true);
    }

    this->remove_task_listen(task_info);
    this->remove_task_info(task_info, true);

    if (post_task_id <= 0)
    {
        post_task_id = task_json["postcondition"].asInt();
    }

    int i = 0;
    while (post_task_id > 0 && player->task_submited_once_set().find(post_task_id) != player->task_submited_once_set().end())
    {
    	if (++i > 3000)
    		break;
    	const Json::Value &post_task_json = CONFIG_INSTANCE->find_task(post_task_id);
    	post_task_id = post_task_json["postcondition"].asInt();
    }

    if (post_task_id > 0)
    {
    	player->set_latest_main_task(post_task_id);
    	const Json::Value &post_task_json = CONFIG_INSTANCE->find_task(post_task_id);
    	if (post_task_json != Json::Value::null)
    	{
    		if (post_task_json["before_accept"]["level"].isArray() && post_task_json["before_accept"]["level"].size() > 0)
			{
				if (player->role_detail().__level >= post_task_json["before_accept"]["level"][0u].asInt())
					player->insert_task(post_task_id, post_task_json);

		        int branch_type = post_task_json["exec"]["condition"][0u]["id"][0u].asInt();
		        //更新最新数据
		        player->accept_branch_request_info(branch_type);
			}
    	}
    }

    if (player->task_map().size() <= 0)
	{
		// 没有主线任务时显示最后一次任务后的相关提示
		Proto81400107 respond;
		respond.set_lastest_task_id(player->latest_main_task());
		player->respond_to_client(ACTIVE_LASTEST_MAIN_TASK, &respond);
	}

    return 0;
}

int TaskImplement::process_fast_finish(TaskInfo *task_info)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
    const Json::Value &fast_finish_json = task_json["exec"]["fast_finish"];
    int fast_flag = fast_finish_json["is_fast"].asInt();
    JUDGE_RETURN(fast_flag > 0, ERROR_CLIENT_OPERATE);

    MapLogicPlayer *player = this->tasker()->task_player();

    int item_id = 0, item_num = 0;
    Money cost;
    if (fast_finish_json.isMember("item"))
    {
        item_id = fast_finish_json["item"][0u].asInt();
        item_num = fast_finish_json["item"][1u].asInt();
    }
    if (fast_finish_json.isMember("money"))
    {
        cost.__gold = fast_finish_json["money"][0u].asInt();
        cost.__bind_gold = fast_finish_json["money"][1u].asInt();
        cost.__copper = fast_finish_json["money"][2u].asInt();
        cost.__bind_copper = fast_finish_json["money"][3u].asInt();

        if (player->pack_detail().__money.__bind_gold < cost.__bind_gold)
        {
            int bind_to_gold = cost.__bind_gold - player->pack_detail().__money.__bind_gold;
            cost.__gold += bind_to_gold;
            cost.__bind_gold = player->pack_detail().__money.__bind_gold;
        }
    }
   
    int use_item = 0, use_money = 0;
    if (fast_flag == 1)
    {
        if (item_num > 0 && item_num <= player->pack_count(item_id))
            use_item = 1;
        else if (cost <= player->pack_detail().__money)
            use_money = 1;
        else
        {
            if (item_num > 0)
                return ERROR_PACKAGE_GOODS_AMOUNT;
            return ERROR_PACKAGE_MONEY_AMOUNT;
        }
    }
    else
    {
        JUDGE_RETURN(item_num <= player->pack_count(item_id), ERROR_PACKAGE_GOODS_AMOUNT);
        JUDGE_RETURN(player->pack_detail().__money >= cost, ERROR_PACKAGE_MONEY_AMOUNT);

        use_item = 1;
        use_money = 1;
    }

    if (use_item == 1)
        player->pack_remove(SerialObj(ITEM_TASK_FAST_FINISH, task_info->__task_id), item_id, item_num);
    if (use_money == 1)
        player->pack_money_sub(cost, SerialObj(SUB_MONEY_TASK_FAST_FINISH, task_info->__task_id));

    player->record_serial(task_info->__task_id, TASK_SERIAL_FAST_FINISH);
    task_info->__fast_finish_rate = fast_finish_json["inc_rate"].asInt();

    this->process_finish(task_info);
//    this->process_submit(task_info);

    return 0;
}

int TaskImplement::process_finish(TaskInfo *task_info)
{
	MapLogicPlayer *player = this->tasker()->task_player();

    this->remove_task_listen(task_info);
    task_info->set_task_status(TaskInfo::TASK_STATUS_FINISH);
    player->notify_task_update(task_info);
    player->record_serial(task_info->__task_id, TASK_SERIAL_FINISH);

    player->check_current_version_open_ui(task_info->__task_id, GameEnum::CHECK_UIT_FINISH_TASK);
    player->cache_tick().update_cache(MapLogicPlayer::CACHE_TASK, true);
    return 0;
}

int TaskImplement::process_listen_lvl_up(TaskInfo *task_info, const int target_level)
{
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		JUDGE_CONTINUE(task_cond != NULL);

		if (task_cond->is_level() == false)
		{
			if (task_cond->__current_value < task_cond->__final_value)
				is_all_pass = false;
			continue;
		}
		if (target_level >= task_cond->__cond_id)
		{
			task_cond->__current_value = task_cond->__final_value;
		}
		else
		{
			is_all_pass = false;
		}
	}
    if (is_all_pass == true)
    {
        this->process_finish(task_info);
    }
    return 0;
}

int TaskImplement::process_listen_branch(TaskInfo *task_info, const int id, const int num)
{
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		JUDGE_CONTINUE(task_cond != NULL);

		if (task_cond->is_branch() == false || task_cond->__cond_id != id)
		{
			if (task_cond->__current_value < task_cond->__final_value)
				is_all_pass = false;
			continue;
		}

		task_cond->__current_value = num;
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
    {
        this->process_finish(task_info);
    }
	return 0;
}

int TaskImplement::process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num)
{
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		JUDGE_CONTINUE(task_cond != NULL);

		if (task_cond->is_kill_monster() == false || task_cond->__cond_id != sort)
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
    {
        this->process_finish(task_info);
    }
    return 0;
}

int TaskImplement::process_listen_collect_item(TaskInfo *task_info, const int id, const int num, const int bind)
{
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;
		if (task_cond->is_collect_item() == false || task_cond->__cond_id != id)
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

int TaskImplement::process_listen_attend(TaskInfo *task_info, const int type, const int sub_type, const int value)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
    const Json::Value &cond_json = task_json["exec"]["condition"];
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;

		if (task_cond->is_attend() == false || task_cond->__cond_id != type)
		{
			if (task_cond->__current_value < task_cond->__final_value)
				is_all_pass = false;
			continue;
		}

		const Json::Value &single_json = cond_json[task_cond->__cond_index];
		if (single_json.isMember("chapter") == true)
		{
            if (single_json["chapter"].asInt() != sub_type)
            {
                is_all_pass = false;
                continue;
            }
		}

		task_cond->__current_value += value;
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

int TaskImplement::process_listen_package_item(TaskInfo *task_info, const int id, const int num, const int bind)
{
	const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
	const Json::Value &cond_json = task_json["exec"]["condition"];
	bool is_all_pass = true;
	for (TaskInfo::TaskConditionList::iterator iter = task_info->__condition_list.begin();
			iter != task_info->__condition_list.end(); ++iter)
	{
		TaskConditon *task_cond = *iter;
		if (task_cond == 0)
			continue;
		if (task_cond->is_package_item() == false || task_cond->__cond_id != id)
		{
			if (task_cond->__current_value < task_cond->__final_value)
				is_all_pass = false;
			continue;
		}

		const Json::Value &single_json = cond_json[task_cond->__cond_index];
		if (single_json.isMember("bind") == true &&
				single_json["bind"].asInt() != -1 &&
				single_json["bind"].asInt() != bind)
			continue;

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

 int TaskImplement::process_listen_enter_special_scene(TaskInfo* task_info, int scene_id)
 {
    const Json::Value &task_json = task_info->conf();
    const Json::Value &cond_json = task_json["exec"]["condition"];

    for (uint i = 0; i < cond_json.size(); ++i)
    {
    	const Json::Value &id_json = cond_json[i]["id"];
    	const std::string &cond_type_str = cond_json[i]["type"].asString();
    	for (uint j = 0; j < id_json.size(); ++j)
		{
    		JUDGE_CONTINUE(cond_type_str == "scene" && scene_id == id_json[j].asInt());
			this->process_finish(task_info);
			return 0;
		}
    }

    this->tasker()->notify_task_update(task_info);
    return 0;
 }

void TaskImplement::recycle_self(void)
{
    MAP_MONITOR->task_imp_pool()->push(this);
}

int TaskImplement::remove_task_listen(TaskInfo *task_info)
{
    this->tasker()->erase_task_listen_item(task_info);
    this->tasker()->task_accepted_attend_set().erase(task_info);
    this->tasker()->task_accepted_collect_set().erase(task_info);
    this->tasker()->task_accepted_lvl_set().erase(task_info);
    this->tasker()->task_accepted_monster_set().erase(task_info);
    this->tasker()->task_accepted_branch_set().erase(task_info);
    this->tasker()->task_accepted_scene_set().erase(task_info);
    return 0;
}

int TaskImplement::remove_task_info(TaskInfo *&task_info, const bool notify)
{
    if (notify == true)
    {
        this->tasker()->notify_task_del(task_info);
    }

    this->tasker()->unbind_task(task_info->__task_id);
    MAP_MONITOR->task_info_pool()->push(task_info);
    task_info = 0;

    return 0;
}
int TaskImplement::new_process_task_reward(TaskInfo *task_info, const int reward_id, const int rate)
{
	MapLogicPlayer *player = this->tasker()->task_player();
    return player->add_reward(reward_id, SerialObj(ADD_FROM_MAIN_TASK, task_info->__task_id));
}

int TaskImplement::process_task_reward(TaskInfo *task_info, const Json::Value &reward_json, const int rate)
{
//    MapLogicPlayer *player = this->tasker()->task_player();
//
//    double real_rate = rate / 100.0;
//    int career_index = player->role_detail().__career - 1;		//职业区分奖励
//    if (career_index != 0 || career_index != 1)
//    {
//    	career_index = 0;
//    }
//
//    int need_pack_grid = 0;
//    if (reward_json["career_indentify"].isMember("goods"))
//    {
//        need_pack_grid += reward_json["career_indentify"]["goods"][career_index].size();
//    }
//
//    need_pack_grid *= real_rate;
//    JUDGE_RETURN(need_pack_grid <= player->pack_left_capacity(), ERROR_PACKAGE_NO_CAPACITY);
//
//    // 处理任务奖励
//    {
//        const Json::Value &career_json = reward_json["career_indentify"];
//        if (career_json.isMember("goods"))
//        {
//            const Json::Value &goods_json = career_json["goods"][career_index];
//
//            ItemObjVec items;
//            GameCommon::make_up_conf_items(items, goods_json);
//
//            player->insert_package(SerialObj(ADD_RROM_TASK_ITEM, task_info->__task_id), items);
//        }
//
//        if (career_json.isMember("money"))
//        {
//            Money money;
//            money.__bind_gold = career_json["money"].asInt();
//            player->pack_money_add(money, SerialObj(ADD_FROM_TASK_MONEY, task_info->__task_id));
//        }
//
//        if (career_json.isMember("exp"))
//        {
//            int add_exp = career_json["exp"].asInt();
//            player->request_add_exp(add_exp, SerialObj(EXP_TASK, task_info->__task_id));
//        }
//
//    }

    return 0;
}

