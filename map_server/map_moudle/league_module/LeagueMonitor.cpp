/*
 * LeagueMonitor.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#include "LeagueMonitor.h"
#include "MapPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "LeagueWarSystem.h"
#include "MMOLeague.h"
#include "LeagueBoss.h"
#include "LeagueReginFightSystem.h"

void MapLeagueInfo::reset()
{
	this->id_ = 0;
	this->force_ = 0;
	this->leader_id_ = 0;
	this->flag_lvl_ = 1;

	this->name_.clear();
	this->leader_.clear();
}

LeagueMonitor::BossTimer::BossTimer(void)
{
	this->league_monitor_ = NULL;
	this->activity_id_ = 0;
}

void LeagueMonitor::BossTimer::init(LeagueMonitor* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->league_monitor_ = parent;
}

void LeagueMonitor::BossTimer::set_activity_id(int activity_id)
{
	this->activity_id_ = activity_id;
}

int LeagueMonitor::BossTimer::activity_id()
{
	return this->activity_id_;
}

int LeagueMonitor::BossTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueMonitor::BossTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->league_monitor_ != NULL, -1);
	return this->league_monitor_->handle_boss_timeout();
}

int LeagueMonitor::LeagueSyncTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueMonitor::LeagueSyncTimer::handle_timeout(const Time_Value &tv)
{
	return 0;
}

LeagueMonitor::LeagueMonitor(void)
{
	this->init_flag_ = 0;
    this->sync_flag_ = 0;

    this->boss_timer_.cancel_timer();
    this->boss_timer_.init(this);
    this->boss_time_info_.reset();

    this->all_map_league_ = new PoolPackage<MapLeagueInfo, Int64>;
    this->boss_package_ = new PoolPackage<LeagueBoss, Int64>;
}

LeagueMonitor::~LeagueMonitor(void)
{
	// TODO Auto-generated destructor stub
}

void LeagueMonitor::init()
{
	this->init_flag_ = true;
	this->sync_flag_ = false;

	MSG_USER("LeagueMonitor::init");
}

void LeagueMonitor::fina()
{
	this->boss_timer_.cancel_timer();
}

void LeagueMonitor::star_league_boss()
{
	int activity_id = CONFIG_INSTANCE->league_boss("activity_id").asInt();
	this->boss_timer_.set_activity_id(activity_id);

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	GameCommon::cal_activity_info(this->boss_time_info_, activity_conf);
	this->boss_timer_.schedule_timer(this->boss_time_info_.refresh_time_);
	MSG_USER("league_boss %d %d", this->boss_time_info_.cur_state_, this->boss_time_info_.refresh_time_);
	JUDGE_RETURN(this->boss_time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_boss_event();
}

int LeagueMonitor::handle_boss_timeout()
{
	int last_state = this->boss_time_info_.cur_state_;
	this->boss_time_info_.set_next_stage();
	return this->handle_boss_i(last_state);
}

int LeagueMonitor::handle_boss_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_boss_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_boss_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_boss_event();
		break;
	}
	}

	this->boss_timer_.cancel_timer();
	this->boss_timer_.schedule_timer(this->boss_time_info_.refresh_time_);

	MSG_USER("league_boss %d, %d, %d", this->boss_time_info_.time_index_,
			this->boss_time_info_.cur_state_, this->boss_time_info_.refresh_time_);
	return 0;
}

int LeagueMonitor::ahead_boss_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->boss_timer_.activity_id(),
			this->boss_time_info_.refresh_time_);
	MSG_USER("league_boss ahead %d %d", this->boss_time_info_.time_index_,
			this->boss_time_info_.refresh_time_);

	return MAP_MONITOR->dispatch_to_logic(INNER_LOGIC_SYNC_LEAGUE_BOSS);
}

int LeagueMonitor::start_boss_event()
{
	GameCommon::map_sync_activity_tips_start(this->boss_timer_.activity_id(),
			this->boss_time_info_.refresh_time_);

	int shout_id = CONFIG_INSTANCE->league_boss("boss_start_shout").asInt();
	BLongMap index_map = this->boss_package_->fetch_index_map();
	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end();
			++iter)
	{
		LeagueBoss* boss = this->find_boss(iter->first);
		JUDGE_CONTINUE(boss != NULL);

		GameCommon::announce(iter->first, shout_id);
	}

	MSG_USER("league_boss start %d %d", this->boss_time_info_.time_index_,
			this->boss_time_info_.refresh_time_);
	return 0;
}

int LeagueMonitor::stop_boss_event()
{
	GameCommon::map_sync_activity_tips_stop(this->boss_timer_.activity_id());

	int shout_id = CONFIG_INSTANCE->league_boss("boss_end_shout").asInt();
	BLongMap index_map = this->boss_package_->fetch_index_map();
	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end();
			++iter)
	{
		LeagueBoss* boss = this->find_boss(iter->first);
		JUDGE_CONTINUE(boss != NULL);

		GameCommon::announce(iter->first, shout_id);
	}

	MSG_USER("league_boss end %d %d", this->boss_time_info_.time_index_,
			this->boss_time_info_.refresh_time_);
	return 0;
}

int LeagueMonitor::is_boss_time()
{
	switch (this->boss_time_info_.cur_state_)
	{
	case GameEnum::ACTIVITY_STATE_START:
	{
		return true;
	}
	}

	return false;
}

int LeagueMonitor::activity_left_tick()
{
	return this->boss_time_info_.fetch_left_time();
}


int LeagueMonitor::fetch_league_camp(Int64 league_id)
{
	if (this->league_camp_map_.count(league_id) == 0)
	{
		this->league_camp_map_[league_id] = MAP_MONITOR->generate_camp_id();
	}

	return this->league_camp_map_[league_id];
}

int LeagueMonitor::summon_league_boss(int gate_id, Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400451*, request, -1);

	LeagueBoss* league_boss = this->find_boss(request->league_index());
	JUDGE_RETURN(league_boss != NULL, -1);

	league_boss->summon_league_boss(msg);

	int shout_id = CONFIG_INSTANCE->league_boss("boss_summon_shout").asInt();
	GameCommon::announce(request->league_index(), shout_id);

	return MAP_MONITOR->dispatch_to_scene(gate_id, role_id, SCENE_LOGIC, msg);
}

int LeagueMonitor::fetch_league_flag(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400454*, request, -1);

	LeagueBoss* league_boss = this->find_boss(request->league_index());
	JUDGE_RETURN(league_boss != NULL, -1);

	league_boss->fetch_flag_level(msg);
	return 0;
}

int LeagueMonitor::recycle_league_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400452*, request, -1);

	LeagueBoss* league_boss = this->find_boss(request->league_index());
	JUDGE_RETURN(league_boss != NULL, -1);

	league_boss->recycle_all_monster();
	league_boss->notify_all_player_exit();
	this->boss_package_->unbind_and_push(request->league_index(), league_boss);
	return 0;
}

int LeagueMonitor::create_map_league(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400450*, request, -1);

	MapLeagueInfo* info = this->find_map_league(request->league_index());
	if (info == NULL)
	{
		info = this->all_map_league_->pop_object();
		info->id_ = request->league_index();
		info->leader_id_ = request->leader_index();
		this->all_map_league_->bind_object(info->id_, info);
	}

	info->name_ = request->name();
	info->leader_ = request->leader();
	info->force_ = request->force();
	info->flag_lvl_ = request->flag_lvl();

	if (info->leader_id_ == 0)
	{
		info->leader_id_ = MMOLeague::fetch_leader_id(info->id_);
	}

	return 0;
}

int LeagueMonitor::recycle_map_league(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400452*, request, -1);

	MapLeagueInfo* info = this->find_map_league(request->league_index());
	this->all_map_league_->unbind_and_push(request->league_index(), info);

	return 0;
}

int LeagueMonitor::update_league_leader(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400457*, request, -1);

	MapLeagueInfo* info = this->find_map_league(request->league_index());
	JUDGE_RETURN(info != NULL, -1);

	info->id_ = request->league_index();
	info->leader_id_ = request->leader_index();
	info->leader_ = request->leader();

	if (LEAGUE_WAR_SYSTEM->is_activity_time() == true)
	{

	}

	return 0;
}

int LeagueMonitor::create_league_boss_scene(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400450*, request, -1);

	JUDGE_RETURN(this->find_boss(request->league_index()) == NULL, -1);

	LeagueBoss* boss = this->boss_package_->pop_object();
	JUDGE_RETURN(boss != NULL, -1);

	boss->init_league_boss_scene(request->flag_lvl());
	boss->run_boss_scene();
	this->boss_package_->bind_object(request->league_index(), boss);

	return MAP_MONITOR->dispatch_to_logic(request);
}

int LeagueMonitor::request_enter_league_boss(int gate_id, Proto30400051* request)
{
	LeagueBoss* boss = this->find_boss(request->league_index());
	JUDGE_RETURN(boss != NULL, ERROR_LEAGUE_NO_EXIST);

	Proto30400052 enter_info;
	enter_info.set_space_id(boss->space_id());
	enter_info.set_scene_mode(SCENE_MODE_LEAGUE);

	MoverCoord enter_pos = boss->fetch_enter_pos();
	enter_info.set_pos_x(enter_pos.pixel_x());
	enter_info.set_pos_y(enter_pos.pixel_y());

	return MAP_MONITOR->respond_enter_scene_begin(gate_id, request, &enter_info);
}


LeagueBoss* LeagueMonitor::find_boss(Int64 league_index)
{
	return this->boss_package_->find_object(league_index);
}

MapLeagueInfo* LeagueMonitor::find_map_league(Int64 id)
{
	return this->all_map_league_->find_object(id);
}

int LeagueMonitor::handle_quit_league(int sid, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400445*, request, -1);

	Int64 role_id = request->role_id();
	if (this->is_in_league_activity() == true)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id,
				RETURN_LEAGUE_QUIT, ERROR_LEAGUE_ACTIVITY_TIME);
	}

	return MAP_MONITOR->dispatch_to_logic(msg, sid, role_id);
}

int LeagueMonitor::handle_rename_league(int sid, Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400455*, request, -1);

	if (this->is_in_league_activity() == true)
	{
		return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id,
				RETURN_RENAME_LEAGUE, ERROR_ACTIVITY_RENAME_LEAGUE);
	}

	Proto31400024 league_rename;
	league_rename.set_name(request->name());
	league_rename.set_index(request->index());
	return MAP_MONITOR->dispatch_to_logic(&league_rename, sid, role_id);
}

int LeagueMonitor::is_in_league_activity()
{
	JUDGE_RETURN(LEAGUE_WAR_SYSTEM->is_activity_time() == true
			|| LRF_MONITOR->is_activity_time() == true
			|| this->is_boss_time() == true, false);
	return true;
}

