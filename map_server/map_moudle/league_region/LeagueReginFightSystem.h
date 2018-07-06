/*
 * LeagueReginFightSystem.h
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#ifndef LEAGUEREGINFIGHTSYSTEM_H_
#define LEAGUEREGINFIGHTSYSTEM_H_

#include "MapMapStruct.h"

struct LRFLeagueInfo
{
	Int64 id_;
	string name_;

	int force_;
	int flag_lvl_;
	string leader_;

	int rank_;
	int space_;
	int camp_;
	string rival_name_;

	int result_;		//0:unknown, 1:win, 2:lose
	int fight_score_;
	int killed_tower_;
	Int64 flag_monster_;

	LRFLeagueInfo();
	void reset();

	void copy(int space, LeagueWarInfo::LeagueWarRank& info);
	void copy_result(int rank, LRFLeagueInfo* info);
};

struct LRFBetSupport
{
	Int64 role_id;
	Int64 support_league_id;
	int result_;

	LRFBetSupport();
};

struct LeagueRegionResult
{
	int finish_;
	Int64 tick_;
	LongMap atted_rank_map_;		//key: rank
	LongPairVec attend_id_vec_;

	typedef std::map<int, LRFLeagueInfo> HistoryMap;	//key: rank
	HistoryMap history_league_;
	LongMap history_result_;

	typedef std::map<Int64, LRFLeagueInfo> LeagueMap;
	typedef std::map<Int64, LRFBetSupport> SupportMap;
	LeagueMap attend_league_;
	SupportMap bet_support_;

	LeagueRegionResult();
	int fetch_result(Int64 id);

	void reset();
	void reset_every_times();
};

class LeagueRegionFightScene;
class LeagueReginFightSystem
{
public:
	enum
	{
		TOTAL_SIZE = 2
	};

	class ActivityTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void init(LeagueReginFightSystem* parent);

	private:
		LeagueReginFightSystem* lrf_system_;
	};

	class TipsTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	LeagueReginFightSystem();
	virtual ~LeagueReginFightSystem();

	const Json::Value& conf();

	int is_arrive_open_day();
	int is_activity_time();
	int validate_cur_lwar();
	int left_activity_time();

	int load_league_war_info_begin(int check = false);
	int load_league_war_info_done(DBShopMode* shop_mode);

	int request_enter_lrf(int sid, Proto30400051* request);
	int apply_lrf_operate(int sid, Int64 role, Message* msg);
	int fetch_lrf_rank_info(int sid, Int64 role, Message* msg);
	int fetch_lrf_support_info(int sid, Int64 role, Message* msg);
	int apply_lrf_support_league(int sid, Int64 role, Message* msg);
	int fetch_lwar_rank_info(int sid, Int64 role, Message* msg);

	void init();
	void stop();
	void init_region_info();
	void adjust_history_info();
	void adjust_cur_info();

	void recycle_league_region(LeagueRegionFightScene* scene);
	void test_league_region(int id, int last);
	void check_and_handle_all_scene();
	void set_result_info(LRFLeagueInfo* win, LRFLeagueInfo* lose);
	void set_bet_result_info(LRFLeagueInfo* league, int result);
	void send_bet_mail(Int64 role, int result);
	void check_and_update_cur_attend_info();

	int handle_activity_timeout();
	int handle_activity_timeout_i(int last_state);
	int handle_tips_timeout();

	int ahead_region_event();
	int start_region_event();
	int stop_region_event();

	int init_region_fight_map();
	int fetch_league_flag_blood(int level);

	Int64 fetch_next_fight_time();
	IntPair fetch_support_state();

	string fetch_city_name(int rank);
	IntStrPair fetch_cur_lrf_info(Int64 id);

	LeagueRegionResult& get_region_result();
	LeagueWarInfo& get_league_war_rank();
	LeagueRegionFightScene* find_lrf_scene(int space_id);

private:
	int scene_id_;
	int open_days_;
	int activity_id_;
	int check_cur_attend_;
	int support_reward_[TOTAL_SIZE];
	int support_mail_[TOTAL_SIZE];

	uint max_rank_;
	uint max_space_;
	Int64 last_league_;

	ActivityTimeInfo time_info_;
	ActivityTimer activity_timer;

	ActivityTimeInfo tips_time_;
	TipsTimer tips_timer_;

	LeagueWarInfo lwar_info_;  		//帮派排名信息
	LeagueRegionResult history_info_;

	LRFLeagueInfo null_info_;
	PoolPackage<LeagueRegionFightScene>* lrf_scene_package_;
};

typedef Singleton<LeagueReginFightSystem> leagueregionfightSingle;
#define LRF_MONITOR    (leagueregionfightSingle::instance())

#endif /* LEAGUEREGINFIGHTSYSTEM_H_ */
