/*
 * LogicTeamer.cpp
 *
 *  Created on: Jul 22, 2013
 *      Author: peizhibi
 */

#include "LogicTeamer.h"
#include "ProtoDefine.h"
#include "TeamPlatform.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "Transaction.h"
#include "MongoDataMap.h"
#include "TipsEnum.h"

LogicTeamer::LogicTeamer()
{

}

LogicTeamer::~LogicTeamer()
{
	// TODO Auto-generated destructor stub
}

void LogicTeamer::reset()
{
	this->team_set_info_.reset();
}

// 同步队伍信息到场景服
int LogicTeamer::sync_team_info_to_scene()
{
//	Proto31400211 ml_team_info;
	Proto30400701 map_team_info;
//
	TeamPanel* team_panel = this->team_panel();
	if (team_panel != NULL)
	{
//		ml_team_info.set_team_index(team_panel->team_index_);
//		ml_team_info.set_leader_id(team_panel->leader_id_);
//
		map_team_info.set_team_index(team_panel->team_index_);
		map_team_info.set_leader_id(team_panel->leader_id_);
//
		for (LongList::iterator iter = team_panel->teamer_set_.begin();
				iter != team_panel->teamer_set_.end(); ++iter)
		{
//			ml_team_info.add_teamer_set(*iter);
			map_team_info.add_teamer_set(*iter);
		}
//
		for (LongList::iterator iter = team_panel->replacement_set_.begin();
				iter != team_panel->replacement_set_.end(); ++iter)
		{
//			ml_team_info.add_replacement_set(*iter);
			map_team_info.add_replacement_set(*iter);
		}
	}
//
//	LOGIC_MONITOR->dispatch_to_scene(this, &ml_team_info);
	LOGIC_MONITOR->dispatch_to_scene(this, &map_team_info);

	return 0;
}

// 已离线的队员登录
int LogicTeamer::teamer_sign_in()
{
	int team_index = TEAM_PLANTFORM->pop_offline_teamer(this->role_id());
	JUDGE_RETURN(team_index > 0, -1);

	TeamPanel* team_panel = this->team_panel(team_index);
	JUDGE_RETURN(team_panel != NULL, -1);

	JUDGE_RETURN(team_panel->validate_teamer(this->role_id()) == true, -1);
	JUDGE_RETURN(team_panel->offline_teamer_info_.count(this->role_id()) > 0, -1);

	team_panel->offline_teamer_info_.erase(this->role_id());
	this->team_index(team_index);

	// TODO: 需要添加队伍聊天处理
	if(0 == team_panel->leader_id_)
	{
		this->appoint_leader_i(this->role_id(), false);
	}

//	this->sync_team_info_to_scene();
	this->notify_all_teamer_team_info();
	this->notify_all_teamer_couple_fb_info();
	this->notify_self_states_to_teamer();

	this->notify_team_info();
	this->fetch_states_of_all_teamers();

	if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
	{
		this->notify_all_teamer_fb_info();
		this->fetch_team_fb_info();
		this->fetch_teamers_ready_state();
		this->fetch_team_use_times();

		// 防止客户端登录慢而被自动踢出
		this->team_set_info_.fb_confirm_tick_ = ::time(NULL);
	}

	return 0;
}

int LogicTeamer::teamer_delay_sign_in()
{
	MSG_USER("DELAY SIGN TEAMER");
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	this->sync_team_info_to_scene();

	if(team_panel->fb_wait_confirm_set_.count(this->role_id()) > 0)
	{
		this->logic_request_switch_fb_team(this->logic_player());
	}

	this->notify_team_info();
	this->fetch_states_of_all_teamers();

	if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
	{
		this->notify_all_teamer_fb_info();
		this->fetch_team_fb_info();
		this->fetch_teamers_ready_state();
		this->fetch_team_use_times();
		this->sync_fb_use_times(GameEnum::TEAM_OPER_DELAY_SIGN_IN);
	}

	this->monitor()->dispatch_to_scene(this->gate_sid(), this->role_id(),
			this->scene_id(), INNER_MAP_TEAM_UPDATE_BLOOD_INFO);

	return 0;
}

int LogicTeamer::teamer_sign_out()
{
	this->set_ready_state(false);
//	this->sign_out_with_team();

	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, 0);
	if(GameEnum::TEAM_STATE_NORMAL == team_panel->team_state_)
	{
		this->logic_quit_team();
	}

	this->notify_all_teamer_team_info();
	this->notify_all_teamer_couple_fb_info();

	return 0;
}

void LogicTeamer::map_enter_scene()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, ;);

	if(false == TEAM_PLANTFORM->can_create_team(this->scene_id()))
	{//不能创建队伍的场景中退出队伍
		this->logic_quit_team();
	}
	else
	{
		this->sync_team_info_to_scene();
		this->notify_all_teamer_team_info();
		this->notify_self_states_to_teamer();

		if((GameEnum::TEAM_STATE_FB_INSIDE == team_panel->team_state_) &&
				(GameCommon::is_script_scene(this->scene_id()) == false))
		{// 从多人副本退出
			this->logic_quit_team();
		}
		else if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
		{
			team_panel->fb_ready_set_.erase(this->role_id());
			this->notify_all_teamer_ready_state(false);
		}
	}
}

// 离线记录offline(队伍中的玩家)
int LogicTeamer::sign_out_with_team()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	this->save_teamer_info_with_team(team_panel);
	OfflineTeamerInfo& teamer_info = team_panel->offline_teamer_info_[this->role_id()];

	teamer_info.role_id_ = this->role_id();
	teamer_info.offline_tick_ = ::time(NULL);
	teamer_info.teamer_info_ = this->role_detail();

	TEAM_PLANTFORM->push_offline_teamer(this->role_id(), team_panel->team_index_);
	return 0;
}

int LogicTeamer::team_index(int team_index)
{
	if (team_index != -1)
	{
		this->role_detail().__team_id = team_index;
	}

	return this->role_detail().__team_id;
}

int LogicTeamer::teamer_state()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, GameEnum::TEAMER_STATE_NONE);

	return team_panel->is_leader(this->role_id()) == true
			? GameEnum::TEAMER_STATE_LEADER : GameEnum::TEAMER_STATE_TEAMER;
}

int LogicTeamer::team_info_output_test(const char *str, Message *msg)
{
#ifndef LOCAL_DEBUG
	return 0;
#endif

	Proto80200001 respond;
	char team_info_str[1024] = {0};

	if(0 == msg)
		sprintf(team_info_str, "%s", str);
	else
		sprintf(team_info_str, "%s msg=%s", str, msg->Utf8DebugString().c_str());

	ProtoChatInfo* chat_info = respond.add_content();
	chat_info->set_channel(0);
	chat_info->set_type(1);
	chat_info->set_name("组队调试");
	chat_info->set_content(team_info_str);

	return this->respond_to_client(80200001, &respond);
}

TeamSetInfo& LogicTeamer::team_set_info()
{
	return this->team_set_info_;
}

TeamPanel* LogicTeamer::team_panel(int team_index)
{
	if (team_index == -1)
	{
		team_index = this->team_index();
	}

	JUDGE_RETURN(team_index > 0, NULL);
	return TEAM_PANEL_PACKAGE->find_object(team_index);
}

int LogicTeamer::logic_fast_join_team(Message* msg)
{
	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL == team_panel, RETURN_TEAM_FAST_JOIN, ERROR_HAVE_TEAM);
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	bool creat = true;
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		TeamPanel* other_team_panel = player->team_panel();
		JUDGE_CONTINUE(other_team_panel != NULL);
		JUDGE_CONTINUE(player->role_id() == other_team_panel->leader_id_
				&& other_team_panel->teamer_full() == false);

//		//自动接受直接同意
//		if (player->team_set_info().auto_accept_ == 1)
//		{
//			Proto10100310 team_info;
//			team_info.set_respone_type(1);
//			team_info.add_role_id(this->role_id());
//			creat = false;
//			player->accept_apply_into_team(&team_info);
//			FINER_PROCESS_NOTIFY(RETURN_TEAM_FAST_JOIN);
//		}
		//申请入队
//		else
		{
			this->logic_apply_into_team(player);
			creat = false;
		}
	}

	//没有就创建队伍
	if (creat)
	{
		this->logic_create_team();
	}
	FINER_PROCESS_NOTIFY(RETURN_TEAM_FAST_JOIN);
}

int LogicTeamer::logic_near_player(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto10100324*, request, RETURN_TEAM_NEAR_PLAYER);
	TeamPanel* team_panel = this->team_panel();
	LogicPlayer* own = dynamic_cast<LogicPlayer* >(this);
	League *own_league = own->league();
	Int64 own_league_id = -1;
	if (own_league != NULL) own_league_id = own_league->leader_index_;

	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_NEAR_PLAYER, ERROR_NO_TEAM);
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	Proto50100324 respond;
	if (own_league_id == -1) respond.mutable_other_info()->set_value(1);

	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		TeamPanel* other_team_panel = player->team_panel();
		JUDGE_CONTINUE(other_team_panel == NULL);
		JUDGE_CONTINUE(player->role_id() != own->role_id());
		JUDGE_CONTINUE(true == player->validate_team_level_limit());

		//同场景
		if (player->scene_id() == own->scene_id())
		{
			ProtoTeamer* teamer_info = respond.add_near_list();
			player->make_up_teamer_info(teamer_info);
		}

		//同帮派
		League *league = player->league();
		Int64 league_id = -1;
		if (league != NULL) league_id = league->leader_index_;
		if (league_id == own_league_id && league_id != -1)
		{
			ProtoTeamer* teamer_info = respond.add_league_list();
			player->make_up_teamer_info(teamer_info);
		}

		//同好友
		if (own->is_friend(player->role_id()))
		{
			ProtoTeamer* teamer_info = respond.add_friend_list();
			player->make_up_teamer_info(teamer_info);
		}

	}

	FINER_PROCESS_RETURN(RETURN_TEAM_NEAR_PLAYER, &respond);
}

int LogicTeamer::logic_near_team_begin(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto10100321*, request, RETURN_TEAM_NEAR_TEAM_INFO);
	Proto30400705 map_team_info;
	map_team_info.set_role_id(this->role_id());
	return LOGIC_MONITOR->dispatch_to_scene(this, &map_team_info);
}

int LogicTeamer::logic_near_team_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100704*, request, -1);
	Proto50100321 respond;
	for (int i = 0; i < request->team_list_size(); ++i)
	{
		int team_id = request->team_list(i);
		TeamPanel* team_panel = this->team_panel(team_id);
		JUDGE_CONTINUE(team_panel != NULL);

		int own_team_id = 0;
		if (this->team_panel() != NULL) own_team_id = this->team_panel()->team_index_;
		JUDGE_CONTINUE(own_team_id != team_panel->team_index_);

		Int64 leader_id = team_panel->leader_id_;
		LogicPlayer* player = NULL;
		if (LOGIC_MONITOR->find_player(leader_id, player) == 0 && player != NULL)
		{
			ProtoTeamer* teamer_info = respond.add_team_info();
			// online player
			player->make_up_teamer_info(teamer_info);

			teamer_info->set_team_num(team_panel->online_count());
		}

	}
	FINER_PROCESS_RETURN(RETURN_TEAM_NEAR_TEAM_INFO, &respond);
}

int LogicTeamer::logic_auto_invite(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100322*, request, RETURN_TEAM_AUTO_INVITE);

	this->team_set_info_.auto_invite_ = request->status();
	Proto50100322 respond;
	respond.set_status(request->status());
	this->notify_team_info();
	FINER_PROCESS_RETURN(RETURN_TEAM_AUTO_INVITE, &respond);
}

int LogicTeamer::logic_auto_accept(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100323*, request, RETURN_TEAM_AUTO_ACCEPT);

	this->team_set_info_.auto_accept_ = request->status();
	Proto50100323 respond;
	respond.set_status(request->status());
	this->notify_team_info();
	FINER_PROCESS_RETURN(RETURN_TEAM_AUTO_ACCEPT, &respond);
}

int LogicTeamer::team_monster_info_update(Message* msg)
{
	//组队共享刷怪
	MSG_DYNAMIC_CAST_RETURN(Proto31400051*, request, -1);
	TeamPanel* team_panel = this->team_panel();

	JUDGE_RETURN(NULL != team_panel, 0);

	if(GameEnum::TEAM_STATE_NORMAL == team_panel->team_state_)
	{
		for (LongList::iterator iter = team_panel->teamer_set_.begin();
				iter != team_panel->teamer_set_.end(); ++iter)
		{
			LogicPlayer* player = this->find_player(*iter);
			JUDGE_CONTINUE(player != NULL);
			JUDGE_CONTINUE(player->role_id() != this->role_id());

			//普通地图才可以刷任务
			JUDGE_CONTINUE(GameCommon::is_normal_scene(player->scene_id()));

			LOGIC_MONITOR->dispatch_to_scene(player, request);
		}
	}
	return 0;
}

