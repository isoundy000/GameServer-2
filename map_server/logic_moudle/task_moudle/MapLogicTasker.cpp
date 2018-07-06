/*
 * MapLogicTasker.cpp
 *
 * Created on: 2013-11-07 10:42
 *     Author: lyz
 */

#include "MapLogicTasker.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "TaskImplement.h"
#include "TaskRoutineImp.h"
#include "TaskTrialImp.h"
#include "SerialRecord.h"
#include "ProtoDefine.h"
#include "GameFont.h"
#include "MLMounter.h"

MapLogicTasker::MapLogicTasker(void) : novice_step_(0),
	lastest_main_task_id_(0),lcontri_day_(0)
{
	::memset(this->uiopen_step_, 0, sizeof(this->uiopen_step_));
}

MapLogicTasker::~MapLogicTasker(void)
{ /*NULL*/ }

MapLogicPlayer *MapLogicTasker::task_player(void)
{
    return dynamic_cast<MapLogicPlayer *>(this);
}

int MapLogicTasker::task_sign_in(const int type)
{
    this->task_construct_main();
    this->task_construct_branch(false);
    this->task_listen_enter_special_scene(this->task_player()->scene_id());

    this->correct_main_task();
    this->refresh_routine_task(Time_Value::gettimeofday(), false);
//    this->refresh_trial_task(false);
//    this->check_and_update_open_ui_by_sign();

    this->new_task_construct_routine(true);
    this->offer_task_construct_routine(true);
    this->league_task_construct_routine(true);
    return 0;
}

int MapLogicTasker::task_sign_out(void)
{
    return 0;
}

bool MapLogicTasker::is_finish_guide_task()
{
	static int task_id = CONFIG_INSTANCE->const_set("new_main_task_id");
	JUDGE_RETURN(task_id != 0, true);

    return this->task_submited_once_.count(task_id) > 0;
}

void MapLogicTasker::task_reset()
{
    TaskInfo *task_info = 0;
    for (TaskMap::iterator iter = this->task_map_.begin();
            iter != this->task_map_.end(); ++iter)
    {
        task_info = iter->second;
        MAP_MONITOR->task_info_pool()->push(task_info);
    }

    this->task_map_.clear();
    this->task_accepted_lvl_.clear();
    this->task_accepted_monster_.clear();
    this->task_accepted_collect_.clear();
    this->task_accepted_attend_.clear();
    this->task_accepted_package_.clear();
    this->task_accepted_scene_.clear();
    this->task_submited_once_.clear();
    this->task_listen_item_.unbind_all();
    this->finish_task_.clear();
    this->task_accepted_branch_.clear();

    this->novice_step_ = 0;
    this->lastest_main_task_id_ = 0;
    ::memset(this->uiopen_step_, 0, sizeof(this->uiopen_step_));

    this->routine_refresh_tick_ = Time_Value::zero;
    this->clear_routine_task(false);
    this->routine_consume_history_.clear();
    this->is_second_routine_.clear();
    this->is_finish_all_routine_.clear();
    this->is_routine_task_.clear();
    this->routine_task_index_.clear();
    this->routine_total_num_.clear();

    this->lcontri_day_tick_ = Time_Value::zero;
    this->lcontri_day_ = 0;

    this->open_ui_set_.clear();
    this->ui_version_ = 0;
}

void MapLogicTasker::adjust_level_by_task(int task_id)
{
	if (task_id == 0)
	{
		task_id = this->latest_main_task();
	}

	TaskInfo* task_info = this->find_task(task_id);
	JUDGE_RETURN(task_info != NULL, ;);
	JUDGE_RETURN(task_info->is_visible() == true, ;);
	JUDGE_RETURN(task_info->is_main_task() == true, ;);

	const Json::Value& conf = task_info->conf();
	JUDGE_RETURN(conf["lvl_compensate"].asInt() == 1, ;);

	MapLogicPlayer* player = this->task_player();
	int need_level = conf["before_accept"]["level"][1u].asInt();
	JUDGE_RETURN(player->role_level() < need_level, ;);
	JUDGE_RETURN(player->add_validate_operate_tick(3,
			GameEnum::TASK_LEVLE_OPERATE) == true, ;);

	player->send_to_map_thread(INNER_MAP_PROP_LEVELUP);
}

int MapLogicTasker::task_time_up(const Time_Value &nowtime)
{
	// 检查日常环式任务是否到刷新时间
    this->refresh_routine_task(nowtime, true);
	return 0;
}

int MapLogicTasker::task_construct_main(bool notify_add)
{
    const GameConfig::ConfigMap &task_map = CONFIG_INSTANCE->main_task_map();
    for (GameConfig::ConfigMap::const_iterator iter = task_map.begin();
            iter != task_map.end(); ++iter)
    {
    	const Json::Value &task_json = *(iter->second);
    	TaskInfo *task_info = this->find_task(iter->first);
        if (task_info != 0)
        {
        	// 处理任务可见到可接的状态变化
        	if (task_info->is_acceptable() == false &&
        			task_info->is_accepted() == false &&
        			task_info->is_finish() == false &&
        			task_info->is_submit() == false)
        	{
        		const Json::Value &task_prev_level_json = task_json["before_accept"]["level"];
        		if (task_prev_level_json.size() >= 3)
        		{
        			if (task_prev_level_json[1u].asInt() <= this->task_player()->role_level() &&
        					this->task_player()->role_level() <= task_prev_level_json[2u].asInt())
        			{
						task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTABLE);
						if (notify_add == true)
						{
							this->notify_task_update(task_info);
						}
        			}
        		}
        	}
            continue;
        }

        if (this->task_submited_once_.find(iter->first) != this->task_submited_once_.end())
        {
        	continue;
        }

        int prev_task = task_json["precondition"].asInt();
        if (prev_task > 0 && this->task_submited_once_.find(prev_task) == this->task_submited_once_.end())
        {
            continue;
        }

        if (task_json["before_accept"]["level"].isArray() && task_json["before_accept"]["level"].size() > 0)
        {
            if (this->task_player()->role_detail().__level < task_json["before_accept"]["level"][0u].asInt())
            {
                continue;
            }
        }

        if (notify_add == true)
        {
            this->insert_task(iter->first, task_json);
    		this->adjust_level_by_task(iter->first);
        }
        else
        {
        	this->init_task(iter->first, task_json);
        }

        if (this->task_map_.find(iter->first) != this->task_map_.end())
        {
        	break;
        }
    }
    return 0;
}

int MapLogicTasker::task_construct_branch(bool notify_add)
{
	JUDGE_RETURN(this->is_finish_guide_task() == true, 0);

    const GameConfig::ConfigMap &task_map = CONFIG_INSTANCE->branch_task_map();
    for (GameConfig::ConfigMap::const_iterator iter = task_map.begin();
            iter != task_map.end(); ++iter)
    {
    	const Json::Value &task_json = *(iter->second);

    	TaskInfo *task_info = this->find_task(iter->first);
        if (task_info != 0)
        {
        	// 处理任务可见到可接的状态变化
        	if (task_info->is_accepted() == false
        			&& task_info->is_finish() == false
        			&& task_info->is_submit() == false)
        	{
        		const Json::Value &task_prev_level_json = task_json["before_accept"]["level"];
				if (task_prev_level_json[1u].asInt() <= this->task_player()->role_level() &&
						this->task_player()->role_level() <= task_prev_level_json[2u].asInt())
				{
					task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTABLE);
					this->task_accept(iter->first);
				}
        	}

            int branch_type = task_json["exec"]["condition"][0u]["id"][0u].asInt();
            //更新最新数据
            this->accept_branch_request_info(branch_type);

            continue;
        }

        if (this->task_submited_once_.find(iter->first) != this->task_submited_once_.end())
        	continue;

        int prev_task = task_json["precondition"].asInt();
        if (prev_task > 0 && this->task_submited_once_.find(prev_task) == this->task_submited_once_.end())
            continue;

        if (task_json["before_accept"]["level"].isArray() && task_json["before_accept"]["level"].size() > 0)
        {
            if (this->task_player()->role_detail().__level < task_json["before_accept"]["level"][0u].asInt())
                continue;
        }

        if (notify_add == true)
        {
            this->insert_task(iter->first, task_json);
        }
        else
        {
            this->init_task(iter->first, task_json);
            task_info = this->find_task(iter->first);
            if (task_info != NULL && task_info->is_acceptable() == true)
            {
            	task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTABLE);
            	this->task_accept(iter->first);
            }
        }

        int branch_type = task_json["exec"]["condition"][0u]["id"][0u].asInt();
        //更新最新数据
        this->accept_branch_request_info(branch_type);
    }
    return 0;
}

int MapLogicTasker::correct_main_task(void)
{
	// 用于登录修正一些错误
	return 0;
}

TaskInfo *MapLogicTasker::init_task(const int task_id, const Json::Value &task_json)
{
    int task_type = task_json["task_type"].asInt();
    switch (task_type)
    {
    case TaskInfo::TASK_GTYPE_MAINLINE:
    {
    	return this->init_main_task(task_id, task_json);
    }
    case TaskInfo::TASK_GTYPE_ROUTINE:
    case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
    case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
    {
    	return this->init_routine_task(task_id, task_json);
    }
    case TaskInfo::TASK_GTYPE_BRANCH:
    {
    	return this->init_branch_task(task_id, task_json);
    }
    }
    
    return 0;
}

int MapLogicTasker::insert_task(const int task_id, const Json::Value &json)
{
	JUDGE_RETURN(this->task_submited_once_.count(task_id) == 0, -1);

	TaskInfo *task_info = this->init_task(task_id, json);
	JUDGE_RETURN(task_info != NULL, -1);

	if (task_info->is_acceptable() == true)
	{
		return this->task_accept(task_id);
	}
	else
	{
		return this->notify_task_add(task_info);
	}
}

TaskInfo *MapLogicTasker::init_branch_task(const int task_id, const Json::Value &task_json)
{
    int level = this->task_player()->role_level();
    int status = TaskInfo::TASK_STATUS_NONE;

    const Json::Value &before_level_json = task_json["before_accept"]["level"];
    if (before_level_json.size() < 3 || (before_level_json[1u].asInt() <= level
    		&& level <= before_level_json[2u].asInt()))
    {
        status = TaskInfo::TASK_STATUS_ACCEPTABLE;
    }

    TaskInfo *task_info = MAP_MONITOR->task_info_pool()->pop();
    task_info->__task_id = task_id;
    task_info->__game_type = task_json["task_type"].asInt();
    task_info->set_task_status(status);

    if (this->init_task_object(task_info, task_json) != 0)
    {
        MAP_MONITOR->task_info_pool()->push(task_info);
        return 0;
    }

    return task_info;
}

TaskInfo *MapLogicTasker::init_main_task(const int task_id, const Json::Value &task_json)
{
    int level = this->task_player()->role_level();
    int status = TaskInfo::TASK_STATUS_VISIBLE;

    const Json::Value &before_level_json = task_json["before_accept"]["level"];
    if (before_level_json.size() < 3 || (before_level_json[1u].asInt() <= level
    		&& level <= before_level_json[2u].asInt()))
    {
        status = TaskInfo::TASK_STATUS_ACCEPTABLE;
    }

    TaskInfo *task_info = MAP_MONITOR->task_info_pool()->pop();
    task_info->__task_id = task_id;
    task_info->__game_type = task_json["task_type"].asInt();
    task_info->set_task_status(status);

    if (this->init_task_object(task_info, task_json) != 0)
    {
        MAP_MONITOR->task_info_pool()->push(task_info);
        return 0;
    }

    this->set_latest_main_task(task_id);
    return task_info;
}

