/*
 * MapTaskStruct.cpp
 *
 * Created on: 2013-11-07 10:14
 *     Author: lyz
 */

#include "MapTaskStruct.h"
#include "TaskImplement.h"
#include "MapMonitor.h"
#include "ProtoInnerPublic.pb.h"

void TaskConditon::reset(void)
{
    this->__type = TASK_CT_NONE;

    this->__current_value 	= 0;
    this->__cond_id 		= 0;
    this->__final_value 	= 0;
    this->__cond_index 		= -1;
    this->__id_list_index 	= -1;
    this->__kill_type 		= 0;
    this->__range_level     = 0;
}

bool TaskConditon::is_no_condtion(void)
{
    return this->__type == TASK_CT_NONE;
}

bool TaskConditon::is_kill_monster(void)
{
    return this->__type == TASK_CT_KILL_MONSTER;
}

bool TaskConditon::is_collect_item(void)
{
    return this->__type == TASK_CT_COLLECT_ITEM;
}

bool TaskConditon::is_level(void)
{
    return this->__type == TASK_CT_LVL;
}

bool TaskConditon::is_attend(void)
{
    return this->__type == TASK_CT_ATTEND;
}

bool TaskConditon::is_script_wave(void)
{
	return this->__type == TASK_CT_SCRIPT_WAVE;
}

bool TaskConditon::is_npc_dialogue(void)
{
    return this->__type == TASK_CT_NPC_DIALOGUE;
}

bool TaskConditon::is_package_item(void)
{
	return this->__type == TASK_CT_PACKAGE;
}

bool TaskConditon::is_patrol(void)
{
	return this->__type == TASK_CT_PATROL;
}

bool TaskConditon::is_scene(void)
{
	return this->__type == TASK_CT_SCENE;
}

bool TaskConditon::is_collect_any(void)
{
    return this->__type == TASK_CT_COLLECT_ANY;
}

bool TaskConditon::is_lvl_monster(void)
{
    return this->__type == TASK_CT_LVL_MONSTER;
}

bool TaskConditon::is_kill_boss(void)
{
    return this->__type == TASK_CT_BOSS;
}

bool TaskConditon::is_branch(void)
{
	return this->__type == TASK_CT_BRANCH;
}

int TaskConditon::fetch_id(int type, const Json::Value& json)
{
	if (type != TASK_CT_SCRIPT_WAVE)
	{
		return json["id"][0u].asInt();
	}

	if (json.isMember("scene") == true)
	{
		return json["scene"].asInt();
	}
	else
	{
		return json["id"][0u].asInt();
	}
}

int TaskConditon::fetch_type(const string& type_str)
{
	if (type_str == "kill")
	{
		 return TaskConditon::TASK_CT_KILL_MONSTER;
	}
	else if (type_str == "collect")
	{
		return TaskConditon::TASK_CT_COLLECT_ITEM;
	}
	else if (type_str == "talk")
	{
		return TaskConditon::TASK_CT_NPC_DIALOGUE;
	}
	else if (type_str == "scene")
	{
		return TaskConditon::TASK_CT_SCENE;
	}
	else if (type_str == "activity")
	{
		return TaskConditon::TASK_CT_ATTEND;
	}
	else if (type_str == "lvl_up")
	{
		return TaskConditon::TASK_CT_LVL;
	}
	else if (type_str == "package")
	{
		return TaskConditon::TASK_CT_PACKAGE;
	}
	else if (type_str == "patrol")
	{
		return TaskConditon::TASK_CT_PATROL;
	}
	else if (type_str == "collect_any")
	{
		return TaskConditon::TASK_CT_COLLECT_ANY;
	}
	else if (type_str == "monster")
	{
		return TaskConditon::TASK_CT_LVL_MONSTER;
	}
	else if (type_str == "boss")
	{
		return TaskConditon::TASK_CT_BOSS;
	}
	else if (type_str == "script_wave")
	{
		return TaskConditon::TASK_CT_SCRIPT_WAVE;
	}
	else if (type_str == "branch")
	{
		return TaskConditon::TASK_CT_BRANCH;
	}

	return 0;
}

