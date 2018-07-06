/*
 * SMBattleSystem.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: lijin
 */

#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "SMBattleSystem.h"
#include "SMBattleScene.h"

SMBattleSysInfo::SMBattleSysInfo(void)
{
	SMBattleSysInfo::reset();
}

void SMBattleSysInfo::reset(void)
{
	this->__activity_id = 0;
	this->__enter_level = 0;
}

SMBattleSystem::CheckStartTimer::CheckStartTimer(void)
{
	this->sm_system_ = NULL;
}

void SMBattleSystem::CheckStartTimer::init(SMBattleSystem* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->sm_system_ = parent;
}

int SMBattleSystem::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int SMBattleSystem::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->sm_system_ != NULL, -1);
	return this->sm_system_->handle_sm_battle_timeout();
}

SMBattleSystem::SMBattleSystem(void)
{
	// TODO Auto-generated constructor stub
	this->sm_scene_package_ = new PoolPackage<SMBattleScene>;
	this->player_map_ = new PoolPackage<SMBattlerInfo, Int64>;
	this->sm_check_timer_.init(this);

	SMBattleSystem::reset();
}

SMBattleSystem::~SMBattleSystem(void)
{
	// TODO Auto-generated destructor stub
}

void SMBattleSystem::reset(void)
{
	this->real_scene_ = 0;
	this->sm_check_timer_.cancel_timer();

	this->time_info_.reset();
	this->battle_info_.reset();
}

void SMBattleSystem::recycle_battle(SMBattleScene* scene)
{
	this->sm_scene_package_->unbind_and_push(scene->space_id(), scene);
}

int SMBattleSystem::client_scene()
{
	return GameEnum::SUN_MOON_BATTLE_ID;
}

int SMBattleSystem::real_scene()
{
	return this->real_scene_;
}

const Json::Value& SMBattleSystem::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->real_scene());
}

SMBattleSysInfo& SMBattleSystem::sm_battle_sys_info()
{
	return this->battle_info_;
}

void SMBattleSystem::start_sm_battle(int real_scene)
{
	this->real_scene_ = real_scene;
	const Json::Value& scene_conf = this->scene_conf();

	int activity_id = scene_conf["activity_id"].asInt();
	this->battle_info_.__activity_id = activity_id;
	this->battle_info_.__enter_level = scene_conf["enter_level"].asInt();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_, activity_conf);
    this->sm_check_timer_.schedule_timer(this->time_info_.refresh_time_);

    MSG_USER("SMBattleSystem %d %d", this->time_info_.time_index_, this->time_info_.refresh_time_);
    JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

    this->start_battle_event();
}

void SMBattleSystem::stop_sm_battle()
{

}

void SMBattleSystem::test_sm_battle(int id, int last)
{
	this->time_info_.cur_state_ = (id + 1) % this->time_info_.time_span_;
	this->time_info_.refresh_time_ = last;
	this->handle_sm_battle_i(id);
}

int SMBattleSystem::ahead_battle_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->battle_info_.__activity_id,
			this->time_info_.refresh_time_);
	MSG_USER("SMBattle ahead %d %d", this->time_info_.time_index_,
			this->time_info_.refresh_time_);
	return 0;
}

int SMBattleSystem::start_battle_event()
{
	this->start_sm_battle_scene();

	GameCommon::announce(this->scene_conf()["shout_start"].asInt());
	GameCommon::map_sync_activity_tips_start(this->battle_info_.__activity_id,
			this->time_info_.refresh_time_);

	MSG_USER("杀戮战场开始 %d", this->time_info_.refresh_time_);
	return 0;
}

int SMBattleSystem::stop_battle_event()
{
	const Json::Value& scene_conf = this->scene_conf();
	GameCommon::announce(scene_conf["shout_finish"].asInt());

	for (SMBattleSceneSet::iterator iter = this->sm_scene_set_.begin();
			iter != this->sm_scene_set_.end(); ++iter)
	{
		SMBattleScene* scene = *iter;
		scene->stop_battle();
	}

	SERIAL_RECORD->record_activity(SERIAL_ACT_SMBATTLE, this->player_map_->size());
	this->sm_scene_set_.clear();

	MSG_USER("杀戮战场结束");
	return 0;
}

