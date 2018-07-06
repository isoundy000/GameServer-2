/*
 * TrvlTeamSystem.cpp
 *
 *  Created on: 2017年5月10日
 *      Author: lyw
 */

#include "TrvlTeamSystem.h"
#include "GameField.h"
#include "LogicStruct.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"
#include "ProtoDefine.h"
#include "MMOTravTeam.h"
#include "MongoDataMap.h"
#include "GameFont.h"
#include "MapMonitor.h"

#define MAX_TRAVEL_TEAMER   3

int TrvlTeamSystem::TeamTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int TrvlTeamSystem::TeamTimer::handle_timeout(const Time_Value &tv)
{
	TRAVEL_TEAM_SYS->handle_time_out_team();
	TRAVEL_TEAM_SYS->process_fetch_trvl_peak_activity_tick();
	return 0;
}

TrvlTeamSystem::TrvlTeamSystem() {
	// TODO Auto-generated constructor stub
	this->travel_team_map_ = new TravelTeamMap();
	this->role_travel_team_map_ = new TravelTeamMap();
	this->travel_team_info_pool_ = new TravelTeamInfoPool();
}

TrvlTeamSystem::~TrvlTeamSystem() {
	// TODO Auto-generated destructor stub
	SAFE_DELETE(this->travel_team_map_);
	SAFE_DELETE(this->role_travel_team_map_);
	SAFE_DELETE(this->travel_team_info_pool_);
}

int TrvlTeamSystem::start(void)
{
	MMOTravTeam::load_local_travel_team(this);

	this->sort_travel_team();

	this->team_timer_.schedule_timer(Time_Value::HOUR);

	this->signup_start_tick_ = Time_Value::zero;
	this->signup_end_tick_ = Time_Value::zero;
	this->quality_start_tick_ = Time_Value::zero;
	this->quality_end_tick_ = Time_Value::zero;
	this->knockout_start_tick_ = Time_Value::zero;
	this->knockout_end_tick_ = Time_Value::zero;
	this->award_start_tick_ = Time_Value::zero;
	this->award_end_tick_ = Time_Value::zero;

	this->process_fetch_trvl_peak_activity_tick();

	return 0;
}

int TrvlTeamSystem::stop(void)
{
	this->save_update_travel_team();
	this->team_timer_.cancel_timer();
	return 0;
}

int TrvlTeamSystem::bind_travel_team(const Int64 team_id, TravelTeamInfo *team_info)
{
	JUDGE_RETURN(team_info != NULL, -1);

	int ret = this->travel_team_map_->bind(team_id, team_info);
	JUDGE_RETURN(ret == 0, ret);

	this->role_travel_team_map_->rebind(team_info->__leader_id, team_info);
	for (TravelTeamInfo::TeamerMap::iterator iter = team_info->__teamer_map.begin();
	        iter != team_info->__teamer_map.end(); ++iter)
	{
	    this->role_travel_team_map_->rebind(iter->first, team_info);
	}
	return 0;
}

int TrvlTeamSystem::unbind_travel_team(const Int64 team_id)
{
	return this->travel_team_map_->unbind(team_id);
}

int TrvlTeamSystem::unbind_and_recycle_travel_team(TravelTeamInfo *team_info)
{
	JUDGE_RETURN(team_info != NULL, -1);

	this->unbind_travel_team(team_info->__team_id);
	this->travel_team_info_pool_->push(team_info);
	return 0;
}

TravelTeamInfo *TrvlTeamSystem::find_travel_team(const Int64 team_id)
{
	TravelTeamInfo *team_info = NULL;
	if (this->travel_team_map_->find(team_id, team_info) == 0)
		return team_info;
	return NULL;
}

TravelTeamInfo *TrvlTeamSystem::find_travel_team_by_role_id(const Int64 role_id)
{
    TravelTeamInfo *team_info = NULL;
    if (this->role_travel_team_map_->find(role_id, team_info) == 0)
        return team_info;
    return NULL;
}

void TrvlTeamSystem::refresh_team_signup(TravelTeamInfo *trvl_team)
{
	if (this->knockout_start_tick_ == Time_Value::zero)
	{
		trvl_team->__is_signup = 0;
	    return;
	}

	if (this->knockout_start_tick_ == trvl_team->__refresh_signup_tick)
	    return ;

	trvl_team->__is_signup = 0;
	trvl_team->__refresh_signup_tick = this->knockout_start_tick_;
}

