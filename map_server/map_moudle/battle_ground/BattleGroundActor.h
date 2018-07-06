/*
 * BattleGroundActor.h
 *
 *  Created on: Mar 19, 2014
 *      Author: lijin
 */

#ifndef BATTLEGROUNDACTOR_H_
#define BATTLEGROUNDACTOR_H_

#include "MapPlayer.h"

class TrvlArenaRole;
class TMArenaRole;

struct TArenaDetail
{
	int stage_;			//段位
	int score_;			//分数
	int draw_flag_;		//是否领取
	Int64 adjust_tick_;	//季赛结算时间

	int win_times_;		//今日胜利次数
	int get_exploit_;	//今日获得功勋
	int attend_times_;	//今日参与次数
	IntMap draw_win_;	//今天领取胜利奖励

	void reset();
	int update(int score);
	const Json::Value& conf();
};

struct TMarenaDetail
{
	int score_;			//积分
	int win_times_;		//今日胜利次数
	int attend_times_;	//今日参加次数

	void reset();
};

class BattleGroundActor : virtual public MapPlayer
{
public:
	BattleGroundActor();
	virtual ~BattleGroundActor();

	virtual void reset();
	virtual void reset_everyday();

	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
	virtual int die_process(const int64_t fighter_id);
    virtual int check_travel_timeout(void);
    virtual int process_relive(const int relive_mode, MoverCoord &relive_coord);
    virtual int obtain_area_info(int request_flag = false);
    
	int read_transfer_sm_battler(Message* msg);
	int sync_transfer_sm_battler(void);

	//玄武战场
	int sm_battle_scene();
	int sm_enter_scene_type();
	int request_join_sm_battle(void);
	int handle_exit_battle_scene(void);

	int scan_sm_battle_rank(Message* msg);
	int request_init_sm_battle_info(void);
	int on_enter_battle_scene(int type);
	int on_exit_battle_scene(int type);

	// common trvl
	void notify_quit_trvl();

	//跨服副本
	int trvl_script_scene();
	int create_trvl_script_team(Message* msg);
	int add_trvl_script_team(Message* msg);
	int fetch_trvl_script_team_list(Message* msg);
	int quit_trvl_script_team(Message* msg);
	int fetch_trvl_script_team_info(Message* msg);
	int start_trvl_script_team(Message* msg);
	int prep_trvl_script_team(Message* msg);
	int kick_trvl_script_team(Message* msg);

	//跨服竞技
	int travel_arena_scene();
	int travel_arena_prep_scene();
	int fetch_travel_arena_info();
	int travel_arena_sign();
	int travel_arena_rank(Message* msg);
	int travel_arena_draw();
	int travel_arena_unsign();
	int request_enter_travel_arena();
	int travel_arena_draw_win(Message* msg);

	int on_enter_prep_tarena_scene(int type);
	int on_exit_prep_tarena_scene(int type);
	int on_enter_travel_arena_scene(int type);
	int on_exit_travel_arena_scene(int type);
	int have_tarena_win_reward();

	void check_tarena_pa_event(int type);
	void update_tarena_score(int score, int exploit = 0, TrvlArenaRole* arena = NULL);
	void update_tarena_score_i(int score);
	void update_tarena_open_activity();
	void update_labour_task_info();
	void tarena_month_settle();

	//争霸天下
	int request_enter_travel_marena();
	int travel_marena_sign();
	int travel_marena_unsign();
	int travel_marena_info();
	int travel_marena_rank_begin();
	int on_enter_prep_tmarena_scene(int type);
	int on_enter_travel_marena_scene(int type);
	int on_exit_travel_marena_scene(int type);

	void update_tmarena_score(int score, int flag = false, TMArenaRole* arena_role = NULL);

	TArenaDetail& tarena_detail();
	TMarenaDetail& tmarena_detail();

	// travel battle
	int travel_battle_scene(void);
    int request_enter_travel_battle(void);
    int request_tbattle_last_rank_list(Message* msg);
    int request_tbattle_history_top_list(Message* msg);
    int request_tbattle_view_player_info(Message* msg);
    int request_tbattle_cur_rank_list(Message* msg);
    int request_tbattle_indiana_list(Message* msg);
    int on_enter_tbattle_scene(int type);
    int on_exit_tbattle_scene(int type);

    int need_kill_amount_to_next_floor(const int floor);
    int notify_tbattle_left_pannel(void);
    int notify_tbattle_enter_next_floor(void);
    int notify_tbattle_fallback_floor(void);
    int notify_tbattle_finish_info(void);
    int check_tbattle_timeout(void);
    int process_tbattle_relive(const int relive_mode, MoverCoord &relive_coord);
    IntSet &tbattle_treasure_buff_set(void);
    void refresh_treasure_prop_buff(const double interval, const double last);
    virtual int inc_tbattle_treasure_status_effect(BasicStatus *status, const int enter_type, const int refresh_type);
    virtual int process_tbattle_treasure_status_timeout(BasicStatus *status);
    int send_trvl_reward_id(const int reward_id, const SerialObj &serial_obj, const int mail_id = 0);
    IntPair interval_score_by_floor(void);
    void tbattle_interval_inc_score(void);
    int process_tbattle_obtain_area_info(const int request_flag);

	int sm_battler_die_process(Int64 fighter_id);
	int travel_arena_die_process(Int64 fighter_id);
	int travel_marena_die_process(Int64 fighter_id);
	int travel_battle_die_process(Int64 fighter_id);

protected:
	TArenaDetail tarena_detail_;
	TMarenaDetail tmarena_detail_;

    Time_Value tbattle_timeout_tick_;
    Time_Value tbattle_score_timeout_;
    IntSet tbattle_treasure_buff_set_;
};

#endif /* BATTLEGROUNDACTOR_H_ */
