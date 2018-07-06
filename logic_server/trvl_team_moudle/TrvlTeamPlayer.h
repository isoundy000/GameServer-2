/*
 * TrvlTeamPlayer.h
 *
 *  Created on: 2017年5月10日
 *      Author: lyw
 */

#ifndef TRVLTEAMPLAYER_H_
#define TRVLTEAMPLAYER_H_

#include "BaseLogicPlayer.h"

class TrvlTeamPlayer : virtual public BaseLogicPlayer
{
public:
	enum
	{
	    SYNC_OP_TYPE_UPDATE = 1,    // 同步更新
	    SYNC_OP_TYPE_DEL    = 2     // 同步删除
	};

public:
	TrvlTeamPlayer();
	virtual ~TrvlTeamPlayer();
	void reset_travel_teamer(void);

	Int64 travel_team_id(void);
	TravelTeamInfo *travel_team(void);
	TravelTeamInfo::TravelTeamer *travel_teamer(void);

	int request_myself_travel_team_info(void);	//获取个人战队信息

	int request_create_travel_team_begin(Message *msg);	//创建战队
	int process_create_travel_team_end(Message *msg);

	int request_local_travel_team_list(Message *msg);
	int request_apply_travel_team(Message *msg);
	int request_travel_team_apply_list();
	int request_reply_travel_team_apply(Message *msg);
	int request_change_travel_team_leader(Message *msg);
	int request_kick_travel_teamer(Message *msg);

	int request_other_travel_team_info(Message *msg);

	int request_invite_to_travel_team(Message *msg);
	int request_reply_invite_travel_team(Message *msg);
	int request_dimiss_travel_team(void);
	int request_leave_travel_team(void);
	int request_set_team_force(Message *msg);
	int request_set_team_auto_type(Message *msg);

	int request_signup_travel_peak();
	int process_set_signup_travpeak_flag(Message *msg);

public:
	int sync_trvl_team_to_trvl_peak_scene(int op_type = 1, Int64 team_id = 0);
	int sync_offline_hook_to_trvl_peak_scene(const Int64 role_id = 0);
	void sync_all_teamer_offline_hook_to_trvl_peak_scene(void);

	void trvl_teamer_sign_out();
	void handle_resex_teamer();
	void handle_rename_teamer();
	void update_teamer_force(int force);
};

#endif /* TRVLTEAMPLAYER_H_ */