void TrvlTeamSystem::fetch_team_list(LongVec& team_vec, const int show_no_full)
{
	team_vec.reserve(this->team_sort_list_.size());

	if (show_no_full == true)
	{
		for (PairObjVec::iterator iter = this->team_sort_list_.begin();
				iter != this->team_sort_list_.end(); ++iter)
		{
			TravelTeamInfo *trvl_team = this->find_travel_team(iter->id_);
			JUDGE_CONTINUE(trvl_team != NULL);
			JUDGE_CONTINUE(this->is_travel_team_full(trvl_team) == false);

			team_vec.push_back(iter->id_);
		}
	}
	else
	{
		for (PairObjVec::iterator iter = this->team_sort_list_.begin();
				iter != this->team_sort_list_.end(); ++iter)
		{
			team_vec.push_back(iter->id_);
		}
	}
}

bool TrvlTeamSystem::is_travel_team_full(TravelTeamInfo *trvl_team)
{
	return (int(trvl_team->__teamer_map.size()) >= MAX_TRAVEL_TEAMER);
}

bool TrvlTeamSystem::is_during_quality_and_knockout(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	return nowtime >= this->quality_start_tick_ && nowtime < this->knockout_end_tick_;
}

bool TrvlTeamSystem::is_during_signup(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	return nowtime >= this->signup_start_tick_ && nowtime < this->signup_end_tick_;
}

bool TrvlTeamSystem::is_during_quality(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	return nowtime >= this->quality_start_tick_ && nowtime < this->quality_end_tick_;
}

bool TrvlTeamSystem::is_during_knockout(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    return nowtime >= this->knockout_start_tick_ && nowtime < this->knockout_end_tick_;
}

bool TrvlTeamSystem::is_during_knockout_bet(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	return nowtime >= this->quality_end_tick_ && nowtime < this->knockout_start_tick_;
}

bool TrvlTeamSystem::is_has_travel_team_name(const string &team_name)
{
	return this->travel_team_name_set_.find(team_name) != this->travel_team_name_set_.end();
}

TravelTeamInfo *TrvlTeamSystem::create_travel_team(void)
{
	TravelTeamInfo *team_info = this->travel_team_info_pool_->pop();
	int64_t team_id = 0;
	LOGIC_MONITOR->fetch_global_value(Global::TRAVEL_TEAM, team_id);
	team_info->__team_id = team_id;
	return team_info;
}

int TrvlTeamSystem::create_travel_team_and_bind(TrvlTeamPlayer *leader, int need_force, string &team_name)
{
	JUDGE_RETURN(leader != NULL, ERROR_SERVER_INNER);
	JUDGE_RETURN(this->is_has_travel_team_name(team_name) == false, ERROR_TRAVEL_TEAM_NAME_REPEAT);

	TravelTeamInfo *travel_team = this->create_travel_team();
	this->init_travel_team(travel_team, need_force, team_name, leader->role_id());

	if (this->bind_travel_team(travel_team->__team_id, travel_team) != 0)
	{
	    this->travel_team_info_pool_->push(travel_team);
	    return ERROR_SERVER_INNER;
	}

	this->travel_team_name_set_.insert(travel_team->__team_name);

	this->sort_travel_team();

	return 0;
}

int TrvlTeamSystem::dimiss_travel_team(TravelTeamInfo *team_info)
{
	JUDGE_RETURN(team_info != NULL, -1);

	this->role_travel_team_map_->unbind(team_info->__leader_id);
	for (TravelTeamInfo::TeamerMap::iterator iter = team_info->__teamer_map.begin();
	        iter != team_info->__teamer_map.end(); ++iter)
	{
	    this->role_travel_team_map_->unbind(iter->first);

//	    MailInformation *mail_info = GameCommon::create_sys_mail(FONT_TRAVEL_TEAM_DIMISS);
//	    GameCommon::request_save_mail(iter->first, mail_info);
	}

	MMOTravTeam::remove_local_travel_team(team_info->__team_id);

	this->travel_team_name_set_.erase(team_info->__team_name);

	return this->unbind_and_recycle_travel_team(team_info);
}

bool TrvlTeamSystem::is_in_travel_team(TravelTeamInfo *team_info, const Int64 teamer_id)
{
	JUDGE_RETURN(team_info != NULL, false);

	TravelTeamInfo::TeamerMap::iterator iter = team_info->__teamer_map.find(teamer_id);
	if (iter != team_info->__teamer_map.end())
	    return true;
	return false;
}

