/*
 * TrvlTeamPlayer.cpp
 *
 *  Created on: 2017年5月10日
 *      Author: lyw
 */

#include "TrvlTeamPlayer.h"
#include "TrvlTeamSystem.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "GameFont.h"
#include "SerialDefine.h"
#include "TipsEnum.h"

TrvlTeamPlayer::TrvlTeamPlayer() {
	// TODO Auto-generated constructor stub

}

TrvlTeamPlayer::~TrvlTeamPlayer() {
	// TODO Auto-generated destructor stub
}

void TrvlTeamPlayer::reset_travel_teamer()
{

}

Int64 TrvlTeamPlayer::travel_team_id(void)
{
	TravelTeamInfo *team_info = this->travel_team();
	JUDGE_RETURN(team_info != NULL, 0);

	return team_info->__team_id;
}

TravelTeamInfo *TrvlTeamPlayer::travel_team(void)
{
	return TRAVEL_TEAM_SYS->find_travel_team_by_role_id(this->role_id());
}

TravelTeamInfo::TravelTeamer *TrvlTeamPlayer::travel_teamer(void)
{
	TravelTeamInfo *travel_team = this->travel_team();
	JUDGE_RETURN(travel_team != NULL, NULL);

	return travel_team->trvl_teamer(this->role_id());
}

int TrvlTeamPlayer::request_myself_travel_team_info()
{
	Proto50101501 respond;

	TravelTeamInfo *travel_team = this->travel_team();
	int is_create = 0;
	if (travel_team != NULL)
	{
	    is_create = 1;
	    TRAVEL_TEAM_SYS->refresh_team_signup(travel_team);
	    respond.set_is_signup(travel_team->__is_signup);
	}
	respond.set_is_create(is_create);

	Time_Value nowtime = Time_Value::gettimeofday();

	Time_Value travel_knockout_tick = TRAVEL_TEAM_SYS->travel_peak_quality_start_tick(),
	           travel_promot_tick = TRAVEL_TEAM_SYS->travel_peak_knockout_start_tick(),
	           travel_signup_start_tick = TRAVEL_TEAM_SYS->travel_peak_signup_start_tick(),
	           travel_signup_end_tick = TRAVEL_TEAM_SYS->travel_peak_signup_end_tick();

	int left_travel_peak_sign = TRAVEL_TEAM_SYS->travel_peak_left_sign_sec();
//	Int64 knockout_tick = travel_knockout_tick.sec();

	if (TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == true
			|| nowtime >= travel_signup_end_tick)
	{
	    left_travel_peak_sign = 0;
	}
	else if (nowtime < travel_signup_start_tick)
	{
		left_travel_peak_sign = -1;
	}

	if (travel_team != NULL)
	{
	    respond.set_team_name(travel_team->__team_name);
	    respond.set_leader_id(travel_team->__leader_id);
	    respond.set_auto_signup(travel_team->__auto_signup);
	    respond.set_auto_accept(travel_team->__auto_accept);
	    respond.set_team_force(travel_team->__need_force);

	    //队长放置第一位置
	    TravelTeamInfo::TeamerMap::iterator leader_iter = travel_team->__teamer_map.find(travel_team->__leader_id);
	    if (leader_iter != travel_team->__teamer_map.end())
	    	leader_iter->second.serialize(respond.add_teamer_info());

	    for (TravelTeamInfo::TeamerMap::iterator iter = travel_team->__teamer_map.begin();
	    		iter != travel_team->__teamer_map.end(); ++iter)
	    {
	    	JUDGE_CONTINUE(travel_team->__leader_id != iter->first);

	    	TravelTeamInfo::TravelTeamer &teamer_info = iter->second;
	    	teamer_info.serialize(respond.add_teamer_info());
	    }
	}

	FINER_PROCESS_RETURN(RETURN_SELF_TRAVEL_TEAM_INFO, &respond);
}