int MapLogicTasker::init_task_condition(TaskInfo *task_info, const Json::Value &task_json)
{
    const Json::Value &cond_json = task_json["exec"]["condition"];
    for (uint i = 0; i < cond_json.size(); ++i)
    {
    	TaskConditon* task_cond = MAP_MONITOR->task_condition_pool()->pop();
    	const std::string &type_str = cond_json[i]["type"].asString();

        task_cond->__type = TaskConditon::fetch_type(type_str);
        task_info->set_logic_type(task_cond->__type);

        task_cond->__cond_index = i;
        if (cond_json[i].isMember("id"))
        {
			task_cond->__id_list_index = 0;
			task_cond->__cond_id = TaskConditon::fetch_id(task_cond->__type, cond_json[i]);
			task_cond->__final_value = cond_json[i]["value"].asInt();
        }
        else if (cond_json[i].isMember("id_list"))
        {
        	task_cond->__id_list_index = rand() % int(cond_json[i]["id_list"].size());
        	task_cond->__cond_id = cond_json[i]["id_list"][task_cond->__id_list_index].asInt();
        	if (cond_json[i].isMember("value_list"))
        	{
        		if (task_cond->is_kill_monster())
        		{
					int index = rand() % cond_json[i]["value_list"].size();
					task_cond->__final_value = cond_json[i]["value_list"][index].asInt();
        		}
        		else
        		{
					int index = (task_cond->__id_list_index >= int(cond_json[i]["value_list"].size()) ? (int(cond_json[i]["value_list"].size()) - 1) : task_cond->__id_list_index);
					task_cond->__final_value = cond_json[i]["value_list"][index].asInt();
        		}
        	}
        	else if (cond_json[i].isMember("value_range_list"))
        	{
        		int index = (task_cond->__id_list_index >= int(cond_json[i]["value_range_list"].size()) ? (int(cond_json[i]["value_range_list"].size()) - 1) : task_cond->__id_list_index);
        		const Json::Value &value_range_json = cond_json[i]["value_range_list"][index];
        		int range = value_range_json[1u].asInt() - value_range_json[0u].asInt() + 1;
        		task_cond->__final_value = value_range_json[0u].asInt() + rand() % range;
        	}

        	MSG_USER("task condition %s %ld %d %d %d %d", this->task_player()->name(), this->task_player()->role_id(),
        			task_info->__task_id, task_cond->__cond_id, task_cond->__final_value, task_cond->__current_value);
        }
        else
        {
            // trial condition
            task_cond->__id_list_index = 0;
            task_cond->__final_value = cond_json[i]["value"].asInt();
        }

        //环式任务优化
        if (cond_json[i].isMember("kill_type"))
        {
        	int kill_type = cond_json[i]["kill_type"].asInt();
    		int task_type = task_json["task_type"].asInt();
 //   		int range_level = cond_json["range_level"].asInt();
    		int range_level = this->task_player()->role_level();

        	if (kill_type == 1)
        	{
        		task_cond->__cond_id = this->fetch_routine_monster_id(task_type, range_level);
        	}

        	task_cond->__kill_type = kill_type;
        	task_cond->__range_level = range_level;
        }


        task_info->__condition_list.push_back(task_cond);
    }

    return 0;
}

int MapLogicTasker::init_task_star(TaskInfo *task_info, const Json::Value &task_json)
{
    JUDGE_RETURN(task_json.isMember("task_star"), 0);

    int rand_value = rand() % 10000;
    int rand_base = 0;
    for (uint i = 0; i < task_json["task_star"].size(); ++i)
    {
        rand_base += int(task_json["task_star"][i][0u].asDouble() * 100);
        if (rand_value < rand_base)
        {
            task_info->__task_star = i + 1;
            break;
        }
    }
    return 0;
}

int MapLogicTasker::init_task_object(TaskInfo *task_info, const Json::Value &task_json)
{
	int serial_type = TASK_SERIAL_VISIBLE;
	if (task_info->is_acceptable())
	{
		serial_type = TASK_SERIAL_ACCEPTABLE;
	}

    this->init_task_condition(task_info, task_json);
    this->init_task_star(task_info, task_json);
    task_info->__task_imp = this->pop_task_imp(task_info->__game_type);
    if (task_info->__task_imp == 0)
        return -1;

    if (this->task_map_.insert(TaskMap::value_type(task_info->__task_id, task_info)).second == false)
        return -1;

    this->record_serial(task_info->__task_id, serial_type);

    if (task_json["before_accept"]["auto_accept"].asInt() == 1
    		&& this->process_accept(task_info->__task_id, true) == 0)
    {
    	this->record_serial(task_info->__task_id, TASK_SERIAL_ACCEPTED);
    }

    return 0;
}

TaskImplement *MapLogicTasker::pop_task_imp(const int game_type)
{
    TaskImplement *task_imp = 0;
    if (game_type == TaskInfo::TASK_GTYPE_MAINLINE || game_type == TaskInfo::TASK_GTYPE_BRANCH)
    {
        task_imp = MAP_MONITOR->task_imp_pool()->pop();
        if (task_imp != 0)
            task_imp->set_tasker(this);
    }
    if (game_type == TaskInfo::TASK_GTYPE_ROUTINE ||
    	game_type == TaskInfo::TASK_GTYPE_OFFER_ROUTINE ||
    	game_type == TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE)
    {
    	TaskRoutineImp *routine_imp = MAP_MONITOR->task_routine_imp_pool()->pop();
        task_imp = routine_imp;
        if (task_imp != 0)
            task_imp->set_tasker(this);
    }
//    if (game_type == TaskInfo::TASK_GTYPE_TRIAL)
//    {
//        TaskTrialImp *trial_imp = MAP_MONITOR->task_trial_imp_pool()->pop();
//        task_imp = trial_imp;
//        if (task_imp != 0)
//            task_imp->set_tasker(this);
//    }
    return task_imp;
}

int MapLogicTasker::task_branch_update(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400023 *, request, -1);
    return this->task_listen_branch(request->id(), request->num());
}

int MapLogicTasker::accept_branch_request_info(int type)
{
	MapLogicPlayer* player = this->task_player();

	if ((type >= GameEnum::BRANCH_FB_BEGIN && type <= GameEnum::BRANCH_FB_END) ||
			type == GameEnum::BRANCH_EXP_FB || type == GameEnum::BRANCH_LEGEND_TOP)
	{
		Proto30400026 info;
		info.set_id(type);
		info.set_num(0);
		player->send_to_map_thread(info);
	}
	else if (type == GameEnum::BRANCH_EQUIP)
	{
		GamePackage* package = player->find_package(GameEnum::INDEX_EQUIP);
		PackageItem* item = package->find_by_index(0);
		JUDGE_RETURN(item != NULL, -1);

		this->task_listen_branch(type, item->__equipment.strengthen_level_);
	}
	else if (type == GameEnum::BRANCH_FIRST_RECHARGE)
	{
		JUDGE_RETURN(player->recharge_detail().__recharge_money > 0, -1);
		this->task_listen_branch(type, 1);
	}
	else if (type == GameEnum::BRANCH_LEAGUE)
	{
		JUDGE_RETURN(player->role_detail().__league_id != 0, -1);
		this->task_listen_branch(type, 1);
	}
	else if (type == GameEnum::BRANCH_LEVEL)
	{
		this->task_listen_branch(type, player->role_level());
	}
	else if (type == GameEnum::BRANCH_WEDDING)
	{
		JUDGE_RETURN(player->role_detail().__partner_id > 0, -1);
		this->task_listen_branch(type, 1);
	}


	return 0;
}

int MapLogicTasker::task_accept(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto11400326 *, request, -1);
    return this->task_accept(request->task_id());
}

int MapLogicTasker::task_accept(int task_id)
{
    MapLogicPlayer *player = this->task_player();

    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
    CONDITION_PLAYER_NOTIFY_RETURN(task_json.empty() == false,
    		RETURN_TASK_ACCEPT, ERROR_CONFIG_NOT_EXIST);

    int npc_id = task_json["before_accept"]["npc"].asInt();
    if (npc_id > 0 && npc_id != 1)
    {
    	Proto11400326 request;
    	request.set_task_id(task_id);
        return this->is_nearby_npc(CLIENT_TASK_ACCEPT, &request, npc_id);
    }
    else
    {
        int ret = this->process_accept(task_id);
        CONDITION_PLAYER_NOTIFY_RETURN(ret == 0, RETURN_TASK_ACCEPT, ret);

        this->record_serial(task_id, TASK_SERIAL_ACCEPTED);
        FINER_PLAYER_PROCESS_NOTIFY(RETURN_TASK_ACCEPT);
    }
}

int MapLogicTasker::task_abandon(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto11400327 *, request, msg, -1);

    MapLogicPlayer *player = this->task_player();
    MSG_DEBUG("abandon task %s %d %d", player->name(), player->role_id(), request->task_id());

    int ret = this->process_abandon(request->task_id());
    CONDITION_PLAYER_NOTIFY_RETURN(ret == 0, RETURN_TASK_ABANDON, ret);

    this->record_serial(request->task_id(), TASK_SERIAL_ABANDON);
    return player->respond_to_client(RETURN_TASK_ABANDON);
}

int MapLogicTasker::task_submit(Message *msg, bool test)
{
	MapLogicPlayer *player = this->task_player();
    MSG_DYNAMIC_CAST_RETURN(Proto11400328 *, request,  -1);

    int task_id = request->task_id();
    int is_double = request->is_double();

    MSG_DEBUG("submit task %s %d %d %d", player->name(), player->role_id(), task_id, is_double);
    CONDITION_PLAYER_NOTIFY_RETURN(request->task_id() > 0, RETURN_TASK_SUBMIT, ERROR_CLIENT_OPERATE);

    const Json::Value &task_json = CONFIG_INSTANCE->find_task(request->task_id());
    CONDITION_PLAYER_NOTIFY_RETURN(task_json.empty() == false, RETURN_TASK_SUBMIT, ERROR_CONFIG_NOT_EXIST);

    int npc_id = task_json["finish"]["npc"].asInt();
    if (test == false && npc_id > 0 && npc_id != 1)
    {
        return this->is_nearby_npc(CLIENT_TASK_SUBMIT, msg, npc_id);
    }

    int ret = this->process_submit(task_id, is_double, test);
    CONDITION_PLAYER_NOTIFY_RETURN_B(ret == 0, RETURN_TASK_SUBMIT, ret, ERROR_TASK_ID);

    this->record_serial(request->task_id(), TASK_SERIAL_SUBMIT);
    player->cache_tick().update_cache(MapLogicPlayer::CACHE_TASK, true);
    FINER_PLAYER_PROCESS_NOTIFY(RETURN_TASK_SUBMIT);
}

int MapLogicTasker::task_talk(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400331 *, request, -1);

	MapLogicPlayer *player = this->task_player();
	MSG_DEBUG("talk task %s %d %d", player->name(), player->role_id(), request->task_id());

	int ret = this->process_talk(request->task_id());
	CONDITION_PLAYER_NOTIFY_RETURN(ret == 0, RETURN_TASK_TALK, ret);

	return player->respond_to_client(RETURN_TASK_TALK);
}

int MapLogicTasker::task_get_list(Message *msg)     // 获取任务列表
{
    if (this->task_map_.size() <= 0 && this->lastest_main_task_id_ > 0)
    {
    	// 没有主线任务时显示最后一次任务后的相关提示
    	Proto81400107 respond;
    	respond.set_lastest_task_id(this->lastest_main_task_id_);
    	this->task_player()->respond_to_client(ACTIVE_LASTEST_MAIN_TASK, &respond);
    }

    Proto51400325 respond;
    respond.set_is_finish_guide(this->is_finish_guide_task());

    for (TaskMap::iterator iter = this->task_map_.begin();
    		iter != this->task_map_.end(); ++iter)
    {
    	TaskInfo* task_info = iter->second;

        if (task_info->is_finish() == false &&
                task_info->is_accepted() == false &&
                task_info->is_acceptable() == false &&
                task_info->is_visible() == false)
            continue;

        ProtoTaskInfo *proto_task_info = respond.add_task_list();
        this->serail_task_info(task_info, proto_task_info);
    }

    for (IntSet::iterator iter = this->finish_task_.begin();
    		iter != this->finish_task_.end(); ++iter)
    {
    	respond.add_task_finish(*iter);
    }

    return this->task_player()->respond_to_client(RETURN_TASK_TRACK_LIST, &respond);
}

int MapLogicTasker::task_npc_list(Message *msg)     // 获取npc列表
{
    DYNAMIC_CAST_RETURN(Proto11400329 *, request, msg, -1);

    MapLogicPlayer *player = this->task_player();

    const BIntSet &task_set = CONFIG_INSTANCE->npc_task_list(request->npc_id());

    Proto51400329 respond;
    TaskInfo *task_info = 0;
    for (BIntSet::iterator iter = task_set.begin();
            iter != task_set.end(); ++iter)
    {
        if ((task_info = this->find_task(*iter)) == 0)
            continue;

        if (task_info->is_finish() == false &&
                task_info->is_accepted() == false &&
                task_info->is_acceptable() == false &&
                task_info->is_visible() == false)
            continue;
        
        const Json::Value &task_json = CONFIG_INSTANCE->find_task(*iter);
        int before_npc = task_json["before_accept"]["npc"].asInt(),
        		accepted_npc = task_json["after_accept"]["npc"].asInt(),
        		finish_npc = task_json["finish"]["npc"].asInt();

        bool is_check_true = false;
        if (is_check_true == false && before_npc == request->npc_id() && before_npc > 0)
        {
        	if (task_info->is_acceptable() == true)
        		is_check_true = true;
        }
        if (is_check_true == false && accepted_npc == request->npc_id() && accepted_npc > 0)
        {
        	if (task_info->is_accepted() == true)
        		is_check_true = true;
        }
        if (is_check_true == false && finish_npc == request->npc_id() && finish_npc > 0)
        {
        	if (task_info->is_finish() == true)
        		is_check_true = true;
        }
        if (is_check_true == false)
        	continue;

        ProtoTaskInfo *proto_task_info = respond.add_task_list();
        this->serail_task_info(task_info, proto_task_info);
    }

    return player->respond_to_client(RETURN_TASK_NPC_LIST, &respond);
}

