/*
 * MapLogicTasker.h
 *
 *  Created on: Nov 6, 2013
 *      Author: glendy
 */

#ifndef MAPLOGICTASKER_H_
#define MAPLOGICTASKER_H_

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "HashMap.h"
#include "GameEnum.h"
#include <map>
#include "MapTaskStruct.h"

class TaskInfo;
class ProtoTaskInfo;
class MapLogicPlayer;
class TaskImplement;
class ItemObj;

typedef std::vector<TaskInfo*> TaskInfoVec;

namespace Json
{
    class Value;
}

enum
{
	BASE_SCALE_NUM	= 10000
};

class MapLogicTasker
{
    friend class MMOTask;
public:
    typedef boost::unordered_map<int, TaskInfo *> TaskMap;
    typedef boost::unordered_set<TaskInfo *> TaskSet;
    typedef boost::unordered_set<int> TaskIdSet;
    typedef HashMap<int, TaskIdSet, NULL_MUTEX> ItemTaskMap;
    typedef std::vector<int> RoutineTaskVec;
    typedef std::set<std::string> OpenUiSet;

public:
	MapLogicTasker(void);
	virtual ~MapLogicTasker(void);

    virtual MapLogicPlayer *task_player(void);
    int task_sign_in(const int type);
    int task_sign_out(void);

    void task_reset();
    void adjust_level_by_task(int task_id = 0);

    int task_time_up(const Time_Value &nowtime);

    bool is_finish_guide_task();
    int task_construct_main(bool notify_add = false);
    int task_construct_branch(bool notify_add = false);

    int correct_main_task(void);
    TaskInfo *init_task(const int task_id, const Json::Value &json);
    int insert_task(const int task_id, const Json::Value &json);
    TaskImplement *pop_task_imp(const int game_type);

    int task_branch_update(Message *msg);	//支线任务数据更新
    int accept_branch_request_info(const int type = 0);		//接受支线任务时向其他进程请求最新数据

    int task_accept(Message *msg);
    int task_accept(int task_id);
    int task_abandon(Message *msg);
    int task_submit(Message *msg, bool test = false);
    int task_talk(Message *msg);

    int task_get_list(Message *msg);    // 获取任务列表
    int task_npc_list(Message *msg);    // 获取npc列表
    int task_fast_finish(Message *msg); // 快速完成任务
    int task_after_check_npc(Message *msg);

    bool is_task_submited(const int task_id);
    bool is_nearby_npc(const int recogn, Message *msg, const int npc_id);
    
    int bind_task(const int task_id, TaskInfo *info);
    int unbind_task(const int task_id);
    TaskInfo *find_task(const int task_id);

    TaskMap &task_map(void);
    TaskSet &task_accepted_lvl_set(void);
    TaskSet &task_accepted_monster_set(void);
    TaskSet &task_accepted_collect_set(void);
    TaskSet &task_accepted_attend_set(void);
    TaskSet &task_accepted_package_set(void);
    TaskSet &task_accepted_scene_set(void);
    TaskSet &task_accepted_branch_set(void);
    TaskIdSet &task_submited_once_set(void);
    ItemTaskMap &task_listen_item_map(void);

    int init_task_listen_item(TaskInfo *task_info);
    int erase_task_listen_item(TaskInfo *task_info, const bool is_finish = false);
    bool is_task_listen_item(const int item_id);

    int task_listen_lvl_up(int target_level);
    int task_listen_kill_monster(const int sort, const int num = 1);
    int task_listen_collect_item(const int id, const int num = 1, const int bind = -1);
    int task_listen_attend(const int type, const int sub_type = 0, const int value = 1);
    int task_listen_package_item(const int id, const int num = 1, const int bind = -1);
    int task_listen_enter_special_scene(int scene_id);
    int task_listen_branch(const int id, const int num = 1);

    int notify_task_add(TaskInfo *task_info);
    int notify_task_del(TaskInfo *task_info);
    int notify_task_update(TaskInfo *task_info);
    int notify_routine_task_status(const int cmd, TaskInfo *task_info);

    int serialize_task(Message *proto);
    int unserialize_task(Message *proto);

    int notify_finished_task_set(void);
    int request_fetch_novice_step(void);
    int request_set_novice_step(Message *msg);
    int novice_step(void);
    void set_novice_step(const int step);
    int latest_main_task(void);
    void set_latest_main_task(const int id);
    int lcontri_day_value(void);
    OpenUiSet &open_ui_set(void);
    int ui_version(void);

    int request_fetch_uiopen_step(void);
    int request_set_uiopen_step(Message *msg);
    void set_uiopen_step(const int step);

    int start_script_wave_task_listen(int scene_id);
    int check_script_wave_task_finish(Message *msg);

    int test_special_task(int task_id);
    int test_task_open_ui(int task_id);

    const Json::Value &task_list_json(void);

