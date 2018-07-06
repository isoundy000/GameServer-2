/*
 * TrvlScriptMonitor.cpp
 *
 *  Created on: Sep 2, 2016
 *      Author: peizhibi
 */

#include "TrvlScriptMonitor.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "BaseScript.h"
#include "TrvlScriptTeam.h"

TrvlScriptRole::TrvlScriptRole()
{
	TrvlScriptRole::reset();
}

void TrvlScriptRole::reset()
{
	BaseServerInfo::reset();
	BaseMember::reset();

	this->sid_ 			= -1;
	this->team_index_	= 0;
	this->script_type_	= 0;
	this->prep_flag_	= 0;
}

void TrvlScriptRole::serialize(ProtoTeamer* proto)
{
	BaseMember::serialize(proto);
	proto->set_prepare(this->prep_flag_);
}

TrvlScriptMonitor::TrvlScriptMonitor()
{
	// TODO Auto-generated constructor stub
	for (int i = 0; i < TOTAL_SCRIPT_TYPE; ++i)
	{
		this->script_package_[i] = new PoolPackage<TrvlScriptTeam>;
	}

	this->init_ = false;
	this->player_map_ = new PoolPackage<TrvlScriptRole, Int64>;
}

TrvlScriptMonitor::~TrvlScriptMonitor()
{
	// TODO Auto-generated destructor stub
}

int TrvlScriptMonitor::fetch_sid(Int64 role)
{
	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	JUDGE_RETURN(trvl_role != NULL, -1);

	return trvl_role->sid_;
}

int TrvlScriptMonitor::scene_to_type(int scene_id)
{
	return scene_id % 100;
}

int TrvlScriptMonitor::handle_team_timeout()
{
	JUDGE_RETURN(this->init_ == true, -1);

	Int64 min_tick = ::time(NULL) - Time_Value::HOUR;
	for (int i = 1; i < TOTAL_SCRIPT_TYPE; ++i)
	{
		PoolPackage<TrvlScriptTeam>* package = this->fetch_package(i);
		JUDGE_CONTINUE(package != NULL);

		BIntMap index_map = package->fetch_index_map();
		for (BIntMap::iterator iter = index_map.begin();
				iter != index_map.end(); ++iter)
		{
			TrvlScriptTeam* team = package->find_object(iter->first);
			JUDGE_CONTINUE(team != NULL);
			JUDGE_CONTINUE(team->set_tick_ < min_tick);
			this->quit_team_i(team->leader_id());
		}
	}

	return 0;
}

PoolPackage<TrvlScriptTeam>* TrvlScriptMonitor::fetch_package(int type)
{
	JUDGE_RETURN(type > 0 && type < TOTAL_SCRIPT_TYPE, NULL);
	return this->script_package_[type];
}

TrvlScriptRole* TrvlScriptMonitor::find_and_pop(Int64 id)
{
	TrvlScriptRole* role = this->find_role(id);
	JUDGE_RETURN(role == NULL, role);

	role = this->player_map_->pop_object();
	role->id_ = id;

	this->player_map_->bind_object(id, role);
	return role;
}

TrvlScriptRole* TrvlScriptMonitor::find_role(Int64 role)
{
	return this->player_map_->find_object(role);
}

TrvlScriptTeam* TrvlScriptMonitor::find_team(TrvlScriptRole* trvl_role)
{
    PoolPackage<TrvlScriptTeam>* pack = this->fetch_package(
    		trvl_role->script_type_);
    JUDGE_RETURN(pack != NULL, NULL);

    return pack->find_object(trvl_role->team_index_);
}

int TrvlScriptMonitor::modify_team_operate(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400502*, request, -1);

	int recogn = 0;
	int type = request->type();

	switch(type)
	{
	default:
	case 0:
	{
		recogn = RETURN_TRVL_SCRIPT_CREATE_TEAM;
		break;
	}
	case 1:
	{
		recogn = RETRUN_TRVL_SCRIPT_ADD_TEAM;
		break;
	}
	}

	string main_version = request->main_version();
	if (main_version.empty() == false
			&& CONFIG_INSTANCE->is_same_main_version(main_version) == false)
	{
	    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
	    		recogn, ERROR_TRVL_SAME_VERSION);
	}

	switch(type)
	{
	case 0:
	{
		return this->create_team(sid, role, request);
	}
	case 1:
	{
		return this->add_team(sid, role, request);
	}
	}

	return -1;
}

