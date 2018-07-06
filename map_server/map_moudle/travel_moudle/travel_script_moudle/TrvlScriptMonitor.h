/*
 * TrvlScriptMonitor.h
 *
 *  Created on: Sep 2, 2016
 *      Author: peizhibi
 */

#ifndef TRVLSCRIPTMONITOR_H_
#define TRVLSCRIPTMONITOR_H_

#include "PubStruct.h"

class TrvlScriptTeam;
class Proto30400502;
class Proto30400503;

struct TrvlScriptRole: public BaseServerInfo, public BaseMember {
	TrvlScriptRole();
	void reset();
	void serialize(ProtoTeamer* proto);

	int sid_;
	int team_index_;
	int script_type_;
	int prep_flag_;		//是否准备
};

class TrvlScriptMonitor {
public:
	enum {
		TOTAL_SCRIPT_TYPE = 9,
		END
	};

public:
	TrvlScriptMonitor();
	~TrvlScriptMonitor();

	int fetch_sid(Int64 role);
	int scene_to_type(int scene_id);
	int handle_team_timeout();

	PoolPackage<TrvlScriptTeam>* fetch_package(int type);
	TrvlScriptRole* find_and_pop(Int64 role);
	TrvlScriptRole* find_role(Int64 role);
	TrvlScriptTeam* find_team(TrvlScriptRole* trvl_role);

	int modify_team_operate(int sid, Int64 role, Message* msg);
	int create_team(int sid, Int64 role, Proto30400502* request);
	int add_team(int sid, Int64 role, Proto30400502* request);

	int inner_team_operate(int sid, Int64 role, Message* msg);
	int fetch_team_list(int sid, Int64 role, Proto30400503* request);
	int quit_team(int sid, Int64 role, Proto30400503* request);
	int fetch_team_info(int sid, Int64 role, Proto30400503* request);
	int start_team_info(int sid, Int64 role, Proto30400503* request);
	int operate_team_info(int sid, Int64 role, Proto30400503* request);
	int kick_team(int sid, Int64 role, Proto30400503* request);

	int fetch_team_info(int sid, Int64 role, TrvlScriptTeam* team);
	int fetch_team_list(Int64 role, int scene_id);
	int fetch_team_list(int sid, Int64 role, int scene_id);
	void notify_all_team_info(TrvlScriptTeam* team);

	int quit_team_i(Int64 role);
	int check_start_team_script(TrvlScriptTeam* team, int all = true);

private:
	void recycle_team(TrvlScriptTeam* team);
	void add_team_i(TrvlScriptTeam* team, int sid, Int64 role,
			Proto30400502* request);

private:
	int init_;
	PoolPackage<TrvlScriptRole, Int64>* player_map_;
	PoolPackage<TrvlScriptTeam>* script_package_[TOTAL_SCRIPT_TYPE];
};

typedef Singleton<TrvlScriptMonitor> TrvlScriptMonitorSingle;
#define TRVL_SCRIPT_MONITOR   (TrvlScriptMonitorSingle::instance())

#endif /* TRVLSCRIPTMONITOR_H_ */
