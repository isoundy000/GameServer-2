/*
 * File Name: JYBackActivitySys.h
 * 
 * Created on: 2017-04-05 20:41:44
 * Author: glendy
 * 
 * Last Modified: 2017-04-07 20:35:17
 * Description: 
 */

#ifndef _JYBACKACTIVITYSYS_H_
#define _JYBACKACTIVITYSYS_H_

#include <vector>
#include <map>
#include "Singleton.h"
#include "GameHeader.h"

class JYBackActivityItem;

class JYBackActivitySys
{
public:
    typedef std::vector<JYBackActivityItem *> ActivityItemList;
    typedef std::map<int, ActivityItemList> TypeActivityItemMap;
    typedef std::map<int, JYBackActivityItem *> ActivityItemIDMap;
public:
    JYBackActivitySys(void);
    ~JYBackActivitySys(void);

    int start(void);
    int stop(void);

    ActivityItemList *find_act_items_by_first_type(const int first_type);
    ActivityItemList *find_act_items_by_second_type(const int second_type);
    JYBackActivityItem *find_act_item_by_id(const int act_id);
    JYBackActivityItem *find_expire_act_item_by_second_type(const int second_type);

    int fetch_back_act_type_list(IntSet &first_type_list);

    TypeActivityItemMap *first_type_to_act_item_map(void);
    TypeActivityItemMap *second_type_to_act_item_map(void);
    ActivityItemIDMap *id_to_act_item_map(void);
    ActivityItemIDMap *expire_second_type_act_item_map(void);

    int check_back_activity_timeout(const Time_Value &nowtime);

    int load_back_activity(void);
    int request_load_back_act_update(void);
    int process_after_load_back_act_update(Transaction *transaction);

    void clear_back_act_expire(void);
    void remove_act_item(JYBackActivityItem *act_item);

    int send_travel_recharge_rank_reward_mail(Message *msg);

    bool is_expire_send_mail_act(const int second_type);
    bool is_rank_act(const int second_type);

protected:
    TypeActivityItemMap *first_type_to_act_item_map_;   // key: first_type
    TypeActivityItemMap *second_type_to_act_item_map_;  // key: second_type
    ActivityItemIDMap *id_to_act_item_map_;             // key: act_id

    Time_Value back_update_tick_;
    Time_Value check_remove_tick_;

    ActivityItemIDMap *expire_second_type_act_item_map_;	// 到期的排行活动，由于发奖励, key: second_type
};

typedef Singleton<JYBackActivitySys> JYBackActivitySysSingle;
#define BACK_ACTIVITY_SYS   (JYBackActivitySysSingle::instance())

#endif //JYBACKACTIVITYSYS_H_