int TrvlTeamPlayer::request_create_travel_team_begin(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101502 *, request, msg, -1);

	JUDGE_RETURN(this->add_validate_operate_tick() == true, 0);

	int need_force = request->need_force();
	string team_name = request->team_name();

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	CONDITION_NOTIFY_RETURN(this->role_detail().__level >= peak_base["level_limit"].asInt(), RETURN_CREATE_TRAVEL_TEAM, ERROR_PLAYER_LEVEL);
	CONDITION_NOTIFY_RETURN(this->travel_team() == NULL, RETURN_CREATE_TRAVEL_TEAM, ERROR_SELF_HAS_TRAVEL_TEM);
	CONDITION_NOTIFY_RETURN(0 < team_name.length() && team_name.length() <= MAX_NAME_LENGTH, RETURN_CREATE_TRAVEL_TEAM, ERROR_CLIENT_OPERATE);

	int utf8_length = string_utf8_len(team_name.c_str());
	CONDITION_NOTIFY_RETURN(0 < utf8_length && utf8_length <= GameEnum::MAX_PLAYER_NAME_LEN, RETURN_CREATE_TRAVEL_TEAM, ERROR_NAME_TOO_LONG);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_has_travel_team_name(team_name) == false, RETURN_CREATE_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_NAME_REPEAT);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality() == false && TRAVEL_TEAM_SYS->is_during_knockout() == false,
			RETURN_CREATE_TRAVEL_TEAM, ERROR_OP_IN_ACTIVITY_TICK);

	Proto31402702 inner_req;
	inner_req.set_need_force(need_force);
	inner_req.set_team_name(team_name);

	const Json::Value &money_json = peak_base["create_money"];
	Money cost(money_json[0u].asInt(), money_json[1u].asInt());

	if (GameCommon::validate_money(cost) == true)
	{
		return LOGIC_MONITOR->dispatch_to_scene(this, &inner_req);
	}
	else
	{
		return this->process_create_travel_team_end(&inner_req);
	}
}

int TrvlTeamPlayer::process_create_travel_team_end(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31402702 *, request, msg, -1);

	int need_force = request->need_force();
	std::string team_name = request->team_name();

	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_has_travel_team_name(team_name) == false, RETURN_CREATE_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_NAME_REPEAT);
	CONDITION_NOTIFY_RETURN(this->travel_team() == NULL, RETURN_CREATE_TRAVEL_TEAM, ERROR_SELF_HAS_TRAVEL_TEM);

	int ret = TRAVEL_TEAM_SYS->create_travel_team_and_bind(this, need_force, team_name);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_CREATE_TRAVEL_TEAM, ret);

	this->record_other_serial(TRAVPEAK_SERIAL, SUB_TRAVPEAK_CREATE_TEAM, need_force);

	this->request_myself_travel_team_info();

	this->sync_trvl_team_to_trvl_peak_scene();
	this->sync_offline_hook_to_trvl_peak_scene();

	FINER_PROCESS_NOTIFY(RETURN_CREATE_TRAVEL_TEAM);
}