void TaskConditon::serialize(ProtoTaskCond *proto_task_cond)
{
    proto_task_cond->set_index(this->__cond_index);
    proto_task_cond->set_value(this->__current_value);
    proto_task_cond->set_final_value(this->__final_value);
    proto_task_cond->set_cond_id(this->__cond_id);
    proto_task_cond->set_cond_type(this->__type);
    proto_task_cond->set_kill_type(this->__kill_type);
    proto_task_cond->set_range_level(this->__range_level);
}

void TaskConditon::serialize(ProtoInnerTaskCond *proto_task_cond)
{
    proto_task_cond->set_type(this->__type);
    proto_task_cond->set_current_value(this->__current_value);
    proto_task_cond->set_cond_index(this->__cond_index);
    proto_task_cond->set_id_list_index(this->__id_list_index);
    proto_task_cond->set_cond_id(this->__cond_id);
    proto_task_cond->set_final_value(this->__final_value);
    proto_task_cond->set_kill_type(this->__kill_type);
    proto_task_cond->set_range_level(this->__range_level);
}

void TaskConditon::unserialize(const ProtoInnerTaskCond &proto_task_cond)
{
    this->__type = proto_task_cond.type();
    this->__current_value = proto_task_cond.current_value();
    this->__cond_index = proto_task_cond.cond_index();
    this->__id_list_index = proto_task_cond.id_list_index();
    this->__cond_id = proto_task_cond.cond_id();
    this->__final_value = proto_task_cond.final_value();
    this->__kill_type = proto_task_cond.kill_type();
    this->__range_level = proto_task_cond.range_level();
}

TaskInfo::TaskInfo(void) :
    __task_id(0), __game_type(0), __prev_task(0), 
    __post_task(0), __task_star(0), __fast_finish_rate(0),
    __fresh_star_times(0), __task_imp(0)
{ /*NULL*/ }

void TaskInfo::reset(void)
{
    this->__task_id = 0;
    this->__game_type = TASK_GTYPE_NONE;

    this->__accept_tick = Time_Value::zero;
    this->__refresh_tick = Time_Value::zero;
 
    this->__prev_task = 0;
    this->__post_task = 0;

    this->__status.reset();
    this->__logic_type.reset();

    this->__task_star = 0;
    this->__fast_finish_rate = 0;
    this->__fresh_star_times = 0;

    if (this->__task_imp != 0)
        this->__task_imp->recycle_self();
    this->__task_imp = 0;

    for (TaskConditionList::iterator iter = this->__condition_list.begin();
            iter != this->__condition_list.end(); ++iter)
    {
        MAP_MONITOR->task_condition_pool()->push(*iter);
    }
    this->__condition_list.clear();
}

bool TaskInfo::is_visible(void)
{
    return this->__status.test(TASK_STATUS_VISIBLE);
}

bool TaskInfo::is_acceptable(void)
{
    return this->__status.test(TASK_STATUS_ACCEPTABLE);
}

bool TaskInfo::is_accepted(void)
{
    return this->__status.test(TASK_STATUS_ACCEPTED);
}

bool TaskInfo::is_finish(void)
{
    return this->__status.test(TASK_STATUS_FINISH);
}

bool TaskInfo::is_submit(void)
{
    return this->__status.test(TASK_STATUS_SUBMIT);
}

void TaskInfo::set_task_status(const int status)
{
    if (status == TASK_STATUS_ACCEPTED)
    {
        this->__status.reset(TASK_STATUS_ACCEPTABLE);
    }
    else if (status == TASK_STATUS_FINISH)
    {
        this->__status.reset(TASK_STATUS_ACCEPTABLE);
        this->__status.reset(TASK_STATUS_ACCEPTED);
    }
    else if (status == TASK_STATUS_SUBMIT)
    {
        this->__status.reset(TASK_STATUS_ACCEPTABLE);
        this->__status.reset(TASK_STATUS_ACCEPTED);
    }
    else if (status == TASK_STATUS_ACCEPTABLE)
    {
        this->__status.set(TASK_STATUS_VISIBLE);
    }
    this->__status.set(status, true);
}

