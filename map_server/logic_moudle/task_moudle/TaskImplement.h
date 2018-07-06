/*
 * TaskImplement.h
 *
 * Created on: 2013-11-06 22:13
 *     Author: lyz
 */

#ifndef _TASKIMPLEMENT_H_
#define _TASKIMPLEMENT_H_

class MapLogicTasker;
class TaskInfo;
namespace Json
{
class Value;
}

class TaskImplement
{
public:
    TaskImplement(void);
    virtual ~TaskImplement(void);

    virtual void reset(void);
    MapLogicTasker *tasker(void);
    void set_tasker(MapLogicTasker *tasker);

    virtual int process_accept(TaskInfo *task_info, const bool is_init = false, const bool is_notify = true);
    virtual int process_abandon(TaskInfo *task_info);
    virtual int process_submit(TaskInfo *task_info, const int is_double);
    virtual int process_fast_finish(TaskInfo *task_info);
    virtual int process_finish(TaskInfo *task_info);
    
    virtual int process_listen_lvl_up(TaskInfo *task_info, const int target_level);
    virtual int process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num = 1);
    virtual int process_listen_collect_item(TaskInfo *task_info, const int id, const int num = 1, const int bind = -1);
    virtual int process_listen_attend(TaskInfo *task_info, const int type, const int sub_type = 0, const int value = 1);
    virtual int process_listen_package_item(TaskInfo *task_info, const int id, const int num = 1, const int bind = -1);
    virtual int process_listen_enter_special_scene(TaskInfo* task_info,int scene_id);

    virtual int process_listen_branch(TaskInfo *task_info, const int id, const int num = 1);
    virtual void recycle_self(void);

    int remove_task_listen(TaskInfo *task_info);
    int remove_task_info(TaskInfo *&task_info, const bool notify = false);
    int process_task_reward(TaskInfo *task_info, const Json::Value &reward_json, const int rate = 100);
    int new_process_task_reward(TaskInfo *task_info, const int reward_id, const int rate = 100);

protected:
    MapLogicTasker *tasker_;
};

#endif //_TASKIMPLEMENT_H_