int TrvlTeamPlayer::request_local_travel_team_list(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101503 *, request, msg, -1);

	LongVec team_set;
	TRAVEL_TEAM_SYS->fetch_team_list(team_set, request->show_no_full());

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page(), team_set.size(), GameEnum::TRVL_TEAM_PAGE_COUNT);

	Proto50101503 respond;
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	for (LongVec::iterator iter = team_set.begin() + page_info.start_index_;
			iter != team_set.end(); ++iter)
	{
		TravelTeamInfo *travel_team = TRAVEL_TEAM_SYS->find_travel_team(*iter);
		JUDGE_CONTINUE(travel_team != NULL);

		ProtoTravelTeam *proto_team = respond.add_team_list();
		proto_team->set_team_id(travel_team->__team_id);
		proto_team->set_team_name(travel_team->__team_name);
		proto_team->set_team_force(travel_team->__need_force);
		proto_team->set_teamer_amount(travel_team->__teamer_map.size());
		proto_team->set_leader_id(travel_team->__leader_id);

		if (travel_team->__apply_map.find(this->role_id()) != travel_team->__apply_map.end())
		{
			proto_team->set_is_apply(1);
		}

		LogicPlayer *leader = this->find_player(travel_team->__leader_id);
		if (leader != NULL)
		{
		    proto_team->set_leader_name(leader->role_detail().__name);
		}
		else
		{
			TravelTeamInfo::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.find(travel_team->__leader_id);
			if (teamer_iter != travel_team->__teamer_map.end())
			{
				TravelTeamInfo::TravelTeamer &teamer_info = teamer_iter->second;
				proto_team->set_leader_name(teamer_info.__teamer_name);
			}
		}

		page_info.add_count_ += 1;
		JUDGE_BREAK(page_info.add_count_ < GameEnum::TRVL_TEAM_PAGE_COUNT);
	}

	FINER_PROCESS_RETURN(RETURN_LOCAL_TRAVEL_TEAM_LIST, &respond);
}

int TrvlTeamPlayer::request_apply_travel_team(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101504 *, request, msg, -1);

	CONDITION_NOTIFY_RETURN(this->travel_team() == NULL, RETURN_APPLY_TRAVEL_TEAM, ERROR_SELF_HAS_TRAVEL_TEM);

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	CONDITION_NOTIFY_RETURN(this->role_detail().__level >= peak_base["level_limit"].asInt(), RETURN_APPLY_TRAVEL_TEAM, ERROR_PLAYER_LEVEL);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_APPLY_TRAVEL_TEAM, ERROR_FORBIT_APPLY_IN_ACTIVITY_TICK);

	Int64 team_id = request->team_id();
	TravelTeamInfo *travel_team = TRAVEL_TEAM_SYS->find_travel_team(team_id);
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_APPLY_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_travel_team_full(travel_team) == false, RETURN_APPLY_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_FULL);
	CONDITION_NOTIFY_RETURN(this->role_detail().__fight_force >= travel_team->__need_force, RETURN_APPLY_TRAVEL_TEAM, ERROR_NOT_ENOUGH_FORCE);

	TRAVEL_TEAM_SYS->insert_apply_set(travel_team, this);

	if (travel_team->__auto_accept > 0)
	{
		int ret = TRAVEL_TEAM_SYS->apply_role_insert_travel_team(travel_team, this->role_id());
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_APPLY_TRAVEL_TEAM, ret);

		TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);
	}

	Proto50101504 respond;
	respond.set_team_id(team_id);
	FINER_PROCESS_RETURN(RETURN_APPLY_TRAVEL_TEAM, &respond);
}

int TrvlTeamPlayer::request_travel_team_apply_list()
{
	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_TRAVEL_TEAM_APPLY_LIST, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_TRAVEL_TEAM_APPLY_LIST, ERROR_NO_TRAVEL_TEAM_LEADER);

	Proto50101505 respond;

	for (TravelTeamInfo::TeamerMap::iterator iter = travel_team->__apply_map.begin();
			iter != travel_team->__apply_map.end(); ++iter)
	{
		TravelTeamInfo::TravelTeamer &applier_info = iter->second;
		ProtoTeamer *proto_teamer = respond.add_teamer_list();
		proto_teamer->set_role_id(applier_info.__teamer_id);
		proto_teamer->set_role_name(applier_info.__teamer_name);
		proto_teamer->set_role_sex(applier_info.__teamer_sex);
		proto_teamer->set_role_career(applier_info.__teamer_career);
		proto_teamer->set_role_level(applier_info.__teamer_level);
		proto_teamer->set_role_force(applier_info.__teamer_force);

		if (this->find_player(applier_info.__teamer_id) != NULL)
			proto_teamer->set_online_flag(1);
		else
			proto_teamer->set_online_flag(2);
	}

	FINER_PROCESS_RETURN(RETURN_TRAVEL_TEAM_APPLY_LIST, &respond);
}