int MapLogicTasker::task_fast_finish(Message *msg) // 快速完成任务
{
    MSG_DYNAMIC_CAST_RETURN(Proto11400330 *, request, -1);

    MapLogicPlayer *player = this->task_player();

    int ret = this->process_fast_finish(request->task_id());
    CONDITION_PLAYER_NOTIFY_RETURN(ret == 0, RETURN_TASK_FAST_FINISH, ret);

    return player->respond_to_client(RETURN_TASK_FAST_FINISH);
}

int MapLogicTasker::task_after_check_npc(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400231 *, request, -1);

    int serial_type = 0, task_id = 0;
    MapLogicPlayer *player = this->task_player();

    int recogn = request->recogn();
    switch (recogn)
    {
	case CLIENT_TASK_ACCEPT:
	{
		Proto11400326 sub_request;
		sub_request.ParsePartialFromString(request->msg_body());

		int ret = this->process_accept(sub_request.task_id());
		JUDGE_RETURN(ret == 0, -1);

		serial_type = TASK_SERIAL_ACCEPTED;
		task_id = sub_request.task_id();

		player->respond_to_client(RETURN_TASK_ACCEPT);
		break;
	}

	case CLIENT_TASK_ABANDON:
	{
		Proto11400327 sub_request;
		sub_request.ParsePartialFromString(request->msg_body());

		int ret = this->process_abandon(sub_request.task_id());
		CONDITION_PLAYER_NOTIFY_RETURN(ret == 0, RETURN_TASK_ABANDON, ret);

		serial_type = TASK_SERIAL_ABANDON;
		task_id = sub_request.task_id();

		player->respond_to_client(RETURN_TASK_ABANDON);
		break;
	}

	case CLIENT_TASK_SUBMIT:
	{
		Proto11400328 sub_request;
		sub_request.ParsePartialFromString(request->msg_body());

		int ret = this->process_submit(sub_request.task_id(), sub_request.is_double());
		CONDITION_PLAYER_NOTIFY_RETURN_B(ret == 0, RETURN_TASK_SUBMIT, ret, ERROR_TASK_ID);

		serial_type = TASK_SERIAL_SUBMIT;
		task_id = sub_request.task_id();

		player->respond_to_client(RETURN_TASK_SUBMIT);
		break;
	}

	default:
	{
		MSG_USER("after check npc %d", recogn);
		return -1;
	}
    }

	this->record_serial(task_id, serial_type);
    player->cache_tick().update_cache(MapLogicPlayer::CACHE_TASK, true);
    return 0;
}

bool MapLogicTasker::is_task_submited(const int task_id)
{
    if (this->task_submited_once_.find(task_id) != this->task_submited_once_.end())
        return true;
    return false;
}

bool MapLogicTasker::is_nearby_npc(const int recogn, Message *msg, const int npc_id)
{
    Proto31400231 request;
    request.set_npc_id(npc_id);
    request.set_recogn(recogn);
    if (msg != 0)
        msg->SerializePartialToString(request.mutable_msg_body());

    return MAP_MONITOR->process_inner_map_request(this->task_player()->role_id(), request);
}

int MapLogicTasker::bind_task(const int task_id, TaskInfo *info)
{
    TaskMap::iterator iter = this->task_map_.find(task_id);
    if (iter != this->task_map_.end())
        return -1;

    if (this->task_map_.insert(TaskMap::value_type(task_id, info)).second == true)
        return 0;
    return -1;
}

int MapLogicTasker::unbind_task(const int task_id)
{
	return this->task_map_.erase(task_id);
}

TaskInfo *MapLogicTasker::find_task(const int task_id)
{
	JUDGE_RETURN(this->task_map_.count(task_id) > 0, NULL);
	return this->task_map_[task_id];
}