int TrvlScriptMonitor::create_team(int sid, Int64 role, Proto30400502* request)
{
    int type = TrvlScriptMonitor::scene_to_type(request->scene_id());
    PoolPackage<TrvlScriptTeam>* pack = this->fetch_package(type);
    JUDGE_RETURN(pack != NULL, -1);

    this->quit_team_i(role);

    TrvlScriptTeam* team= pack->pop_object();
    team->script_type_ 	= type;
    team->scene_id_ 	= request->scene_id();
    team->limit_force_ 	= request->limit_force();
    team->sceret_ 		= request->sceret();
    team->auto_start_ 	= request->auto_start();
    team->push_teamer(role);

    this->init_ = true;
    this->add_team_i(team, sid, role, request);
    team->set_team_name();
    pack->bind_object(team->index(), team);
    this->fetch_team_info(sid, role, team);

    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
    		RETURN_TRVL_SCRIPT_CREATE_TEAM);
}

int TrvlScriptMonitor::add_team(int sid, Int64 role, Proto30400502* request)
{
    int type = TrvlScriptMonitor::scene_to_type(request->scene_id());
    PoolPackage<TrvlScriptTeam>* pack = this->fetch_package(type);
    JUDGE_RETURN(pack != NULL, -1);

    TrvlScriptTeam* team = pack->find_object(request->index());
    if (team == NULL)
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_TEAM_NO_EXIST);
    }

    if (this->player_map_->find_object(role) != NULL)
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_SELF_HAVE_TEAM);
    }

    if (team->is_full() == true)
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_TEAM_FULL);
    }

    if (team->is_in_team(role) == true)
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_CLIENT_OPERATE);
    }

    if (team->sceret_ != request->sceret())
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_TRVL_SCRIPT_SCERET);
    }

    if (team->start_fb_ == true)
    {
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
				RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_TRVL_SCRIPT_START);
    }

    team->push_teamer(role);
    this->add_team_i(team, sid, role, request);
    this->notify_all_team_info(team);

    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
    		RETRUN_TRVL_SCRIPT_ADD_TEAM);
}

int TrvlScriptMonitor::inner_team_operate(int sid, Int64 role, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400503*, request, -1);
	switch (request->type())
	{
	case 0:
	{
		//队伍列表
		return this->fetch_team_list(sid, role, request);
	}
	case 1:
	{
		//退出队伍
		return this->quit_team(sid, role, request);
	}
	case 2:
	{
		return this->fetch_team_info(sid, role, request);
	}
	case 3:
	{
		//开始组队副本
		return this->start_team_info(sid, role, request);
	}
	case 4:
	{
		//准备，取消操作
		return this->operate_team_info(sid, role, request);
	}
	case 5:
	{
		//踢出组队
		return this->kick_team(sid, role, request);
	}
	}

	return -1;
}

int TrvlScriptMonitor::fetch_team_list(int sid, Int64 role, Proto30400503* request)
{
	return this->fetch_team_list(sid, role, request->scene_id());
}

int TrvlScriptMonitor::quit_team(int sid, Int64 role, Proto30400503* request)
{
	int src_scene = this->quit_team_i(role);
	JUDGE_RETURN(src_scene > 0, 0);

    this->fetch_team_list(sid, role, src_scene);
    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
    		RETURN_TRVL_SCRIPT_QUIT_TEAM);
}

int TrvlScriptMonitor::fetch_team_info(int sid, Int64 role, Proto30400503* request)
{
	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	if (trvl_role == NULL)
	{
		return this->fetch_team_list(sid, role, request);
	}

	TrvlScriptTeam* team = this->find_team(trvl_role);
	if (team == NULL)
	{
		return this->fetch_team_list(sid, role, request);
	}

	{
		return this->fetch_team_info(sid, role, team);
	}
}