int TrvlTeamSystem::insert_travel_teamer(TravelTeamInfo *team_info, TravelTeamInfo::TravelTeamer &teamer)
{
	JUDGE_RETURN(this->find_travel_team_by_role_id(teamer.__teamer_id) == NULL, ERROR_TARGET_HAS_TRAVEL_TEAM);
	JUDGE_RETURN(this->is_in_travel_team(team_info, teamer.__teamer_id) == false, ERROR_PLAYER_IN_TRAVEL_TEAM);
	JUDGE_RETURN(team_info->__teamer_map.size() < MAX_TRAVEL_TEAMER, ERROR_TRAVEL_TEAM_FULL);

	TravelTeamInfo::TravelTeamer &teamer_info = team_info->__teamer_map[teamer.__teamer_id];
	this->copy_to_travel_teamer(teamer_info, teamer);
	teamer_info.__join_tick   = Time_Value::gettimeofday();
	teamer_info.__logout_tick = Time_Value::gettimeofday();

	this->role_travel_team_map_->rebind(teamer_info.__teamer_id, team_info);

	if (team_info->__teamer_map.size() >= MAX_TRAVEL_TEAMER)
	{
	    team_info->__apply_map.clear();
	}

	return 0;
}

int TrvlTeamSystem::remove_travel_teamer(TravelTeamInfo *team_info, const Int64 teamer_id)
{
	this->role_travel_team_map_->unbind(teamer_id);

	TravelTeamInfo::TeamerMap::iterator iter = team_info->__teamer_map.find(teamer_id);
	JUDGE_RETURN(iter != team_info->__teamer_map.end(), -1);

	team_info->__teamer_map.erase(iter);
	return 0;
}

void TrvlTeamSystem::init_travel_team(TravelTeamInfo *team_info, int need_force, string &team_name, Int64 leader_id)
{
	team_info->__need_force = need_force;
	team_info->__team_name = team_name;
	team_info->__leader_id = leader_id;
	team_info->__create_tick = Time_Value::gettimeofday();

	TravelTeamInfo::TravelTeamer &teamer_info = team_info->__teamer_map[leader_id];
	LogicPlayer *player = NULL;
	if (LOGIC_MONITOR->find_player(leader_id, player) == 0)
	{
	    this->copy_to_travel_teamer(teamer_info, player);
	    teamer_info.__join_tick   = Time_Value::gettimeofday();
	    teamer_info.__logout_tick = Time_Value::gettimeofday();
	}
}

void TrvlTeamSystem::copy_to_travel_teamer(TravelTeamInfo::TravelTeamer &teamer, BaseLogicPlayer *player)
{
	teamer.__teamer_id = player->role_id();
	teamer.__teamer_name = player->role_detail().__name;
	teamer.__teamer_sex = player->role_detail().__sex;
	teamer.__teamer_career = player->role_detail().__career;
	teamer.__teamer_level = player->role_level();
	teamer.__teamer_force = player->role_detail().__fight_force;
}

void TrvlTeamSystem::copy_to_travel_teamer(TravelTeamInfo::TravelTeamer &teamer, TravelTeamInfo::TravelTeamer &applier)
{
	teamer.__teamer_id = applier.__teamer_id;
	teamer.__teamer_name = applier.__teamer_name;
	teamer.__teamer_sex = applier.__teamer_sex;
	teamer.__teamer_career = applier.__teamer_career;
	teamer.__teamer_level = applier.__teamer_level;
	teamer.__teamer_force = applier.__teamer_force;
}

int TrvlTeamSystem::insert_apply_set(TravelTeamInfo *team_info, BaseLogicPlayer *player)
{
	JUDGE_RETURN(team_info != NULL && player != NULL, -1);
	JUDGE_RETURN(this->find_travel_team_by_role_id(player->role_id()) == NULL, ERROR_TARGET_HAS_TRAVEL_TEAM);

	TravelTeamInfo::TravelTeamer &applier_info = team_info->__apply_map[player->role_id()];
	this->copy_to_travel_teamer(applier_info, player);

	return 0;
}

int TrvlTeamSystem::remove_apply_set(TravelTeamInfo *team_info, const Int64 apply_id)
{
	JUDGE_RETURN(team_info != NULL, -1);

	team_info->__apply_map.erase(apply_id);
	return 0;
}

int TrvlTeamSystem::apply_role_insert_travel_team(TravelTeamInfo *team_info, const Int64 apply_id)
{
	JUDGE_RETURN(team_info != NULL, -1);

	if (this->find_travel_team_by_role_id(apply_id) != NULL)
	{
	    this->remove_apply_set(team_info, apply_id);
	    return ERROR_TARGET_HAS_TRAVEL_TEAM;
	}
	JUDGE_RETURN(team_info->__apply_map.count(apply_id) > 0, ERROR_CLIENT_OPERATE);

	int ret = this->insert_travel_teamer(team_info, team_info->__apply_map[apply_id]);
	JUDGE_RETURN(ret == 0, ret);

	for (TravelTeamMap::iterator iter = this->travel_team_map_->begin();
			iter != this->travel_team_map_->end(); ++iter)
	{
		TravelTeamInfo *travel_team = iter->second;
		this->remove_apply_set(travel_team, apply_id);
	}

	return 0;
}