MapLogicTasker::TaskMap &MapLogicTasker::task_map(void)
{
    return this->task_map_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_lvl_set(void)
{
    return this->task_accepted_lvl_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_monster_set(void)
{
    return this->task_accepted_monster_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_collect_set(void)
{
    return this->task_accepted_collect_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_attend_set(void)
{
    return this->task_accepted_attend_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_scene_set(void)
{
    return this->task_accepted_scene_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_package_set(void)
{
	return this->task_accepted_package_;
}

MapLogicTasker::TaskSet &MapLogicTasker::task_accepted_branch_set(void)
{
    return this->task_accepted_branch_;
}

MapLogicTasker::TaskIdSet &MapLogicTasker::task_submited_once_set(void)
{
    return this->task_submited_once_;
}

MapLogicTasker::ItemTaskMap &MapLogicTasker::task_listen_item_map(void)
{
	return this->task_listen_item_;
}



int MapLogicTasker::init_task_listen_item(TaskInfo *task_info)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);

    const Json::Value &cond_json = task_json["exec"]["condition"];
    for (uint i = 0; i < cond_json.size(); ++i)
    {
        if (cond_json[i]["type"].asString() != "package")
            continue;

        int item_id = cond_json[i]["id"][0u].asInt();
        this->task_listen_item_[item_id].insert(task_info->__task_id);
    }

    return 0;
}

int MapLogicTasker::erase_task_listen_item(TaskInfo *task_info, const bool is_finish)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);

    const Json::Value &cond_json = task_json["exec"]["condition"];
    for (uint i = 0; i < cond_json.size(); ++i)
    {
        if (cond_json[i]["type"].asString() != "package")
            continue;

        int item_id = cond_json[i]["id"][0u].asInt();

        if (is_finish == true && cond_json[i]["is_remove"].asInt() == 1)
        {
            if (cond_json[i].isMember("bind") == false ||
                    cond_json[i]["bind"].asInt() == -1)
            {
                this->task_player()->pack_remove(SerialObj(ITEM_TASK_REMOVE, task_info->__task_id), item_id, cond_json[i]["value"].asInt());
            }
            else if (cond_json[i]["bind"].asInt() == 0)
            {
                int item_num = cond_json[i]["value"].asInt();
                while (item_num > 0)
                {
                    PackageItem *item = this->task_player()->pack_find_by_unbind_id(item_id);
                    if (item == 0)
                        break;

                    if (item->__amount > item_num)
                    {
                        this->task_player()->pack_remove(SerialObj(ITEM_TASK_REMOVE, task_info->__task_id), item, item_num);
                        item_num = 0;
                    }
                    else
                    {
                        item_num -= item->__amount;
                        this->task_player()->pack_remove(SerialObj(ITEM_TASK_REMOVE, task_info->__task_id), item, item->__amount);
                    }
                }
            }
            else
            {
                int item_num = cond_json[i]["value"].asInt();
                while (item_num > 0)
                {
                    PackageItem *item = this->task_player()->pack_find_by_bind_id(item_id);
                    if (item == 0)
                    {
                        item = this->task_player()->pack_find_by_unbind_id(item_id);
                        if (item == 0)
                            break;
                    }

                    if (item->__amount > item_num)
                    {
                        this->task_player()->pack_remove(SerialObj(ITEM_TASK_REMOVE, task_info->__task_id), item, item_num);
                        item_num = 0;
                    }
                    else
                    {
                        item_num -= item->__amount;
                        this->task_player()->pack_remove(SerialObj(ITEM_TASK_REMOVE, task_info->__task_id), item, item->__amount);
                    }
                }
            }
        }

        if (this->task_listen_item_.find(item_id) == false)
            continue;

        TaskIdSet &id_set = this->task_listen_item_[item_id];
        id_set.erase(task_info->__task_id);
        if (id_set.size() <= 0)
            this->task_listen_item_.unbind(item_id);
    }

    return 0;
}

bool MapLogicTasker::is_task_listen_item(const int item_id)
{
    return this->task_listen_item_.find(item_id);
}

int MapLogicTasker::task_listen_lvl_up(int target_level)
{
    TaskInfo *task_info = 0;
    std::vector<TaskInfo *> task_vc;
    for (TaskSet::iterator iter = this->task_accepted_lvl_.begin();
            iter != this->task_accepted_lvl_.end(); ++iter)
    {
        task_info = *iter;
        if (task_info->is_accepted() == false)
            continue;
        if (task_info->is_logic_level_up() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (std::vector<TaskInfo *>::iterator iter = task_vc.begin();
    		iter != task_vc.end(); ++iter)
    {
    	task_info = *iter;
    	task_info->task_imp()->process_listen_lvl_up(task_info, target_level);
    }

    MapLogicPlayer* player = this->task_player();
    if (GameCommon::is_normal_scene(player->scene_id()))
    {
    	this->check_current_version_open_ui(target_level, GameEnum::CHECK_UIT_LEVELUP);
    }

    this->task_construct_main(true);
    this->task_construct_branch(true);
//    this->task_listen_branch(GameEnum::BRANCH_LEVEL, player->role_level());

    /*
    if (this->task_player()->role_level() >= CONFIG_INSTANCE->task_setting()["trial_level"].asInt())
        this->notify_virtual_trial_task_for_novice();
     */
    if (this->task_player()->role_level() >= CONFIG_INSTANCE->task_setting()["daliy_routine_start_level"].asInt())
        this->new_task_construct_routine(true);

    if (this->task_player()->role_level() >= CONFIG_INSTANCE->task_setting()["offer_routine_start_level"].asInt())
        this->offer_task_construct_routine(true);

    if (this->task_player()->role_level() >= CONFIG_INSTANCE->task_setting()["league_routine_start_level"].asInt()
    		&& this->task_player()->role_detail().__league_id != 0)
        this->league_task_construct_routine(true);


    return 0;
}

int MapLogicTasker::task_listen_branch(const int id, const int num)
{
    TaskInfo *task_info = 0;
    std::vector<TaskInfo *> task_vc;
    for (TaskSet::iterator iter = this->task_accepted_branch_.begin();
            iter != this->task_accepted_branch_.end(); ++iter)
    {
        task_info = *iter;
        if (task_info->is_accepted() == false || task_info->is_logic_branch() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (std::vector<TaskInfo *>::iterator iter = task_vc.begin();
    		iter != task_vc.end(); ++iter)
    {
    	task_info = *iter;
    	task_info->task_imp()->process_listen_branch(task_info, id, num);
    }
    return 0;
}

int MapLogicTasker::task_listen_kill_monster(const int sort, const int num)
{
    TaskInfo *task_info = 0;
    std::vector<TaskInfo *> task_vc;
    for (TaskSet::iterator iter = this->task_accepted_monster_.begin();
            iter != this->task_accepted_monster_.end(); ++iter)
    {
        task_info = *iter;
        if (task_info->is_accepted() == false)
            continue;
        if (task_info->is_logic_kill_monster() == false &&
        		task_info->is_logic_lvl_monster() == false &&
        		task_info->is_logic_kill_boss() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (std::vector<TaskInfo *>::iterator iter = task_vc.begin();
    		iter != task_vc.end(); ++iter)
    {
    	task_info = *iter;
    	task_info->task_imp()->process_listen_kill_monster(task_info, sort, num);
    }
    return 0;
}

int MapLogicTasker::task_listen_collect_item(const int id, const int num, const int bind)
{
    TaskInfo *task_info = 0;
    std::vector<TaskInfo *> task_vc;
    for (TaskSet::iterator iter = this->task_accepted_collect_.begin();
            iter != this->task_accepted_collect_.end(); ++iter)
    {
        task_info = *iter;
        if (task_info->is_accepted() == false)
            continue;
        if (task_info->is_logic_collect_item() == false &&
        		task_info->is_logic_collect_any() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (std::vector<TaskInfo *>::iterator iter = task_vc.begin();
    		iter != task_vc.end(); ++iter)
    {
    	task_info = *iter;
        task_info->task_imp()->process_listen_collect_item(task_info, id, num, bind);
    }
    return 0;
}

int MapLogicTasker::task_listen_attend(const int type, const int sub_type, const int inc_value)
{
    int value = inc_value;
//    if (type == GameEnum::ACTIVITY_LEAGUE_DONATE)
//    {
//        this->refresh_daily_league_contri();
//        if (inc_value > 0)
//            this->lcontri_day_ += inc_value;
//        value = this->lcontri_day_;
//    }

    TaskInfoVec task_vc;
    for (TaskSet::iterator iter = this->task_accepted_attend_.begin();
            iter != this->task_accepted_attend_.end(); ++iter)
    {
    	TaskInfo *task_info = *iter;
        if (task_info->is_accepted() == false)
            continue;
        if (task_info->is_logic_attend() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (TaskInfoVec::iterator iter = task_vc.begin(); iter != task_vc.end(); ++iter)
    {
    	TaskInfo *task_info = *iter;
        task_info->task_imp()->process_listen_attend(task_info, type, sub_type, value);
    }
    return 0;
}

int MapLogicTasker::task_listen_package_item(const int id, const int num, const int bind)
{
    TaskInfoVec task_vc;
    for (TaskSet::iterator iter = this->task_accepted_package_.begin();
            iter != this->task_accepted_package_.end(); ++iter)
    {
    	TaskInfo *task_info = *iter;
        if (task_info->is_accepted() == false)
            continue;
        if (task_info->is_logic_package() == false)
            continue;

        task_vc.push_back(task_info);
    }

    for (TaskInfoVec::iterator iter = task_vc.begin(); iter != task_vc.end(); ++iter)
    {
    	TaskInfo *task_info = *iter;
        task_info->task_imp()->process_listen_package_item(task_info, id, num, bind);
    }

    return 0;
}

int MapLogicTasker::task_listen_enter_special_scene(int scene_id)
{
	TaskInfoVec task_vc;
	for (TaskSet::iterator iter = this->task_accepted_scene_.begin();
			iter != this->task_accepted_scene_.end(); ++iter)
	{
		TaskInfo* task_info = *iter;
		JUDGE_CONTINUE(task_info->is_accepted() == true);
		JUDGE_CONTINUE(task_info->is_logic_scene() == true);
		task_vc.push_back(task_info);
	}

	for (TaskInfoVec::iterator iter = task_vc.begin(); iter != task_vc.end(); ++iter)
	{
		TaskInfo* task_info = *iter;
		task_info->task_imp()->process_listen_enter_special_scene(task_info,scene_id);
	}

	return 0;
}


int MapLogicTasker::process_accept(int task_id, bool is_init)
{
//    if (this->task_type(task_id) == TaskInfo::TASK_GTYPE_TRIAL)
//    {
//        return this->process_accept_trial(task_id, is_init);
//    }

    TaskInfo *task_info = this->find_task(task_id);
    JUDGE_RETURN(task_info != 0, ERROR_TASK_ID);
    JUDGE_RETURN(task_info->is_acceptable() == true, ERROR_TASK_STATUS);

    if (task_info->is_routine_task())
    {
    	this->is_second_routine_[task_info->__game_type] = 1;
    }

    int ret = task_info->task_imp()->process_accept(task_info, is_init);
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int MapLogicTasker::process_abandon(const int task_id)
{
    TaskInfo *task_info = this->find_task(task_id);
    JUDGE_RETURN(task_info != 0, ERROR_TASK_ID);

    JUDGE_RETURN(task_info->is_accepted() == true, ERROR_TASK_STATUS);
    JUDGE_RETURN(task_info->is_main_task() == false, ERROR_TASK_ABANDON);

    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
    JUDGE_RETURN(task_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
    JUDGE_RETURN(task_json["exec"]["abandon"].asInt() == 1, ERROR_TASK_ABANDON);

    return task_info->task_imp()->process_abandon(task_info);
}

int MapLogicTasker::process_submit(const int task_id, const int is_double, bool test)
{
    TaskInfo *task_info = this->find_task(task_id);
    JUDGE_RETURN(task_info != 0, ERROR_TASK_ID);

    if (test == false)
    {
    	JUDGE_RETURN(task_info->is_finish() == true, ERROR_TASK_STATUS);
    }

    int ret = task_info->task_imp()->process_submit(task_info, is_double);
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int MapLogicTasker::process_fast_finish(const int task_id)
{
    TaskInfo *task_info = this->find_task(task_id);
    JUDGE_RETURN(task_info != 0, ERROR_TASK_ID);

    JUDGE_RETURN(task_info->is_finish() == false, ERROR_TASK_STATUS);

    if (task_info->is_routine_task())
    {
    	this->is_second_routine_[task_info->__game_type] = 1;
    }

    return task_info->task_imp()->process_fast_finish(task_info);
}

int MapLogicTasker::process_talk(const int task_id)
{
	TaskInfo *task_info = this->find_task(task_id);
	JUDGE_RETURN(task_info != 0, ERROR_TASK_ID);

	JUDGE_RETURN(task_info->is_logic_patrol() == true && task_info->is_accepted() == true, ERROR_TASK_STATUS);

	task_info->set_task_status(TaskInfo::TASK_STATUS_FINISH);
	this->notify_task_update(task_info);
	return 0;
}

int MapLogicTasker::serail_task_info(TaskInfo *task_info, ProtoTaskInfo *proto_task_info)
{
    task_info->serialize(proto_task_info);
    if (task_info->is_routine_task())
    {
    	proto_task_info->set_is_first_routine((this->is_second_routine_[task_info->__game_type] == 0));
    	proto_task_info->set_is_routine(this->is_routine_task_[task_info->__game_type]);
    	proto_task_info->set_routine_index(this->routine_task_index_[task_info->__game_type]);
    	proto_task_info->set_routine_total(this->routine_total_num_[task_info->__game_type]);
    }
    return 0;
}

int MapLogicTasker::notify_task_add(TaskInfo *task_info)
{
    int task_status = task_info->task_status();
    JUDGE_RETURN(task_status > 0, -1);

    const int TASK_ADD_CMD = 1/*, TASK_DEL_CMD = 2, TASK_UPDATE_CMD = 3*/;

    if (task_info->is_routine_task())
    {
        return this->notify_routine_task_status(TASK_ADD_CMD, task_info);
    }

    Proto81400104 respond;
    respond.set_cmd(TASK_ADD_CMD);
    respond.set_is_finish_guide(this->is_finish_guide_task());

    ProtoTaskInfo *ptask_info = respond.mutable_task_info();
    this->serail_task_info(task_info, ptask_info);

    return this->task_player()->respond_to_client(ACTIVE_TASK_UPDATE, &respond);
}

int MapLogicTasker::notify_task_del(TaskInfo *task_info)
{
    const int /*TASK_ADD_CMD = 1, */TASK_DEL_CMD = 2/*, TASK_UPDATE_CMD = 3*/;
    if (task_info->is_routine_task())
    {
        return this->notify_routine_task_status(TASK_DEL_CMD, task_info);
    }

    Proto81400104 respond;
    respond.set_cmd(TASK_DEL_CMD);
    respond.set_is_finish_guide(this->is_finish_guide_task());

    ProtoTaskInfo *ptask_info = respond.mutable_task_info();
    ptask_info->set_task_id(task_info->__task_id);
    ptask_info->set_type(task_info->__game_type);
    ptask_info->set_status(task_info->task_status());
    return this->task_player()->respond_to_client(ACTIVE_TASK_UPDATE, &respond);
}

int MapLogicTasker::notify_task_update(TaskInfo *task_info)
{
    const int /*TASK_ADD_CMD = 1, TASK_DEL_CMD = 2, */TASK_UPDATE_CMD = 3;
    if (task_info->is_routine_task())
    {
        return this->notify_routine_task_status(TASK_UPDATE_CMD, task_info);
    }

    Proto81400104 respond;
    respond.set_cmd(TASK_UPDATE_CMD);
    respond.set_is_finish_guide(this->is_finish_guide_task());
    ProtoTaskInfo *ptask_info = respond.mutable_task_info();
    this->serail_task_info(task_info, ptask_info);
    return this->task_player()->respond_to_client(ACTIVE_TASK_UPDATE, &respond);
}

int MapLogicTasker::notify_routine_task_status(const int cmd, TaskInfo *task_info)
{
    int status = task_info->task_status();
    TaskConditon *task_cond_obj = task_info->task_condition(0);
    int total_size = this->routine_total_num_[task_info->__game_type],
    	cur_index = this->routine_task_index_[task_info->__game_type];

    Proto81400203 respond;
    respond.set_cmd(cmd);
    respond.set_task_id(task_info->__task_id);
    respond.set_type(task_info->__game_type);
    respond.set_status(status);
    respond.set_is_first_routine((this->is_second_routine_[task_info->__game_type] == 0));

    respond.set_total_routine_size(total_size);
    respond.set_routine_task_index(cur_index);

    if (task_cond_obj != 0)
    {
        respond.set_cond_type(task_cond_obj->__type);
        respond.set_cond_index(0);
        respond.set_cond_id(task_cond_obj->__cond_id);
        respond.set_cur_value(task_cond_obj->__current_value);
        respond.set_final_value(task_cond_obj->__final_value);
        respond.set_kill_type(task_cond_obj->__kill_type);
        respond.set_range_level(task_cond_obj->__range_level);
    }
    return this->task_player()->respond_to_client(ACTIVE_TASK_ROUTINE_UPDATE, &respond);
}

int MapLogicTasker::serialize_task(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400108 *, request, -1);

    for (TaskMap::iterator iter = this->task_map_.begin();
            iter != this->task_map_.end(); ++iter)
    {
        TaskInfo *task_info = iter->second;
        task_info->serialize(request->add_task_list());
    }

    for (TaskIdSet::iterator iter = this->task_submited_once_.begin();
            iter != this->task_submited_once_.end(); ++iter)
    {
    	request->add_submited_task(*iter);
    }

    for (IntSet::iterator iter = this->finish_task_.begin();
    		iter != this->finish_task_.end(); ++iter)
    {
    	request->add_finish_task(*iter);
    }

//    request->set_is_first_rename(this->task_player()->role_detail().__is_first_rename);
    request->set_novice_step(this->novice_step());
    request->set_latest_main_task(this->lastest_main_task_id_);

    for (int i = 0; i <= GameEnum::UIOPEN_STEP_SIZE; ++i)
    {
    	request->add_uiopen_step(this->uiopen_step_[i]);
    }

    for (int i = TaskInfo::TASK_GTYPE_ROUTINE; i < TaskInfo::TASK_GTYPE_END; ++i)
    {
    	if (i == TaskInfo::TASK_GTYPE_ROUTINE
    			|| i == TaskInfo::TASK_GTYPE_OFFER_ROUTINE
    			|| i == TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE)
    	{
    		request->add_is_finish_all_routine(this->is_finish_all_routine_[i]);
    		request->add_is_second_routine(this->is_second_routine_[i]);
    		request->add_routine_index(this->routine_task_index_[i]);
    		request->add_is_routine_task(this->is_routine_task_[i]);
    		request->add_total_num(this->routine_total_num_[i]);
    	}
    }

    request->set_routine_refresh_tick(this->routine_refresh_tick_.sec());
    request->set_lcontri_tick(this->lcontri_day_tick_.sec());
    request->set_lcontri_day(this->lcontri_day_);
    request->set_ui_version(this->ui_version_);

    for (TaskIdSet::iterator iter = this->routine_consume_history_.begin();
    		iter != this->routine_consume_history_.end(); ++iter)
    {
    	request->add_routine_consume_history(*iter);
    }

//    for (OpenUiSet::iterator iter = this->open_ui_set().begin();
//    		iter != this->open_ui_set().end(); ++iter)
//    {
//        request->add_open_ui_list(*iter);
//    }

    return 0;
}

int MapLogicTasker::unserialize_task(Message *proto)
{
    DYNAMIC_CAST_RETURN(Proto31400108 *, request, proto, -1);

    int task_info_size = request->task_list_size();
    for (int i = 0; i < task_info_size; ++i)
    {
        const ProtoInnerTaskInfo &inner_task_info = request->task_list(i);

        TaskInfo *task_info = this->task_player()->monitor()->task_info_pool()->pop();
        task_info->__task_id = inner_task_info.task_id();
        if (this->task_map_.insert(TaskMap::value_type(task_info->__task_id, task_info)).second == false)
        {
            this->task_player()->monitor()->task_info_pool()->push(task_info);
            continue;
        }

        task_info->unserialize(inner_task_info);
        task_info->__task_imp = this->pop_task_imp(task_info->__game_type);

        if (task_info->is_logic_level_up())
            this->task_accepted_lvl_set().insert(task_info);
        if (task_info->is_logic_kill_monster() ||
        		task_info->is_logic_lvl_monster() ||
        		task_info->is_logic_kill_boss())
            this->task_accepted_monster_set().insert(task_info);
        if (task_info->is_logic_collect_item() ||
        		task_info->is_logic_collect_any())
            this->task_accepted_collect_set().insert(task_info);
        if (task_info->is_logic_attend())
            this->task_accepted_attend_set().insert(task_info);
        if (task_info->is_logic_scene())
            this->task_accepted_scene_set().insert(task_info);
        if (task_info->is_logic_package())
        {
        	this->init_task_listen_item(task_info);
            this->task_accepted_package_set().insert(task_info);
        }
        if (task_info->is_branch_task())
        {
        	this->task_accepted_branch_set().insert(task_info);
        }
    }

    for (int i = 0; i < request->submited_task_size(); ++i)
    {
        this->task_submited_once_.insert(request->submited_task(i));
    }

    for (int i = 0; i < request->finish_task_size(); ++i)
    {
    	this->insert_finish_task(request->finish_task(i));
    }

//    this->task_player()->role_detail().__is_first_rename = request->is_first_rename();
    this->novice_step_ = request->novice_step();
    this->lastest_main_task_id_ = request->latest_main_task();
    for (int i = 0; i < request->uiopen_step_size(); ++i)
    {
    	this->uiopen_step_[i] = request->uiopen_step(i);
    }

    for (int i = 0; i < request->routine_index_size(); ++i)
    {
    	int type = 3;
    	switch (i)
    	{
    	case 0 :
    		type = TaskInfo::TASK_GTYPE_ROUTINE;
    		break;
    	case 1 :
    		type = TaskInfo::TASK_GTYPE_OFFER_ROUTINE;
    		break;
    	case 2 :
    		type = TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE;
    		break;
    	default :
    		type = TaskInfo::TASK_GTYPE_ROUTINE;
    	}

    	this->routine_task_index_[type] = request->routine_index(i);
    	this->is_finish_all_routine_[type] = request->is_finish_all_routine(i);
    	this->is_second_routine_[type] = request->is_second_routine(i);
    	this->routine_total_num_[type] = request->total_num(i);
    	this->is_routine_task_[type] = request->is_routine_task(i);
    }

    this->routine_refresh_tick_.sec(request->routine_refresh_tick());
    this->lcontri_day_tick_.sec(request->lcontri_tick());
    this->lcontri_day_ = request->lcontri_day();
    this->ui_version_ = request->ui_version();

    for (int i = 0; i < request->routine_consume_history_size(); ++i)
        this->routine_consume_history_.insert(request->routine_consume_history(i));
//    for (int i = 0; i < request->open_ui_list_size(); ++i)
//        this->open_ui_set().insert(request->open_ui_list(i));

    return 0;
}

int MapLogicTasker::notify_finished_task_set(void)
{
//	MapLogicPlayer* player = this->task_player();
//    if (GameCommon::is_normal_scene(player->scene_id()))
//    {
//    	this->check_current_version_open_ui(player->role_level(),
//    			GameEnum::CHECK_UIT_LEVELUP);
//    }
//
//	Proto81400106 respond;
//    for (OpenUiSet::iterator iter = this->open_ui_set().begin();
//    		iter != this->open_ui_set().end(); ++iter)
//    {
//        respond.add_pass_ui(*iter);
//    }
//
//    Proto31400602 inner_req;
//    inner_req.set_msg_body(respond.SerializeAsString());
//    return player->send_to_map_thread(inner_req);
	return 0;
}

int MapLogicTasker::request_fetch_novice_step(void)
{
	Proto51400333 respond;
	respond.set_step(this->novice_step_);
	return this->task_player()->respond_to_client(RETURN_FETCH_NOVICE_STEP, &respond);
}

int MapLogicTasker::request_set_novice_step(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto11400334 *, request, msg, -1);
	this->novice_step_ = request->step();
	return this->task_player()->respond_to_client(RETURN_SET_NOVICE_STEP);
}

int MapLogicTasker::novice_step(void)
{
	return this->novice_step_;
}

void MapLogicTasker::set_novice_step(const int step)
{
	this->novice_step_ = step;
}

int MapLogicTasker::latest_main_task(void)
{
	return this->lastest_main_task_id_;
}

void MapLogicTasker::set_latest_main_task(const int id)
{
	this->lastest_main_task_id_ = id;
}

int MapLogicTasker::lcontri_day_value(void)
{
	return this->lcontri_day_;
}

MapLogicTasker::OpenUiSet &MapLogicTasker::open_ui_set(void)
{
	return this->open_ui_set_;
}

int MapLogicTasker::ui_version(void)
{
    return this->ui_version_;
}

int MapLogicTasker::request_fetch_uiopen_step(void)
{
	Proto51400336 respond;
	for (int i = 0; i <= GameEnum::UIOPEN_STEP_SIZE; ++i)
	{
		respond.add_step(this->uiopen_step_[i]);
	}
	return this->task_player()->respond_to_client(RETURN_FETCH_UIOPEN_STEP, &respond);
}

int MapLogicTasker::request_set_uiopen_step(Message *msg)
{
	MapLogicPlayer *player = this->task_player();
    DYNAMIC_CAST_RETURN(Proto11400337 *, request, msg, -1);

    this->set_uiopen_step(request->step());
    return player->respond_to_client(RETURN_SET_UIOPEN_STEP);
}

void MapLogicTasker::set_uiopen_step(const int step)
{
	if (step < 0 || step > GameEnum::UIOPEN_STEP_SIZE)
		return;

	this->uiopen_step_[step] = 1;
}

int MapLogicTasker::start_script_wave_task_listen(int scene_id)
{
	for (TaskMap::iterator iter = this->task_map_.begin();
			iter != this->task_map_.end(); ++iter)
	{
		TaskInfo *task_info = iter->second;
		JUDGE_CONTINUE(task_info->is_accepted() == true);
		JUDGE_CONTINUE(task_info->is_logic_script_wave() == true);
		JUDGE_CONTINUE(task_info->__condition_list.empty() == false);

		TaskConditon* cond = task_info->__condition_list[0];
		JUDGE_CONTINUE(cond->__cond_id == scene_id)

		Proto31400601 req;
		req.set_script_sort(scene_id);
		this->task_player()->send_to_map_thread(req);
	}
	return 0;
}

int MapLogicTasker::check_script_wave_task_finish(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400601*, request, -1);

	int scene_id = request->script_sort();
	int passed = request->pass_chapter();

	for (TaskMap::iterator iter = this->task_map_.begin();
			iter != this->task_map_.end(); ++iter)
	{
		TaskInfo *task_info = iter->second;
		JUDGE_CONTINUE(task_info->is_accepted() == true);
		JUDGE_CONTINUE(task_info->is_logic_script_wave() == true);
		JUDGE_CONTINUE(task_info->__condition_list.empty() == false);

		TaskConditon* cond = task_info->__condition_list[0];
		JUDGE_CONTINUE(cond->__cond_id == scene_id);
		JUDGE_CONTINUE(passed >= cond->__final_value);

		task_info->task_imp()->process_finish(task_info);
		this->task_player()->request_force_exit_system(scene_id);
	}

	return 0;
}

int MapLogicTasker::test_special_task(int task_id)
{
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
    JUDGE_RETURN(task_json != Json::Value::null, -1);

    this->task_submited_once_.clear();
    this->task_accepted_lvl_.clear();
    this->task_accepted_monster_.clear();
    this->task_accepted_collect_.clear();
    this->task_accepted_attend_.clear();
    this->task_accepted_package_.clear();
    this->task_accepted_scene_.clear();
    this->task_listen_item_.clear();
    this->task_accepted_branch_.clear();

    int prev_task_id = task_json["precondition"].asInt();
    this->lastest_main_task_id_ = prev_task_id;

    for (TaskMap::iterator iter = this->task_map_.begin(); iter != this->task_map_.end(); ++iter)
    {
        this->notify_task_del(iter->second);
        MAP_MONITOR->task_info_pool()->push(iter->second);
    }
    this->task_map_.clear();
    

    while (prev_task_id > 0)
    {
        this->task_submited_once_.insert(prev_task_id);

        const Json::Value &prev_task_json = CONFIG_INSTANCE->find_task(prev_task_id);
        prev_task_id = prev_task_json["precondition"].asInt();
    }

    this->insert_task(task_id, task_json);
    return 0; 
}

int MapLogicTasker::test_task_open_ui(int task_id)
{
	if (task_id != 0)
	{
		this->check_current_version_open_ui(task_id, GameEnum::CHECK_UIT_SUBMIT_TASK);
		return 0;
	}

	for (BStrIntMap::iterator iter = CONFIG_INSTANCE->task_fun_open_map().begin();
			iter != CONFIG_INSTANCE->task_fun_open_map().end(); ++iter)
	{
		this->check_current_version_open_ui(iter->second, GameEnum::CHECK_UIT_SUBMIT_TASK);
	}
	return 0;
}

int MapLogicTasker::fetch_routine_monster_id(int type, int range_level, int monster_id)
{
//	range_level = this->task_player()->role_level();
	const Json::Value& lvl_json = CONFIG_INSTANCE->role_level(0, range_level);
	JUDGE_RETURN(lvl_json != Json::Value::null, -1);

	switch(type)
	{
	case TaskInfo::TASK_GTYPE_ROUTINE:
	{
		string c_str = "daily_monster_list";
		int size = (int)lvl_json[c_str].size();
		JUDGE_RETURN(size > 0, 0);
		int rand_index = ::std::rand() % size;
		return lvl_json[c_str][rand_index].asInt();
	}
	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
	{
		string c_str = "league_monster_list";
		int size = (int)lvl_json[c_str].size();
		JUDGE_RETURN(size > 0, 0);
		int rand_index = ::std::rand() % size;
		return lvl_json[c_str][rand_index].asInt();
	}
	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
	{
		string c_str = "offer_monster_list";
		int size = (int)lvl_json[c_str].size();
		for (int i = 0; i < size; ++i)
			if (lvl_json[c_str][i].asInt() == monster_id) return 1;
		return 0;
	}
	}
	return 0;
}

void MapLogicTasker::refresh_routine_task(const Time_Value &nowtime, const bool notify/*= true*/)
{
    JUDGE_RETURN(this->routine_refresh_tick_ <= nowtime, ;);

    this->routine_refresh_tick_ = next_day(0, 0, nowtime);
    this->clear_routine_task(notify, TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE);
    this->clear_routine_task(notify, TaskInfo::TASK_GTYPE_OFFER_ROUTINE);
    this->clear_routine_task(notify);
}

int MapLogicTasker::open_league_task(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400324 *, request, -1);

    this->task_player()->role_detail().__league_id = request->league_id();
    this->league_task_construct_routine(true);
	this->task_listen_branch(GameEnum::BRANCH_LEAGUE, 1);
	return 0;
}

int MapLogicTasker::league_task_construct_routine(const bool notify_add)
{
	JUDGE_RETURN(this->is_routine_task_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] == 0, 0);
	JUDGE_RETURN(this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] == 0, 0);
	JUDGE_RETURN(this->is_finish_guide_task(), 0);

	//开放等级
    int start_level = CONFIG_INSTANCE->task_setting()["league_routine_start_level"].asInt();
    JUDGE_RETURN(this->task_player()->role_level() >= start_level, 0);
    JUDGE_RETURN(this->task_player()->role_detail().__league_id != 0, 0);

    this->clear_routine_task(true, TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE);

    TaskIdSet t_consume_set;

    this->routine_task_index_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = 0;

    int size = (int)this->task_list_json()["league_routine_list"].size();
    this->routine_total_num_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = size;
    int task_id = this->task_list_json()["league_routine_list"]
        [this->routine_task_index_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE]].asInt();


    // 插入第一个帮派日常任务
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);

    if (notify_add == true)
    	this->insert_task(task_id, task_json);
    else
        this->init_task(task_id, task_json);

    this->is_routine_task_[TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE] = 1;

    return 0;
}

int MapLogicTasker::offer_task_construct_routine(const bool notify_add)
{
	JUDGE_RETURN(this->is_routine_task_ [TaskInfo::TASK_GTYPE_OFFER_ROUTINE]== 0, 0);
	JUDGE_RETURN(this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] == 0, 0);
	JUDGE_RETURN(this->is_finish_guide_task(), 0);

	//开放等级
    int start_level = CONFIG_INSTANCE->task_setting()["offer_routine_start_level"].asInt();
    JUDGE_RETURN(this->task_player()->role_level() >= start_level, 0);
    JUDGE_RETURN(this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_ROUTINE] == 1, 0);

    this->clear_routine_task(true, TaskInfo::TASK_GTYPE_OFFER_ROUTINE);

    TaskIdSet t_consume_set;

    this->routine_task_index_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = 0;

    int size = (int)this->task_list_json()["offer_routine_list"].size();
    if (size <= 0) return -1;
    this->routine_total_num_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = size;
    int task_id = this->task_list_json()["offer_routine_list"]
             [this->routine_task_index_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE]].asInt();


    // 插入第一个悬赏日常任务
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);

    if (notify_add == true)
    	this->insert_task(task_id, task_json);
    else
        this->init_task(task_id, task_json);

    this->is_routine_task_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] = 1;

    return 0;
}

int MapLogicTasker::new_task_construct_routine(const bool notify_add)
{
	JUDGE_RETURN(this->is_routine_task_[TaskInfo::TASK_GTYPE_ROUTINE] == 0, 0);
	JUDGE_RETURN(this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_ROUTINE] == 0, 0);
	JUDGE_RETURN(this->is_finish_guide_task(), 0);
	//开放等级
    int start_level = CONFIG_INSTANCE->task_setting()["daliy_routine_start_level"].asInt();
    JUDGE_RETURN(this->task_player()->role_level() >= start_level, 0);

    this->clear_routine_task(true);

    TaskIdSet t_consume_set;

    this->routine_task_index_[TaskInfo::TASK_GTYPE_ROUTINE] = 0;

    int size = (int)this->task_list_json()["daliy_routine_list"].size();
    this->routine_total_num_[TaskInfo::TASK_GTYPE_ROUTINE] = size;
    int task_id = this->task_list_json()["daliy_routine_list"]
            [this->routine_task_index_[TaskInfo::TASK_GTYPE_ROUTINE]].asInt();


    // 插入第一个日常任务
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);

    if (notify_add == true)
    	this->insert_task(task_id, task_json);
    else
        this->init_task(task_id, task_json);

    this->is_routine_task_[TaskInfo::TASK_GTYPE_ROUTINE] = 1;

    return 0;
}
/*
int MapLogicTasker::task_construct_routine(const bool notify_add)
{
	JUDGE_RETURN(this->is_routine_task_ == 0, 0);

	//开放等级
    int start_level = CONFIG_INSTANCE->task_setting()["daliy_routine_start_level"].asInt();
    JUDGE_RETURN(this->task_player()->role_level() >= start_level, 0);

    this->clear_routine_task(true);

    this->notify_delete_virtual_routine_task();


    MapLogicPlayer *player = this->task_player();

    RoutineTaskVec t_liveness_vc, t_script_vc, t_arena_vc, t_consume_vc, t_other_vc, t_rand_vc, t_other_scene_vc;
    TaskIdSet t_consume_set;
    int level = this->task_player()->role_level();

    const GameConfig::ConfigMap &routine_task_map = CONFIG_INSTANCE->routine_task_map();
    for (GameConfig::ConfigMap::const_iterator iter = routine_task_map.begin();
            iter != routine_task_map.end(); ++iter)
    {
        const Json::Value &task_json = *(iter->second);

        {
            const Json::Value &before_level_json = task_json["before_accept"]["level"];
            if (level < before_level_json[0u].asInt())
                continue;
            if (level < before_level_json[1u].asInt() || before_level_json[2u].asInt() < level)
                continue;
        }
        int task_sub_type = task_json["sub_type"].asInt(), task_belong_scene = task_json["exec"]["belong_scene"].asInt();
        if (task_belong_scene > 0)
        {
            int jump_step = CONFIG_INSTANCE->scene_jump(player->scene_id(), task_belong_scene);
            if ((jump_step < 0 || jump_step > 3) && task_sub_type == TaskInfo::TASK_ST_ROUTINE_OTHER)
            {
            	t_other_scene_vc.push_back(iter->first);
                continue;
            }
        }
		switch (task_sub_type)
		{
			case TaskInfo::TASK_ST_ROUTINE_LIVENESS:
				t_liveness_vc.push_back(iter->first);
				break;
			case TaskInfo::TASK_ST_ROUTINE_SCRIPT:
				t_script_vc.push_back(iter->first);
				break;
			case TaskInfo::TASK_ST_ROUTINE_ARENA:
				t_arena_vc.push_back(iter->first);
				break;
			case TaskInfo::TASK_ST_ROUTINE_CONSUME:
				t_consume_vc.push_back(iter->first);
				t_consume_set.insert(iter->first);
				break;
			case TaskInfo::TASK_ST_ROUTINE_OTHER:
				t_other_vc.push_back(iter->first);
				break;
		}
    }

    // 每种子类型任务至少取一个, 同一个任务当天不出现两次
    IntSet selected_task_set;
    if (t_script_vc.size() > 0)
    {
        int index = rand() % t_script_vc.size();
        this->routine_task_vc_.push_back(t_script_vc[index]);
        selected_task_set.insert(t_script_vc[index]);
        for (int i = 0; i < int(t_script_vc.size()); ++i)
        {
            if (i == index)
                continue;
            t_rand_vc.push_back(t_script_vc[i]);
        }
    }
    if (t_arena_vc.size() > 0)
    {
        int index = rand() % t_arena_vc.size();
        this->routine_task_vc_.push_back(t_arena_vc[index]);
        selected_task_set.insert(t_arena_vc[index]);
        for (int i = 0; i < int(t_arena_vc.size()); ++i)
        {
            if (i == index)
                continue;
            t_rand_vc.push_back(t_arena_vc[i]);
        }
    }
    // 消费任务不重复前几天的任务
    if (t_consume_vc.size() > 0)
    {
        if (t_consume_vc.size() <= this->routine_consume_history_.size())
            this->routine_consume_history_.clear();
        int index = rand() % t_consume_vc.size(), i = index;
        do {
            int consume_task = t_consume_vc[i];
            if (this->routine_consume_history_.find(consume_task) == this->routine_consume_history_.end())
            {
                this->routine_consume_history_.insert(consume_task);
                this->routine_task_vc_.push_back(consume_task);
                selected_task_set.insert(consume_task);
                break;
            }
            i = (i + 1) % t_consume_vc.size();
        } while(i != index);
        for (int j = 0; j < int(t_consume_vc.size()); ++j)
        {
            int consume_task = t_consume_vc[j];
            if (this->routine_consume_history_.find(consume_task) != this->routine_consume_history_.end())
                continue;
            t_rand_vc.push_back(consume_task);
        }
    }

    // 日常任务数量控制
    int routine_max_size = CONFIG_INSTANCE->task_setting()["daliy_routine_number"].asInt();
    if (routine_max_size <= 0)
    	routine_max_size = 7;
    if (t_liveness_vc.size() > 0)
        --routine_max_size;
    int left_size = routine_max_size - this->routine_task_vc_.size();

    if (t_other_vc.size() > 0)
    {
        for (RoutineTaskVec::iterator iter = t_other_vc.begin(); 
                iter != t_other_vc.end(); ++iter)
        {
            t_rand_vc.push_back(*iter);
        }
    }
    if (left_size > int(t_rand_vc.size()))
    {
    	std::random_shuffle(t_other_scene_vc.begin(), t_other_scene_vc.end());
    	int need_size = left_size - t_rand_vc.size();
    	for (int i = 0; i < int(t_other_scene_vc.size()) && i < need_size; ++i)
    		t_rand_vc.push_back(t_other_scene_vc[i]);
    }
    std::random_shuffle(t_rand_vc.begin(), t_rand_vc.end());

    for (int i = 0; i < left_size && i < int(t_rand_vc.size()); ++i)
    {
        this->routine_task_vc_.push_back(t_rand_vc[i]);
    }
    std::random_shuffle(this->routine_task_vc_.begin(), this->routine_task_vc_.end());
    
    if (t_liveness_vc.size() > 0)
    {
        int index = rand() % t_liveness_vc.size();
        this->routine_task_vc_.push_back(t_liveness_vc[index]);
    }

    // 插入第一个日常任务
    if (this->routine_task_vc_.size() > 0)
    {
    	for (RoutineTaskVec::iterator iter = this->routine_task_vc_.begin(); 
                iter != this->routine_task_vc_.end(); ++iter)
    	{
    		MSG_USER("routine task %s %ld %d", this->task_player()->role_name().c_str(),
    				this->task_player()->role_id(), *iter);
    	}
        this->routine_task_index_ = 0;
        int first_task = this->routine_task_vc_[this->routine_task_index_];
        const Json::Value &task_json = CONFIG_INSTANCE->find_task(first_task);

        if (notify_add == true)
            this->insert_task(first_task, task_json);
        else
            this->init_task(first_task, task_json);

        this->is_routine_task_ = 1;
    }
    return 0;
}
*/
void MapLogicTasker::clear_routine_task(const bool notify/* = true*/, const int type)
{
    std::vector<TaskInfo *> remove_task_vc;
    for (TaskMap::iterator iter = this->task_map_.begin();
            iter != this->task_map_.end(); ++iter)
    {
        TaskInfo *task_info = iter->second;
        if (task_info->__game_type != type)
            continue;

        remove_task_vc.push_back(task_info);
    }
    for (std::vector<TaskInfo *>::iterator iter = remove_task_vc.begin(); 
            iter != remove_task_vc.end(); ++iter)
    {
        TaskInfo *task_info = *iter;
        if (task_info->task_imp() != 0)
        {
        	TaskImplement *task_imp = task_info->task_imp();
        	task_imp->remove_task_listen(task_info);
        	task_imp->remove_task_info(task_info, notify);
        }
    }
//    if (notify == false) return ;

	this->routine_task_index_[type] = -1;
	this->is_finish_all_routine_[type] = 0;
	this->is_routine_task_[type] = 0;

}

