/*
 * MapLeaguer.h
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#ifndef MAPLEAGUER_H_
#define MAPLEAGUER_H_

#include "MapPlayer.h"

class Proto30400011;

struct MapLeaguerInfo
{
	int flag_lvl_;
	Int64 leader_id_;
	IntMap skill_map_;
	IntVec skill_prop_set_;

	MapLeaguerInfo();
	void reset();
};

class MapLeaguer : virtual public MapPlayer
{
public:
	class RefreshTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

	    MapLeaguer* father_;
	};

	MapLeaguer();
	virtual ~MapLeaguer();

	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int obtain_area_info(int request_flag = false);

	virtual int die_process(const int64_t fighter_id = 0);
	virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);
	virtual int modify_magic_by_notify(const double value, const int fight_tips = FIGHT_TIPS_BLANK);

    virtual int sign_out(const bool is_save_player);

	virtual int request_relive(Message* msg);
	virtual int notify_client_enter_info();
	virtual int is_in_safe_area();

	int request_exit_league();

	int enter_area_field(const int type);
	int enter_league_boss(const int type);
	int enter_collectchests(const int type);
	int enter_answer_activity(const int type);
	int enter_hotspring_activity(const int type);

	void leaguer_reset();
	void leaguer_set_nextday();
	void leaguer_exit_scene(int type);

	int set_self_league_info(Message* msg);
	int check_cur_scene_validate();

	//answer_activity
	int request_player_wait_time();

	// arena
	int notify_enter_arena_info();
	int notify_arena_fight_info();

	// league boss
	int request_enter_league_boss_begin();

	// escort
	int open_pescort_car_info(int type = 0);
	int respond_escort_open_info();

	int upgrade_escort_level(Message* msg);
	int upgrade_escort_level_done(Message* msg);
	int respond_escort_upgrade_info();

	int select_pescort_car_begin(Message* msg);
	int respond_escort_start_info();
	int select_pescort_car_done(const Int64 rob_id = 0, const string& rob_name = "");

	int escort_seek_help();
	int transfer_to_escort_npc(Message* msg);
	int rob_escort(const int64_t fighter_id = 0);
	int wish_escort();
	int protect_escort(Message* msg);
	int insert_protect_buff();
	int insert_escort_speed_buff();
	int escort_stop_protect();
	int escort_refresh(void);


	// transfer
	int sync_transfer_leaguer();
	int read_transfer_leaguer(Message* msg);

	int sync_league_fb_flag(Message* msg);
	int update_sword_pool_info();

	int update_leaguer_info(Message* msg);

	int update_cornucopia_task_info();
	int update_labour_task_info();

	void init_league_flag_property();
	void init_league_skill_property();
	void add_flag_attr(Proto30400011* request, int add_index, int attr);

	MapLeaguerInfo &leaguer_info();

private:
	void area_die_process(Int64 fighter_id);

	RefreshTimer escort_refresh_timer_;
	MapLeaguerInfo map_leaguer_info_;
};

#endif /* MAPLEAGUER_H_ */
