/*
 * LogicMayActivityer.h
 *
 *  Created on: 2017年4月13日
 *      Author: lyw
 */

#ifndef LOGICMAYACTIVITYER_H_
#define LOGICMAYACTIVITYER_H_

#include "BaseLogicPlayer.h"
#include "ActivityStruct.h"

class Proto50100252;

class LogicMayActivityer : virtual public BaseLogicPlayer
{
public:
    struct CachedTimer : public GameTimer
    {
        CachedTimer(void);
        virtual ~CachedTimer(void);

    	virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);

        LogicPlayer *player_;
    };
public:
	LogicMayActivityer();
	virtual ~LogicMayActivityer();
	void reset_may_activityer();

	void reset_everyday();
	void daily_login_refresh_day();
	void refresh_daily_login_data();

	int fetch_cur_time_open_state(int activity_id);

	int fetch_may_activity_list();
	int fetch_may_activity(Message* msg);
	int fetch_may_activity_info(int index);
	int draw_may_activity_reward(Message* msg);
	int draw_may_activity_reward(MayActDetail::ActInfo& s_act_t_item,
			uint index, int num);

	int inner_act_buy_operate(Message* msg);			//购买返回操作

	//夫妻同心购买在线时间
	int request_add_couple_tick_begin(Message* msg);
	int request_add_couple_tick_end(Message* msg);

	//每日限购
	int request_may_activity_buy_begin(Message* msg);
	int request_may_activity_buy_done(Message* msg);

	//天天跑酷
	int request_daily_run_friend_list(Message* msg);
	int daily_run_friend_list(int index, int page = 1);
	int request_start_run_begin(Message* msg);
	int request_start_run_end(Message* msg);
	int request_daily_run_buy_begin(Message* msg);
	int request_daily_run_buy_end(Message* msg);
	int requrst_daily_run_send(Message* msg);
	int request_daily_run_jump_begin(Message* msg);
	int request_daily_run_jump_end(Message* msg);

	//红包活动
	int fetch_grab_red_packet_info(Message *msg);
	int fetch_act_open_time_index(MayActDetail::ActInfo *act_info);
	int fetch_red_packet_reward_info(Message *msg);
	int fetch_red_packet_reward(Message *msg);
	int notify_cur_group_reward_info(MayActDetail::ActInfo *act_info);
	int notify_is_open_red_packet();
	int fetch_single_red_packet_info(int group);
	void init_red_act_info_list();
	void update_red_act_info_list();
	void clear_red_act_info();
	int fetch_red_packet_state_by_group(Message *msg);

	//绝版时装
	int fetch_fashion_info(Message *msg);
	int fetch_fashion_buy_begin(Message *msg);
	int fetch_fashion_buy_End(Message *msg);
	int fetch_fashion_reward(Message *msg);

	//活动光荣
	int fetch_labour_info(Message *msg);
	CornucopiaTask* fetch_labour_task(int task_id);
	int update_labour_activity_value(Message* msg);
	void update_labour_activity_value(int task_id, int value, int type = 0);
	int is_labour_task_open_time(int task_id);
	int is_all_labour_task_finish();

	//神秘兑换
	int fetch_change_buy_begin(Message *msg);
	int fetch_change_buy_end(Message *msg);
	void reset_every_day_by_change(int type = 0);
	void test_clear_data_by_change();

	int may_rand_slot_item(int activity_id, int slot_num = 1);

	MayActivityer &may_detail();

	void may_act_start_tick(LogicPlayer *player);
	void may_act_stop_tick();

	int may_act_test_reset(int type);
	void test_set_login_day(int day);

public:
	void update_daily_login_recharge_sign_handle();
	void update_daily_recharge_recharge_sign_handle();
	void update_may_act_one_value(int first_type, int day, int value);
	void update_may_act_one_value_by_index(int first_type, int day, int index, int value);
	void notify_may_activity_update(MayActDetail::ActInfo* act_info);	//红点
	void may_activity_announce(int index, uint reward_index);			//传闻
	MayActivityer::ActTypeItem& find_may_act_item(int index);

protected:
	void refresh_may_act();
	void refresh_act_type_item(int index);
	int has_may_activity_reward(MayActDetail::ActInfo& act_info);
	int has_may_activity_reward(MayActDetail::ActInfo& act_info, int index);
	void serialize_sub_value(Proto50100252 *respond, int index);
	void check_rand_role_view(MayActDetail::ActInfo& act_info);
	void create_rand_role_view(MayActDetail::ActInfo& act_info);
	void send_run_direct_award(int before, int after, int base);

private:
	MayActivityer may_detial_;

	CachedTimer timer_;
};

#endif /* LOGICMAYACTIVITYER_H_ */