int TrvlTeamPlayer::request_reply_travel_team_apply(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101506 *, request, msg, -1);

	Int64 apply_id = request->apply_id();
	int reply_type = request->reply();

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_REPLY_TRAVEL_TEAM_APPLY, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_REPLY_TRAVEL_TEAM_APPLY, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_REPLY_TRAVEL_TEAM_APPLY, ERROR_OP_IN_ACTIVITY_TICK);

	switch (reply_type)
	{
	case 1:
	{
		CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_travel_team_full(travel_team) == false,
				RETURN_REPLY_TRAVEL_TEAM_APPLY, ERROR_TRAVEL_TEAM_FULL);
		CONDITION_NOTIFY_RETURN(travel_team->__apply_map.count(apply_id) > 0,
				RETURN_REPLY_TRAVEL_TEAM_APPLY, ERROR_CLIENT_OPERATE);

		int ret = TRAVEL_TEAM_SYS->apply_role_insert_travel_team(travel_team, apply_id);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REPLY_TRAVEL_TEAM_APPLY, ret);

		TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

		// 接受加入提示
		LogicPlayer *applier = this->find_player(apply_id);
		if (applier != NULL)
		{
			Proto80101509 act_notify;
			act_notify.set_type(1);
			act_notify.set_team_name(travel_team->__team_name);
			applier->respond_to_client(ACTIVE_TRAVPEAK_TEAMER_VARY, &act_notify);
		}
		break;
	}
	case 2:
	{
		TRAVEL_TEAM_SYS->remove_apply_set(travel_team, apply_id);
		break;
	}
	case 3:
	{
		travel_team->__apply_map.clear();
		break;
	}
	default:
		break;
	}

	this->sync_trvl_team_to_trvl_peak_scene();

	LogicPlayer *applier = this->find_player(apply_id);
	if (applier != NULL)
	{
		applier->sync_offline_hook_to_trvl_peak_scene();
	}
	else
	{
		this->sync_offline_hook_to_trvl_peak_scene(apply_id);
	}

	this->request_travel_team_apply_list();

	FINER_PROCESS_NOTIFY(RETURN_REPLY_TRAVEL_TEAM_APPLY);
}

int TrvlTeamPlayer::request_change_travel_team_leader(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101507 *, request, msg, -1);

	Int64 new_leader_id = request->target_id();

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_CHANGE_TRAVEL_TEAM_LEADER, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_CHANGE_TRAVEL_TEAM_LEADER, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id != new_leader_id, RETURN_CHANGE_TRAVEL_TEAM_LEADER, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_in_travel_team(travel_team, new_leader_id) == true, RETURN_CHANGE_TRAVEL_TEAM_LEADER, ERROR_NOT_IN_TRAVEL_TEAM);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_CHANGE_TRAVEL_TEAM_LEADER, ERROR_OP_IN_ACTIVITY_TICK);

	travel_team->__leader_id = new_leader_id;

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	this->sync_trvl_team_to_trvl_peak_scene();

	FINER_PROCESS_NOTIFY(RETURN_CHANGE_TRAVEL_TEAM_LEADER);
}

