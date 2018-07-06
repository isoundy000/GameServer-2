/*
 * LeagueMonitor.h
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#ifndef LEAGUEMONITOR_H_
#define LEAGUEMONITOR_H_

#include "MapMapStruct.h"
//#include "LeagueBoss.h"

struct MapLeagueInfo
{
	void reset();

	Int64 id_;
	string name_;

	int force_;
	int flag_lvl_;		//帮旗等级
	Int64 leader_id_;
	string leader_;
};

class LeagueBoss;

class LeagueMonitor
{
public:
	class BossTimer : public GameTimer
	{
	public:
		BossTimer(void);
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
		void init(LeagueMonitor* parent);

		void set_activity_id(int activity_id);
		int activity_id();

	private:
		LeagueMonitor* league_monitor_;
		int activity_id_;
	};

	class LeagueSyncTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	LeagueMonitor(void);
	~LeagueMonitor(void);

	void init();
	void fina();

	void star_league_boss();
	int handle_boss_timeout();
	int handle_boss_i(int state);

	int ahead_boss_event();
	int start_boss_event();
	int stop_boss_event();

	int is_boss_time();
	int activity_left_tick();
	int fetch_league_camp(Int64 league_id);

	//map league
	int create_map_league(Message* msg);
	int recycle_map_league(Message* msg);
	int update_league_leader(Message* msg);

	//league boss
	int create_league_boss_scene(Message* msg);
	int request_enter_league_boss(int gate_id, Proto30400051* request);
	int fetch_league_flag(Message* msg);
	int summon_league_boss(int gate_id, Int64 role_id, Message* msg);
	int recycle_league_boss(Message* msg);

	int handle_quit_league(int sid, Message* msg);
	int handle_rename_league(int sid, Int64 role_id, Message* msg);

	int is_in_league_activity();

public:
	LeagueBoss* find_boss(Int64 league_index);
	MapLeagueInfo* find_map_league(Int64 id);

private:
	int init_flag_;
	int sync_flag_;

	LongMap league_camp_map_;
	BIntLongMap league_space_map_;

	BossTimer boss_timer_;
	ActivityTimeInfo boss_time_info_;

	PoolPackage<MapLeagueInfo, Int64>* all_map_league_;
	PoolPackage<LeagueBoss, Int64>* boss_package_;
};

typedef Singleton<LeagueMonitor> 	LeagueMonitorSingle;
#define LEAGUE_MONITOR     			LeagueMonitorSingle::instance()

#endif /* LEAGUEMONITOR_H_ */