// 获取组队&队伍信息
int LogicTeamer::logic_fetch_team_info()
{
	this->notify_team_info();
	TeamPanel* team_panel = this->team_panel();
//	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_OPEN_TEAM_PANEL, ERROR_TEAM_NO_EXIST);

	if (NULL != team_panel)
	{
		if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
		{
			this->fetch_team_fb_info();
			this->fetch_teamers_ready_state();
			this->fetch_team_use_times();
			this->sync_fb_use_times(GameEnum::TEAM_OPER_TEAMER_INFO);
		}
	}

	FINER_PROCESS_NOTIFY(RETURN_TEAM_OPEN_TEAM_PANEL);
}

int LogicTeamer::logic_close_team_info()
{
	FINER_PROCESS_NOTIFY(RETURN_TEAM_CLOSE_TEAM_PANEL);
}

int LogicTeamer::logic_create_team()
{
	int ret = this->create_team_i();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_CREATE_TEAM, ret);

	this->notify_team_info();
	this->sync_team_info_to_scene();

	if(NULL != this->team_panel())
		MSG_USER("create team team_id = %d", this->team_panel()->team_index_);
	FINER_PROCESS_NOTIFY(RETURN_TEAM_CREATE_TEAM);
}

// 请求组队
int LogicTeamer::logic_request_org_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100307*, request, RETURN_TEAM_ORG_TEAM);

	CONDITION_NOTIFY_RETURN(true == this->validate_team_level_limit(), RETURN_TEAM_ORG_TEAM,
			ERROR_PLAYER_LEVEL_LIMIT);

	CONDITION_NOTIFY_RETURN(TEAM_PLANTFORM->can_create_team(this->scene_id()) == true,
			RETURN_TEAM_ORG_TEAM, ERROR_SCENE_CREATE_TEAM);

	CONDITION_NOTIFY_RETURN(this->role_id() != request->role_id(),
			RETURN_TEAM_ORG_TEAM, ERROR_CLIENT_OPERATE);

	TeamPanel* team_panel = this->team_panel();
	LogicPlayer* player = this->find_player(request->role_id());
	CONDITION_NOTIFY_RETURN(player != NULL, RETURN_TEAM_ORG_TEAM,
			ERROR_PLAYER_OFFLINE);

	// 自己黑名单..
	LogicPlayer* this_player = this->logic_player();
	LogicSocialerDetail &socialer_detail = this_player->socialer_detail();

	// 对方黑名单..
	LogicSocialerDetail &peer_socialer_detail = player->socialer_detail();

	TeamPanel* peer_team_panel = player->team_panel();
	int ret = 0;

	if(NULL != team_panel)
	{
		if(NULL != peer_team_panel)
		{
			// 对方已经在队伍中
			CONDITION_NOTIFY_RETURN(team_panel->team_index_ != peer_team_panel->team_index_,
					RETURN_TEAM_ORG_TEAM, ERROR_IS_TEAMER);

			// 对方已经有队伍
			return this->respond_to_client_error(RETURN_TEAM_ORG_TEAM, ERROR_HAVE_TEAM);
		}

		CONDITION_NOTIFY_RETURN(socialer_detail.__black_list.count(player->role_id()) <= 0,
				RETURN_TEAM_ORG_TEAM, ERROR_IN_SELF_BLACK_LIST);
		CONDITION_NOTIFY_RETURN(peer_socialer_detail.__black_list.count(this->role_id()) <= 0,
				RETURN_TEAM_ORG_TEAM, ERROR_IN_BLACK_LIST);

		ret = this->logic_invite_into_team(player);
		CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_ORG_TEAM, ret);
	}
	else
	{
		if(NULL != peer_team_panel)
		{
			CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_INSIDE != peer_team_panel->team_state_,
					RETURN_TEAM_ORG_TEAM, ERROR_CANT_JOIN_IN_FB);

			// 队长黑名单..
			LogicPlayer* leader = this->find_player(peer_team_panel->leader_id_);
			CONDITION_NOTIFY_RETURN(leader != NULL, RETURN_TEAM_ORG_TEAM,
					ERROR_PLAYER_OFFLINE);

			LogicSocialerDetail &leader_socialer_detail = leader->socialer_detail();
			CONDITION_NOTIFY_RETURN(socialer_detail.__black_list.count(leader->role_id()) <= 0,
					RETURN_TEAM_ORG_TEAM, ERROR_IN_SELF_BLACK_LIST);
			CONDITION_NOTIFY_RETURN(leader_socialer_detail.__black_list.count(this->role_id()) <= 0,
					RETURN_TEAM_ORG_TEAM, ERROR_IN_BLACK_LIST);

			ret = this->logic_apply_into_team(leader);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_ORG_TEAM, ret);
		}
		else
		{
			ret = this->logic_invite_into_team(player);
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_ORG_TEAM, ret);
		}
	}
	this->notify_all_teamer_team_info();
	return this->respond_to_client(RETURN_TEAM_ORG_TEAM);


//	{
//		Proto10100309 inner_req;
//		inner_req.set_respone_type(GameEnum::TEAM_OPER_AGREE);
//		inner_req.add_role_id(this->role_id());
//		player->accept_invite_into_team(&inner_req);
//	}
	return 0;

}

// 邀请进入队伍
int LogicTeamer::logic_invite_into_team(LogicPlayer* player)
{
	JUDGE_RETURN(player->team_panel() == NULL, ERROR_HAVE_TEAM);

	JUDGE_RETURN(true == this->validate_team_level_limit(), ERROR_PLAYER_LEVEL_LIMIT);

	TeamPanel* team_panel = this->team_panel();
	if(NULL != team_panel)
	{
		JUDGE_RETURN(team_panel->teamer_full() == false, ERROR_TEAM_FULL);
	}
	JUDGE_RETURN(player->team_set_info().validate_inviter(
			this->role_id()) == true, ERROR_TEAM_INVITED);


	player->team_set_info().team_inviter_map_[this->role_id()] = ::time(NULL);

	//自动接受直接同意
	if (player->team_set_info().auto_invite_ == 1)
	{
		Proto10100309 team_info;
		team_info.set_respone_type(1);
		team_info.add_role_id(this->role_id());
		player->accept_invite_into_team(&team_info);
		return 0;
	}

	this->notify_team_oper_info(player, GameEnum::TEAM_OPER_INVITE);

	TEAM_PLANTFORM->add_replier(player->role_id());
	this->notify_all_teamer_team_info();
	return 0;
}

// 申请加入队伍
int LogicTeamer::logic_apply_into_team(LogicPlayer* player)
{
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_TEAM_NO_EXIST);
	JUDGE_RETURN(team_panel->teamer_full() == false, ERROR_TEAM_FULL);

	TeamSetInfo& team_set_info = player->team_set_info();
	JUDGE_RETURN(team_set_info.validate_applier(this->role_id()) == true,
			ERROR_TEAM_APPLIED);

	team_set_info.team_applier_map_[this->role_id()] = ::time(NULL);

	//自动接受直接同意
	if (player->team_set_info().auto_accept_ == 1)
	{
		Proto10100310 team_info;
		team_info.set_respone_type(1);
		team_info.add_role_id(this->role_id());
		player->accept_apply_into_team(&team_info);
		return 0;
	}


	this->notify_team_oper_info(player, GameEnum::TEAM_OPER_APPLY);
	this->notify_all_teamer_team_info();
	TEAM_PLANTFORM->add_replier(player->role_id());
	return 0;
}

// 请求转换到副本队伍
int LogicTeamer::logic_request_switch_fb_team(LogicPlayer* player)
{
	player->team_set_info_.fb_confirm_tick_ = time(NULL);
	FINER_PLAYER_PROCESS_NOTIFY(ACTIVE_REQUEST_SWITCH_TO_FB_TEAM);
}
// 处理 对【对方邀请加入】的响应
int LogicTeamer::accept_invite_into_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100309*, request, RETURN_TEAM_ACCEPT_INVITE);

	CONDITION_NOTIFY_RETURN(TEAM_PLANTFORM->can_create_team(this->scene_id()) == true,
			RETURN_TEAM_ACCEPT_INVITE, ERROR_SCENE_CREATE_TEAM);

	for (int i = 0; i < request->role_id_size(); ++i)
	{
		Int64 role_id = request->role_id(i);//

		if(this->team_set_info_.team_inviter_map_.count(role_id) <= 0)
		{
			// 操作超时
			this->respond_to_client_error(RETURN_TEAM_ACCEPT_INVITE, ERROR_OPERATE_TIME_OUT);
			continue;
		}
		this->team_set_info_.team_inviter_map_.erase(role_id);

		LogicPlayer* player = this->find_player(role_id);
		JUDGE_CONTINUE(player != NULL);

		int respone_type = 0;

		if((this->team_panel() != NULL))
		{
			//返回已有队伍
			this->respond_to_client_error(RETURN_TEAM_ACCEPT_INVITE,  ERROR_SELF_HAVE_TEAM);
		}

		if ((this->team_panel() == NULL) && (request->respone_type() == true))
		{
			// accept
			if (player->accept_invite_team_i(this->role_id()) == 0)
			{
				respone_type = GameEnum::TEAM_OPER_AGREE;
			}
			else
			{
				respone_type = GameEnum::TEAM_OPER_FULL;
			}
		}
		else
		{
			respone_type = GameEnum::TEAM_OPER_REJECT;
		}

		player->notify_team_oper_result(GameEnum::TEAM_OPER_INVITE, respone_type,
				this->name());
		if(player->team_panel() && player->team_panel()->team_state_ == GameEnum::TEAM_STATE_FB_INSIDE)
		{
		   return this->respond_to_client_error(RETURN_TEAM_ACCEPT_INVITE, ERROR_CANT_JOIN_IN_FB);
		}
		if(player->team_panel() && player->team_panel()->team_state_ == GameEnum::TEAM_STATE_FB_ORG && respone_type == GameEnum::TEAM_OPER_AGREE)
		{
		    this->respond_to_client(RETURN_TEAM_ACCEPT_INVITE);
			return this->logic_request_switch_fb_team(this->logic_player());
		}
	}
	this->notify_all_teamer_team_info();
	FINER_PROCESS_NOTIFY(RETURN_TEAM_ACCEPT_INVITE);
}

// 处理 对【对方申请入队】的响应
int LogicTeamer::accept_apply_into_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100310*, request, RETURN_TEAM_ACCPET_APPLY);

	CONDITION_NOTIFY_RETURN(TEAM_PLANTFORM->can_create_team(this->scene_id()) == true,
			RETURN_TEAM_ACCPET_APPLY, ERROR_SCENE_CREATE_TEAM);

	for (int i = 0; i < request->role_id_size(); ++i)
	{
		Int64 role_id = request->role_id(i);

		if(this->team_set_info_.team_applier_map_.count(role_id) <= 0)
		{
			this->respond_to_client_error(RETURN_TEAM_ACCPET_APPLY, ERROR_OPERATE_TIME_OUT);
			continue;
		}
		this->team_set_info_.team_applier_map_.erase(role_id);

		LogicPlayer* player = this->find_player(role_id);
		JUDGE_CONTINUE(player != NULL);

		int respone_type = 0;

		if(NULL != player->team_panel())
		{
			this->respond_to_client_error(RETURN_TEAM_ACCPET_APPLY,  ERROR_HAVE_TEAM);
			continue;
		}

		if (request->respone_type() == true)
		{
			// accept
			if (this->accept_apply_team_i(role_id) != 0)
			{
				respone_type = GameEnum::TEAM_OPER_FULL;
			}
			else
			{
				respone_type = GameEnum::TEAM_OPER_AGREE;
			}
		}
		else
		{
			// reject
			respone_type = GameEnum::TEAM_OPER_REJECT;
		}

		player->notify_team_oper_result(GameEnum::TEAM_OPER_APPLY, respone_type,
				this->name());
	}
	this->notify_all_teamer_team_info();
	FINER_PROCESS_NOTIFY(RETURN_TEAM_ACCPET_APPLY);
}

// 提升队长
int LogicTeamer::logic_appoint_leader(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100303*, request, RETURN_TEAM_APPOINT_LEADER);

	int64_t role_id = request->role_id();

	CONDITION_NOTIFY_RETURN(this->role_id() != role_id,
			RETURN_TEAM_APPOINT_LEADER,	ERROR_CLIENT_OPERATE);

	TeamPanel *team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_APPOINT_LEADER, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->teamer_state() == GameEnum::TEAMER_STATE_LEADER,
			RETURN_TEAM_APPOINT_LEADER, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(team_panel->validate_teamer(role_id),
			RETURN_TEAM_APPOINT_LEADER, ERROR_MACHINE_CANT_BE_LEADER);

	int ret = this->appoint_leader_i(request->role_id());
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_APPOINT_LEADER, ret);

	this->sync_all_teamer_team_info();
	this->notify_all_teamer_team_info();

	if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
	{
		this->cancel_all_teamer_ready_state();
		this->notify_all_teamer_use_times();
	}
	this->notify_all_teamer_team_info();
	FINER_PROCESS_NOTIFY(RETURN_TEAM_APPOINT_LEADER);
}

