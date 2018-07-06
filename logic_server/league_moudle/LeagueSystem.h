/*
 * LeagueSystem.h
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#ifndef LEAGUESYSTEM_H_
#define LEAGUESYSTEM_H_

#include "LeagueStruct.h"

class LeagueSystem
{
public:
	class StartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	class SortTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	LeagueSystem();
	~LeagueSystem();

	void start();
	void stop();

	int send_league_escort_reward(Message* msg);
	int send_league_escort_reward(const ProtoThreeObj& three_obj);

	int add_league_flag_exp(Message* msg);
	int send_league_member_reward(MailInformation* send_mail,
			Int64 league_index, int reward);

	int update_martial_role_label(Message* msg);
	int add_league_member_contri(Message* msg);
	int add_league_member_contri(Int64 league_id, Int64 role_id, int add_contri);
	int add_league_member_contri(League* league, Int64 role_id, int add_contri);

	void sort_league();
	void league_check_everyday(bool force_reset = false);

	//服务器启动时加载帮派领地(boss)场景
	int load_league_boss_scene();
	int try_start_boss_scene();
	int check_boss_scene_start(Message* msg);
	int sync_league_boss_info(League* league);
	int sync_leader_to_map(League* league);

	void check_and_dismiss_league(League* league);
	void dismiss_league(League* league);

	void erase_all_apply(long role_index);
	void fetch_league_set(int joinable, LongVec& league_set);

	int max_level();
	int fetch_league_pos(int league_pos);

	bool validate_quit_league(int scene_id);
	bool player_have_league(Int64 role_index);
	bool is_league_name_same(const string& league_name);

	uint max_member(int league_lvl);
	uint max_league_pos_count(int league_lvl, int league_pos);

	Int64 fetch_new_league_index();
	int creat_league_lvl(int create_type);

	PoolPackage<League, Int64>* league_package();

	const Json::Value& league_shop(int shop_type);
	const Json::Value& league_set_up(uint league_lvl);

	League* find_league(Int64 league_index);

	void save_all_league(int direct_save = false);
	void save_league(const Int64 league_id);
	void set_league_war_label(Int64 league_index);

	int check_league_leader();
	int handle_league_fb_finish(Message* msg);

	int handle_league_boss_finish(Message* msg);
	int handle_rename_league_finish(Message* msg);

    int league_size(void);

	void remove_store(Int64 league_index);
	LeagueStore* find_lstore(Int64 league_index);

    int notify_lescort_state(Message *msg);
    int map_act_fetch_league_rank_info(Message* msg);
    int map_act_fetch_league_rank_info(int size);
    BLongSet& league_fb_flag();
    void insert_fb_flag(Int64);
    void sync_fb_flag(Int64);

    int inner_send_lwar_first_win(Message* msg);	//发送第一战区获胜帮派帮众称号
    int set_region_result(Message* msg);

private:
    FourObjVec league_set_;
    LongMap league_id_map_;	//服务器启动时存放帮派id

	PoolPackage<League, Int64> league_package_;

    BLongSet lfb_flag;//已领取仙盟副本奖励角色id
    StartTimer start_timer_;
    SortTimer sort_timer_;
};

typedef Singleton<LeagueSystem> 	LeagueSystemSingle;
#define LEAGUE_SYSTEM   			LeagueSystemSingle::instance()
#define LEAGUE_PACKAGE   			LEAGUE_SYSTEM->league_package()

#endif /* LEAGUESYSTEM_H_ */