int TrvlTeamSystem::notify_all_teamer(TravelTeamInfo *travel_team, const int recogn, Message *msg)
{
	LogicPlayer *player = NULL;
	for (TravelTeamInfo::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
	        teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
	{
	    TravelTeamInfo::TravelTeamer &teamer_info = teamer_iter->second;
	    if (LOGIC_MONITOR->find_player(teamer_info.__teamer_id, player) == 0)
	        player->respond_to_client(recogn, msg);
	}
	return 0;
}

Time_Value &TrvlTeamSystem::travel_peak_quality_start_tick(void)
{
	return this->quality_start_tick_;
}

Time_Value &TrvlTeamSystem::travel_peak_knockout_start_tick(void)
{
	return this->knockout_start_tick_;
}

Time_Value &TrvlTeamSystem::travel_peak_signup_start_tick(void)
{
    return this->signup_start_tick_;
}

Time_Value &TrvlTeamSystem::travel_peak_signup_end_tick(void)
{
    return this->signup_end_tick_;
}

int TrvlTeamSystem::travel_peak_left_sign_sec(void)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	if (this->signup_start_tick_ <= nowtime && nowtime < this->signup_end_tick_)
	{
		int left_sec = this->signup_end_tick_.sec() - nowtime.sec();
		if (left_sec < 0)
			left_sec = 0;
		return left_sec;
	}
	return 0;
}

int TrvlTeamSystem::save_update_travel_team(void)
{
	for (TravelTeamMap::iterator iter = this->travel_team_map_->begin();
			iter != this->travel_team_map_->end(); ++iter)
	{
	    Int64 team_id = iter->first;
	    TravelTeamInfo *travel_team = iter->second;
	    JUDGE_CONTINUE(travel_team != NULL);

	    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();

	    MMOTravTeam::update_local_travel_team_data(travel_team, data_map);

	    TRANSACTION_MONITOR->request_mongo_transaction(team_id, TRANS_SAVE_LOCAL_TRAV_TEAM, data_map);
	}

	return 0;
}

int TrvlTeamSystem::set_update_teamer_info(Int64 team_id, Int64 role_id)
{
	JUDGE_RETURN(team_id > 0 && role_id > 0, 0);

	this->update_teamer_map_[role_id] = team_id;
	return 0;
}

void TrvlTeamSystem::sort_travel_team(void)
{
	this->team_sort_list_.clear();

	for (TravelTeamMap::iterator iter = this->travel_team_map_->begin();
			iter != this->travel_team_map_->end(); ++iter)
	{
	    TravelTeamInfo *travel_team = iter->second;
	    JUDGE_CONTINUE(travel_team != NULL);

	    PairObj obj;
	    obj.id_ = travel_team->__team_id;
	    obj.value_ = travel_team->__create_tick.sec();
	    this->team_sort_list_.push_back(obj);
	}

	std::sort(this->team_sort_list_.begin(), this->team_sort_list_.end(), GameCommon::pair_comp_by_desc);
}

void TrvlTeamSystem::handle_time_out_team()
{
	JUDGE_RETURN(this->is_during_quality_and_knockout() == false, ;);

	LongSet dimiss_set, change_set;

	for (TravelTeamMap::iterator iter = this->travel_team_map_->begin();
			iter != this->travel_team_map_->end(); ++iter)
	{
		TravelTeamInfo *travel_team = iter->second;
		JUDGE_CONTINUE(travel_team != NULL);

		if (travel_team->team_is_pass_miss_day())
		{
			dimiss_set.insert(iter->first);
		}
		else if (travel_team->member_is_pass_miss_day(travel_team->__leader_id))
		{
			change_set.insert(iter->first);
		}
	}

	for (LongSet::iterator iter = dimiss_set.begin(); iter != dimiss_set.end(); ++iter)
	{
		TravelTeamInfo *travel_team = this->find_travel_team(*iter);
		JUDGE_CONTINUE(travel_team != NULL);

		this->sync_trvl_team_to_trvl_peak_scene(TrvlTeamPlayer::SYNC_OP_TYPE_DEL, travel_team->__team_id);
		this->dimiss_travel_team(travel_team);
	}

	for (LongSet::iterator iter = change_set.begin(); iter != change_set.end(); ++iter)
	{
		TravelTeamInfo *travel_team = this->find_travel_team(*iter);
		JUDGE_CONTINUE(travel_team != NULL);

		PairObjVec teamer_vec;
		for (TravelTeamInfo::TeamerMap::iterator it = travel_team->__teamer_map.begin();
				it != travel_team->__teamer_map.end(); ++it)
		{
			JUDGE_CONTINUE(it->first != travel_team->__leader_id);
			JUDGE_CONTINUE(travel_team->member_is_pass_miss_day(it->first) == false)

			PairObj obj(it->first, it->second.__teamer_force);
			teamer_vec.push_back(obj);
		}
		JUDGE_CONTINUE(teamer_vec.size() > 0);

		std::sort(teamer_vec.begin(), teamer_vec.end(), GameCommon::pair_comp_by_desc);

		travel_team->__leader_id = teamer_vec[0].id_;
		this->notify_all_teamer(travel_team, ACTIVE_TRAVPEAK_TEAMER_VARY);

		this->sync_trvl_team_to_trvl_peak_scene(TrvlTeamPlayer::SYNC_OP_TYPE_UPDATE, travel_team->__team_id);
	}
}