// 踢出队员(副本内不能踢队员，否则要加上推出副本处理)
int LogicTeamer::logic_kick_teamer(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100304*, request, RETURN_TEAM_KICK_TEAMER);
	int64_t role_id = request->role_id();

	CONDITION_NOTIFY_RETURN(this->role_id() != role_id,
			RETURN_TEAM_KICK_TEAMER, ERROR_CLIENT_OPERATE);

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(team_panel != NULL, RETURN_TEAM_KICK_TEAMER,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(team_panel->is_leader(this->role_id()) == true,
			RETURN_TEAM_KICK_TEAMER, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_INSIDE !=team_panel->team_state_,
			RETURN_TEAM_KICK_TEAMER, ERROR_SCRIPT_CANT_KICK);

	if(team_panel->rpm_teamer_info_.count(role_id) > 0)
	{//剔除化身
		team_panel->erase_replacement(role_id);
		this->sync_all_teamer_team_info();
		this->notify_all_teamer_team_info();

		FINER_PROCESS_NOTIFY(RETURN_TEAM_KICK_TEAMER);
	}

	CONDITION_NOTIFY_RETURN(team_panel->validate_teamer(role_id) == true,
			RETURN_TEAM_KICK_TEAMER, ERROR_CLIENT_OPERATE);

	team_panel->erase_teamer(role_id);

	this->check_and_dismiss_team();
	this->notify_other_quit_team(request->role_id(), false);
	this->save_teamer_info_quit_team();

	// 准备状态
	if(team_panel->team_state_ == GameEnum::TEAM_STATE_FB_ORG)
	{
		for (LongList::iterator iter = team_panel->teamer_set_.begin();
				iter != team_panel->teamer_set_.end(); ++iter)
		{
			LogicPlayer* player = NULL;
			JUDGE_CONTINUE(LOGIC_MONITOR->find_player(*iter, player) == 0);

			player->fetch_teamers_ready_state();
			player->fetch_team_use_times();
		}
	}
	this->notify_all_teamer_team_info();
	FINER_PROCESS_NOTIFY(RETURN_TEAM_KICK_TEAMER);
}

int LogicTeamer::check_and_quit_team()
{
	JUDGE_RETURN(this->teamer_state() > 0, -1);
	JUDGE_RETURN(LOGIC_MONITOR->limit_team_scene(this->scene_id()) == true, -1);

	return this->logic_quit_team();
}

// 退出队伍
int LogicTeamer::logic_quit_team()
{
//	this->team_info_output_test("离开队伍");
	int ret = this->quit_team_i();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_QUIT_TEAM, ret);

	this->check_and_dismiss_team();
	this->handle_quit_team(true);
	this->notify_team_info();
	this->notify_all_teamer_team_info();
	this->notify_all_teamer_couple_fb_info();
	FINER_PROCESS_NOTIFY(RETURN_TEAM_QUIT_TEAM);
}

// 通知全队玩家
int LogicTeamer::notify_all_teamer(int recogn, Message* msg)
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	team_panel->notify_all_teamer(recogn, msg);
	return 0;
}

int LogicTeamer::create_team_i()
{
	JUDGE_RETURN(this->team_panel() == NULL, ERROR_CLIENT_OPERATE);

	JUDGE_RETURN(true == TEAM_PLANTFORM->can_create_team(this->scene_id()),
			ERROR_SCENE_CREATE_TEAM);

	JUDGE_RETURN(true == this->validate_team_level_limit(), ERROR_PLAYER_LEVEL_LIMIT);

	TeamPanel* team_panel = TEAM_PANEL_PACKAGE->pop_object();
	JUDGE_RETURN(team_panel != NULL, ERROR_SERVER_INNER);

	team_panel->leader_id_ = this->role_id();
	team_panel->leader_name_ = this->name();
	team_panel->push_teamer(this->role_id(), GameEnum::TEAMER_PUSH_FRONT);

	TEAM_PANEL_PACKAGE->bind_object(team_panel->team_index_, team_panel);

	this->team_index(team_panel->team_index_);
	this->logic_player()->chat_establish_team();
	this->logic_player()->chat_join_team(team_panel->team_index_);
	return 0;
}

// 从TeamPanel删除玩家,(如果离线玩家是队长,提升其他玩家为队长)
int LogicTeamer::quit_team_i()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_CLIENT_OPERATE);
	this->logic_player()->chat_leave_team();
	this->check_and_leave_fb();
	this->save_teamer_info_quit_team();

	if (team_panel->is_leader(this->role_id()) == true)
	{
		LogicPlayer* player = team_panel->find_online_teamer(false);

		if (player != NULL)
		{
			// 传false参数会从队伍移除
			this->appoint_leader_i(player->role_id(), false);
		}
		else
		{
			team_panel->leader_id_ = 0;
			team_panel->erase_teamer(this->role_id());
		}
	}
	else
	{
		team_panel->erase_teamer(this->role_id());
	}

	return 0;
}

// 解散队伍
int LogicTeamer::dismiss_team_i()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	team_panel->replacement_set_.clear();
	team_panel->rpm_teamer_info_.clear();

	for (TeamPanel::OfflineTeamerMap::iterator iter = team_panel->offline_teamer_info_.begin();
			iter != team_panel->offline_teamer_info_.end(); ++iter)
	{
		team_panel->save_offline_teamer_info(iter->first, true);
	}
	team_panel->offline_teamer_info_.clear();

	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);
		player->handle_quit_team(true);
		player->check_and_leave_fb();
		player->save_teamer_info_quit_team();
	}

	this->logic_player()->chat_dismiss_team();

	TEAM_PLANTFORM->remove_offline_team(team_panel->team_index_);
	TEAM_PANEL_PACKAGE->unbind_and_push(team_panel->team_index_, team_panel);
	return 0;
}

// 邀请被接受
int LogicTeamer::accept_invite_team_i(int64_t role_id)
{
	TeamPanel* team_panel = this->check_and_create_team();
	JUDGE_RETURN(team_panel != NULL, ERROR_SCENE_CREATE_TEAM);

	if (team_panel->fb_id_ > 0)
		return this->handle_enter_team(role_id, team_panel, GameEnum::TEAM_OPER_FB_INVITE);
	else
		return this->handle_enter_team(role_id, team_panel, GameEnum::TEAM_OPER_INVITE);
}

// 接受申请
int LogicTeamer::accept_apply_team_i(int64_t role_id)
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_TEAM_NO_EXIST);

	if (team_panel->fb_id_ > 0)
		return this->handle_enter_team(role_id, team_panel, GameEnum::TEAM_OPER_FB_APPLY);
	else
		return this->handle_enter_team(role_id, team_panel, GameEnum::TEAM_OPER_APPLY);
}

// 提升队长
int LogicTeamer::appoint_leader_i(int64_t role_id, int push_self)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, ERROR_PLAYER_OFFLINE);

	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_CLIENT_OPERATE);
	JUDGE_RETURN(team_panel->validate_teamer(role_id) == true, ERROR_CLIENT_OPERATE);

	team_panel->leader_id_ = role_id;
	team_panel->leader_name_ = player->name();

	team_panel->fb_ready_set_.erase(role_id);
	team_panel->teamer_set_.remove(role_id);
	team_panel->teamer_set_.remove(this->role_id());

	team_panel->push_teamer(role_id, GameEnum::TEAMER_PUSH_FRONT);
	if (push_self == true)
	{
		team_panel->push_teamer(this->role_id(), GameEnum::TEAMER_PUSH_BACK);
	}

	return 0;
}

int LogicTeamer::check_and_leave_fb()
{
	JUDGE_RETURN(true == GameCommon::is_script_scene(this->scene_id()), -1);

	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);
	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_INSIDE == team_panel->team_state_, -1);

	return this->monitor()->dispatch_to_scene(this->gate_sid(),
			this->role_id(),this->scene_id(), CLIENT_REQUEST_EXIT_SYSTEM);
}

// 队伍有人数变化时信息更新
int LogicTeamer::check_and_dismiss_team()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	int team_count = team_panel->teamer_set_.size();
	if (TEAM_PLANTFORM->can_dismiss_team(team_count) == true)
	{
		this->dismiss_team_i();
	}
	else
	{
		this->sync_all_teamer_team_info();
		this->notify_all_teamer_team_info();
		this->notify_all_teamer_couple_fb_info();
	}

	return 0;
}

TeamPanel* LogicTeamer::check_and_create_team()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel == NULL, team_panel);

	this->create_team_i();
	team_panel = this->team_panel();

	return team_panel;
}

// 通知玩家队伍信息(普通组队,只有队员信息)
int LogicTeamer::notify_team_info()
{
	TeamPanel* team_panel = this->team_panel();
	Proto80400306 team_info;
	if (NULL == team_panel)
	{
		team_info.set_no_team(1);
	}
	else
	{
		this->fetch_team_info(&team_info);
	}
	//MSG_USER("TEAM INFO %s team_id = %d", this->name(), team_panel->team_index_);

	team_info.set_auto_invite(this->team_set_info_.auto_invite_);
	team_info.set_auto_accept(this->team_set_info_.auto_accept_);

	FINER_PROCESS_RETURN(ACTIVE_TEAM_INFO, &team_info);
}

// 玩家获取副本组队面板的全部信息
int LogicTeamer::notify_fb_team_info()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80400306 team_info;
	this->fetch_team_info(&team_info);
	this->respond_to_client(ACTIVE_TEAM_INFO, &team_info);

	this->fetch_teamers_ready_state();
	this->fetch_team_fb_info();
	this->fetch_team_use_times();

	return 0;
}

// 对全部队伍玩家同步队伍信息到场景线程
int LogicTeamer::sync_all_teamer_team_info()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);
		player->sync_team_info_to_scene();
	}

	return 0;
}

int LogicTeamer::replace_leader()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	this->appoint_leader_i(this->role_id(), false);
	this->notify_all_teamer_team_info();

	if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
	{
		this->cancel_all_teamer_ready_state();
		this->notify_all_teamer_use_times();
	}

	return 0;
}

// 通知全部队员 队伍的队员信息
int LogicTeamer::notify_all_teamer_team_info()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	Proto80400306 team_info;
	this->fetch_team_info(&team_info);
	team_info.set_auto_invite(this->team_set_info_.auto_invite_);
	team_info.set_auto_accept(this->team_set_info_.auto_accept_);

	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);


		//婚姻信息更新
		player->request_wedding_pannel();
		player->request_wedding_before(NULL);

		player->respond_to_client(ACTIVE_TEAM_INFO, &team_info);
	}

	return 0;
}

int LogicTeamer::notify_all_teamer_couple_fb_info()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);
	JUDGE_RETURN(team_panel->fb_id_ == GameEnum::TEAM_COUPLE_FB, -1);

	Proto80100410 respond;
	respond.set_fb_id(team_panel->fb_id_);

	if (this->validate_enter_couple_fb() == 0)
		respond.set_team_state(true);
	else
		respond.set_team_state(false);

	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_SEND_COUPLE_TEAM_INFO, &respond);
	}

	return 0;
}

int LogicTeamer::notify_team_oper_result(int oper_type, int result, const char* role_name)
{
	Proto80400310 oper_result;

	oper_result.set_oper_type(oper_type);
	oper_result.set_oper_result(result);
	oper_result.set_oper_name(role_name);

	FINER_PROCESS_RETURN(ACTIVE_TEAM_OPER_RESULT, &oper_result);
}

// 邀请/申请 队伍
int LogicTeamer::notify_team_oper_info(LogicPlayer* player, int oper_type)
{
	Proto80400309 tips_info;

	tips_info.set_oper_type(oper_type);
	tips_info.set_role_id(this->role_id());
	tips_info.set_role_name(this->name());
	tips_info.set_role_level(this->role_level());
	tips_info.set_fight_force(this->role_detail().__fight_force);

	LogicPlayer* own = dynamic_cast<LogicPlayer *>(this);
	League *league = own->league();
	if (league != NULL) tips_info.set_league_name(league->league_name_);

	FINER_PLAYER_PROCESS_RETURN(ACTIVE_TEAM_OPER_INFO, &tips_info);
}

//离开队伍提示
int LogicTeamer::notify_self_quit_team(int quit_type)
{
	Proto80400308 quit_info;
	quit_info.set_quit_type(quit_type);
	FINER_PROCESS_RETURN(ACTIVE_TEAM_QUIT, &quit_info);
}

// 通知玩家已离开队伍
int LogicTeamer::notify_other_quit_team(int64_t role_id, int quit_type)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, -1);

	player->handle_quit_team(quit_type);
	return 0;
}