void TaskInfo::reset_task_status(const int status)
{
    this->__status.reset(status);
}

bool TaskInfo::is_main_task(void)
{
    return this->__game_type == TASK_GTYPE_MAINLINE;
}

bool TaskInfo::is_branch_task(void)
{
    return this->__game_type == TASK_GTYPE_BRANCH;
}

bool TaskInfo::is_routine_task(void)
{
    return (this->__game_type == TASK_GTYPE_ROUTINE ||
    		this->__game_type == TASK_GTYPE_OFFER_ROUTINE ||
    		this->__game_type == TASK_GTYPE_LEAGUE_ROUTINE);
}

bool TaskInfo::is_league_task(void)
{
    return this->__game_type == TASK_GTYPE_LEAGUE;
}

void TaskInfo::set_logic_type(const int type)
{
    this->__logic_type.set(type, true);
}

void TaskInfo::reset_logic_type(const int type)
{
    this->__logic_type.reset(type);
}

bool TaskInfo::is_logic_kill_monster(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_KILL_MONSTER);
}

bool TaskInfo::is_logic_collect_item(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_COLLECT_ITEM);
}

bool TaskInfo::is_logic_npc_dialogue(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_NPC_DIALOGUE);
}

bool TaskInfo::is_logic_level_up(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_LVL);
}

bool TaskInfo::is_logic_attend(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_ATTEND);
}

bool TaskInfo::is_logic_script_wave(void)
{
	return this->__logic_type.test(TaskConditon::TASK_CT_SCRIPT_WAVE);
}

bool TaskInfo::is_logic_package(void)
{
	return this->__logic_type.test(TaskConditon::TASK_CT_PACKAGE);
}

bool TaskInfo::is_logic_patrol(void)
{
	return this->__logic_type.test(TaskConditon::TASK_CT_PATROL);
}

bool TaskInfo::is_logic_scene(void)
{
	return this->__logic_type.test(TaskConditon::TASK_CT_SCENE);
}

bool TaskInfo::is_logic_collect_any(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_COLLECT_ANY);
}

bool TaskInfo::is_logic_lvl_monster(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_LVL_MONSTER);
}

bool TaskInfo::is_logic_kill_boss(void)
{
    return this->__logic_type.test(TaskConditon::TASK_CT_BOSS);
}

bool TaskInfo::is_logic_branch(void)
{
	return this->__logic_type.test(TaskConditon::TASK_CT_BRANCH);
}

TaskImplement *TaskInfo::task_imp(void)
{
    return this->__task_imp;
}

int TaskInfo::condition_size(void)
{
    return this->__condition_list.size();
}

TaskConditon *TaskInfo::task_condition(const int index)
{
    if (index < 0 || index >= int(this->__condition_list.size()))
        return 0;
    return this->__condition_list[index];
}

int TaskInfo::task_status(void)
{
    if (this->is_submit())
    {
        return TASK_STATUS_SUBMIT;
    }
    else if (this->is_finish())
    {
        return TASK_STATUS_FINISH;
    }
    else if (this->is_accepted())
    {
        return TASK_STATUS_ACCEPTED;
    }
    else if (this->is_acceptable())
    {
        return TASK_STATUS_ACCEPTABLE;
    }
    else if (this->is_visible())
    {
        return TASK_STATUS_VISIBLE;
    }
    else
    {
        return TASK_STATUS_NONE;
    }
}

const Json::Value& TaskInfo::conf()
{
	return CONFIG_INSTANCE->find_task(this->__task_id);
}

void TaskInfo::serialize(ProtoTaskInfo *proto_task)
{
    proto_task->set_task_id(this->__task_id);
    proto_task->set_type(this->__game_type);
    proto_task->set_status(this->task_status());
    proto_task->set_task_star(this->__task_star);
    proto_task->set_fast_finish_rate(this->__fast_finish_rate);

    int cond_size = this->condition_size();
    for (int i = 0; i < cond_size; ++i)
    {
        TaskConditon *task_cond = this->task_condition(i);
        JUDGE_CONTINUE(task_cond != NULL);

        ProtoTaskCond *proto_task_cond = proto_task->add_cond_list();
        task_cond->serialize(proto_task_cond);
    }
}

