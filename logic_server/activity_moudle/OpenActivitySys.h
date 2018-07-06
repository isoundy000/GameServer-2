/*
 * OpenActivitySys.h
 *
 *  Created on: Sep 3, 2014
 *      Author: jinxing
 */

#ifndef LOGICOPENACTIVITYSYSTEM_H_
#define LOGICOPENACTIVITYSYSTEM_H_

#include "ActivityStruct.h"

/*
 * 开服合服活动
 * */
class OpenActivitySys : public BackSetActDetail
{
public:
	OpenActivitySys();
	virtual ~OpenActivitySys();
	virtual int after_load_activity(DBShopMode* shop_mode);

	int start(void);
	int stop(int test = -1);
	int midnight_handle_timeout(int test_day = -1, int act_type = -1);
	int cur_open_day();
	int cur_combine_day();
	int validate_open_day(int day);
	int validate_combine_day(int day);

	int main_act_type();
	int is_open_activity_id(int act_index);
	int is_return_activity_id(int act_index);
	int is_combine_activity_id(int act_index);
	int is_combine_return_activity(int act_index);
	int is_open_activity();
	int is_return_activity();
	int is_combine_activity();
	int is_combine_return_activity();

	void init_open_activity();		//开服活动
	void init_return_activity();	//返利活动
	void init_combine_activity();	//合服活动
	void init_combine_return_activity();//合服返利活动
	void init_all_activity(GameConfig::ConfigMap& act_map);
	void init_cur_day_act();
	void save_open_activity_sys();

	void handle_act_finish();
	void handle_act_finish(BackSetActDetail::ActTypeItem* act_t_item);
	void update_activity_sys(int cur_day = -1, int act_type = -1);
	void after_load_db_update();
	void load_rank_combine_activity_consume();

	void send_rank_player_reward(BackSetActDetail::ActTypeItem* act_t_item, ActItem& act_item);
	void send_undrawed_player_reward(BackSetActDetail::ActTypeItem* act_t_item, ActItem& act_item);

	int update_lwar_activity_rank_info(Message* msg);
	void send_lwar_activity_rank_award();
	void test_reset_lwar_activity();

	void send_cornucopia_mail_reward();
	void init_cornucopia_act();
	void load_cornucopia_act();

	IntMap& fetch_act_list();
	BackSetActDetail::ActTypeItem* find_open_item(int type);

private:
	int cur_open_day_;		//当前开服天数
	int cur_combine_day_;	//当前合服天数
	int cur_main_act_;		//当前活动类型
	IntMap act_list_;		//当前的活动
};

typedef Singleton<OpenActivitySys> LogicOpenActivitySingle;
#define LOGIC_OPEN_ACT_SYS	LogicOpenActivitySingle::instance()

#endif /* LOGICOPENACTIVITY_H_ */