// 玩家获取队伍全部玩家信息
int LogicTeamer::fetch_team_info(Proto80400306* team_info)
{
	JUDGE_RETURN(team_info != NULL, -1);
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		Int64 teamate_id = *iter;
		if (team_panel->offline_teamer_info_.count(teamate_id) > 0)
		{
			// offline player
			ProtoTeamer* teamer_info = team_info->add_teamer_set();
			team_panel->offline_teamer_info_[teamate_id].make_up_teamer_info(teamer_info);
		}
		else
		{
			LogicPlayer* player = this->find_player(teamate_id);
			JUDGE_CONTINUE(player != NULL);

			// online player
			ProtoTeamer* teamer_info = team_info->add_teamer_set();
			player->make_up_teamer_info(teamer_info);

			teamer_info->set_team_num(team_panel->online_count());

		}
	}

	// replacement player
	for (LongList::iterator iter = team_panel->replacement_set_.begin();
			iter != team_panel->replacement_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(team_panel->rpm_teamer_info_.count(*iter) > 0);
		ProtoTeamer* teamer_info = team_info->add_teamer_set();
		team_panel->rpm_teamer_info_[*iter].make_up_teamer_info(teamer_info);
	}

	team_info->set_leader_id(team_panel->leader_id_);

	if(GameEnum::TEAM_STATE_NORMAL == team_panel->team_state_)
	{
		team_info->set_team_state(GameEnum::TEAM_STATE_NORMAL);
	}
	else if((GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_) ||
			(GameEnum::TEAM_STATE_FB_INSIDE == team_panel->team_state_))
	{
		team_info->set_team_state(GameEnum::TEAM_STATE_FB_ORG);
	}

	return 0;
}

int LogicTeamer::make_up_teamer_info(ProtoTeamer* teamer_info)
{
	LogicRoleDetail& role_detail = this->role_detail();

	role_detail.make_up_teamer_info(this->role_id(), teamer_info);
	teamer_info->set_online_flag(GameEnum::TEAMER_ONLINE);

	LogicPlayer* player = dynamic_cast<LogicPlayer *>(this);
	League *league = player->league();
	if (league != NULL) teamer_info->set_league_name(league->league_name_);

	teamer_info->set_team_limit(GameEnum::MAX_TEAMER_COUNT);

	for (ThreeObjMap::iterator iter = role_detail.mount_info_.begin();
			iter != role_detail.mount_info_.end(); ++iter)
	{
		ProtoPairObj* info = teamer_info->add_mount_info();
		info->set_obj_id(iter->second.id_);
		info->set_obj_value(iter->second.tick_);
	}

	ProtoThreeObj* info = teamer_info->add_show_info();
	info->set_id(this->role_detail().fashion_id_);
	info->set_value(this->role_detail().fashion_color_);

	return 0;
}

int LogicTeamer::handle_enter_team(int64_t role_id, TeamPanel* team_panel, const int oper_type)
{
	JUDGE_RETURN(team_panel != NULL, ERROR_TEAM_NO_EXIST);
	JUDGE_RETURN(team_panel->teamer_full() == false, ERROR_TEAM_FULL);

	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, ERROR_PLAYER_OFFLINE);

	JUDGE_RETURN(player->team_panel() == NULL, ERROR_HAVE_TEAM);
	JUDGE_RETURN(TEAM_PLANTFORM->can_create_team(player->scene_id())
			== true, ERROR_SCENE_CREATE_TEAM);

	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_INSIDE != team_panel->team_state_,
			ERROR_CANT_JOIN_IN_FB);
	JUDGE_RETURN(false == team_panel->validate_replacement(role_id),
			ERROR_TEAM_HAS_REPLACEMENT);

	player->team_index(team_panel->team_index_);
	player->chat_join_team(team_panel->team_index_);

	team_panel->push_teamer(role_id, GameEnum::TEAMER_PUSH_BACK);
	team_panel->erase_replacement(role_id);

	this->sync_all_teamer_team_info();
	this->notify_all_teamer_team_info();
	this->notify_all_teamer_couple_fb_info();

	player->fetch_states_of_all_teamers();
	player->notify_self_states_to_teamer();
	//player->logic_fetch_team_info();

	if(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_)
	{
		player->notify_all_teamer_ready_state(false);
		player->fetch_teamers_ready_state();
		player->fetch_team_fb_info();
		player->sync_fb_use_times(oper_type);
	}

	return 0;
}

int LogicTeamer::handle_quit_team(int quit_type)
{
	this->logic_player()->chat_leave_team();

	this->team_index(0);
	this->sync_team_info_to_scene();
	this->notify_self_quit_team(quit_type);

	return 0;
}

int LogicTeamer::handle_request_time_out()
{
	int auto_reject_time = CONFIG_INSTANCE->tiny("auto_reject_time").asInt();
	for(BLongMap::iterator iter = team_set_info().team_applier_map_.begin();
			iter != team_set_info().team_applier_map_.end(); )
	{
		if(true == GameCommon::validate_time_span(iter->second, auto_reject_time))
		{
			//超时处理
			reply_invite_into_team(iter->first, GameEnum::TEAM_OPER_REJECT);

			Proto80400311 respond;
			respond.set_role_id(iter->first);
			this->respond_to_client(ACTIVE_TEAM_AUTO_REJECT, &respond);

			iter = team_set_info().team_applier_map_.erase(iter);
			continue;
		}

		iter++;
	}

	for(BLongMap::iterator iter = team_set_info().team_inviter_map_.begin();
			iter != team_set_info().team_inviter_map_.end(); )
	{
		if(true == GameCommon::validate_time_span(iter->second, auto_reject_time))
		{
			//超时处理
			reply_apply_into_team(iter->first, GameEnum::TEAM_OPER_REJECT);

			Proto80400311 respond;
			respond.set_role_id(iter->first);
			this->respond_to_client(ACTIVE_TEAM_AUTO_REJECT, &respond);

			iter = team_set_info().team_inviter_map_.erase(iter);
			continue;
		}

		iter++;
	}

	int request_size = 0;
	do
	{
		TeamPanel* team_panel = this->team_panel();
		JUDGE_BREAK(NULL != team_panel);
		JUDGE_BREAK(team_panel->fb_wait_confirm_set_.count(this->role_id()) > 0);
		int fb_confirm_tick_ = team_set_info().fb_confirm_tick_;

		if(true == GameCommon::validate_time_span(fb_confirm_tick_, auto_reject_time))
		{
			//自动拒绝进入副本准备，并退出队伍
			this->reply_confirm_switch_fb_team(GameEnum::TEAM_OPER_REJECT);
			break;
		}

		request_size += 1;
	}
	while(0);

	request_size += team_set_info().team_inviter_map_.size();
	request_size += team_set_info().team_applier_map_.size();

	return MAX(request_size , 0);
}

int LogicTeamer::reply_invite_into_team(int64_t role_id, bool respone)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, -1);

	int respone_type = 0;
	if (this->team_panel() == NULL && true == respone && player->team_panel() != NULL)
	{
		// accept
		if (player->accept_invite_team_i(this->role_id()) == 0)
		{
			respone_type = GameEnum::TEAM_OPER_AGREE;
		}
		else
		{
			respone_type = GameEnum::TEAM_OPER_FULL;//已在另一个队伍中
		}
	}
	else
	{
		respone_type = GameEnum::TEAM_OPER_REJECT;
	}

	player->notify_team_oper_result(GameEnum::TEAM_OPER_INVITE, respone_type, this->name());

	return respone_type;
}

int LogicTeamer::reply_apply_into_team(int64_t role_id, bool respone)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL, -1);

	int respone_type = 0;
	if (true == respone)
	{
		// accept
		if (this->accept_apply_team_i(role_id) != 0)
		{
			respone_type = GameEnum::TEAM_OPER_FULL;
		}
		else
		{
			respone_type = GameEnum::TEAM_OPER_AGREE;
		}
	}
	else
	{
		// reject
		respone_type = GameEnum::TEAM_OPER_REJECT;
	}

	player->notify_team_oper_result(GameEnum::TEAM_OPER_APPLY, respone_type, this->name());

	return respone_type;
}

int LogicTeamer::reply_confirm_switch_fb_team(bool respone)
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, ERROR_NO_TEAM);
	team_panel->fb_wait_confirm_set_.erase(this->role_id());

	if(true == respone)
	{
		this->fetch_team_use_times();
		this->fetch_teamers_ready_state();
		this->fetch_team_fb_info();
		this->notify_team_info();
	}
	else
	{
		int ret = this->quit_team_i();
		JUDGE_RETURN(ret == 0, ret);

		this->check_and_dismiss_team();
		this->handle_quit_team(true);
		this->save_teamer_info_quit_team();
	}

	return 0;
}

// 获取其他队员的状态
int LogicTeamer::fetch_states_of_all_teamers()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	Proto80400402 teamer_states;
	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter != this->role_id());
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		// 是否同场景
		bool same_scence = (player->scene_id() == this->scene_id());

		teamer_states.set_role_id(player->role_id());
		teamer_states.set_nearby(same_scence);
		teamer_states.set_alive(true);
		teamer_states.set_online(true);
	}

	this->respond_to_client(ACTIVE_NOTIFY_TEAMER_STATE, &teamer_states);

	for(TeamPanel::OfflineTeamerMap::iterator iter = team_panel->offline_teamer_info_.begin();
			iter != team_panel->offline_teamer_info_.end(); ++iter)
	{
		// 通知离线
		Proto80400402 teamer_states;
		teamer_states.set_role_id(iter->first);
		teamer_states.set_online(false);
		teamer_states.set_nearby(false);
		teamer_states.set_alive(true);

		this->respond_to_client(ACTIVE_NOTIFY_TEAMER_STATE, &teamer_states);
	}

	return 0;
}

// 通知其他队员自身状态
int LogicTeamer::notify_self_states_to_teamer()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	Proto80400402 teamer_states;
	teamer_states.set_role_id(this->role_id());
	teamer_states.set_alive(true);
	teamer_states.set_online(true);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter != this->role_id());
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		// 是否同场景
		bool same_scence = player->scene_id() == this->scene_id();
		teamer_states.set_nearby(same_scence);

		player->respond_to_client(ACTIVE_NOTIFY_TEAMER_STATE, &teamer_states);
	}

	return 0;
}

int LogicTeamer::notify_offline_state_to_teamer()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, -1);

	Proto80400402 teamer_states;
	teamer_states.set_role_id(this->role_id());
	teamer_states.set_nearby(false);
	teamer_states.set_online(false);
	teamer_states.set_alive(true);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter != this->role_id());
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_NOTIFY_TEAMER_STATE, &teamer_states);
	}

	return 0;
}

int LogicTeamer::team_fb_organize(Message* msg)
{
	this->team_info_output_test("建立副本队伍/提升为副本队伍", msg);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100401*, request, RETURN_TEAM_FB_ORGANIZE);

	CONDITION_NOTIFY_RETURN(true == this->validate_team_level_limit(), RETURN_TEAM_FB_ORGANIZE,
			ERROR_PLAYER_LEVEL_LIMIT);

	int fb_id = request->fb_id();
	CONDITION_NOTIFY_RETURN(true == TEAM_PLANTFORM->can_create_team(this->scene_id()),
			RETURN_TEAM_FB_ORGANIZE, ERROR_CLIENT_OPERATE);

	TeamPanel* team_panel = this->team_panel();
	if(NULL == team_panel)
	{
		int ret = validate_fb(fb_id, GameEnum::EXTRA_CONDITION_SEVEN_PROGRESS
				| GameEnum::EXTRA_CONDITION_SEVEN_TIME);
		CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_ORGANIZE, ret);

		// 创建队伍
		ret = create_team_i();
		CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_ORGANIZE, ret);

		team_panel = this->team_panel();
		CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_ORGANIZE, ERROR_SERVER_INNER);
	}
	else
	{
		CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
				RETURN_TEAM_FB_ORGANIZE, ERROR_NO_LEADER);

		int ret = validate_fb(fb_id, GameEnum::EXTRA_CONDITION_SEVEN_PROGRESS
				| GameEnum::EXTRA_CONDITION_SEVEN_TIME);
		CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_ORGANIZE, ret);

		CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_NORMAL == team_panel->team_state_,
				RETURN_TEAM_FB_ORGANIZE, ERROR_CLIENT_OPERATE);

		for(LongList::iterator iter = team_panel->teamer_set_.begin();
				iter != team_panel->teamer_set_.end(); ++iter)
		{
			JUDGE_CONTINUE((*iter) != this->role_id());
			team_panel->fb_wait_confirm_set_[*iter] = *iter;// 记录等待确认的玩家

			// 发送进入准备请求
			LogicPlayer *player = NULL;
			JUDGE_CONTINUE(0 == LOGIC_MONITOR->find_player(*iter, player));
			JUDGE_CONTINUE(NULL != player);

			this->logic_request_switch_fb_team(player);
			player->team_info_output_test("主动消息：询问是否接受提升副本队伍");
			TEAM_PLANTFORM->add_replier(*iter);
		}
	}

	team_panel->fb_id_ = fb_id;
	team_panel->team_state_ = GameEnum::TEAM_STATE_FB_ORG;

	this->respond_to_client(RETURN_TEAM_FB_ORGANIZE);

	this->sync_all_teamer_fb_use_times();
	this->cancel_all_teamer_ready_state();
	this->notify_all_teamer_fb_info();
	this->notify_all_teamer_team_info();	//更新副本组队面版
	this->sync_all_teamer_team_info();
	return 0;
}

