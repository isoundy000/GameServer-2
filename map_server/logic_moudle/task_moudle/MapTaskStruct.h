/*
 * MapTaskStruct.h
 *
 * Created on: 2013-11-06 21:45
 *     Author: lyz
 */

#ifndef _MAPTASKSTRUCT_H_
#define _MAPTASKSTRUCT_H_

#include <stdint.h>
#include "Time_Value.h"
#include <vector>
#include <bitset>
#include <boost/unordered_set.hpp>

namespace Json
{
	class Value;
}

class ProtoTaskCond;
class ProtoInnerTaskCond;
class ProtoTaskInfo;
class ProtoInnerTaskInfo;
class ProtoTrialTask;

namespace GameEnum
{
    enum CHEKC_OPEN_UI_TYPE
    {
        CHECK_UIT_LEVELUP = 1,
        CHECK_UIT_ACCEPTED_TASK = 2,
        CHECK_UIT_SUBMIT_TASK = 3,
        CHECK_UIT_FINISH_TASK = 4,
        CHECK_UIT_TALISMAN = 5,
        CHECK_UIT_OTHER = 6,
        CHECK_UIT_END
    };
};

struct TaskConditon
{
    enum TASK_COND_TYPE
    {
        TASK_CT_NONE = 0,
        TASK_CT_KILL_MONSTER = 1,   // 杀怪
        TASK_CT_COLLECT_ITEM = 2,   // 采集
        TASK_CT_NPC_DIALOGUE = 3,   // NPC对话
        TASK_CT_LVL = 4,            // 等级
        TASK_CT_ATTEND = 5,         // 参加活动
        TASK_CT_SCRIPT_WAVE = 6,    // 副本波数
        TASK_CT_RECHARGE = 7,       // 充值任务
        TASK_CT_PACKAGE = 8,		// 背包物品
        TASK_CT_PATROL = 9,			// 寻路任务（直接从已接任务跳直接提交的任务）
        TASK_CT_SCENE = 10,        	// 进入场景
        TASK_CT_COLLECT_ANY = 11,   // 指定场景收集
        TASK_CT_LVL_MONSTER = 12,   // 指定场景杀死指定数量的怪
        TASK_CT_BOSS = 13,          // 指定场景杀死指定个数的BOSS

        TASK_CT_BRANCH	= 14, 		// 支线任务 //目标 及 目标值
//        TASK_CT_EQUIP = 14,			// 锻造强化任务
//        TASK_CT_FB = 15,		// 副本任务
//        TASK_CT_LEAGUE	= 16,		// 帮派功能任务
//        TASK_CT_FIRST_RECHARGE = 17,	// 首充任务

        TASK_CT_END
    };

    int __type;     // 类型: TASK_COND_TYPE
    int __current_value;
    
    int __cond_index;		// 多条件类型的索引
    int __id_list_index;	// 多ＩＤ类型的索引
    int __cond_id;
    int __final_value;

    int __kill_type;
    int __range_level;

    void reset(void);
    bool is_no_condtion(void);
    bool is_kill_monster(void);
    bool is_collect_item(void);
    bool is_level(void);
    bool is_attend(void);
    bool is_script_wave(void);
    bool is_npc_dialogue(void);
    bool is_package_item(void);
    bool is_patrol(void);
    bool is_scene(void);
    bool is_collect_any(void);
    bool is_lvl_monster(void);
    bool is_kill_boss(void);
    bool is_branch(void);

    static int fetch_id(int type, const Json::Value& json);
    static int fetch_type(const std::string& type_str);

    void serialize(ProtoTaskCond *proto_task_cond);
    void serialize(ProtoInnerTaskCond *proto_task_cond);
    void unserialize(const ProtoInnerTaskCond &proto_task_cond);
};

class TaskImplement;
struct TaskInfo
{
    enum TASK_STATUS
    {
        TASK_STATUS_NONE = 0,
        TASK_STATUS_VISIBLE = 1,    // 任务可见
        TASK_STATUS_ACCEPTABLE = 2, // 任务可接
        TASK_STATUS_ACCEPTED = 3,   // 任务已接
        TASK_STATUS_FINISH = 4,     // 任务已完成
        TASK_STATUS_SUBMIT = 5,     // 任务已提交

