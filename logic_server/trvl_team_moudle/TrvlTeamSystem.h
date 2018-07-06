/*
 * TrvlTeamSystem.h
 *
 *  Created on: 2017年5月10日
 *      Author: lyw
 */

#ifndef TRVLTEAMSYSTEM_H_
#define TRVLTEAMSYSTEM_H_

#include "GameHeader.h"
#include "ObjectPoolEx.h"
#include "LogicStruct.h"

class TrvlTeamPlayer;
class BaseLogicPlayer;
class TravelTeamer;

template<class Key, class Value, class HSMUTEX>
class HashMap;

class TrvlTeamSystem
{
	friend class MMOTravTeam;

public:
	typedef HashMap<Int64, TravelTeamInfo *, NULL_MUTEX> TravelTeamMap;
	typedef ObjectPoolEx<TravelTeamInfo> TravelTeamInfoPool;
	typedef std::set<std::string> TravelTeamNameSet;

	class TeamTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	TrvlTeamSystem();
	virtual ~TrvlTeamSystem();

	int start(void);
	int stop(void);

	int bind_travel_team(const Int64 team_id, TravelTeamInfo *team_info);
	int unbind_travel_team(const Int64 team_id);
	int unbind_and_recycle_travel_team(TravelTeamInfo *team_info);

	TravelTeamInfo *find_travel_team(const Int64 team_id);
	TravelTeamInfo *find_travel_team_by_role_id(const Int64 role_id);

	void refresh_team_signup(TravelTeamInfo *trvl_team);
	void fetch_team_list(LongVec& team_vec, const int show_no_full);

	bool is_travel_team_full(TravelTeamInfo *trvl_team);

	bool is_during_quality_and_knockout(void);
	bool is_during_signup(void);
	bool is_during_quality(void);
	bool is_during_knockout(void);
	bool is_during_knockout_bet(void);

	bool is_has_travel_team_name(const string &team_name);
	TravelTeamInfo *create_travel_team(void);
	int create_travel_team_and_bind(TrvlTeamPlayer *leader, int need_force, string &team_name);
	int dimiss_travel_team(TravelTeamInfo *team_info);

	bool is_in_travel_team(TravelTeamInfo *team_info, const Int64 teamer_id);
	int insert_travel_teamer(TravelTeamInfo *team_info, TravelTeamInfo::TravelTeamer &teamer);
	int remove_travel_teamer(TravelTeamInfo *team_info, const Int64 teamer_id);

	void init_travel_team(TravelTeamInfo *team_info, int need_force, string &team_name, Int64 leader_id);

	void copy_to_travel_teamer(TravelTeamInfo::TravelTeamer &teamer_info, BaseLogicPlayer *player);
	void copy_to_travel_teamer(TravelTeamInfo::TravelTeamer &teamer, TravelTeamInfo::TravelTeamer &applier);

	int insert_apply_set(TravelTeamInfo *team_info, BaseLogicPlayer *player);
	int remove_apply_set(TravelTeamInfo *team_info, const Int64 apply_id);

	int apply_role_insert_travel_team(TravelTeamInfo *team_info, const Int64 apply_id);
	int notify_all_teamer(TravelTeamInfo *travel_team, const int recogn, Message *msg = NULL);

	Time_Value &travel_peak_quality_start_tick(void);
	Time_Value &travel_peak_knockout_start_tick(void);
	Time_Value &travel_peak_signup_start_tick(void);
	Time_Value &travel_peak_signup_end_tick(void);
	int travel_peak_left_sign_sec(void);

	int save_update_travel_team(void);
	int set_update_teamer_info(Int64 team_id, Int64 role_id);

	void sort_travel_team(void);
	void handle_time_out_team();
	void update_teamer_force();

	int sync_trvl_team_to_trvl_peak_scene(int op_type, Int64 team_id, int gate_sid = 0, TrvlTeamPlayer *player = NULL);
	int process_fetch_trvl_peak_activity_tick();
	int process_sync_trvl_peak_activity_tick(Message *msg);
	int update_trvl_team_signup_state(Message *msg);

private:
	TravelTeamInfoPool *travel_team_info_pool_;
	TravelTeamMap *travel_team_map_;        	// key: team_id, value: TravelTeamInfo *
	TravelTeamMap *role_travel_team_map_;   	// key: role_id, value: TravelTeamInfo *
	TravelTeamNameSet travel_team_name_set_;    // key: team name
	PairObjVec team_sort_list_;           		// 排序战队

	LongMap update_teamer_map_;			// 定时更新有战力变化的战队

	TeamTimer team_timer_;

	Time_Value signup_start_tick_;
	Time_Value signup_end_tick_;
	Time_Value quality_start_tick_;
	Time_Value quality_end_tick_;
	Time_Value knockout_start_tick_;
	Time_Value knockout_end_tick_;

	Time_Value award_start_tick_;
	Time_Value award_end_tick_;

};

typedef Singleton<TrvlTeamSystem> TrvlTeamSystemSingle;
#define TRAVEL_TEAM_SYS (TrvlTeamSystemSingle::instance())

#endif /* TRVLTEAMSYSTEM_H_ */