int LogicTeamer::team_fb_broadcast_recruit()
{
//	TeamPanel* team_panel = this->team_panel();
//	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_BROADCAST_RECRUIT,
//			ERROR_NO_TEAM);
//
//	CONDITION_NOTIFY_RETURN(this->team_set_info().validate_borcast_recuit(), RETURN_TEAM_FB_BROADCAST_RECRUIT,
//			ERROR_OPERATE_TOO_FAST);
//
//	this->team_set_info().borcast_recuit_tick_ = time(NULL);
//
//	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
//			RETURN_TEAM_FB_BROADCAST_RECRUIT, ERROR_NO_LEADER);
//
//	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
//			RETURN_TEAM_FB_BROADCAST_RECRUIT, ERROR_NO_SCRIPT_TEAM);
//
//	int ret = this->validate_fb(team_panel->fb_id_);
//	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_BROADCAST_RECRUIT, ret);
//
//	CONDITION_NOTIFY_RETURN(false == team_panel->teamer_full(),
//			RETURN_TEAM_FB_BROADCAST_RECRUIT, ERROR_TEAM_FULL);
//
//	int vacancy_count = GameEnum::MAX_TEAMER_COUNT - team_panel->teamer_count();
//
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_role_detail(para_vec,
//			this->role_id(), std::string(this->role_detail().__name), true);
//    {
//        const Json::Value &script_json = CONFIG_INSTANCE->script(team_panel->fb_id_);
//        if (script_json.isMember("name") && script_json["name"].asString().empty() == false)
//        {
//            GameCommon::push_brocast_para_string(para_vec, script_json["name"].asString());
//        }
//        else
//        {
//            const Json::Value &scene_json = script_json["scene"][0u];
//            GameCommon::push_brocast_para_string(para_vec, scene_json["name"].asString());
//        }
//    }
//	GameCommon::push_brocast_para_int(para_vec, vacancy_count);
//
//	char str_buff[256] = {0};
//	string client_subfix = CONFIG_INSTANCE->tiny("team_recruit_subfix").asString();
//	if(!client_subfix.empty())
//	{
//		sprintf(str_buff, client_subfix.c_str(), this->role_id());
//		GameCommon::push_brocast_para_string(para_vec, str_buff);
//	}
//
//	this->monitor()->announce_world(SHOUT_ALL_TEAM_FB_RECRUIT, para_vec);
//	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_BROADCAST_RECRUIT);
	return 0;
}

int LogicTeamer::team_fb_organize_cancel()
{
	this->team_info_output_test("取消副本队伍");
	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_ORG_CANCEL, ERROR_NO_TEAM);
	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_ORG_CANCEL, ERROR_NO_LEADER);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG ==  team_panel->team_state_,
			RETURN_TEAM_FB_ORG_CANCEL, ERROR_NO_SCRIPT_TEAM);

	this->erase_team_fb_info();

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		LogicPlayer *player = this->find_player(*iter);
		JUDGE_CONTINUE(NULL != player);

		player->sync_team_info_to_scene();
		player->notify_team_info();

		JUDGE_CONTINUE(*iter != this->role_id());
		player->respond_to_client(ACTIVE_TEAM_FB_ORG_CANCEL);
		player->team_info_output_test("主动消息：关闭副本界面");
	}

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_ORG_CANCEL);
}

int LogicTeamer::team_fb_recruit_replacement(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100417*, request, RETURN_TEAM_FB_REPLACEMENT_RECRUIT);
	int64_t role_id = request->role_id();

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_NO_TEAM);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_NO_SCRIPT_TEAM);

	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_NO_LEADER);

	CONDITION_NOTIFY_RETURN(NULL != team_panel->rpm_recomand_info_,
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_SERVER_INNER);

	CONDITION_NOTIFY_RETURN(false == team_panel->teamer_full(),
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_TEAM_FULL);

	int ret = this->validate_fb(team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ret);

	int i=0;
	ReplacementRoleInfo *rpm_role_info = NULL;
	for( ; i< team_panel->rpm_recomand_info_->__info_count; ++i)
	{
		rpm_role_info = &(team_panel->rpm_recomand_info_->__role_info_list[i]);
		JUDGE_BREAK(role_id != rpm_role_info->__role_id);

		rpm_role_info = NULL;
	}

	CONDITION_NOTIFY_RETURN(NULL != rpm_role_info,
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_CLIENT_OPERATE);

	// 判断是否已有同一化身或玩家在队伍中
	CONDITION_NOTIFY_RETURN(false == team_panel->validate_teamer(role_id),
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_IS_TEAMER);

	CONDITION_NOTIFY_RETURN(false == team_panel->validate_replacement(role_id),
			RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_TEAM_HAS_REPLACEMENT);

	CONDITION_NOTIFY_RETURN(team_panel->rpm_teamer_info_.count(role_id) <= 0,
				RETURN_TEAM_FB_REPLACEMENT_RECRUIT, ERROR_TEAM_HAS_REPLACEMENT);

	ReplacementInfo &insert_rpm_info = team_panel->rpm_teamer_info_[role_id];
	insert_rpm_info.set_replacement_info(rpm_role_info);
	team_panel->replacement_set_.push_back(role_id);

    this->notify_all_teamer_team_info();
    this->sync_all_teamer_team_info();

	return 0;
}

int LogicTeamer::team_fb_friend_list()
{
	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_FRIEND_LIST, ERROR_NO_TEAM);
	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_FRIEND_LIST, ERROR_NO_SCRIPT_TEAM);

	int fb_id = team_panel->fb_id_;
	int ret = this->validate_fb(fb_id);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_FRIEND_LIST, ret);

	LogicPlayer* player = NULL;
	Proto50100403 respond;

	LogicSocialerDetail socialer_detail = this->logic_player()->socialer_detail();
	for(LongMap::iterator iter = socialer_detail.__friend_list.begin();
			iter != socialer_detail.__friend_list.end(); ++iter)
	{
		player = this->find_player(iter->first);
		JUDGE_CONTINUE(NULL != player);

		// 是否已经有队伍
		JUDGE_CONTINUE(NULL == player->team_panel())

		JUDGE_CONTINUE(0 == player->validate_fb(fb_id));
		player->make_up_teamer_info(respond.add_role_list());
	}

	FINER_PROCESS_RETURN(RETURN_TEAM_FB_FRIEND_LIST, &respond);
	return 0;
}

int LogicTeamer::team_fb_leaguer_list()
{
	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_LEAGUER_LIST, ERROR_NO_TEAM);
	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_LEAGUER_LIST, ERROR_NO_SCRIPT_TEAM);

	League* league = this->logic_player()->league();
	CONDITION_NOTIFY_RETURN(NULL != league, RETURN_TEAM_FB_LEAGUER_LIST, ERROR_LEAGUE_NO_EXIST);

	int ret = this->validate_fb(team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_LEAGUER_LIST, ret);

	PairObjVec league_member;
	league->fetch_sort_member(league_member);
	LogicPlayer* player = NULL;
	Proto50100404 respond;

	for(PairObjVec::iterator iter = league_member.begin();
			iter != league_member.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->id_ != this->role_id());// 跳过自己
		JUDGE_CONTINUE(iter->value_ == 0);// 跳过离线

		player = this->find_player(iter->id_);
		JUDGE_CONTINUE(NULL != player);
		JUDGE_CONTINUE(NULL == player->team_panel());
		JUDGE_CONTINUE(0 == player->validate_fb(team_panel->fb_id_));

		player->make_up_teamer_info(respond.add_role_list());
	}

	FINER_PROCESS_RETURN(RETURN_TEAM_FB_LEAGUER_LIST, &respond);
	return  0;
}

int LogicTeamer::team_fb_invite_into_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100405*, request, RETURN_TEAM_FB_INVITE_INTO_TEAM);

	Int64 role_id = request->role_id();
	TeamPanel* team_panel = this->team_panel();
	if(request->is_couple() == true)
	{
		LogicPlayer* my_player = this->find_player(this->role_id());
		CONDITION_NOTIFY_RETURN(NULL != my_player, RETURN_TEAM_FB_INVITE_INTO_TEAM,
				ERROR_SERVER_INNER);
		CONDITION_NOTIFY_RETURN(my_player->is_has_wedding() == true,
				RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_COUPLE_CAN_ENTER_COUPLE_FB);

		role_id = my_player->fetch_wedding_partner_id();

		if (NULL == team_panel)
		{
			// 创建队伍
			int ret = create_team_i();
			CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_ORG_TEAM, ret);

			this->notify_team_info();
			this->sync_team_info_to_scene();
		}
		team_panel = this->team_panel();
		CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_ORG_TEAM, ERROR_SERVER_INNER);

		team_panel->fb_id_ = GameEnum::TEAM_COUPLE_FB;

		this->sync_all_teamer_fb_use_times();
	}

	LogicPlayer* player = this->find_player(role_id);
	CONDITION_NOTIFY_RETURN(NULL != player, RETURN_TEAM_FB_INVITE_INTO_TEAM,
			ERROR_PLAYER_OFFLINE);

	CONDITION_NOTIFY_RETURN(true == this->validate_team_level_limit(), RETURN_TEAM_FB_INVITE_INTO_TEAM,
			ERROR_PLAYER_LEVEL_LIMIT);

	JUDGE_RETURN(player->team_set_info().validate_inviter(
			this->role_id()) == true, ERROR_TEAM_INVITED);

	TeamPanel* peer_team_panel = player->team_panel();

	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_NO_TEAM);

	if (request->is_couple() == false)
	{
		CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
				RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_CLIENT_OPERATE);
	}

	int ret = this->validate_fb(team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_INVITE_INTO_TEAM, ret);

	if(NULL != peer_team_panel)
	{
		CONDITION_NOTIFY_RETURN(team_panel->team_index_ != peer_team_panel->team_index_,
				RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_IS_TEAMER);
	}

	CONDITION_NOTIFY_RETURN(0 == player->validate_fb(team_panel->fb_id_),
			RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(NULL == peer_team_panel,
			RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_HAVE_TEAM);

	CONDITION_NOTIFY_RETURN(false == team_panel->validate_replacement(role_id),
			RETURN_TEAM_FB_INVITE_INTO_TEAM, ERROR_TEAM_HAS_REPLACEMENT);

	// 向玩家发送邀请信息
	ret = this->team_fb_notify_invite_info(player);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_INVITE_INTO_TEAM, ret);

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_INVITE_INTO_TEAM);
}

int LogicTeamer::team_fb_request_enter_couple_fb()
{
	int ret = this->validate_enter_couple_fb();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_FB_ENTER_COUPLE_FB, ret);

	LogicPlayer* my_player = this->find_player(this->role_id());
	Int64 partner_id = my_player->fetch_wedding_partner_id();
	LogicPlayer* player = this->find_player(partner_id);

	TeamPanel* team_panel = this->team_panel();
	team_panel->fb_id_ = GameEnum::TEAM_COUPLE_FB;

	this->team_set_info_.auto_accept_ = false;
	this->sync_all_teamer_fb_use_times();

	// 向对方发送确认进入副本通知
	player->respond_to_client(ACTIVE_NOTIFY_COUPLE_ENTER_FB);

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_ENTER_COUPLE_FB);
}

int LogicTeamer::team_fb_respond_enter_couple_fb(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100420*, request, RETURN_TEAM_FB_ENTER_COUPLE_FB_RESPOND);

	int ret = this->validate_enter_couple_fb();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_FB_ENTER_COUPLE_FB_RESPOND, ret);

	LogicPlayer* my_player = this->find_player(this->role_id());
	Int64 partner_id = my_player->fetch_wedding_partner_id();
	LogicPlayer* player = this->find_player(partner_id);

	int respond_state = request->state();
	player->handle_respond_enter_couple_fb(respond_state);

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_ENTER_COUPLE_FB_RESPOND);
}

int LogicTeamer::validate_enter_couple_fb()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_TEAM_NO_EXIST);
	JUDGE_RETURN(team_panel->teamer_set_.size() == 2, ERROR_COUPLE_CAN_ENTER_COUPLE_FB);

	LogicPlayer* my_player = this->find_player(this->role_id());
	JUDGE_RETURN(NULL != my_player, ERROR_SERVER_INNER);
	JUDGE_RETURN(my_player->is_has_wedding() == true, ERROR_COUPLE_CAN_ENTER_COUPLE_FB);

	Int64 partner_id = my_player->fetch_wedding_partner_id();
	LogicPlayer* player = this->find_player(partner_id);
	JUDGE_RETURN(NULL != player, ERROR_PLAYER_OFFLINE);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE((*iter) != this->role_id());
		JUDGE_RETURN((*iter) == partner_id, ERROR_COUPLE_CAN_ENTER_COUPLE_FB);
	}

	return 0;
}