int TrvlTeamPlayer::request_kick_travel_teamer(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101508 *, request, msg, -1);

	Int64 teamer_id = request->teamer_id();

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_KICK_TRAVEL_TEAMER, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_KICK_TRAVEL_TEAMER, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(teamer_id != this->role_id(), RETURN_KICK_TRAVEL_TEAMER, ERROR_TRAVEL_TEAM_LEADER_LEAVE);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_in_travel_team(travel_team, teamer_id) == true, RETURN_KICK_TRAVEL_TEAMER, ERROR_NOT_IN_TRAVEL_TEAM);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_KICK_TRAVEL_TEAMER, ERROR_OP_IN_ACTIVITY_TICK);

	TRAVEL_TEAM_SYS->remove_travel_teamer(travel_team, teamer_id);
	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	// 被队伍踢出提示
	LogicPlayer *applier = this->find_player(teamer_id);
	if (applier != NULL)
	{
		Proto80101509 act_notify;
		act_notify.set_type(2);
		act_notify.set_team_name(travel_team->__team_name);
		applier->respond_to_client(ACTIVE_TRAVPEAK_TEAMER_VARY, &act_notify);
	}

	this->sync_trvl_team_to_trvl_peak_scene();

	FINER_PROCESS_NOTIFY(RETURN_KICK_TRAVEL_TEAMER);
}

int TrvlTeamPlayer::request_other_travel_team_info(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101510 *, request, msg, -1);
	Int64 team_id = request->team_id();

	TravelTeamInfo *trvl_team = TRAVEL_TEAM_SYS->find_travel_team(team_id);
	if (trvl_team != NULL)
	{
		Proto50101510 respond;
		respond.set_team_id(trvl_team->__team_id);
		respond.set_team_name(trvl_team->__team_name);

		TravelTeamInfo::TeamerMap::iterator leader_iter = trvl_team->__teamer_map.find(trvl_team->__leader_id);
		if (leader_iter != trvl_team->__teamer_map.end())
			leader_iter->second.serialize(respond.add_teamer_list());

		for (TravelTeamInfo::TeamerMap::iterator iter = trvl_team->__teamer_map.begin();
				iter != trvl_team->__teamer_map.end(); ++iter)
		{
			JUDGE_CONTINUE(trvl_team->__leader_id != iter->first);

			TravelTeamInfo::TravelTeamer &teamer_info = iter->second;
			teamer_info.serialize(respond.add_teamer_list());
		}

		FINER_PROCESS_RETURN(RETURN_OTHER_TRAVEL_TEAM_INFO, &respond);
	}
	else
	{
		Proto30402005 inner_req;
		inner_req.set_team_id(team_id);
		return this->dispatch_to_scene_server(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
	}
}

int TrvlTeamPlayer::request_invite_to_travel_team(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101511 *, request, msg, -1);

	Int64 role_id = request->role_id();

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_INVITE_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_INVITE_TRAVEL_TEAM, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_INVITE_TRAVEL_TEAM, ERROR_OP_IN_ACTIVITY_TICK);

	LogicPlayer *applier = this->find_player(role_id);
	CONDITION_NOTIFY_RETURN(applier != NULL, RETURN_INVITE_TRAVEL_TEAM, ERROR_PLAYER_OFFLINE);
	CONDITION_NOTIFY_RETURN(applier->travel_team() == NULL, RETURN_INVITE_TRAVEL_TEAM, ERROR_TARGET_HAS_TRAVEL_TEAM);

	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	CONDITION_NOTIFY_RETURN(applier->role_detail().__level >= peak_base["level_limit"].asInt(), RETURN_CREATE_TRAVEL_TEAM, ERROR_PLAYER_LEVEL);

	travel_team->__invite_role_set.insert(applier->role_id());

	Proto80101501 respond;
	respond.set_team_id(travel_team->__team_id);
	respond.set_team_name(travel_team->__team_name);
	respond.set_leader_id(this->role_id());
	respond.set_leader_name(this->role_detail().__name);
	applier->respond_to_client(ACTIVE_INVITE_TRAVEL_TEAM, &respond);

	FINER_PROCESS_NOTIFY(RETURN_INVITE_TRAVEL_TEAM);
}