TaskInfo *MapLogicTasker::init_routine_task(const int task_id, const Json::Value &task_json)
{
    TaskInfo *task_info = MAP_MONITOR->task_info_pool()->pop();
    task_info->__task_id = task_id;
    task_info->__game_type = task_json["task_type"].asInt();
    task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTABLE);

    if (this->init_task_object(task_info, task_json) != 0)
    {
        MAP_MONITOR->task_info_pool()->push(task_info);
        return 0;
    }

    return task_info;
}

TaskInfo *MapLogicTasker::init_trial_task(const int task_id, const Json::Value &task_json)
{
//    TaskInfo *task_info = MAP_MONITOR->task_info_pool()->pop();
//    task_info->__task_id = task_id;
//    task_info->__game_type = task_json["task_type"].asInt();
//    task_info->set_task_status(TaskInfo::TASK_STATUS_ACCEPTABLE);
//
//    if (this->init_task_object(task_info, task_json) != 0)
//    {
//        MAP_MONITOR->task_info_pool()->push(task_info);
//        return 0;
//    }
//
//    this->trial_task_set().insert(task_id);
//
//    return task_info;
	return 0;
}

int MapLogicTasker::generate_next_routine_task(int type)
{
	switch (type)
	{
		case  TaskInfo::TASK_GTYPE_ROUTINE :
		{
			++this->routine_task_index_[type];
			//资源找回
			this->task_player()->refresh_restore_info(GameEnum::ES_ACT_DAILY_ROUTINE,
					this->task_player()->role_level(), 1);
			//成就
			this->task_player()->update_achieve_info(GameEnum::DAILY_TASK, 1);

			if (this->routine_task_index_[type] >= this->routine_total_num_[type])
			{
				this->is_finish_all_routine_[type] = 1;
				this->routine_task_index_[type] = -1;

			    if (this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] == 0 &&
			    		this->is_routine_task_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] == 0)
			    	this->offer_task_construct_routine(true);	//接悬赏任务

				return ERROR_ROUTINE_FINISH_ALL;
			}

			int next_routine_task = this->task_list_json()["daliy_routine_list"][this->routine_task_index_[type]].asInt();
			const Json::Value &task_json = CONFIG_INSTANCE->find_task(next_routine_task);
            return this->insert_task(next_routine_task, task_json);
		}
		case  TaskInfo::TASK_GTYPE_OFFER_ROUTINE :
		{
			++this->routine_task_index_[type];

			//资源找回
			this->task_player()->refresh_restore_info(GameEnum::ES_ACT_OFFER_ROUTINE,
					this->task_player()->role_level(), 1);
			//成就
			this->task_player()->update_achieve_info(GameEnum::REWARD_TASK, 1);

			if (this->routine_task_index_[type] >= this->routine_total_num_[type])
			{
				this->is_finish_all_routine_[type] = 1;
				this->routine_task_index_[type] = -1;
				return ERROR_ROUTINE_FINISH_ALL;
			}

			int next_routine_task = this->task_list_json()["offer_routine_list"][this->routine_task_index_[type]].asInt();
			const Json::Value &task_json = CONFIG_INSTANCE->find_task(next_routine_task);
            return this->insert_task(next_routine_task, task_json);
		}
		case  TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE :
		{
			++this->routine_task_index_[type];
			//资源找回
			this->task_player()->refresh_restore_info(GameEnum::ES_ACT_LEAGUE_ROUTINE,
					this->task_player()->role_level(), 1);
			//成就
			this->task_player()->update_achieve_info(GameEnum::LEAGUE_TASK, 1);

			if (this->routine_task_index_[type] >= this->routine_total_num_[type])
			{
				this->is_finish_all_routine_[type] = 1;
				this->routine_task_index_[type] = -1;
				return ERROR_ROUTINE_FINISH_ALL;
			}

			int next_routine_task = this->task_list_json()["league_routine_list"][this->routine_task_index_[type]].asInt();
			const Json::Value &task_json = CONFIG_INSTANCE->find_task(next_routine_task);
            return this->insert_task(next_routine_task, task_json);
		}
	}
	return 0;
}
/*
int MapLogicTasker::request_routine_ui_status(void)
{

    MapLogicPlayer *player = this->task_player();

    const int ROUTINE_STATUS_NO_START = 0, ROUTINE_STATUS_DOING = 1, ROUTINE_STATUS_FINISH_ALL = 2;

    Proto51400343 respond;
    if (this->is_routine_task_ == 0)
    {
        if (this->is_finish_all_routine_ == 1)
            respond.set_status(ROUTINE_STATUS_FINISH_ALL);
        else
            respond.set_status(ROUTINE_STATUS_NO_START);
    }
    else
    {
        if (this->is_finish_all_routine_ == 1)
            respond.set_status(ROUTINE_STATUS_FINISH_ALL);
        else
            respond.set_status(ROUTINE_STATUS_DOING);
    }

    return player->respond_to_client(RETURN_ROUTINE_UI_STATUS, &respond);
}
*/
/*
int MapLogicTasker::request_routine_ui_detail(void)
{
    MapLogicPlayer *player = this->task_player();

    // 修正主线没有提交就获取日常的数据
    const Json::Value &task_setting_json = CONFIG_INSTANCE->task_setting();
    int prev_main_task_id = task_setting_json["virtual_routine_appear"].asInt();
    if (this->task_submited_once_.find(prev_main_task_id) == this->task_submited_once_.end())
    {
    	TaskInfo *task_info = this->find_task(prev_main_task_id);
    	if (task_info != NULL)
    	{
    		if (task_info->is_finish() == true || task_info->is_accepted() == true)
    			this->process_submit(prev_main_task_id, 0);
    		else
    			this->process_accept(prev_main_task_id);
    	}
    }

    this->refresh_routine_task(Time_Value::gettimeofday(), true);
    if (this->is_routine_task_ == 0 && this->is_finish_all_routine_ == 0)
    {
        //this->task_construct_routine(true);
    	this->new_task_construct_routine();
    }

    const Json::Value &finish_extra_json = CONFIG_INSTANCE->routine_wish();

    Proto51400344 respond;

    { // 完成时额外奖励经验
        const Json::Value &exp_json = finish_extra_json["exp"];
        int first_level = exp_json["first_level"].asInt();
        if (player->role_level() >= first_level)
            respond.set_extra_exp(GameCommon::json_by_index(exp_json["award"], player->role_level() - first_level).asInt());
    }
    { // 完成时额外奖励的道具
        const Json::Value &item_json = finish_extra_json["item"];
        int first_level = item_json["first_level"].asInt();
        if (player->role_level() >= first_level)
        {
            const Json::Value &level_item_json = GameCommon::json_by_index(item_json["award"], player->role_level() - first_level);
            for (uint i = 0; i < level_item_json.size(); ++i)
            {
                ProtoItem *proto_item = respond.add_extra_item();
                proto_item->set_id(level_item_json[i][0u].asInt());
                proto_item->set_amount(level_item_json[i][1u].asInt());
                proto_item->set_bind(level_item_json[i][2u].asInt());
            }
        }
    }

    {
        // 一键完成环式任务需要的元宝
        int per_task_gold = task_setting_json["daliy_routine_fast_money"].asInt();
        int left_routine_task_amount = this->left_routine_task_amount();
        ProtoMoney *proto_money = respond.mutable_fast_gold();
        proto_money->set_gold(left_routine_task_amount * per_task_gold);
    }
    {
        // 当前环信息
    	int task_index = this->routine_task_index_ + 1;
    	if (task_index >= int(this->routine_task_vc_.size()))
    		task_index = int(this->routine_task_vc_.size());
        respond.set_routine_task_index(task_index);
        respond.set_total_routine_size(task_setting_json["daliy_routine_number"].asInt());
    }
    {
        // 当前环式任务信息
        TaskInfo *task_info = this->current_routine_task();
        if (task_info != NULL && task_info->is_routine_task())
        {
            this->serail_task_info(task_info, respond.mutable_routine_task());

            const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
            // 当前任务奖励信息
            int award_exp = 0;
            Money award_money;
            std::map<int, ItemObj> item_map;
            double star_rate = this->calc_star_to_rate(task_info->__task_star, task_json);
            this->calc_routine_task_award(&award_exp, award_money, item_map, task_json, star_rate);

            award_money.serialize(respond.mutable_task_award_money());
            respond.set_task_award_exp(award_exp);
            for (std::map<int, ItemObj>::iterator iter = item_map.begin(); iter != item_map.end(); ++iter)
            {
                ItemObj &item_obj = iter->second;
                item_obj.serialize(respond.add_task_award_item());
            }

#ifdef TEST_COMMAND
            char msg[514] = {0,};
            ::sprintf(msg, "当前日常ID: %d", task_info->__task_id);
            this->task_player()->test_msg_in_chat(msg);
#endif
        }
    }
    // 双倍奖励花费
    {
        int double_award_need_money = task_setting_json["routine_double_award_money"].asInt();
        ProtoMoney *proto_money = respond.mutable_double_award_money();
        proto_money->set_gold(double_award_need_money);
    }
    //奖励礼包
    {
    TaskInfo *task_info = this->current_routine_task();
    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_info->__task_id);
    int award_id = task_json["award"]["award_id"].asInt();
    
    respond.set_extra_award_id(GameEnum::ROUTINE_TASK_EXTRA_AWARD_NUM);
    respond.set_award_id(award_id);
    }
    return player->respond_to_client(RETURN_ROUTINE_UI_DETAIL, &respond);
}
*/
int MapLogicTasker::routine_task_index(int type)
{
	return this->routine_task_index_[type];
}