int LogicTeamer::handle_respond_enter_couple_fb(int respond_state)
{
	CONDITION_NOTIFY_RETURN(respond_state == true, RETURN_TEAM_FB_ENTER_COUPLE_FB,
			ERROR_COUPLE_REJECT_ENTER_FB);

	int ret = this->validate_enter_couple_fb();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TEAM_FB_ENTER_COUPLE_FB, ret);

	TeamPanel* team_panel = this->team_panel();
	team_panel->fb_id_ = GameEnum::TEAM_COUPLE_FB;

	ret = validate_fb(team_panel->fb_id_, GameEnum::EXTRA_CONDITION_SEVEN_PROGRESS
			| GameEnum::EXTRA_CONDITION_SEVEN_TIME);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_ENTER_COUPLE_FB, ret);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		CONDITION_NOTIFY_RETURN(team_panel->use_times_set_[(*iter)] > 0,
				RETURN_TEAM_FB_ENTER_COUPLE_FB, ERROR_SCRIPT_FINISH_TIMES);
	}

	const Json::Value &script_json = CONFIG_INSTANCE->script(team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(script_json != Json::nullValue, RETURN_TEAM_FB_ENTER_COUPLE_FB,
			ERROR_CONFIG_NOT_EXIST);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		CONDITION_NOTIFY_RETURN(player->role_level() >= script_json["prev_condition"]["level"].asInt(),
				RETURN_TEAM_FB_ENTER_COUPLE_FB, ERROR_TEAMER_LEVEL_LIMIT);
	}

	team_panel->team_state_ = GameEnum::TEAM_STATE_FB_INSIDE;

	Proto10400901 enter_request;
	enter_request.set_script_sort(team_panel->fb_id_);
	enter_request.set_team_id(team_panel->team_index_);
	for (LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		enter_request.add_team_set(*iter);

		LogicPlayer *player = this->find_player(*iter);
		JUDGE_CONTINUE(NULL != player);

		CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(player->scene_id()),
				RETURN_TEAM_FB_ENTER_COUPLE_FB, ERROR_NOT_IN_NORMAL_SCENE);
	}

	this->monitor()->dispatch_to_scene(this, &enter_request);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		JUDGE_CONTINUE(this->role_id() != *iter);
		LogicPlayer *player = this->find_player(*iter);
		JUDGE_CONTINUE(NULL != player);

		this->monitor()->dispatch_to_scene(player, &enter_request);
	}

	return 0;
}

int LogicTeamer::request_couple_fb_send_times()
{
	LogicPlayer* my_player = this->find_player(this->role_id());
	CONDITION_NOTIFY_RETURN(NULL != my_player, RETURN_TEAM_FB_REQUEST_GET_TIMES,
			ERROR_SERVER_INNER);
	CONDITION_NOTIFY_RETURN(my_player->is_has_wedding() == true,
			RETURN_TEAM_FB_REQUEST_GET_TIMES, ERROR_COUPLE_CAN_ENTER_COUPLE_FB);

	Int64 partner_id = my_player->fetch_wedding_partner_id();
	LogicPlayer* player = this->find_player(partner_id);
	CONDITION_NOTIFY_RETURN(NULL != player, RETURN_TEAM_FB_REQUEST_GET_TIMES,
			ERROR_PLAYER_OFFLINE);

	player->respond_to_client(ACTIVE_REQUEST_GET_TIMES);

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_REQUEST_GET_TIMES);
}

int LogicTeamer::team_fb_apply_into_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100406*, request, RETURN_TEAM_FB_APPLY_INTO_TEAM);

	CONDITION_NOTIFY_RETURN(true == this->validate_team_level_limit(), RETURN_TEAM_FB_APPLY_INTO_TEAM,
			ERROR_PLAYER_LEVEL_LIMIT);

	Int64 role_id = request->leader_id();

	CONDITION_NOTIFY_RETURN(role_id != this->role_id(),
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_CLIENT_OPERATE);

	LogicPlayer* player = this->find_player(role_id);
	CONDITION_NOTIFY_RETURN(NULL != player,
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_PLAYER_OFFLINE);

	CONDITION_NOTIFY_RETURN(player->team_set_info().validate_applier(
			this->role_id()) == true, RETURN_TEAM_FB_APPLY_INTO_TEAM,
			ERROR_TEAM_APPLIED);

	TeamPanel* team_panel = this->team_panel();
	TeamPanel* peer_team_panel = player->team_panel();

	CONDITION_NOTIFY_RETURN(NULL != peer_team_panel,
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_TEAM_NO_EXIST);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_INSIDE != peer_team_panel->team_state_,
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_CANT_JOIN_IN_FB);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == peer_team_panel->team_state_,
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_OPERATE_TIME_OUT);

	if(NULL != team_panel)
	{
		CONDITION_NOTIFY_RETURN(team_panel->team_index_ != peer_team_panel->team_index_,
				RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_IS_TEAMER);
	}

	CONDITION_NOTIFY_RETURN(false == peer_team_panel->teamer_full(),
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_TEAM_FULL);

	CONDITION_NOTIFY_RETURN(NULL == team_panel,
			RETURN_TEAM_FB_APPLY_INTO_TEAM, ERROR_SELF_HAVE_TEAM);

	int ret = this->validate_fb(peer_team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_APPLY_INTO_TEAM, ret);

	// 向玩家发送邀请信息
	ret = this->team_fb_notify_apply_info(player);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_APPLY_INTO_TEAM, ret);

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_APPLY_INTO_TEAM);
}

int LogicTeamer::team_fb_respond_invite(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100407*, request, RETURN_TEAM_FB_RESPOND_INVITE);

	for (int i = 0; i < request->role_id_size(); ++i)
	{
		Int64 role_id = request->role_id(i);//

		if(this->team_set_info_.team_inviter_map_.count(role_id) <= 0)
		{// 操作超时
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_INVITE, ERROR_OPERATE_TIME_OUT);
			continue;
		}

		this->team_set_info_.team_inviter_map_.erase(role_id);

		LogicPlayer* player = this->find_player(role_id);
		if(NULL == player)
		{
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_INVITE, ERROR_PLAYER_OFFLINE);
			continue;
		}

		TeamPanel *peer_team_panel = player->team_panel();
		int respone_type = GameEnum::TEAM_OPER_REJECT;

		if((this->team_panel() != NULL))
		{//返回自己已有队伍
			respone_type = GameEnum::TEAM_OPER_REJECT;
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_INVITE, ERROR_SELF_HAVE_TEAM);
		}
		else if(request->oper_type() == GameEnum::TEAM_OPER_AGREE)
		{// accept
			if(NULL != peer_team_panel && (peer_team_panel->validate_replacement(this->role_id())) == true)
			{// 已有队伍并且已存在化身
				respone_type = GameEnum::TEAM_OPER_REJECT;
				this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_INVITE, ERROR_OPERATE_TIME_OUT);
			}
			else if (player->accept_invite_team_i(this->role_id()) == 0)
			{
				respone_type = GameEnum::TEAM_OPER_AGREE;
			}
			else
			{
				respone_type = GameEnum::TEAM_OPER_FULL;
			}
		}
		else
		{
			respone_type = GameEnum::TEAM_OPER_REJECT;
		}

		player->notify_team_oper_result(GameEnum::TEAM_OPER_INVITE, respone_type, this->name());
	}

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_RESPOND_INVITE);
}

int LogicTeamer::team_fb_respond_apply(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100408*, request, RETURN_TEAM_FB_RESPOND_APPLY);

	Proto50100408 respond;
	for (int i = 0; i < request->role_id_size(); ++i)
	{
		Int64 role_id = request->role_id(i);

		if(this->team_set_info_.team_applier_map_.count(role_id) <= 0)
		{
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_APPLY, ERROR_OPERATE_TIME_OUT);
			continue;
		}

		this->team_set_info_.team_applier_map_.erase(role_id);

		LogicPlayer* player = this->find_player(role_id);
		if(NULL == player)
		{
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_APPLY, ERROR_PLAYER_OFFLINE);
			continue;
		}

		if(NULL != player->team_panel())
		{//  对方已有队伍，不通知对方被拒绝
			this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_APPLY,  ERROR_HAVE_TEAM);
			continue;
		}

		int respone_type = GameEnum::TEAM_OPER_REJECT;
		if (request->oper_type() == GameEnum::TEAM_OPER_AGREE)
		{
			TeamPanel *team_panel = this->team_panel();
			if(NULL == team_panel)
			{
				respone_type = GameEnum::TEAM_OPER_REJECT;
				this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_APPLY, ERROR_NO_TEAM);
			}
			else if(true == team_panel->validate_replacement(role_id))
			{
				respone_type = GameEnum::TEAM_OPER_REJECT;
				this->respond_to_client_error(RETURN_TEAM_FB_RESPOND_APPLY, ERROR_TEAM_HAS_REPLACEMENT);
			}
			// accept
			else if (this->accept_apply_team_i(role_id) != 0)
			{
				respone_type = GameEnum::TEAM_OPER_FULL;
			}
			else
			{
				respone_type = GameEnum::TEAM_OPER_AGREE;
			}
		}
		else
		{// reject
			respone_type = GameEnum::TEAM_OPER_REJECT;
		}

		if(NULL != this->team_panel() && GameEnum::TEAM_STATE_FB_ORG == this->team_panel()->team_state_)
		{
			player->fetch_team_fb_info();
			player->notify_team_oper_result(GameEnum::TEAM_OPER_FB_APPLY, respone_type, this->name());
			respond.add_role_id(role_id);
		}
		else
		{
			player->notify_team_oper_result(GameEnum::TEAM_OPER_APPLY, respone_type, this->name());
		}
	}

	return this->respond_to_client(RETURN_TEAM_FB_RESPOND_APPLY, &respond);
}

int LogicTeamer::team_fb_respond_team_switch(Message* msg)
{
	this->team_info_output_test("回应转换请求", msg);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100410*, request, RETURN_TEAM_FB_CONFIRM_SWITCH);

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_CONFIRM_SWITCH, ERROR_NO_TEAM);
	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_CONFIRM_SWITCH, ERROR_NO_SCRIPT_TEAM);

	int ret = this->reply_confirm_switch_fb_team(request->oper_type());

	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_CONFIRM_SWITCH, ret);
	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_CONFIRM_SWITCH);
}

int LogicTeamer::team_fb_get_ready(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100409*, request, RETURN_TEAM_FB_GET_READY);

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_GET_READY, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_GET_READY, ERROR_NO_SCRIPT_TEAM);

	int fb_id = team_panel->fb_id_;
	bool is_ready = request->ready_state();
	if(false == is_ready)
	{// 取消准备，不作验证
		team_panel->fb_ready_set_.erase(this->role_id());
		this->notify_all_teamer_ready_state(is_ready);
		FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_GET_READY);
	}
	else
	{// 验证进入准备
		int ret = validate_fb(fb_id);
		if (ret == ERROR_SELF_LEVEL_LIMIT)
		{
			LogicPlayer *leader = NULL;
			if (this->monitor()->find_player(team_panel->leader_id_, leader) == 0)
			{
				leader->notify_tips_info(GameEnum::FORMAT_MSG_SCRIPT_READY_LEVEL_LIMIT,
						this->name());
			}
		}
		CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_GET_READY, ret);

		CONDITION_NOTIFY_RETURN(team_panel->use_times_set_[this->role_id()] > 0,
					RETURN_TEAM_FB_GET_READY, ERROR_SCRIPT_FINISH_TIMES);
		Proto30400704 scene_req;
		scene_req.set_script_sort(team_panel->fb_id_);
		return this->monitor()->dispatch_to_scene(this, &scene_req);

//		team_panel->fb_ready_set_[this->role_id()] = this->role_id();
//		this->notify_all_teamer_ready_state(is_ready);
//		FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_GET_READY);
	}
}

int LogicTeamer::team_fb_get_ready_from_scene(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400704*, request, RETURN_TEAM_FB_GET_READY);

	int req_result = request->req_result();
	int script_sort = request->script_sort();
	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_GET_READY, ERROR_SERVER_INNER);
	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_GET_READY, ERROR_NO_SCRIPT_TEAM);
	CONDITION_NOTIFY_RETURN(team_panel->fb_id_ == script_sort,
			RETURN_TEAM_FB_GET_READY, ERROR_SERVER_INNER);
	if(0 == req_result)
	{
		team_panel->fb_ready_set_[this->role_id()] = this->role_id();
		this->notify_all_teamer_ready_state(true);
		FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_GET_READY);
	}
	else
	{
		return this->respond_to_client_error(
				RETURN_TEAM_FB_GET_READY, req_result);
	}
}

int LogicTeamer::team_fb_change_fb(Message* msg)
{
	this->team_info_output_test("更换副本", msg);
	MSG_DYNAMIC_CAST_RETURN(Proto10100413*, request, RETURN_TEAM_FB_CHANGE_FB);
	int fb_id = request->fb_id();

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel, RETURN_TEAM_FB_CHANGE_FB, ERROR_NO_TEAM);

	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_CHANGE_FB, ERROR_NO_LEADER);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_CHANGE_FB, ERROR_NO_SCRIPT_TEAM);

	int ret = validate_fb(fb_id, GameEnum::EXTRA_CONDITION_SEVEN_PROGRESS
			| GameEnum::EXTRA_CONDITION_SEVEN_TIME);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_CHANGE_FB, ret);

	CONDITION_NOTIFY_RETURN(fb_id != team_panel->fb_id_,
			RETURN_TEAM_FB_CHANGE_FB, ERROR_CHANGE_SAME_FB);

	team_panel->fb_id_ = fb_id;

	this->respond_to_client(RETURN_TEAM_FB_CHANGE_FB);
	this->notify_all_teamer_fb_info();
	this->sync_all_teamer_fb_use_times();// 获取次数