int TrvlTeamPlayer::request_reply_invite_travel_team(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101512 *, request, msg, -1);
	Int64 team_id = request->team_id();
	int reply_type = request->reply_type();

	TravelTeamInfo *travel_team = TRAVEL_TEAM_SYS->find_travel_team(team_id);
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_REPLY_TRAVEL_TEAM_INVITE, ERROR_TRAVEL_TEAM_DIMISSED);

	if (reply_type == 2)
	{
	    FINER_PROCESS_NOTIFY(RETURN_REPLY_TRAVEL_TEAM_INVITE);
	}

	CONDITION_NOTIFY_RETURN(this->travel_team() == NULL, RETURN_REPLY_TRAVEL_TEAM_INVITE, ERROR_SELF_HAS_TRAVEL_TEM);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_travel_team_full(travel_team) == false, RETURN_REPLY_TRAVEL_TEAM_INVITE, ERROR_TRAVEL_TEAM_FULL);
	CONDITION_NOTIFY_RETURN(travel_team->__invite_role_set.find(this->role_id()) != travel_team->__invite_role_set.end(),
	            RETURN_REPLY_TRAVEL_TEAM_INVITE, ERROR_NO_INVITE_TRAVEL_TEAM);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_REPLY_TRAVEL_TEAM_INVITE, ERROR_OP_IN_ACTIVITY_TICK);

	travel_team->__invite_role_set.erase(this->role_id());

	TRAVEL_TEAM_SYS->insert_apply_set(travel_team, this);

	int ret = TRAVEL_TEAM_SYS->apply_role_insert_travel_team(travel_team, this->role_id());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REPLY_TRAVEL_TEAM_INVITE, ret);

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	this->sync_trvl_team_to_trvl_peak_scene();
	this->sync_offline_hook_to_trvl_peak_scene();

	FINER_PROCESS_NOTIFY(RETURN_REPLY_TRAVEL_TEAM_INVITE);
}

int TrvlTeamPlayer::request_dimiss_travel_team(void)
{
	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_DIMISS_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_DIMISS_TRAVEL_TEAM, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_DIMISS_TRAVEL_TEAM, ERROR_OP_IN_ACTIVITY_TICK);

	this->sync_trvl_team_to_trvl_peak_scene(SYNC_OP_TYPE_DEL);

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);
	TRAVEL_TEAM_SYS->dimiss_travel_team(travel_team);

	FINER_PROCESS_NOTIFY(RETURN_DIMISS_TRAVEL_TEAM);
}

int TrvlTeamPlayer::request_leave_travel_team(void)
{
	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_LEAVE_TRAVEL_TEAM, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_LEAVE_TRAVEL_TEAM, ERROR_OP_IN_ACTIVITY_TICK);

	if (travel_team->__leader_id != this->role_id())
	{
		TRAVEL_TEAM_SYS->remove_travel_teamer(travel_team, this->role_id());
		TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

		this->sync_trvl_team_to_trvl_peak_scene(SYNC_OP_TYPE_UPDATE, travel_team->__team_id);
	}
	else
	{
		PairObjVec teamer_vec;
		for (TravelTeamInfo::TeamerMap::iterator it = travel_team->__teamer_map.begin();
				it != travel_team->__teamer_map.end(); ++it)
		{
			JUDGE_CONTINUE(it->first != travel_team->__leader_id);
			JUDGE_CONTINUE(travel_team->member_is_pass_miss_day(it->first) == false)

			PairObj obj(it->first, it->second.__teamer_force);
			teamer_vec.push_back(obj);
		}

		if (teamer_vec.size() > 0)
		{
			std::sort(teamer_vec.begin(), teamer_vec.end(), GameCommon::pair_comp_by_desc);
			travel_team->__leader_id = teamer_vec[0].id_;

			TRAVEL_TEAM_SYS->remove_travel_teamer(travel_team, this->role_id());
			TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

			this->sync_trvl_team_to_trvl_peak_scene(SYNC_OP_TYPE_UPDATE, travel_team->__team_id);
		}
		else
		{
			TRAVEL_TEAM_SYS->remove_travel_teamer(travel_team, this->role_id());
			TRAVEL_TEAM_SYS->dimiss_travel_team(travel_team);

			this->sync_trvl_team_to_trvl_peak_scene(SYNC_OP_TYPE_DEL, travel_team->__team_id);
		}
	}

	this->request_myself_travel_team_info();

	FINER_PROCESS_NOTIFY(RETURN_LEAVE_TRAVEL_TEAM);
}

