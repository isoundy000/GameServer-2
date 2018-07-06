/*
 * LeagueWarSystem.h
 *
 *  Created on: 2016年9月19日
 *      Author: lyw
 */

#ifndef LEAGUEWARSYSTEM_H_
#define LEAGUEWARSYSTEM_H_

//#include "PubStruct.h"
#include "MapMapStruct.h"

class LeagueWarScene;
class Proto30400051;
class Proto80400385;
class Proto80400386;
class Proto50400351;
class Proto50400353;

struct LeagueWarSysInfo
{
	LeagueWarSysInfo(void);
	void reset(void);

	int __activity_id;
	int __enter_level;
};

struct FirstSpaceWinLeague
{
	Int64 league_index_;
	string league_name_;
	string leader_name_;

	FirstSpaceWinLeague(void);
	void reset(void);
};

class LeagueWarSystem
{
public:
	typedef std::vector<LeagueWarScene*> LeagueWarSceneSet;
	typedef std::map<Int64, LWarLeagueInfo> AllLeagueInfoMap;

	class CheckStartTimer : public GameTimer
	{
	public:
		CheckStartTimer(void);
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void init(LeagueWarSystem* parent);

	private:
		LeagueWarSystem* lwar_system_;
	};

public:
	LeagueWarSystem();
	virtual ~LeagueWarSystem();

	void reset(void);
	void recycle_lwar(LeagueWarScene* scene);

	int client_scene();
	int real_scene();

	void start_league_war(int real_scene);
	void stop_league_war();

	int request_enter_lwar(int sid, Proto30400051* request);
	int request_exit_lwar(Int64 league_index, Int64 role_id);

	void test_lwar(int id, int set_time);
	void test_open_space();

public:
	int handle_lwar_timeout();
	int handle_lwar_i(int state);

	int is_test();
	int lwar_left_time();
	int is_today_activity();
	int is_activity_time();
	int validate_player_level(int role_level);

	int ahead_lwar_event();
	int start_lwar_event();
	int stop_lwar_event();

	void update_leauge_score(LWarLeagueInfo& lwar_league);

	int find_league_rank(Proto50400351* respond);		//旧的排名消息号
	int find_all_league_rank(Proto50400353* respond); //新的排名消息号
	int find_player_league_rank(Proto50400351* respond, Int64 league_index);

	LWarRoleInfo* find_lwar(Int64 role_id);
	LeagueWarScene* fetch_lwar_scene(Int64 role_id);
	LeagueWarScene* pop_lwar_scene(int space_id);
	LeagueWarScene* find_lwar_scene(int space_id);

	void set_first_space_winner(LWarLeagueInfo& lwar_league);
	void fetch_first_space_winner(Proto80400386* respond);
	void update_leader_info(Int64 league_id, Int64 leader_id, string leader);

	Int64 fetch_first_win_league_id();

	int notify_all_player(int value);
	int find_open_space_num();

	void find_open_space(Proto80400385* respond);
	void init_force_info(LeagueWarInfo::LeagueWarRank& war_rank);
	void init_league_map(Int64 role_id, Int64 league_id, string league_name, int space_id);

private:
	int real_scene_;
	int is_test_;

	ActivityTimeInfo time_info_;
	LeagueWarSysInfo lwar_detail_;
	FirstSpaceWinLeague first_win_;
	LeagueWarSceneSet lwar_scene_set_;
	CheckStartTimer lwar_check_timer_;
	LeagueWarInfo lwar_info_;

	PoolPackage<LeagueWarScene>* lwar_scene_package_;
	PoolPackage<LWarRoleInfo, Int64>* player_map_;

	AllLeagueInfoMap league_map_;

	ThreeObjVec league_score_rank_;
};

typedef Singleton<LeagueWarSystem> LeagueWarSystemSingleton;
#define LEAGUE_WAR_SYSTEM  LeagueWarSystemSingleton::instance()

#endif /* LEAGUEWARSYSTEM_H_ */