//	this->cancel_all_teamer_ready_state();// 重新准备

	return 0;
}

int LogicTeamer::team_fb_enter_fb(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100414*, request, RETURN_TEAM_FB_ENTER_FB);
	TeamPanel* team_panel = this->team_panel();
	int request_sort = request->script_sort();

	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_ENTER_FB, ERROR_NO_TEAM);

	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_ENTER_FB, ERROR_NO_LEADER);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_ENTER_FB, ERROR_NO_SCRIPT_TEAM);

	int ret = validate_fb(team_panel->fb_id_, GameEnum::EXTRA_CONDITION_SEVEN_PROGRESS
			| GameEnum::EXTRA_CONDITION_SEVEN_TIME);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_ENTER_FB, ret);

	CONDITION_NOTIFY_RETURN(team_panel->use_times_set_[this->role_id()] > 0,
		RETURN_TEAM_FB_GET_READY, ERROR_SCRIPT_FINISH_TIMES);
	CONDITION_NOTIFY_RETURN(team_panel->fb_id_ == request_sort,
			RETURN_TEAM_FB_ENTER_FB, ERROR_CLIENT_OPERATE);

	const Json::Value &script_json = CONFIG_INSTANCE->script(team_panel->fb_id_);
	JUDGE_RETURN(script_json != Json::nullValue, ERROR_CONFIG_NOT_EXIST);

	CONDITION_NOTIFY_RETURN(team_panel->teamer_count() >= script_json["prev_condition"]["min_teamer"].asInt(),
			RETURN_TEAM_FB_ENTER_FB, ERROR_TEAM_MIN_N);

	std::vector<std::string> level_limit_name_list, scene_limit_name_list;

	bool all_ready = true;
	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		JUDGE_CONTINUE(false == team_panel->is_leader(*iter));

		LogicPlayer* player = this->find_player(*iter);
		if(NULL == player)
		{
			team_panel->fb_ready_set_.erase(*iter);
			this->fetch_teamers_ready_state();
			all_ready = false;
			continue;
		}

		if (player->role_level() < script_json["prev_condition"]["level"].asInt())
			level_limit_name_list.push_back(player->role_detail().__name);

		if(team_panel->fb_ready_set_.count(*iter) <= 0)
		{
			all_ready = false;
		}
	}

	if (level_limit_name_list.size() > 0)
	{

		if (level_limit_name_list.size() == 1)
		{
			this->notify_tips_info(GameEnum::FORMAT_MSG_SCRIPT_ONE_LEVEL_LIMIT,
					level_limit_name_list[0].c_str());
		}
		else if (level_limit_name_list.size() == 2)
		{
			this->notify_tips_info(GameEnum::FORMAT_MSG_SCRIPT_TWO_LEVEL_LIMIT,
					level_limit_name_list[0].c_str(),
					level_limit_name_list[1].c_str());
		}
		return 0;
	}

	if (scene_limit_name_list.size() > 0)
	{
		if (scene_limit_name_list.size() == 1)
		{
			this->notify_tips_info(GameEnum::FORMAT_MSG_SCRIPT_ONE_SPECIAL_SPACE,
					scene_limit_name_list[0].c_str());
		}
		else if (scene_limit_name_list.size() == 2)
		{
			this->notify_tips_info(GameEnum::FORMAT_MSG_SCRIPT_TWO_SPECIAL_SPACE,
					scene_limit_name_list[0].c_str(),
					scene_limit_name_list[1].c_str());
		}
		return 0;
	}

	CONDITION_NOTIFY_RETURN(true == all_ready,
			RETURN_TEAM_FB_ENTER_FB, ERROR_FB_NOT_ALL_READY);

	Proto30400703 scene_req;
	scene_req.set_script_sort(team_panel->fb_id_);
	for(LongList::iterator iter = team_panel->replacement_set_.begin();
			iter != team_panel->replacement_set_.end(); ++ iter)
	{
		scene_req.add_replacement_set(*iter);
	}

	return this->monitor()->dispatch_to_scene(this, &scene_req);
}


int LogicTeamer::team_fb_enter_fb_from_scene(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400703*, request, -1);

	int req_result = request->req_result();
	CONDITION_NOTIFY_RETURN(0 == req_result,
			RETURN_TEAM_FB_ENTER_FB, req_result);

	TeamPanel* team_panel = this->team_panel();

	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_ENTER_FB, ERROR_NO_TEAM);

	CONDITION_NOTIFY_RETURN(request->script_sort() == team_panel->fb_id_,
			RETURN_TEAM_FB_ENTER_FB, ERROR_SERVER_INNER);
	team_panel->team_state_ = GameEnum::TEAM_STATE_FB_INSIDE;

	this->respond_to_client(RETURN_TEAM_FB_ENTER_FB);

	Proto10400901 enter_request;
	enter_request.set_script_sort(request->script_sort());

	for(int i=0; i<request->replacement_set_size(); ++i )
	{
		enter_request.add_replacements_set(request->replacement_set(i));
	}
	this->monitor()->dispatch_to_scene(this, &enter_request);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++ iter)
	{
		JUDGE_CONTINUE(this->role_id() != *iter);
		LogicPlayer *player = this->find_player(*iter);
		JUDGE_CONTINUE(NULL != player);

		this->monitor()->dispatch_to_scene(player, &enter_request);
	}
	return 0;
}

int LogicTeamer::team_fb_leave_fb(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100702*, request, -1);

	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);
	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_INSIDE == team_panel->team_state_ , -1);

	if(request->script_sort() != team_panel->fb_id_)
	{
		MSG_USER("ERROR NOT THE SAME SCRIPT");
	}

	this->erase_team_fb_info();
	this->notify_all_teamer_team_info();
	this->sync_all_teamer_team_info();

	return 0;
}

int LogicTeamer::team_fb_dismiss_team()
{
	TeamPanel* team_panel = this->team_panel();

	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_DISMISS_TEAM, ERROR_NO_TEAM);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_INSIDE != team_panel->team_state_,
			RETURN_TEAM_FB_DISMISS_TEAM, ERROR_DISMISS_TEAM_IN_FB);

	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_DISMISS_TEAM, ERROR_NO_LEADER);

	this->dismiss_team_i();
	this->sync_all_teamer_team_info();
	this->notify_all_teamer_team_info();
	this->notify_all_teamer_couple_fb_info();

	FINER_PROCESS_NOTIFY(RETURN_TEAM_FB_DISMISS_TEAM);
}

// 副本结束清除化身
int LogicTeamer::team_fb_clean_replacement()
{
	TeamPanel* team_panel = this->team_panel();

	JUDGE_RETURN(NULL != team_panel, ERROR_CLIENT_OPERATE);
	JUDGE_RETURN(true == team_panel->is_leader(this->role_id()), ERROR_NO_LEADER);

	team_panel->replacement_set_.clear();
	team_panel->rpm_teamer_info_.clear();

	this->notify_all_teamer_team_info();
	this->sync_all_teamer_team_info();

	return 0;
}

int LogicTeamer::team_fb_remove_replacement(int64_t role_id)
{
	TeamPanel* team_panel = this->team_panel();

	JUDGE_RETURN(NULL != team_panel, ERROR_CLIENT_OPERATE);
	JUDGE_RETURN(true == team_panel->is_leader(this->role_id()), ERROR_NO_LEADER);
	JUDGE_RETURN(team_panel->rpm_teamer_info_.count(role_id) > 0, ERROR_CLIENT_OPERATE);

	team_panel->erase_replacement(role_id);

	this->notify_all_teamer_team_info();
	this->sync_all_teamer_team_info();

	return 0;
}

int LogicTeamer::begin_get_rpm_recomand_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10100416*, request, -1);
	int is_introduction = request->is_introduction();

	TeamPanel* team_panel = this->team_panel();
	CONDITION_NOTIFY_RETURN(NULL != team_panel,
			RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(true == team_panel->is_leader(this->role_id()),
			RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_NO_LEADER);

	CONDITION_NOTIFY_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_,
			RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_NO_SCRIPT_TEAM);

	int ret = this->validate_fb(team_panel->fb_id_);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_TEAM_FB_REPLACEMENT_LIST, ret);

	RpmRecomandInfo* rpm_recomand_info = this->monitor()->rpm_recomand_info_pool()->pop();

	CONDITION_NOTIFY_RETURN(NULL != rpm_recomand_info,
			RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_SERVER_INNER);

	rpm_recomand_info->__leader_force = this->role_detail().__fight_force;

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		rpm_recomand_info->__teamates_id[*iter] = *iter;
	}

	for(LongList::iterator iter = team_panel->replacement_set_.begin();
			iter != team_panel->replacement_set_.end(); ++iter)
	{
		rpm_recomand_info->__teamates_id[*iter] = *iter;
	}

	int trans_recogn = TRANS_GET_RPM_RECOMAND_INFO;
	if(is_introduction)
		trans_recogn = TRANS_GET_RPM_INTORDUCTION_INFO;

	if(0 != TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			trans_recogn, DB_RPM_RECOMAND_INFO, rpm_recomand_info,
			this->monitor()->rpm_recomand_info_pool(), this->monitor()->logic_unit()))
	{
		this->monitor()->rpm_recomand_info_pool()->push(rpm_recomand_info);
		this->respond_to_client_error(RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_SERVER_INNER);
		return -1;
	}

	return 0;
}

int LogicTeamer::after_get_rpm_recomand_info(Transaction* trans)
{
	JUDGE_RETURN(trans != NULL, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return this->respond_to_client_error(RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_SERVER_INNER);
	}

	TransactionData* trans_data = trans->fetch_data(DB_RPM_RECOMAND_INFO);
	if(trans_data == 0)
	{
		trans->rollback();
		return this->respond_to_client_error(RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_SERVER_INNER);
	}

	RpmRecomandInfo* rpm_recomand_info = trans_data->__data.__rpm_recomand_info;
	trans_data->__data.__rpm_recomand_info = NULL;
	trans->summit();

	TeamPanel* team_panel = this->team_panel();

	if(NULL == team_panel || false == team_panel->is_leader(this->role_id()))
	{
		this->monitor()->rpm_recomand_info_pool()->push(rpm_recomand_info);
		return this->respond_to_client_error(RETURN_TEAM_FB_REPLACEMENT_LIST, ERROR_SERVER_INNER);
	}

	if(NULL != team_panel->rpm_recomand_info_)
		this->monitor()->rpm_recomand_info_pool()->push(team_panel->rpm_recomand_info_);

	team_panel->rpm_recomand_info_ = rpm_recomand_info;

	Proto50100416 respond;
	for(int i=0; i< rpm_recomand_info->__info_count; ++i)
	{
		JUDGE_BREAK(i < RPM_LIST_LENGTH);
		ReplacementRoleInfo &rpm_info = rpm_recomand_info->__role_info_list[i];
		ProtoRpmRoleInfo* proto_info = respond.add_replacement_role_list();
		proto_info->set_role_id(rpm_info.__role_id);
		proto_info->set_force(rpm_info.__fight_force);
		proto_info->set_level(rpm_info.__level);
		proto_info->set_role_name(rpm_info.__name);
		proto_info->set_career(rpm_info.__career);
	}

	FINER_PROCESS_RETURN(RETURN_TEAM_FB_REPLACEMENT_LIST, &respond);

}

// 通知玩家邀请加入副本信息(主动)
int LogicTeamer::team_fb_notify_invite_info(LogicPlayer* player)
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_HAVE_TEAM);

	if(NULL != team_panel)
	{
		JUDGE_RETURN(team_panel->teamer_full() == false, ERROR_TEAM_FULL);
	}

	JUDGE_RETURN(player->team_set_info().validate_inviter(
			this->role_id()) == true, ERROR_TEAM_INVITED);

	player->team_set_info().team_inviter_map_[this->role_id()] = ::time(NULL);
	this->team_fb_notify_request(player, GameEnum::TEAM_OPER_INVITE);

	TEAM_PLANTFORM->add_replier(player->role_id());
	return 0;
}

// 通知玩家申请加入副本信息(主动)
int LogicTeamer::team_fb_notify_apply_info(LogicPlayer* player)
{
	TeamPanel* team_panel = player->team_panel();
	JUDGE_RETURN(team_panel != NULL, ERROR_TEAM_NO_EXIST);
	JUDGE_RETURN(team_panel->teamer_full() == false, ERROR_TEAM_FULL);

	TeamSetInfo& team_set_info = player->team_set_info();
	JUDGE_RETURN(team_set_info.validate_applier(this->role_id()) == true,
			ERROR_TEAM_APPLIED);

	team_set_info.team_applier_map_[this->role_id()] = ::time(NULL);
	TEAM_PLANTFORM->add_replier(player->role_id());
	{
		Proto10100408 req;
		req.add_role_id(this->role_id());
		req.set_oper_type(GameEnum::TEAM_OPER_AGREE);
		player->team_fb_respond_apply(&req);
	}
	return 0;
}