int SMBattleSystem::sm_battle_left_time()
{
	return this->sm_check_timer_.left_second();
}

int SMBattleSystem::start_sm_battle_scene()
{
	JUDGE_RETURN(this->sm_scene_set_.empty() == true, -1);

	this->player_map_->clear();
	this->sm_scene_set_.clear();
	this->sm_scene_package_->clear();

	int space_count = this->scene_conf()["init_space"].asInt();
	for (int space_index = 0; space_index < space_count; ++space_index)
	{
		this->pop_battle_scene(space_index);
	}

	return 0;
}

int SMBattleSystem::handle_sm_battle_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_sm_battle_i(last_state);
}

int SMBattleSystem::handle_sm_battle_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_battle_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_battle_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		GameCommon::map_sync_activity_tips_stop(this->battle_info_.__activity_id,
				this->time_info_.fetch_after_end_left_time());
		this->stop_battle_event();
		break;
	}
	}

	this->sm_check_timer_.cancel_timer();
	this->sm_check_timer_.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("SMBattle %d, %d, %d", this->time_info_.time_index_,
			this->time_info_.cur_state_, this->time_info_.refresh_time_);
	return 0;
}

int SMBattleSystem::is_activity_time()
{
	switch (this->time_info_.cur_state_)
	{
	case GameEnum::ACTIVITY_STATE_AHEAD:
	case GameEnum::ACTIVITY_STATE_START:
	{
		return true;
	}
	}

	return false;
}

int SMBattleSystem::validate_player_level(int role_level)
{
	return role_level >= this->battle_info_.__enter_level;
}

int SMBattleSystem::request_enter_battle(int sid, Proto30400051* request)
{
	JUDGE_RETURN(this->is_activity_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(this->validate_player_level(request->role_level()) == true, ERROR_PLAYER_LEVEL);

	Int64 role_id = request->role_id();
	SMBattleScene* scene = this->fetch_battle_scene(role_id);
	JUDGE_RETURN(scene != NULL, ERROR_CLIENT_OPERATE);

	SMBattlerInfo* battler = this->find_battler(role_id);
	if (battler == NULL)
	{
		battler = this->player_map_->pop_object();
		this->player_map_->bind_object(role_id, battler);

		battler->id_ 			= role_id;
		battler->space_ 		= scene->space_id();
		battler->camp_index_  	= scene->fetch_camp_index();
	}

	battler->force_ = request->force();
	scene->register_player(battler);

	Proto30400052 enter_info;
	enter_info.set_space_id(scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_BATTLE_GROUND);

	const Json::Value& enter_point = this->scene_conf(
			)["enter_point"][battler->camp_index_];
	enter_info.set_pos_x(enter_point[0u].asInt());
	enter_info.set_pos_y(enter_point[1u].asInt());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

SMBattlerInfo* SMBattleSystem::find_battler(Int64 role_id)
{
	return this->player_map_->find_object(role_id);
}

SMBattleScene* SMBattleSystem::pop_battle_scene(int space_id)
{
	SMBattleScene* sm_scene = this->sm_scene_package_->pop_object();
	JUDGE_RETURN(sm_scene != NULL, NULL);

	sm_scene->init_sm_scene(this->real_scene_, space_id);
	sm_scene->run_scene();

	this->sm_scene_package_->bind_object(space_id, sm_scene);
	this->sm_scene_set_.push_back(sm_scene);

	return sm_scene;
}

SMBattleScene* SMBattleSystem::fetch_battle_scene(Int64 role_id)
{
	SMBattlerInfo* battler = this->find_battler(role_id);
	if (battler != NULL)
	{
		return this->sm_scene_package_->find_object(battler->space_);
	}

	for (SMBattleSceneSet::iterator iter = this->sm_scene_set_.begin();
			iter != this->sm_scene_set_.end(); ++iter)
	{
		SMBattleScene* scene = *iter;
		JUDGE_CONTINUE(scene->is_player_full() == false);
		return scene;
	}

	return this->pop_battle_scene(this->sm_scene_set_.size());
}

SMBattleScene* SMBattleSystem::find_battle_scene(int space_id)
{
	return this->sm_scene_package_->find_object(space_id);
}