int MapLogicTasker::routine_total_num(int type)
{
	return this->routine_total_num_[type];
}

int MapLogicTasker::left_routine_task_amount(int type)
{
	int routine_amount = this->routine_total_num_[type];

	if (this->is_routine_task_[type] == 0)
    {
	    if (this->is_finish_all_routine_[type] == 0)
	    	return routine_amount;
	    else
	    	return 0;
    }
	else
	{
		if (this->is_finish_all_routine_[type] == 1)
	    	return 0;

	    int left_size = routine_amount - this->routine_task_index_[type];
//	    TaskInfo *current_routine = this->current_routine_task(type);
//	    if (current_routine != NULL && (current_routine->is_finish() == true || current_routine->is_submit() == true))
//	    	--left_size;

//	    if (left_size < 0) left_size = 0;
	    return left_size;
	}
    return 0;
}

const Json::Value &MapLogicTasker::task_list_json()
{
	int level = this->task_player()->role_level();
	return CONFIG_INSTANCE->role_level(0,level);
}

int MapLogicTasker::current_routine_task_id(int type)
{
    switch(type)
    {
    	case TaskInfo::TASK_GTYPE_ROUTINE:
    		return this->task_list_json()["daliy_routine_list"][this->routine_task_index_[type]].asInt();
    	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
    		return this->task_list_json()["offer_routine_list"][this->routine_task_index_[type]].asInt();
    	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
    		return this->task_list_json()["league_routine_list"][this->routine_task_index_[type]].asInt();
    }
    return 0;
}

