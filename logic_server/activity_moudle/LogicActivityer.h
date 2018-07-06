/*
 * LogicActivityer.h
 *
 *  Created on: Oct 13, 2016
 *      Author: peizhibi
 */

#ifndef LOGICACTIVITYER_H_
#define LOGICACTIVITYER_H_

#include "BaseLogicPlayer.h"
#include "ActivityStruct.h"

class LogicActivityer : virtual public BaseLogicPlayer
{
public:
	LogicActivityer();
	virtual ~LogicActivityer();

	void reset_everyday();
	void reset_activityer();
	void refresh_act_type_item(int index);
	void combine_activity_first_times();

	MLActivityerDetial& fetch_act_detail();
	MLActivityerDetial::ActTypeItem& find_act_type_item(int index);
	BackSetActDetail::ActTypeItem* fetch_activity_by_index(int index);

	int is_activity_day_clear(int index);
	int is_festival_activity(int index);
	int has_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item);
	int has_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item,
			int second_index);
	int is_no_send_activity(int index);

	int fetch_left_activity_time(int index);
	int fetch_activity_info(int index);
	int fetch_activity_draw_serial(int index);

	//开服活动
	int fetch_open_activity_list();
	int fetch_open_activity(Message* msg);
	int fetch_open_activity(BackSetActDetail::ActTypeItem* act_t_item);
	int draw_open_activity_reward(Message* msg);
	int draw_open_activity_reward(BackSetActDetail::ActTypeItem& s_act_t_item,
			uint second_index, int num);
	int draw_open_activity_item(BackSetActDetail::ActTypeItem& s_act_t_item,
			uint second_index, int num);
	int draw_once_open_activity(BackSetActDetail::ActTypeItem& s_act_t_item,
			int second_index);
	int update_open_activity_info(Message* msg);

	int request_activity_buy_begin(Message* msg);	//进行兑换/购买类活动接口
	int request_activity_buy_done(Message* msg);

	void update_open_activity_force(int force);		//玩家战力
	void update_open_activity_level(int level);		//玩家等级
	void update_open_activity_mount(int type, int value);	//战骑类
	void update_open_activity_area_rank();	//江湖榜排名
	void update_open_activity_total_recharge(int gold);
	void update_open_activity_recharge(int gold); //累充
	void update_open_activity_value(int first_type, int day, int value);	//一个值
	void update_open_activity_total_value(int first_type, int day, int value);	//总动员
	void update_open_activity_total_recharge_value(int first_type, int day, int value);	//团购
	void update_open_activity_rank(int first_type, int day, int value, int sub);	//排行榜
	void notify_open_activity_update(BackSetActDetail::ActTypeItem* s_act_t_item,
			int force_value = -1);
	static void notify_all_player_red_point(BackSetActDetail::ActTypeItem* s_act_t_item,
			int force_value = -1);
	void act_login_check_red_point();

	//返利活动
	void update_return_activity_mount(int type, int value);
	void update_return_activity_recharge(int gold);
	void update_return_activity_value(const PairObj& pair, int value);	//一个值

	//节日活动
	int fetch_festival_activity_list();

	void update_fest_activity_login();	//登录奖励
	void update_fest_boss_hurt(int reward_index);	//BOSS
	void update_fest_activity_consume_money(int total_money);	//消费活动

	//合服活动
	void update_combine_activity_login(int recharge = false); //登录奖励
	void update_combine_activity_mount(int type, int value); //进阶奖励
	void update_combine_activity_recharge(int today_gold, int inc_gold);
	void update_combine_activity_consum(int inc_gold);

	void update_combine_activity_value(const PairObj& pair, int day, int value);	//一个值
	void update_combine_activity_value_b(const PairObj& pair, int day, int value);
	void update_combine_activity_rank(const PairObj& pair, int day, int value, Int64 sub);	//排行榜
	int fetch_self_combine_activity_consum(BackSetActDetail::ActTypeItem* s_act_t_item);
	int fetch_combine_activity_consum_rank(int data_type);

	//合服返利活动
	void update_combine_return_activity_mount(int type, int value);
	void update_combine_return_activity_recharge(int gold);
	void update_combine_return_activity_value(const PairObj& pair, int value);	//一个值

	//聚宝盘
	int fetch_cornucopia_msg(Message* msg);
	int fetch_cornucopia_info(int activity_id);
	int fetch_cornucopia_reward(Message* msg); 				//获取全服奖励
	int fetch_cornucopia_person_gold(Int64 role_id);
	int fetch_server_recharge();
	CornucopiaTask* fetch_cornucopia_task(int task_id);
	int update_cornucopia_activity_value(Message* msg);
	void update_cornucopia_activity_value(int task_id, int value, int type = 0);
	int update_cornucopia_activity_recharge(Message* msg);
	void update_cornucopia_activity_recharge(int cur_gold);
	int	check_cornucopia_task_finish_state(int task_id);
	void refresh_cornucopia_activity_reward_state();
	void refresh_cornucopia_activity_liveness();
	int	fetch_cornucopia_mail_reward();
	int	reset_cornucopia_info();
	int is_cornucopia_task_open_time(int task_id);

	//累计登录
	void 	update_open_activity_cumulative_login();
	int 	set_cumulative_login_info_state(int index);
	void 	cumulative_login_logic(int cumulative_day);
	int 	update_cumulative_logic_info_state(Message *msg);
	void 	reset_cumulative_login();

	void test_reset_acrivity(int index);
	void test_reset_act_update_tick(int act_index);

protected:
	MLActivityerDetial drawed_detial_;
	int activity_rank_line_;
};

#endif /* LOGICACTIVITYER_H_ */
