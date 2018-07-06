/*
 * File Name: LogicBackActivityer.h
 * 
 * Created on: 2017-04-05 21:14:13
 * Author: glendy
 * 
 * Last Modified: 2017-04-08 11:34:30
 * Description: 
 */

#ifndef _LOGICBACKACTIVITYER_H_
#define _LOGICBACKACTIVITYER_H_

#include "BaseLogicPlayer.h"
#include "ActivityStruct.h"

class LogicBackActivityer : virtual public BaseLogicPlayer
{
public:
    typedef std::map<int, JYBackActivityPlayerRecord> BackActivityRecordMap;
    typedef std::vector<JYBackActivityItem *> ActivityItemList;
public:
    virtual ~LogicBackActivityer(void);

    void reset_back_activity(void);
    void check_finish_back_act_every_day(void);

    int fetch_back_activity_list(Message *msg);
    int fetch_single_back_activity_info(Message *msg);

    int find_reward_item_list(const int act_id, const int reward_id, JYBackActivityItem::ItemList &reward_item_list, IntSet &reward_value);
    int fetch_reward_item_list(const int act_id, const int reward_id, IntMap &bind_item_map, IntMap &unbind_item_map, IntSet &reward_value);
    int draw_single_back_activity_reward(Message *msg);

    int notify_activity_icon_info(void);

    JYBackActivityPlayerRecord *find_player_act_record(const int act_id);

    int process_back_act_reward_state_after_insert(Message *msg);

    void update_back_act_accu_recharge(const int daily_gold);
    void update_back_act_repeat_recharge(const int daily_gold);
    void update_back_act_accu_consum(const int daily_consum);
    void update_back_act_travel_recharge_rank(const int daily_gold);

    BackActivityRecordMap &back_act_player_rec_map(void);

    int fetch_back_travel_rank_info(Message *msg);

protected:
    void refresh_back_act_record_daily_value(JYBackActivityPlayerRecord *player_act_record);
    void check_back_act_reward(JYBackActivityItem *act_item, JYBackActivityPlayerRecord *player_act_record);

    void update_back_act_record_by_daily_value(ActivityItemList *act_list, const int daily_value);
    void update_back_act_record_by_inc_value(ActivityItemList *act_list, const int inc_value);
    void update_single_cond_value_after_draw(JYBackActivityPlayerRecord *player_act_record);

    void init_player_act_record_by_act_item(JYBackActivityItem *act_item, JYBackActivityPlayerRecord *player_act_record);
    void check_and_finish_last_activity_record(JYBackActivityPlayerRecord *player_act_record);
    void send_back_act_reward_mail(JYBackActivityPlayerRecord *player_act_record);

    int fetch_accum_recharge_gold(JYBackActivityItem *act_item);
    int fetch_accum_consum(JYBackActivityItem *act_item);
    int fetch_reward_amount_state(JYBackActivityItem *act_item, const int reward_amount, const int drawed_amount);

    void check_and_update_reward_map(const int reward_id, JYBackActivityItem::Reward &reward_info, JYBackActivityPlayerRecord *player_act_record);

protected:
    BackActivityRecordMap player_act_record_map_;
    IntSet reward_first_type_set_;		// 有奖励的活动类型集合,用于显示红点
    IntSet reward_act_id_set_;			// 有奖励的活动ID集合,用于显示红点
    Time_Value back_act_draw_op_tick_;
};

#endif //LOGICBACKACTIVITYER_H_