int TrvlScriptMonitor::start_team_info(int sid, Int64 role, Proto30400503* request)
{
	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	JUDGE_RETURN(trvl_role != NULL, -1);

	TrvlScriptTeam* team = this->find_team(trvl_role);
	JUDGE_RETURN(team != NULL && team->is_leader(role) == true, -1);

	int ret = this->check_start_team_script(team, false);
	JUDGE_RETURN(ret != 0, 0);

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
			RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_CLIENT_OPERATE);
}

int TrvlScriptMonitor::operate_team_info(int sid, Int64 role, Proto30400503* request)
{
	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	JUDGE_RETURN(trvl_role != NULL, -1);

	TrvlScriptTeam* team = this->find_team(trvl_role);
	JUDGE_RETURN(team != NULL, -1);

	trvl_role->prep_flag_ = request->index();
	this->notify_all_team_info(team);

	JUDGE_RETURN(trvl_role->prep_flag_ == true, 0);
	JUDGE_RETURN(team->auto_start_ == true, 0);

	this->check_start_team_script(team);
	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
			RETURN_TRVL_SCRIPT_PREV_OPER);
}

int TrvlScriptMonitor::kick_team(int sid, Int64 role, Proto30400503* request)
{
	JUDGE_RETURN(request->role() != role, -1);

	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	JUDGE_RETURN(trvl_role != NULL, -1);

	TrvlScriptTeam* team = this->find_team(trvl_role);
	JUDGE_RETURN(team != NULL, -1);
	JUDGE_RETURN(team->is_leader(role) == true, -1);

	team->erase_teamer(request->role());
	this->notify_all_team_info(team);
	this->fetch_team_list(request->role(), team->scene_id_);

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role,
			RETURN_TRVL_SCRIPT_KICK);
}