void TrvlTeamSystem::update_teamer_force()
{
	JUDGE_RETURN(this->update_teamer_map_.size() > 0, ;);

	Proto30402014 inner_req;

	for (LongMap::iterator iter = this->update_teamer_map_.begin();
			iter != this->update_teamer_map_.end(); ++iter)
	{
		Int64 role_id = iter->first;
		Int64 team_id = iter->second;
		TravelTeamInfo *travel_team = this->find_travel_team(team_id);
		JUDGE_CONTINUE(travel_team != NULL);

		TravelTeamInfo::TravelTeamer *trvl_teamer = travel_team->trvl_teamer(role_id);
		JUDGE_CONTINUE(trvl_teamer != NULL);

		ProtoTeamForceInfo *force_info = inner_req.add_force_info();
		force_info->set_team_id(team_id);
		force_info->set_role_id(role_id);
		force_info->set_force(trvl_teamer->__teamer_force);
		force_info->set_level(trvl_teamer->__teamer_level);
	}

	this->update_teamer_map_.clear();

	LOGIC_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
}

int TrvlTeamSystem::sync_trvl_team_to_trvl_peak_scene(int op_type, Int64 team_id, int gate_sid, TrvlTeamPlayer *player)
{
	JUDGE_RETURN(team_id > 0, 0);

	Proto30402001 inner_req;
	inner_req.set_team_id(team_id);
	inner_req.set_op_type(op_type);

	TravelTeamInfo *travel_team = this->find_travel_team(team_id);
	if (travel_team != NULL)
	{
		inner_req.set_team_id(travel_team->__team_id);
		inner_req.set_op_type(op_type);
		inner_req.set_team_name(travel_team->__team_name);
		inner_req.set_need_force(travel_team->__need_force);
		inner_req.set_leader_id(travel_team->__leader_id);

	    for (TravelTeamInfo::TeamerMap::iterator teamer_iter = travel_team->__teamer_map.begin();
				teamer_iter != travel_team->__teamer_map.end(); ++teamer_iter)
		{
			TravelTeamInfo::TravelTeamer &teamer_info = teamer_iter->second;
			ProtoTeamer *proto_teamer = inner_req.add_teamer_list();
			teamer_info.serialize(proto_teamer);
		}
	}

	ProtoServer* server = inner_req.mutable_server_info();
	if (player != NULL)
	{
		player->role_detail().serialize(server, true);
	}

	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
}

int TrvlTeamSystem::process_fetch_trvl_peak_activity_tick()
{
	Proto30402009 inner_req;
	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_PRE_SCENE_ID, &inner_req);
}

int TrvlTeamSystem::process_sync_trvl_peak_activity_tick(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30402010 *, request, msg, -1);

	Time_Value nowtime = Time_Value::gettimeofday();

	this->signup_start_tick_.sec(request->signup_start());
	this->signup_end_tick_.sec(request->signup_end());
	this->quality_start_tick_.sec(request->quality_start());
	this->quality_end_tick_.sec(request->quality_end());

	this->knockout_start_tick_.sec(request->knockout_start());
	this->knockout_end_tick_.sec(request->knockout_end());

	return 0;
}

int TrvlTeamSystem::update_trvl_team_signup_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101909*, request, -1);

	Int64 team_id = request->team_id();
	TravelTeamInfo *travel_team = this->find_travel_team(team_id);
	JUDGE_RETURN(travel_team != NULL, 0);

	travel_team->__is_signup = request->is_signup();
	return 0;
}