    int fetch_routine_monster_id(int type, int range_level, int monster_id = 0);  //日常帮派返回随机ID 悬赏判断是不是在范围内
    void refresh_routine_task(const Time_Value &nowtime, const bool notify = true);
    int task_construct_routine(const bool notify_add = false);
    int new_task_construct_routine(const bool notify_add = false);
    void clear_routine_task(const bool notify = true, const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    int generate_next_routine_task(const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    int request_routine_ui_status(void);
//  int request_routine_ui_detail(void);
    int left_routine_task_amount(const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    int current_routine_task_id(const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    TaskInfo *current_routine_task(const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    bool is_last_routine_task(const int type = TaskInfo::TASK_GTYPE_ROUTINE);

    int routine_task_index(const int type = TaskInfo::TASK_GTYPE_ROUTINE);
    int routine_total_num(const int type = TaskInfo::TASK_GTYPE_ROUTINE);

    int offer_task_construct_routine(const bool notify_add = false);
    int league_task_construct_routine(const bool notify_add = false);
    int open_league_task(Message *msg);
    int request_routine_world_chat(void);
    int request_routine_open_market(void);
    int refresh_daily_league_contri(void);

    int routine_fast_finish(const Json::Value &effect, PackageItem *pack_item);

    int request_refresh_task_star(Message *msg);
    int request_routine_finish_all(Message *msg);

    int calc_routine_task_award(int *award_exp, Money &award_money, std::map<int, ItemObj> &item_map, const Json::Value &task_json, const double real_rate = 0.0);
    int calc_routine_extra_award(int *award_exp, Money &award_money, std::map<int, ItemObj> &item_map);
    double calc_star_to_rate(const int star, const Json::Value &task_json);

    int check_finish_task(int id);
    int check_current_version_open_ui(const int id, const int type,const char* key = NULL);

    void insert_finish_task(int task_id);

    int update_sword_pool_info(int num, int type);
    int update_cornucopia_task_info(int num, int type);
    int record_serial(int task_id, int serial_type);

protected:
    int process_accept(int task_id, bool is_init = false);
    int process_abandon(const int task_id);
    int process_submit(const int task_id, const int is_double, bool test = false);
    int process_fast_finish(const int task_id);
    int process_talk(const int task_id);

    int serail_task_info(TaskInfo *task_info, ProtoTaskInfo *proto_task_info);

    TaskInfo *init_main_task(const int task_id, const Json::Value &json);
    TaskInfo *init_routine_task(const int task_id, const Json::Value &json);
    TaskInfo *init_trial_task(const int task_id, const Json::Value &json);
    TaskInfo *init_branch_task(const int task_id, const Json::Value &json);

    int init_task_condition(TaskInfo *task_info, const Json::Value &task_json);
    int init_task_star(TaskInfo *task_info, const Json::Value &task_json);
    int init_task_object(TaskInfo *task_info, const Json::Value &json);

    int update_open_ui_set(const Json::Value &json);
    int check_and_update_open_ui_by_sign(void);

    int is_finish_all_league_routine();
    int is_league_routine_task();

protected:
    TaskMap task_map_;              // 所有任务
    TaskSet task_accepted_lvl_;     // 监听升级的任务集
    TaskSet task_accepted_monster_; // 监听杀怪的任务集
    TaskSet task_accepted_collect_; // 监听采集的任务集
    TaskSet task_accepted_attend_;  // 监听参加活动的任务集
    TaskSet task_accepted_package_; // 监听收集道具的任务集
    TaskSet task_accepted_scene_; // 监听进入特定场景的任务集
    TaskSet task_accepted_branch_; //监听支线任务集

    TaskIdSet task_submited_once_;  // 只提交一次
    std::set<int> finish_task_;		//已完成且需要记录的任务

    ItemTaskMap task_listen_item_;	// 任务需要收集的物品集合
    int novice_step_;	// 新手步数
    int lastest_main_task_id_;	// 最后完成的主线任务
    int uiopen_step_[GameEnum::UIOPEN_STEP_SIZE + 1];	// UI开启步数

    // 日常环式任务
    Time_Value routine_refresh_tick_;	// 日常刷新时间
    IntMap routine_task_index_;			// 当前正在进行的日常任务
    IntMap routine_total_num_;				//总环数
    IntMap is_finish_all_routine_;     // 是否完成所有日常任务
    IntMap is_routine_task_;           // 正在进行日常任务


    TaskIdSet routine_consume_history_; // 用于计算消费任务
    IntMap is_second_routine_;			// 是否非第一次进行日常

    //
    Time_Value lcontri_day_tick_;   // 宗派贡献每日刷新时间
    int lcontri_day_;               // 每日贡献值

    OpenUiSet open_ui_set_;         // 新版控制UI的字段
    int ui_version_;                // 任务对应UI的版本号
};


#endif /* MAPLOGICTASKER_H_ */
