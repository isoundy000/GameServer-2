/*
 * ScriptSystem.h
 *
 *  Created on: 2016年10月26日
 *      Author: lyw
 */

#ifndef SCRIPTSYSTEM_H_
#define SCRIPTSYSTEM_H_

#include "ScriptStruct.h"
#include "PubStruct.h"
#include "ProtoDefine.h"
#include "MapMapStruct.h"

class ScriptSystem
{
public:
	enum
	{
		RANK_PAGE = 10,

		END
	};

public:
	ScriptSystem();
	virtual ~ScriptSystem();

	void star();
	void stop();

	int sweep_update_top_rank(int gate_id, Message* msg);
	int update_legend_top_rank(MapPlayer* player, int script_type, int floor);
	int update_legend_top_rank_info(Int64 role_id, std::string name, int fight_score, int script_type, int floor);
	int request_fetch_rank_info(int gate_id, Int64 role_id, Message* msg);
	int fetch_rank_info(int gate_id, Int64 role_id, int num1 = 1, int num2 = 10);
	int fetch_sword_rank_info(int gate_id, Int64 role_id, int page);

	void check_rank_reset();
	void test_rank_reset();

	int add_couple_script_times(Message* msg);
	int login_fetch_couple_script_times(int sid, Int64 role_id, Message* msg);

	int set_double_script(int gate_id, Message* msg);
	int fetch_script_mult(int script_type);
	int sweep_fetch_script_mult(int sid, Int64 role_id, Message* msg);

protected:
	LegendTopPlayer &top_player(int script_type);
	ThreeObjVec &rank_vec(int script_type);
	LegendTopPlayer::PlayerInfo *player_info(int script_type, Int64 role_id);
	void sort_rank(int script_type);

private:
	LegendTopPlayer legend_player_;
	LegendTopPlayer sword_player_;
	ThreeObjVec legend_rank_vec_;
	ThreeObjVec sword_rank_vec_;
	LongMap role_map_;
	IntMap double_script_map_;
};

typedef Singleton<ScriptSystem> ScriptSystemSingleton;
#define SCRIPT_SYSTEM  ScriptSystemSingleton::instance()

#endif /* SCRIPTSYSTEM_H_ */
