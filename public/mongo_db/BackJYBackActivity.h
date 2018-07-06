/*
 * File Name: BackJYBackActivity.h
 * 
 * Created on: 2017-04-07 16:50:38
 * Author: glendy
 * 
 * Last Modified: 2017-04-08 13:18:35
 * Description: 
 */

#ifndef _BACKJYBACKACTIVITY_H_
#define _BACKJYBACKACTIVITY_H_

#include "MongoTable.h"
#include <vector>

class JYBackActivitySys;
class MongoDataMap;
class JYBackActivityItem;
class LogicPlayer;

class BackJYBackActivity : public MongoTable
{
public:
    virtual ~BackJYBackActivity(void);

    static int cond_typ_from_second_type(const int second_type);
    static int reward_type_from_second_type(const int second_type);
    static int bson_to_act_item(BSONObj &obj, JYBackActivityItem *act_item);

    static int load_back_activity(JYBackActivitySys *activity_sys);

    static int request_load_update_data(JYBackActivitySys *activity_sys, MongoDataMap *data_map);
    static int update_from_load(JYBackActivitySys *activity_sys, MongoDataMap *data_map, std::vector<int> &update_id_list);
    static int update_act_update_flag(const int act_id, MongoDataMap *data_map);

    int load_player_back_act_rec(LogicPlayer *player);
    static int update_data(LogicPlayer *player, MongoDataMap *data_map);
    
protected:
    virtual void ensure_all_index(void);
};

#endif //BACKJYBACKACTIVITY_H_