        TASK_STATUS_END
    };
    enum TASK_GAME_TYPE
    {
        TASK_GTYPE_NONE = 0,
        TASK_GTYPE_MAINLINE = 1,    // 主线任务
        TASK_GTYPE_BRANCH = 2,      // 支线任务
        TASK_GTYPE_ROUTINE = 3,     // 日常环式任务
        TASK_GTYPE_LEAGUE = 4,      // 宗派任务
        TASK_GTYPE_OFFER_ROUTINE = 6,     // 悬赏环式任务
        TASK_GTYPE_LEAGUE_ROUTINE = 7,     // 帮派环式任务

        TASK_GTYPE_END
    };

    enum TASK_SUB_TYPE
    {
        TASK_ST_ROUTINE_LIVENESS = 301, // 日常活跃度
        TASK_ST_ROUTINE_SCRIPT = 302,   // 日常副本
        TASK_ST_ROUTINE_ARENA = 303,    // 日常竞技场
        TASK_ST_ROUTINE_CONSUME = 304,  // 日常消费
        TASK_ST_ROUTINE_OTHER = 305     // 日常其他
    };
    typedef TaskConditon::TASK_COND_TYPE TASK_LOGIC_TYPE;

    int __task_id;
    int __game_type;            // 类型: TASK_GAME_TYPE
    Time_Value __accept_tick;   // 接受时间戳
    Time_Value __refresh_tick;  // 刷新时间戳(日常等定期刷新的任务)

    typedef std::vector<TaskConditon *> TaskConditionList;
    TaskConditionList __condition_list;     // 任务的条件集

    int __prev_task;    // 前置任务,若为0则从配置文件中读取
    int __post_task;    // 后置任务,若为0则从配置文件中读取

    typedef std::bitset<TASK_STATUS_END> TaskStatusFlag;
    TaskStatusFlag __status;    // 任务状态集合
    typedef std::bitset<TaskConditon::TASK_CT_END> TaskLogicType;
    TaskLogicType __logic_type;

    int __task_star;        // 任务的星级,没有则为0
    int __fast_finish_rate; // 快速完成的倍数奖励
    int __fresh_star_times; // 该任务刷新星级次数

    TaskImplement *__task_imp;  // 任务执行器(根据TASK_LOGIC_TYPE选择);

    TaskInfo(void);
    void reset(void);
    bool is_visible(void);      // 可见
    bool is_acceptable(void);   // 可授受
    bool is_accepted(void);     // 已接受
    bool is_finish(void);       // 已完成未提交
    bool is_submit(void);       // 已提交
    void set_task_status(const int status);
    void reset_task_status(const int status);

    bool is_main_task(void);
    bool is_branch_task(void);
    bool is_routine_task(void);
    bool is_offer_task(void);
    bool is_league_task(void);

    void set_logic_type(const int type);
    void reset_logic_type(const int type);
    bool is_logic_kill_monster(void);
    bool is_logic_collect_item(void);
    bool is_logic_npc_dialogue(void);
    bool is_logic_level_up(void);
    bool is_logic_attend(void);
    bool is_logic_script_wave(void);
    bool is_logic_package(void);
    bool is_logic_patrol(void);
    bool is_logic_scene(void);
    bool is_logic_collect_any(void);
    bool is_logic_lvl_monster(void);
    bool is_logic_kill_boss(void);

    bool is_logic_branch(void);

    TaskImplement *task_imp(void);
    int condition_size(void);
    TaskConditon *task_condition(const int index = 0);
    int task_status(void);

    const Json::Value& conf();

    void serialize(ProtoTaskInfo *proto_task);
    void serialize(ProtoInnerTaskInfo *proto_task);
    void serialize(ProtoTrialTask *proto_task);
    void unserialize(const ProtoInnerTaskInfo &proto_task);
};

#endif //_MAPTASKSTRUCT_H_
