/*
 * TaskTrialImp.h
 *
 * Created on: 2015-07-23 14:32
 *     Author: lyz
 */

#ifndef _TASKTRIALIMP_H_
#define _TASKTRIALIMP_H_

#include "TaskRoutineImp.h"

class TaskTrialImp : public TaskRoutineImp
{
public:
    virtual ~TaskTrialImp(void);
    virtual void recycle_self(void);

    virtual int process_listen_kill_monster(TaskInfo *task_info, const int sort, const int num = 1);
    virtual int process_listen_collect_item(TaskInfo *task_info, const int id, const int num = 1, const int bind = -1);
    virtual int process_finish(TaskInfo *task_info);
    virtual int process_submit(TaskInfo *task_info, const int is_double);
    virtual int process_abandon(TaskInfo *task_info);

};

#endif //_TASKTRIALIMP_H_