int TrvlTeamPlayer::request_set_team_force(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101515 *, request, msg, -1);

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_SET_TEAM_FORCE, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_SET_TEAM_FORCE, ERROR_NO_TRAVEL_TEAM_LEADER);
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_quality_and_knockout() == false, RETURN_SET_TEAM_FORCE, ERROR_OP_IN_ACTIVITY_TICK);

	travel_team->__need_force = request->need_force();
	if (request->name().empty() == false)
	{
		travel_team->__team_name = request->name();
	}

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	FINER_PROCESS_NOTIFY(RETURN_SET_TEAM_FORCE);
}

int TrvlTeamPlayer::request_set_team_auto_type(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10101516 *, request, msg, -1);

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_SET_TEAM_AUTO_TYPE, ERROR_TRAVEL_TEAM_DIMISSED);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_SET_TEAM_AUTO_TYPE, ERROR_NO_TRAVEL_TEAM_LEADER);

	if (request->auto_type() == 1)
	{
		travel_team->__auto_accept = request->is_auto();
	}
	else
	{
		travel_team->__auto_signup = request->is_auto();
	}

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	FINER_PROCESS_NOTIFY(RETURN_SET_TEAM_AUTO_TYPE);
}

int TrvlTeamPlayer::request_signup_travel_peak()
{
	CONDITION_NOTIFY_RETURN(TRAVEL_TEAM_SYS->is_during_signup() == true, RETURN_SIGNUP_TRAVEL_PEAK, ERROR_TRAVEL_PEAK_NOT_SIGNUP_TICK);

	int open_day = CONFIG_INSTANCE->open_server_days();
	const Json::Value &peak_base = CONFIG_INSTANCE->travel_peak_base();
	CONDITION_NOTIFY_RETURN(open_day >= peak_base["open_day"].asInt() || CONFIG_INSTANCE->do_combine_server(), RETURN_SIGNUP_TRAVEL_PEAK, ERROR_OPEN_SERVER_DAY);

	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_SIGNUP_TRAVEL_PEAK, ERROR_NO_TRAVEL_TEAM);
	CONDITION_NOTIFY_RETURN(travel_team->__leader_id == this->role_id(), RETURN_SIGNUP_TRAVEL_PEAK, ERROR_NO_TRAVEL_TEAM_LEADER);
//	CONDITION_NOTIFY_RETURN(travel_team->__teamer_map.size() == 3, RETURN_SIGNUP_TRAVEL_PEAK, ERROR_TEAMER_NOT_ENOUGH);
	TRAVEL_TEAM_SYS->refresh_team_signup(travel_team);
	CONDITION_NOTIFY_RETURN(travel_team->__is_signup == 0, RETURN_SIGNUP_TRAVEL_PEAK, ERROR_HAS_SIGNUP_TRAVPEAK);

	travel_team->__is_signup = 1;
	this->sync_trvl_team_to_trvl_peak_scene();

	Proto30402006 inner_req;
	inner_req.set_team_id(travel_team->__team_id);
	LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);

	this->sync_all_teamer_offline_hook_to_trvl_peak_scene();

	TRAVEL_TEAM_SYS->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

	return 0;
}

int TrvlTeamPlayer::process_set_signup_travpeak_flag(Message *msg)
{
	TravelTeamInfo *travel_team = this->travel_team();
	CONDITION_NOTIFY_RETURN(travel_team != NULL, RETURN_SIGNUP_TRAVEL_PEAK, ERROR_NO_TRAVEL_TEAM);

	// 跨服成功报名后才处理报名标识
	travel_team->__is_signup = 1;

	FINER_PROCESS_NOTIFY(RETURN_SIGNUP_TRAVEL_PEAK);
}