int TrvlScriptMonitor::fetch_team_info(int sid, Int64 role, TrvlScriptTeam* team)
{
	Proto50400815 respond;
	respond.set_index(team->index());
	respond.set_scene_id(team->scene_id_);
	respond.set_auto_start(team->auto_start_);
	respond.set_limit_force(team->limit_force_);
	respond.set_secret(team->sceret_.empty() == false);

	for (LongList::iterator iter = team->teamer_list_.begin();
			iter != team->teamer_list_.end(); ++iter)
	{
		TrvlScriptRole* trvl_role = this->player_map_->find_object(*iter);
		JUDGE_CONTINUE(trvl_role != NULL);

		ProtoTeamer* proto = respond.add_teamer_list();
		trvl_role->serialize(proto);
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

int TrvlScriptMonitor::fetch_team_list(Int64 role, int scene_id)
{
	int sid = this->fetch_sid(role);
	return this->fetch_team_list(sid, role, scene_id);
}

int TrvlScriptMonitor::fetch_team_list(int sid, Int64 role, int scene_id)
{
	JUDGE_RETURN(sid >= 0, -1);

    int type = TrvlScriptMonitor::scene_to_type(scene_id);
    PoolPackage<TrvlScriptTeam>* pack = this->fetch_package(type);
    JUDGE_RETURN(pack != NULL, -1);

    Proto50400812 respond;
    respond.set_scene_id(scene_id);

    BIntMap team_map = pack->fetch_index_map();
    for (BIntMap::iterator iter = team_map.begin(); iter != team_map.end(); ++iter)
    {
    	TrvlScriptTeam* team = pack->find_object(iter->first);
    	JUDGE_CONTINUE(team != NULL);
    	JUDGE_CONTINUE(team->start_fb_ == false);

    	ProtoTravelTeam* proto = respond.add_team_list();
    	team->make_up_team_list(proto);
    }

    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

int TrvlScriptMonitor::quit_team_i(Int64 role)
{
	TrvlScriptRole* trvl_role = this->player_map_->find_object(role);
	JUDGE_RETURN(trvl_role != NULL, -1);

    TrvlScriptTeam* team = this->find_team(trvl_role);
	this->player_map_->unbind_and_push(role, trvl_role);
	JUDGE_RETURN(team != NULL, -1);

    int scene_id = team->scene_id_;
    team->erase_teamer(role);

    if (team->is_empty() == true)
    {
    	//recycle
    	this->recycle_team(team);
    }
    else
    {
    	//notify
    	team->set_team_name();
    	this->notify_all_team_info(team);
    }

	return scene_id;
}

int TrvlScriptMonitor::check_start_team_script(TrvlScriptTeam* team, int all)
{
	int cur = 0;
	for (LongList::iterator iter = team->teamer_list_.begin();
			iter != team->teamer_list_.end(); ++iter)
	{
		TrvlScriptRole* trvl_role = this->player_map_->find_object(*iter);
		JUDGE_CONTINUE(trvl_role != NULL);

		cur += 1;
		JUDGE_CONTINUE(trvl_role->prep_flag_ == 0);

		return -1;
	}

	if (all == true)
	{
		JUDGE_RETURN(cur >= BaseTeam::MAX_COUNT, -1);
	}

	team->start_fb_ = true;

	Proto10400901 request;
	request.set_team_id(team->index());
	request.set_script_sort(team->scene_id_);

//	Proto31400211 ml_team_info;
//	ml_team_info.set_travel_team(true);
//	ml_team_info.set_team_index(team->index());
//	ml_team_info.set_leader_id(team->leader_id());

	Proto30400701 map_team_info;
	map_team_info.set_travel_team(true);
	map_team_info.set_team_index(team->index());
	map_team_info.set_leader_id(team->leader_id());

	for (LongList::iterator iter = team->teamer_list_.begin();
			iter != team->teamer_list_.end(); ++iter)
	{
//		ml_team_info.add_teamer_set(*iter);
		map_team_info.add_teamer_set(*iter);
		request.add_team_set(*iter);
	}

	for (LongList::iterator iter = team->teamer_list_.begin();
			iter != team->teamer_list_.end(); ++iter)
	{
		TrvlScriptRole* trvl_role = this->player_map_->find_object(*iter);
		JUDGE_CONTINUE(trvl_role != NULL);

		int sid = this->fetch_sid(trvl_role->id_);
		JUDGE_CONTINUE(sid >= 0);

//		MAP_MONITOR->dispatch_to_scene_by_gate(sid, trvl_role->id_, &ml_team_info);
		MAP_MONITOR->dispatch_to_scene_by_gate(sid, trvl_role->id_, &map_team_info);
		MAP_MONITOR->dispatch_to_scene_by_gate(sid, trvl_role->id_, &request);
	}

	return 0;
}

void TrvlScriptMonitor::notify_all_team_info(TrvlScriptTeam* team)
{
	JUDGE_RETURN(team != NULL, ;);

	for (LongList::iterator iter = team->teamer_list_.begin();
			iter != team->teamer_list_.end(); ++iter)
	{
		int sid = this->fetch_sid(*iter);
		JUDGE_CONTINUE(sid >= 0);
		this->fetch_team_info(sid, *iter, team);
	}
}

void TrvlScriptMonitor::recycle_team(TrvlScriptTeam* team)
{
	JUDGE_RETURN(team != NULL, ;);

    PoolPackage<TrvlScriptTeam>* pack = this->fetch_package(team->script_type_);
    JUDGE_RETURN(pack != NULL, ;);

	pack->unbind_and_push(team->index(), team);
}

void TrvlScriptMonitor::add_team_i(TrvlScriptTeam* team, int sid, Int64 role, Proto30400502* request)
{
    TrvlScriptRole* trvl_role = this->find_and_pop(role);
    trvl_role->team_index_ 	= team->index();
    trvl_role->script_type_ = team->script_type_;
    trvl_role->prep_flag_ 	= request->auto_start();

    trvl_role->sid_	= sid;
    trvl_role->BaseMember::unserialize(request->self_info());
    trvl_role->BaseServerInfo::unserialize(request->server_info());
}


