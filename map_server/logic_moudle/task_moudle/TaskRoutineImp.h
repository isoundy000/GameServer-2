/*
 * TaskRoutineImp.h
 *
 * Created on: 2014-09-02 10:03
 *     Author: lyz
 */

#ifndef _TASKROUTINEIMP_H_
#define _TASKROUTINEIMP_H_

#include "TaskImplement.h"

class TaskRoutineImp : public TaskImplement
{
public:
    TaskRoutineImp(void);
    virtual ~TaskRoutineImp(void);

    virtual void reset(void);

    virtual void recycle_self(void);
    virtual int process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num = 1);

//    virtual int process_listen_collect_item(TaskInfo *task_info, const int id, const int num = 1, const int bind = -1);
    virtual int process_finish(TaskInfo *task_info);
    virtual int process_submit(TaskInfo *task_info, const int is_double);

    int new_process_task_reward(TaskInfo *task_info);
protected:


    int process_task_level_reward(TaskInfo *task_info, const int rate = 100);
    
};

#endif //_TASKROUTINEIMP_H_