TaskInfo *MapLogicTasker::current_routine_task(int type)
{
    int task_id = this->current_routine_task_id(type);
    if (task_id <= 0)
        return NULL;
    return this->find_task(task_id);
}

bool MapLogicTasker::is_last_routine_task(int type)
{
	if (this->routine_task_index_[type] == this->routine_total_num_[type])
		return true;
	return false;
}

int MapLogicTasker::request_routine_world_chat(void)
{
	return 0;
}

int MapLogicTasker::request_routine_open_market(void)
{
    return 0;
}

int MapLogicTasker::refresh_daily_league_contri(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->lcontri_day_tick_ < nowtime, 0);

    this->lcontri_day_tick_ = next_day(0, 0, nowtime);
    this->lcontri_day_ = 0;

    return 0;
}

int MapLogicTasker::routine_fast_finish(const Json::Value &effect, PackageItem *pack_item)
{
    bool routine_accepted = true;
    TaskInfo *task_info = 0;
    for (TaskMap::iterator iter = this->task_map_.begin(); iter != this->task_map_.end(); ++iter)
    {
        TaskInfo *info = iter->second;
        if (info->is_routine_task() == false)
            continue;
        if (info->is_accepted() == false)
        {
            routine_accepted = false;
            continue;
        }

        task_info = info;
        break;
    }
    if (task_info == 0)
    {
        if (routine_accepted == false)
            return ERROR_TASK_NO_ACCEPT;
        return ERROR_NO_TASK_CAN_FINISH;
    }

    int ret = task_info->__task_imp->process_finish(task_info);
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int MapLogicTasker::update_open_ui_set(const Json::Value &json)
{
//    const Json::Value &accept_json = json["accept"];
//    for (uint i = 0; i < accept_json.size(); ++i)
//    {
//        int task_id = accept_json[i][0u].asInt();
//        if (this->is_task_submited(task_id) == true)
//        {
//            this->open_ui_set().insert(accept_json[i][1u].asString());
//            continue;
//        }
//        TaskInfo *task_info = this->find_task(task_id);
//        if (task_info != NULL && task_info->is_accepted() == true)
//            this->open_ui_set().insert(accept_json[i][1u].asString());
//    }
//    const Json::Value &finish_json = json["finish"];
//    for (uint i = 0; i < finish_json.size(); ++i)
//    {
//        int task_id = finish_json[i][0u].asInt();
//        if (this->is_task_submited(task_id) == true)
//        {
//            this->open_ui_set().insert(finish_json[i][1u].asString());
//            continue;
//        }
//        TaskInfo *task_info = this->find_task(task_id);
//        if (task_info != NULL && task_info->is_finish() == true)
//            this->open_ui_set().insert(finish_json[i][1u].asString());
//    }
//    const Json::Value &submit_json = json["submit"];
//    for (uint i = 0; i < submit_json.size(); ++i)
//    {
//        int task_id = submit_json[i][0u].asInt();
//        if (this->is_task_submited(task_id))
//            this->open_ui_set().insert(submit_json[i][1u].asString());
//    }
//    const Json::Value &level_json = json["levelUP"];
//    for (uint i = 0; i < level_json.size(); ++i)
//    {
//        int level = level_json[i][0u].asInt();
//        if (this->task_player()->role_level() >= level)
//            this->open_ui_set().insert(level_json[i][1u].asString());
//    }
//    const Json::Value &otherUI_json = json["otherUI"];
//    {
//    	if(otherUI_json.isMember("mount_grade"))
//    	{
//    		MountDetail& mount = this->task_player()->mount_detail();
//			int condition = otherUI_json["mount_grade"][0u].asInt();
//			if(mount.mount_grade_ >= condition)
//				 this->open_ui_set().insert(otherUI_json["mount_grade"][1u].asString());
//
//    	}
//    }

    return 0;
}

int MapLogicTasker::check_and_update_open_ui_by_sign(void)
{
//    const Json::Value &prev_version_json = CONFIG_INSTANCE->task_setting()["novice_open_ui"]["prev_version"];
//    const Json::Value &cur_version_json = CONFIG_INSTANCE->task_setting()["novice_open_ui"]["current_version"];
//    if (this->ui_version_ != cur_version_json["version"].asInt())
//    {
//        this->update_open_ui_set(prev_version_json);
//        this->ui_version_ = cur_version_json["version"].asInt();
//    }
//
//    this->update_open_ui_set(cur_version_json);
    return 0;
}

int MapLogicTasker::check_finish_task(int id)
{
	JUDGE_RETURN(id > 0, true);
	return this->finish_task_.count(id) > 0;
}

int MapLogicTasker::check_current_version_open_ui(const int id, const int type,const char* key)
{
    MapLogicPlayer *player = this->task_player();
    if (type == GameEnum::CHECK_UIT_FINISH_TASK)
    {
    }
    else if (type == GameEnum::CHECK_UIT_SUBMIT_TASK)
    {
    	const Json::Value& task_json = CONFIG_INSTANCE->find_task(id);
        if (task_json.isMember("skill") == true)
        {
        	int sex = player->role_detail().__sex;
        	player->request_sync_map_skill(task_json["skill"][sex - 1].asInt());
        }

        if (task_json.isMember("save") == true)
        {
        	player->insert_finish_task(id);
        }

        //通过完成任务开启功能
        player->open_smelt(id);
    	player->mount_try_task(id);
    	player->spool_handle_player_task(id);
    	player->fashion_handle_player_task(id);
    	player->transfer_handle_player_task(id);
    	player->check_league_open_task(id);
        player->change_magic_weapon_status(GameEnum::RAMA_GET_FROM_TASK, id);
        player->check_open_rama(id);
        player->check_open_illustration(id);
    }

    return 0;
}

int MapLogicTasker::request_refresh_task_star(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto11400345 *, request, msg, -1);
//
//    MapLogicPlayer *player = this->task_player();
//
//    int task_id = request->task_id();
//    TaskInfo *task_info = this->find_task(task_id);
//    CONDITION_PLAYER_NOTIFY_RETURN(task_info != NULL && task_info->is_routine_task(),
//            RETURN_ROUTINE_FRESH_STAR, ERROR_CLIENT_OPERATE);
//
//    const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
//    CONDITION_PLAYER_NOTIFY_RETURN(task_json.isMember("task_star"),
//            RETURN_ROUTINE_FRESH_STAR, ERROR_CONFIG_NOT_EXIST);
//    CONDITION_PLAYER_NOTIFY_RETURN(task_info->__task_star < int(task_json["task_star"].size()),
//            RETURN_ROUTINE_FRESH_STAR, ERROR_TASK_TOP_STAR);
//
//    const Json::Value &task_setting_json = CONFIG_INSTANCE->task_setting();
//    Money cost;
//    cost.__bind_copper = task_setting_json["routine_star_fresh_copper"].asInt();
//    GameCommon::adjust_money(cost, player->own_money());
//    CONDITION_PLAYER_NOTIFY_RETURN(player->validate_money(cost) == true,
//            RETURN_ROUTINE_FRESH_STAR, ERROR_PACK_BIND_COPPER_AMOUNT);
//
//    player->pack_money_sub(cost, SerialObj(SUB_MONEY_TASK_STAR, task_id));
//
//    this->init_task_star(task_info, task_json);
//
//    ++task_info->__fresh_star_times;
//    if (task_info->__fresh_star_times >= task_setting_json["routine_protect_star_full"][0u].asInt())
//    {
//        task_info->__task_star = int(task_json["task_star"].size());
//    }
//
//    Proto51400345 respond;
//    respond.set_task_id(task_id);
//    respond.set_star_level(task_info->__task_star);
//    return player->respond_to_client(RETURN_ROUTINE_FRESH_STAR, &respond);
	return 0;
}

