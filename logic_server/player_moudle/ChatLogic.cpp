/*
 * ChatLogic.cpp
 *
 *  Created on: 2013-7-8
 *      Author: root
 */

#include "ChatLogic.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"

ChatLogic::ChatLogic()
{
	// TODO Auto-generated constructor stub

}

ChatLogic::~ChatLogic()
{
	// TODO Auto-generated destructor stub
}

int ChatLogic::chat_establish_team(void)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto30200108 team_request;
	team_request.set_leader_id(player->role_id());
	team_request.set_team_id(team_panel->team_index_);
	return player->monitor()->dispatch_to_chat(player,&team_request);
}

int ChatLogic::chat_dismiss_team(void)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto30200109 team_request;
	team_request.set_leader_id(player->role_id());
	team_request.set_team_id(team_panel->team_index_);
	return player->monitor()->dispatch_to_chat(player,&team_request);
}

int ChatLogic::chat_join_team(int team_id)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto30200110 team_request;
	team_request.set_leader_id(team_panel->leader_id_);
	team_request.set_team_id(team_panel->team_index_);
	team_request.set_role_id(player->role_id());
	return player->monitor()->dispatch_to_chat(player,&team_request);
}

int ChatLogic::chat_leave_team(void)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto30200111 team_request;
	team_request.set_leader_id(0);
	team_request.set_team_id(team_panel->team_index_);
	team_request.set_role_id(player->role_id());
	return player->monitor()->dispatch_to_chat(player,&team_request);
}

int ChatLogic::chat_establish_league(Int64 league_id)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200104 league_request;
	league_request.set_master_id(player->role_id());
	league_request.set_league_id(league_id);
	return player->monitor()->dispatch_to_chat(player,&league_request);
}


int ChatLogic::chat_dismiss_league(Int64 league_id)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200105 league_request;
	league_request.set_master_id(player->role_id());
	league_request.set_league_id(league_id);
	return player->monitor()->dispatch_to_chat(player,&league_request);
}

int ChatLogic::chat_join_league(Int64 league_id, string league_name)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200106 league_request;
	league_request.set_master_id(0);
	league_request.set_league_id(league_id);
	league_request.set_league_name(league_name);
	player->monitor()->dispatch_to_chat(player,&league_request);

	Proto80200004 respond;
	respond.set_chat_channel(CHANNEL_LEAGUE);
	respond.set_opra(ChatLogic::JOIN_STATUS);
	player->monitor()->dispatch_to_client(player->gate_sid(), player->role_id(), 0, &respond);
	return 0;
}

int ChatLogic::chat_leave_league(Int64 league_id)
{
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200107 league_request;
	league_request.set_master_id(0);
	league_request.set_league_id(league_id);
	league_request.set_role_id(player->role_id());
	player->monitor()->dispatch_to_chat(player,&league_request);

	Proto80200004 respond;
	respond.set_chat_channel(CHANNEL_LEAGUE);
	respond.set_opra(ChatLogic::QUIT_STATUS);
	player->monitor()->dispatch_to_client(player->gate_sid(), player->role_id(), 0, &respond);
	return 0;
}

int ChatLogic::chat_add_black_list(LongVec& role_id_set)
{
	JUDGE_RETURN(role_id_set.size() > 0, 0);
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200113 request;
	LongVec::iterator it = role_id_set.begin();
	for(; it != role_id_set.end(); ++it)
	{
		request.add_role_id_set(*it);
	}
	return player->monitor()->dispatch_to_chat(player,&request);
}

int ChatLogic::chat_remove_black_list(LongVec& role_id_set)
{
	JUDGE_RETURN(role_id_set.size() > 0, 0);
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200114 request;
	LongVec::iterator it = role_id_set.begin();
	for(; it != role_id_set.end(); ++it)
	{
		request.add_role_id_set(*it);
	}
	return player->monitor()->dispatch_to_chat(player,&request);
}

int ChatLogic::chat_add_friend_list(LongVec& role_id_set)
{
	JUDGE_RETURN(role_id_set.size() > 0, 0);
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200120 request;
	LongVec::iterator it = role_id_set.begin();
	for(; it != role_id_set.end(); ++it)
	{
		request.add_role_id_set(*it);
	}
	return player->monitor()->dispatch_to_chat(player,&request);
}

int ChatLogic::chat_remove_friend_list(LongVec& role_id_set)
{
	JUDGE_RETURN(role_id_set.size() > 0, 0);
	LogicPlayer *player = dynamic_cast<LogicPlayer*>(this);

	Proto30200121 request;
	LongVec::iterator it = role_id_set.begin();
	for(; it != role_id_set.end(); ++it)
	{
		request.add_role_id_set(*it);
	}
	return player->monitor()->dispatch_to_chat(player,&request);
}