void TaskInfo::serialize(ProtoInnerTaskInfo *proto_task)
{
    proto_task->set_task_id(this->__task_id);
    proto_task->set_game_type(this->__game_type);
    proto_task->set_accept_tick_sec(this->__accept_tick.sec());
    proto_task->set_accept_tick_usec(this->__accept_tick.usec());
    proto_task->set_refresh_tick_sec(this->__refresh_tick.sec());
    proto_task->set_refresh_tick_usec(this->__refresh_tick.usec());
    proto_task->set_prev_task(this->__prev_task);
    proto_task->set_post_task(this->__post_task);
    proto_task->set_task_star(this->__task_star);
    proto_task->set_fast_finish_rate(this->__fast_finish_rate);
    proto_task->set_fresh_star_times(this->__fresh_star_times);

    uint64_t status_flag = 0;
    int uint64_len = sizeof(uint64_t) * 8;
    int i = TaskInfo::TASK_STATUS_END - 1;
    for (i = (i >= uint64_len ? (uint64_len - 1) : i); i >= 0; --i)
    {
        status_flag = status_flag << 1;
        if (this->__status.test(i))
            status_flag |= 1;
    }
    proto_task->set_status(status_flag);

    uint64_t logic_type_flag = 0;
    i = TaskConditon::TASK_CT_END - 1;
    for (i = (i >= uint64_len ? (uint64_len - 1) : i); i >= 0; --i)
    {
        logic_type_flag = logic_type_flag << 1;
        if (this->__logic_type.test(i))
            logic_type_flag |= 1;
    }
    proto_task->set_logic_type(logic_type_flag);

    for (TaskInfo::TaskConditionList::iterator cond_iter = this->__condition_list.begin();
            cond_iter != this->__condition_list.end(); ++cond_iter)
    {
        TaskConditon *task_cond = *cond_iter;
        task_cond->serialize(proto_task->add_task_cond_list());
    }
}

void TaskInfo::serialize(ProtoTrialTask *proto_task)
{
    proto_task->set_task_id(this->__task_id);
    proto_task->set_status(this->task_status());

    if (this->condition_size() > 0)
    {
        TaskConditon *task_cond = this->__condition_list[0];
        proto_task->set_value(task_cond->__current_value);
        proto_task->set_final_value(task_cond->__final_value);
    }
}

void TaskInfo::unserialize(const ProtoInnerTaskInfo &proto_task)
{
    this->__task_id = proto_task.task_id();
    this->__game_type = proto_task.game_type();
    this->__accept_tick = Time_Value(proto_task.accept_tick_sec(), proto_task.accept_tick_usec());
    this->__refresh_tick = Time_Value(proto_task.refresh_tick_sec(), proto_task.refresh_tick_usec());
    this->__prev_task = proto_task.prev_task();
    this->__post_task = proto_task.post_task();
    this->__task_star = proto_task.task_star();
    this->__fast_finish_rate = proto_task.fast_finish_rate();
    this->__fresh_star_times = proto_task.fresh_star_times();

    uint64_t status_flag = proto_task.status(),
             tmp_flag = 0;
    int uint64_len = sizeof(uint64_t) * 8;
    for (int j = 0; j < TaskInfo::TASK_STATUS_END && j < uint64_len; ++j)
    {
        tmp_flag = 1 << j;
        if ((status_flag & tmp_flag) != 0)
            this->__status.set(j);
    }
    uint64_t logic_type_flag = proto_task.logic_type();
    for (int j = 0; j < TaskConditon::TASK_CT_END && j < uint64_len; ++j)
    {
        tmp_flag = 1 << j;
        if ((logic_type_flag & tmp_flag) != 0)
            this->__logic_type.set(j);
    }

    int cond_list_size = proto_task.task_cond_list_size();
    for (int j = 0; j < cond_list_size; ++j)
    {
        TaskConditon *task_cond = MAP_MONITOR->task_condition_pool()->pop();
        task_cond->unserialize(proto_task.task_cond_list(j));
        this->__condition_list.push_back(task_cond);
    }
}