int MapLogicTasker::request_routine_finish_all(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400346 *, request, -1);

	int type = request->type();
    MapLogicPlayer *player = this->task_player();

    int left_routine_task_amount = this->left_routine_task_amount(type);
    CONDITION_PLAYER_NOTIFY_RETURN(left_routine_task_amount > 0, RETURN_ROUTINE_FINISH_ALL, ERROR_NO_TASK_CAN_FINISH);

    CONDITION_PLAYER_NOTIFY_RETURN(player->vip_type() > 0, RETURN_ROUTINE_FINISH_ALL, ERROR_VIP_LEVEL);
    const Json::Value &vip_json = CONFIG_INSTANCE->vip(player->vip_type());
    CONDITION_PLAYER_NOTIFY_RETURN(vip_json["routine_fast"].asInt() == 1, RETURN_ROUTINE_FINISH_ALL, ERROR_VIP_LEVEL);

    const Json::Value &task_setting_json = CONFIG_INSTANCE->task_setting();
    int per_task_gold = task_setting_json["routine_fast_money"].asInt();

    Money cost;
    cost.__gold = left_routine_task_amount * per_task_gold;
    CONDITION_PLAYER_NOTIFY_RETURN(player->validate_money(cost), RETURN_ROUTINE_FINISH_ALL, ERROR_PACKAGE_GOLD_AMOUNT);
    Money ret;
    int scale = CONFIG_INSTANCE->vip(player->vip_type())["fast_return"].asInt();
    ret.__bind_gold = cost.__gold * scale / BASE_SCALE_NUM;

    Proto51400346 respond;
    respond.set_type(type);
    ProtoMoney* money = respond.mutable_money();
    ProtoMoney*	return_money = respond.mutable_return_money();
    cost.serialize(money);
    ret.serialize(return_money);
    respond.set_scale(scale / 100);

    string type_name;
    int type_index = this->routine_task_index_[type],
    	type_num = this->routine_total_num_[type];
    switch (type)
    {
	case TaskInfo::TASK_GTYPE_ROUTINE:
	{
		type_name = "daliy_routine_list";
		break;
	}
	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
	{
		type_name = "offer_routine_list";
		break;
	}
	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
	{
		type_name = "league_routine_list";
		break;
	}
	default:
		type_name = "daliy_routine_list";
		type_index = this->routine_task_index_[TaskInfo::TASK_GTYPE_ROUTINE];
		type_num = this->routine_total_num_[TaskInfo::TASK_GTYPE_ROUTINE];
		break;
    }

    for (int index = type_index; index < type_num; ++index)
    {
        int task_id = this->task_list_json()[type_name][index].asInt();
        TaskInfo *task = NULL;
        if (this->find_task(task_id) != 0)
        {
        	task = this->find_task(task_id);
        }
        else
        {
            const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
            task = this->init_task(task_id, task_json);
        }

        if (task == NULL) continue;
        TaskRoutineImp *routine_imp = dynamic_cast<TaskRoutineImp *>(task->task_imp());
        routine_imp->new_process_task_reward(task);
        this->routine_task_index_[type]++;

        JUDGE_CONTINUE(task->task_imp() != NULL);
        routine_imp->remove_task_listen(task);
        routine_imp->remove_task_info(task, true);
    }
    this->clear_routine_task(true,type);

	this->routine_task_index_[type] = this->routine_total_num_[type];
	this->is_finish_all_routine_[type] = 1;
	if (type == TaskInfo::TASK_GTYPE_ROUTINE && this->is_finish_all_routine_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] == 0
			&& this->is_routine_task_[TaskInfo::TASK_GTYPE_OFFER_ROUTINE] == 0)
	{
    	this->offer_task_construct_routine(true);	//接悬赏任务
	}


    GameCommon::adjust_money(cost, player->own_money());
    player->pack_money_sub(cost, SerialObj(SUB_MONEY_FAST_FINISH_ROUTINE));
    player->pack_money_add(ret,SerialObj(ADD_FROM_VIP_RETURN));

    for (int index = 0; index < type_num; ++index)
    {
        int task_id = this->task_list_json()[type_name][index].asInt();
        const Json::Value &task_json = CONFIG_INSTANCE->find_task(task_id);
        TaskInfo *task_info = this->init_task(task_id, task_json);
        TaskRoutineImp *routine_imp = dynamic_cast<TaskRoutineImp *>(task_info->task_imp());
        JUDGE_CONTINUE(task_info != NULL);
        JUDGE_CONTINUE(task_info->task_imp() != NULL);
        routine_imp->remove_task_listen(task_info);
        routine_imp->remove_task_info(task_info, true);
    }

    //更新剑池任务
    this->update_sword_pool_info(left_routine_task_amount, type);

    this->update_cornucopia_task_info(left_routine_task_amount, type);

    int es_type = -1, achieve_type = -1;
    switch (type)
    {
	case TaskInfo::TASK_GTYPE_ROUTINE:
	{
		es_type = GameEnum::ES_ACT_DAILY_ROUTINE;
		achieve_type = GameEnum::DAILY_TASK;
		break;
	}
	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
	{
		es_type = GameEnum::ES_ACT_OFFER_ROUTINE;
		achieve_type = GameEnum::REWARD_TASK;
		break;
	}
	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
	{
		es_type = GameEnum::ES_ACT_LEAGUE_ROUTINE;
		achieve_type = GameEnum::LEAGUE_TASK;
		break;
	}
	default:
		break;
    }
	//资源找回
	this->task_player()->refresh_restore_info(es_type, this->task_player()->role_level(), left_routine_task_amount);
	//成就
	this->task_player()->update_achieve_info(achieve_type, left_routine_task_amount);

    return player->respond_to_client(RETURN_ROUTINE_FINISH_ALL, &respond);
}

int MapLogicTasker::calc_routine_task_award(int *award_exp, Money &award_money, std::map<int, ItemObj> &item_map, const Json::Value &task_json, const double real_rate)
{
    MapLogicPlayer *player = this->task_player();

    const Json::Value &level_award_json = task_json["level_award"];
    const Json::Value &level_json = level_award_json["level"];
    
    int level = player->role_level(), level_index = 0;

    for (uint i = 0; i < level_json.size(); ++i)
    {
        if (level_json[i].asInt() == level)
        {
        	level_index = i;
            break;
        }
    }
    if (level_index >= int(level_json.size()))
        level_index = int(level_json.size()) - 1;
    JUDGE_RETURN(level_index >= 0, ERROR_CONFIG_ERROR);

    // 计算任务的等级奖励
    {
        // normal
        const Json::Value &normal_json = level_award_json["normal"];
        if (normal_json.isMember("goods"))
        {
            const Json::Value &goods_json = GameCommon::json_by_index(normal_json["goods"], level_index);
            for (uint i = 0; i < goods_json.size(); ++i)
            {
                int item_id = goods_json[i][0u].asInt();
                ItemObj &item_obj = item_map[item_id];
                item_obj.id_ = item_id;
    
                int item_amount = goods_json[i][1u].asInt();
                if (real_rate > 0.0000001)
                    item_amount = int(item_amount * real_rate);
                item_obj.amount_ += item_amount;
                item_obj.bind_ = 2;
                if (goods_json[i].size() >= 3)
                	item_obj.bind_ = goods_json[i][2u].asInt();
            }
        }

        if (normal_json.isMember("money"))
        {
            const Json::Value &money_json = GameCommon::json_by_index(normal_json["money"], level_index);
            int gold = money_json[0u].asInt(), bind_gold = money_json[1u].asInt(),
                copper = money_json[2u].asInt(), bind_copper = money_json[3u].asInt();

            if (real_rate > 0.000001)
            {
                bind_copper = int(bind_copper * real_rate);
                copper = int(copper * real_rate);
                bind_gold = int(bind_gold * real_rate);
                gold = int(gold * real_rate);
            }

            award_money.__bind_copper += bind_copper;
            award_money.__copper += copper;
            award_money.__bind_gold += bind_gold;
            award_money.__gold += gold;
        }

        // add exp
        if (normal_json.isMember("exp"))
        {
            const Json::Value &exp_json = GameCommon::json_by_index(normal_json["exp"], level_index);
            int inc_exp = exp_json.asInt();
            if (real_rate > 0.000001)
                inc_exp = int(inc_exp * real_rate);
            *award_exp += inc_exp;
        }
    }
    return 0;
}

int MapLogicTasker::calc_routine_extra_award(int *award_exp, Money &award_money, std::map<int, ItemObj> &item_map)
{
    return 0;
}

double MapLogicTasker::calc_star_to_rate(const int star, const Json::Value &task_json)
{
    int star_rate = 0;
    if (star > 0)
    {
        star_rate = GameCommon::json_by_index(task_json["task_star"], star - 1)[1u].asInt();
    }
    if (star_rate > 0)
        return star_rate / 100.0;
    return 0.0;
}

void MapLogicTasker::insert_finish_task(int task_id)
{
	JUDGE_RETURN(task_id > 0, ;);
	this->finish_task_.insert(task_id);
}

int MapLogicTasker::update_sword_pool_info(int num, int type)
{
	Proto31402901 inner_res;
	inner_res.set_left_add_flag(1);
	inner_res.set_left_add_num(num);
	int task_id = 0;

	switch(type)
	{
	case TaskInfo::TASK_GTYPE_ROUTINE:
	{
		task_id = GameEnum::SPOOL_TASK_DAILY;
		break;
	}
	case TaskInfo::TASK_GTYPE_OFFER_ROUTINE:
	{
		task_id = GameEnum::SPOOL_TASK_REWARD;
		break;
	}
	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
	{
		task_id = GameEnum::SPOOL_TASK_LEAGUE;
		break;
	}
	default:
		break;
	}
	inner_res.set_task_id(task_id);

	MSG_USER("MapLogicTasker, update_sword_pool_info, Proto31402901: %s", inner_res.Utf8DebugString().c_str());

	MapLogicPlayer *player = this->task_player();
	JUDGE_RETURN(player != NULL, 0);

	player->update_task_info(&inner_res);

	return 0;
}

int MapLogicTasker::record_serial(int task_id, int serial_type)
{
	MapLogicPlayer *player = this->task_player();
    return SERIAL_RECORD->record_task(player, player->agent_code(), player->market_code(),
            serial_type, task_id, player->role_level());
}

int MapLogicTasker::update_cornucopia_task_info(int num, int type)
{
	MSG_USER("MapLogicTasker::update_cornucopia_task_info start type:%d", type);
	Proto31403200 task_info;
	int task_id = 0;
	switch(type)
	{
	case TaskInfo::TASK_GTYPE_ROUTINE:
	{
		task_id = GameEnum::CORNUCOPIA_TASK_DAILY;
		break;
	}
	case TaskInfo::TASK_GTYPE_LEAGUE_ROUTINE:
	{
		task_id = GameEnum::CORNUCOPIA_TASK_LEAGUE;
		break;
	}
	default:
		return -1;
	}

	if(task_id == 0)
		return -1;
	task_info.set_task_id(task_id);
	task_info.set_task_finish_count(num);
	MSG_USER("MapLogicTasker, update_cornucopia_task_info, Proto31403200: %s", task_info.Utf8DebugString().c_str());
	MapLogicPlayer *player = this->task_player();
	return MAP_MONITOR->dispatch_to_logic(player, &task_info);
}