// 副本组队请求信息(主动)
int LogicTeamer::team_fb_notify_request(LogicPlayer* player, int oper_type)
{
	if(GameEnum::TEAM_OPER_APPLY == oper_type)
	{
		Proto80100402 applicant_info;
		this->make_up_teamer_info(applicant_info.mutable_applicant());

		FINER_PLAYER_PROCESS_RETURN(ACTIVE_INVITOR_INFO, &applicant_info);
	}
	else if(GameEnum::TEAM_OPER_INVITE == oper_type)
	{
		TeamPanel* team_panel = this->team_panel();
		JUDGE_RETURN(NULL != team_panel, ERROR_NO_TEAM);

		Proto80100401 invitor_info;
		invitor_info.set_fb_id(team_panel->fb_id_);

		this->make_up_teamer_info(invitor_info.mutable_invitor());
		FINER_PLAYER_PROCESS_RETURN(ACTIVE_APPLICANT_INFO, &invitor_info);
	}

	return 0;
}

int LogicTeamer::notify_all_teamer_fb_info()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80100403 fb_team_info;
	fb_team_info.set_fb_id(team_panel->fb_id_);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_TEAM_FB_INFO, &fb_team_info);
	}

	return 0;
}

// 通知队伍其他玩家自己的准备状态
int LogicTeamer::notify_all_teamer_ready_state(bool is_ready)
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);
	JUDGE_RETURN(false == team_panel->is_leader(this->role_id()), 0);

	Proto80100404 ready_info;
	ProtoFBReadyInfo* info = ready_info.add_ready_info();
	info->set_role_id(this->role_id());
	info->set_is_ready(is_ready);

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_TEAM_FB_READY_INFO, &ready_info);
		//MSG_USER([%s] <-----%s, player->name() , ready_info.Utf8DebugString().c_str());
	}

	return 0;
}

// 玩家获取面板到副本，难度信息
int LogicTeamer::fetch_team_fb_info()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80100403 fb_team_info;
	fb_team_info.set_fb_id(team_panel->fb_id_);
	FINER_PROCESS_RETURN(ACTIVE_TEAM_FB_INFO, &fb_team_info);
}

// 玩家获取队伍全部玩家的准备状态
int LogicTeamer::fetch_teamers_ready_state()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80100404 ready_info;

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		int64_t role_id = *iter;
		JUDGE_CONTINUE(false == team_panel->is_leader(role_id));
		bool ready_state = (team_panel->fb_ready_set_.count(role_id) > 0);

		ProtoFBReadyInfo* info = ready_info.add_ready_info();
		info->set_role_id(role_id);
		info->set_is_ready(ready_state);
	}

	//MSG_USER([%s] <-----%s,this->name() , ready_info.Utf8DebugString().c_str());
	FINER_PROCESS_RETURN(ACTIVE_TEAM_FB_READY_INFO, &ready_info);
}

int LogicTeamer::erase_team_fb_info()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, ERROR_SERVER_INNER);

	team_panel->team_state_ = GameEnum::TEAM_STATE_NORMAL;

	team_panel->replacement_set_.clear();
	team_panel->use_times_set_.clear();
	team_panel->fb_ready_set_.clear();
	team_panel->rpm_teamer_info_.clear();
	team_panel->fb_wait_confirm_set_.clear();
	team_panel->fb_id_ = 0;

	return 0;
}

int LogicTeamer::set_ready_state(bool is_ready)
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, ERROR_SERVER_INNER);
	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_ORG == team_panel->team_state_, ERROR_NO_SCRIPT_TEAM);
	int fb_id = team_panel->fb_id_;

	if(false == is_ready)
	{// 取消准备，不作验证
		team_panel->fb_ready_set_.erase(this->role_id());
		this->notify_all_teamer_ready_state(is_ready);
	}
	else
	{// 验证进入准备
		int ret = validate_fb(fb_id);
		JUDGE_RETURN(0 == ret, ret);
		JUDGE_RETURN(team_panel->use_times_set_[this->role_id()] > 0, ERROR_SCRIPT_FINISH_TIMES);

		team_panel->fb_ready_set_[this->role_id()] = this->role_id();
		this->notify_all_teamer_ready_state(is_ready);
	}

	return 0;
}
// 取消队伍全部玩家的准备状态
int LogicTeamer::cancel_all_teamer_ready_state()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	team_panel->fb_ready_set_.clear();

	Proto80100404 ready_info;
	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(false == team_panel->is_leader(*iter));
		ProtoFBReadyInfo* info = ready_info.add_ready_info();
		info->set_role_id(*iter);
		info->set_is_ready(false);
	}

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_TEAM_FB_READY_INFO, &ready_info);
		//MSG_USER([%s] <-----%s,player->name() , ready_info.Utf8DebugString().c_str());
	}

	return 0;
}

int LogicTeamer::validate_fb(int script_sort, int extra_condition/*=GameEnum::EXTRA_CONDITION_NONE*/)
{
	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	JUDGE_RETURN(script_json != Json::nullValue, ERROR_CONFIG_NOT_EXIST);

	if(extra_condition & GameEnum::EXTRA_CONDITION_TEAM_EXISTS)
	{
		TeamPanel *team_panel = this->team_panel();
		JUDGE_RETURN(NULL != team_panel, ERROR_NO_TEAM);
	}

	const Json::Value &prev_json = script_json["prev_condition"];
	JUDGE_RETURN(this->role_detail().__level >= prev_json["level"].asInt(),	ERROR_SELF_LEVEL_LIMIT);
	// 需要为多人副本
	JUDGE_RETURN(0 == script_json["is_single"].asInt(), ERROR_CLIENT_OPERATE);
	return 0;
}

int LogicTeamer::validate_use_times(int script_sort, int use_times)
{
	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	JUDGE_RETURN(script_json != Json::nullValue, ERROR_CONFIG_NOT_EXIST);

    int total_times = script_json["prev_condition"]["finish_times"].asInt();
    total_times += GameCommon::script_vip_extra_use_times(this->vip_type(), script_sort);

    int left_times  = total_times - use_times;
    JUDGE_RETURN(left_times > 0, ERROR_SCRIPT_FINISH_TIMES);

    return 0;
}

bool LogicTeamer::is_get_ready(int64_t role_id)
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, false);
	JUDGE_RETURN(true == team_panel->validate_teamer(role_id), false);
	return (team_panel->fb_ready_set_.count(role_id) > 0);
}

bool LogicTeamer::is_runing_script()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, false);
	JUDGE_RETURN(GameEnum::TEAM_STATE_FB_INSIDE == team_panel->team_state_, false);

	return true;
}

int LogicTeamer::sync_fb_use_times(const int oper_type)
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	int scrip_sort = team_panel->fb_id_;
	Proto30400905 req;
	req.set_script_sort(scrip_sort);
	req.set_oper_type(oper_type);

	return this->monitor()->dispatch_to_scene(this, &req);
}

int LogicTeamer::finish_sync_fb_use_times(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100701*, request, -1);
	int script_sort = request->script_sort();
	int left_times = request->use_times();

	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);
	JUDGE_RETURN(script_sort == team_panel->fb_id_, -1)

	team_panel->use_times_set_[this->role_id()] = left_times;
	this->notify_all_teamer_use_times();

	if (request->oper_type() == GameEnum::TEAM_OPER_FB_APPLY)
	{
		if (team_panel->use_times_set_[this->role_id()] > 0)
		{
			// 副本申请入队自动进入准备状态
			Proto10100409 req;
			req.set_ready_state(true);
			this->team_fb_get_ready(&req);
		}
	}

	return 0;
}

int LogicTeamer::sync_all_teamer_fb_use_times()
{
	TeamPanel* team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel,-1);

	team_panel->use_times_set_.clear();
	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->sync_fb_use_times(GameEnum::TEAM_OPER_SYNC_FB_TIMES);
	}

	return 0;
}

int LogicTeamer::notify_all_teamer_use_times()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80100406 use_times_set;
	for(LongMap::iterator iter = team_panel->use_times_set_.begin();
			iter != team_panel->use_times_set_.end(); ++iter)
	{
		ProtoFBUseTimes *use_info = use_times_set.add_use_times_set();
		use_info->set_role_id(iter->first);
		use_info->set_use_times((int)iter->second);
	}

	for(LongList::iterator iter = team_panel->teamer_set_.begin();
			iter != team_panel->teamer_set_.end(); ++iter)
	{
		LogicPlayer *player = this->find_player(*iter);
		JUDGE_CONTINUE(NULL != player);

		player->respond_to_client(ACTIVE_TEAMER_FB_USE_TIMES, &use_times_set);
		player->team_info_output_test("次数信息", &use_times_set);
	}

	return 0;
}

int LogicTeamer::fetch_team_use_times()
{
	TeamPanel *team_panel = this->team_panel();
	JUDGE_RETURN(NULL != team_panel, -1);

	Proto80100406 use_times_set;
	for(LongMap::iterator iter = team_panel->use_times_set_.begin();
			iter != team_panel->use_times_set_.end(); ++iter)
	{
		ProtoFBUseTimes *use_info = use_times_set.add_use_times_set();
		use_info->set_role_id(iter->first);
		use_info->set_use_times((int)iter->second);
	}

	this->team_info_output_test("次数信息", &use_times_set);
	FINER_PROCESS_RETURN(ACTIVE_TEAMER_FB_USE_TIMES, &use_times_set);
}

int LogicTeamer::validate_map_team_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100703*, request, -1);

//	if(NULL == this->team_panel())
//	{
//		this->sync_team_info_to_scene();
//		this->notify_self_quit_team(true);
//		this->save_teamer_info_quit_team();
//
//		if(true == request->in_team_script())
//		{// 在副本中登录但是没有队伍
//			this->monitor()->dispatch_to_scene(this->gate_sid(), this->role_id(),
//					this->scene_id(), CLIENT_REQUEST_EXIT_SYSTEM);
//		}
//	}
//	else
//	{// 队伍为副本战斗状态，玩家不再副本中
//		if(this->team_panel()->team_state_ == GameEnum::TEAM_STATE_FB_INSIDE &&
//				false == request->in_team_script())
//		{
//			this->logic_quit_team();
//		}
//	}

	return 0;
}

int LogicTeamer::save_teamer_info_with_team(TeamPanel *team_panel)
{
//	JUDGE_RETURN(NULL != team_panel, -1);
//	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//	JUDGE_RETURN(NULL != data_map, -1);
//
//	JUDGE_RETURN(true == team_panel->validate_teamer(this->role_id()), -1);
//	MMOTeam::update_team_info(this->role_id(), team_panel, data_map);
//
//	return TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_UPDATE_OFFLINE_TEAM_INFO,
//			DB_MONGO_DATA_MAP, data_map, POOL_MONITOR->mongo_data_map_pool());
	return 0;
}

int LogicTeamer::save_teamer_info_quit_team()
{
//	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//	JUDGE_RETURN(NULL != data_map, -1);
//
//	MMOTeam::update_team_info(this->role_id(), NULL, data_map);
//
//	return TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_UPDATE_OFFLINE_TEAM_INFO,
//			DB_MONGO_DATA_MAP, data_map, POOL_MONITOR->mongo_data_map_pool());
	return 0;
}

bool LogicTeamer::validate_team_level_limit()
{
	int use_team_level = 1;
	const Json::Value& limit_json = CONFIG_INSTANCE->normal_team(1);
	if(limit_json != Json::Value::null && limit_json["open_level"].asInt() != 0)
		use_team_level = limit_json["open_level"].asInt();

	return this->role_level() >= use_team_level;
}

int LogicTeamer::team_fb_deduct_strength(int script_id)
{
	return 0;
}

bool LogicTeamer::team_fb_valiate_strength_enougth(int script_id)
{
	return false;
}

int LogicTeamer::sync_player_enter_result(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100108*, request, -1);
	int is_success = request->is_success();
	int script_sort = request->script_sort();
	if(is_success == 0)
	{
		TeamPanel *team_panel = this->team_panel();
	    JUDGE_RETURN(team_panel != 0,-1);
	    if(!GameCommon::is_script_scene(this->scene_id()))
	    {
			team_panel->no_enter_nums_++;
			if((uint)team_panel->no_enter_nums_ == this->team_panel()->teamer_set_.size())
			{
				if(team_panel->team_state_ == GameEnum::TEAM_STATE_FB_INSIDE)
				{
					team_panel->team_state_ = GameEnum::TEAM_STATE_NORMAL; //撤回状态
					team_panel->no_enter_nums_ = 0;
				}
			}
	    }
	}
	else
	if(is_success == 1)
	{
		if(this->team_fb_valiate_strength_enougth(script_sort))
		{//扣减体力
			this->team_fb_deduct_strength(script_sort);
		}

	}

	return 0;
}