int TrvlTeamPlayer::sync_trvl_team_to_trvl_peak_scene(int op_type, Int64 org_team_id)
{
	Int64 team_id = org_team_id;
	if (team_id <= 0)
		team_id = this->travel_team_id();

	TRAVEL_TEAM_SYS->sync_trvl_team_to_trvl_peak_scene(op_type, team_id, this->gate_sid(), this);

	return 0;
}

int TrvlTeamPlayer::sync_offline_hook_to_trvl_peak_scene(const Int64 src_role_id)
{
	JUDGE_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false, 0);

	Int64 role_id = src_role_id;
	if (role_id <= 0)
	    role_id = this->role_id();
	TravelTeamInfo *travel_team = TRAVEL_TEAM_SYS->find_travel_team_by_role_id(role_id);
	JUDGE_RETURN(travel_team != NULL, 0);

	Proto30402003 inner_req;
	inner_req.set_team_id(travel_team->__team_id);
	inner_req.set_role_id(role_id);
	return this->dispatch_to_map_server(&inner_req);
}

void TrvlTeamPlayer::sync_all_teamer_offline_hook_to_trvl_peak_scene(void)
{
	TravelTeamInfo *travel_team = this->travel_team();
	LogicPlayer *player = NULL;

	for (TravelTeamInfo::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
	        teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
	{
	    TravelTeamInfo::TravelTeamer &teamer_info = teamer_iter->second;
	    player = this->find_player(teamer_info.__teamer_id);

	    if (player != NULL)
	    {
	        player->sync_offline_hook_to_trvl_peak_scene();
	    }
	    else
	    {
	        this->sync_offline_hook_to_trvl_peak_scene(teamer_info.__teamer_id);
	    }
	}
}

void TrvlTeamPlayer::trvl_teamer_sign_out()
{
	TravelTeamInfo *travel_team = this->travel_team();
	JUDGE_RETURN(travel_team != NULL, ;);

	TravelTeamInfo::TravelTeamer *trvl_teamer = this->travel_teamer();
	JUDGE_RETURN(trvl_teamer != NULL, ;);

	trvl_teamer->__logout_tick = Time_Value::gettimeofday();
	travel_team->__last_logout_tick = Time_Value::gettimeofday();
}

void TrvlTeamPlayer::handle_resex_teamer()
{
	TravelTeamInfo::TravelTeamer *trvl_teamer = this->travel_teamer();
	JUDGE_RETURN(trvl_teamer != NULL, ;);

	trvl_teamer->__teamer_sex = this->role_detail().__sex;
	trvl_teamer->__teamer_career = this->role_detail().__career;

	this->sync_trvl_team_to_trvl_peak_scene();
	this->sync_offline_hook_to_trvl_peak_scene();
}

void TrvlTeamPlayer::handle_rename_teamer()
{
	TravelTeamInfo::TravelTeamer *trvl_teamer = this->travel_teamer();
	JUDGE_RETURN(trvl_teamer != NULL, ;);

	trvl_teamer->__teamer_name = this->name();

	this->sync_trvl_team_to_trvl_peak_scene();
	this->sync_offline_hook_to_trvl_peak_scene();
}

void TrvlTeamPlayer::update_teamer_force(int force)
{
	TravelTeamInfo::TravelTeamer *trvl_teamer = this->travel_teamer();
	JUDGE_RETURN(trvl_teamer != NULL, ;);

	trvl_teamer->__teamer_force = force;
	trvl_teamer->__teamer_level = this->role_level();

	TRAVEL_TEAM_SYS->set_update_teamer_info(this->travel_team_id(), this->role_id());
}


