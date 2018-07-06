/*
 * TrvlScriptTeam.h
 *
 *  Created on: Sep 2, 2016
 *      Author: peizhibi
 */

#ifndef TRVLSCRIPTTEAM_H_
#define TRVLSCRIPTTEAM_H_

#include "PubStruct.h"

class ProtoTravelTeam;

struct BaseTeam
{
	enum
	{
		MAX_COUNT 			= 3,

		NORAML_TEAM 		= 1,
		TRVL_SCRIPT_TEAM 	= 2,

		END
	};

	BaseTeam();
	void reset();

	void push_teamer(Int64 role);
	void erase_teamer(Int64 role);

	int index();
	int is_full();
	int is_empty();
	int is_leader(Int64 role);
	int is_in_team(Int64 role);
	Int64 leader_id();

	int index_;
	int type_;				//类型
	LongList teamer_list_;	//队员，第一为队长

	int limit_force_;	//战力限制
	int auto_start_;	//满员后自动开始

	string sceret_;		//密码
	string team_name_;	//队伍名字
};

struct TrvlScriptTeam : public BaseTeam
{
	void reset();
	void set_team_name();
	void make_up_team_list(ProtoTravelTeam* proto);

	int is_timeout();

	int scene_id_;
	int script_type_;
	int start_fb_;		//是否已开始副本
	Int64 set_tick_;
};


#endif /* TRVLSCRIPTTEAM_H_ */
