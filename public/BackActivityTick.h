/*
 * BackActivityTick.h
 *
 * Created on: 2015-05-08 11:41
 *     Author: lyz
 */

#ifndef _BACKACTIVITYTICK_H_
#define _BACKACTIVITYTICK_H_

#include "Singleton.h"
#include "Time_Value.h"
#include "GameHeader.h"
#include <map>

class BaseUnit;
class Transaction;

class BackActivityTick
{
public:
    struct ActivityTickInfo
    {
        Time_Value __begin_tick;
        Time_Value __end_tick;

        void reset(void);
    };
    typedef std::map<int, ActivityTickInfo> ActivityTickMap;
public:
    BackActivityTick(void);

    int init(void);
    int check_update_back_activity_tick(BaseUnit *unit);
    int update_activity_tick_in_logic_server(Transaction *trans);
    int sync_activity_tick(Message *msg);

    bool is_has_activity(const int activity_id);
    Time_Value activity_begin_tick(const int activity_id);
    Time_Value activity_end_tick(const int activity_id);

    void update_version(void);
    ActivityTickMap &next_activity_tick_map(void);
    ActivityTickMap &cur_activity_tick_map(void);

protected:
    int version_cnt_;
    ActivityTickMap activity_tick_map_[2];
};

typedef Singleton<BackActivityTick> BackActivityTickSingle;
#define BACK_ACTIVITY_TICK  (BackActivityTickSingle::instance())

#endif //_BACKACTIVITYTICK_H_
