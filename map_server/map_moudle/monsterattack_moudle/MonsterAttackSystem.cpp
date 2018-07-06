/*
 * MonsterAttackSystem.cpp
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#include "MonsterAttackSystem.h"
#include "MonsterAttackScene.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "MMOWorldBoss.h"

MonsterAttackSysInfo::MonsterAttackSysInfo(void)
{
	this->reset();
}

void MonsterAttackSysInfo::reset(void)
{
	this->__activity_id = 0;
	this->__enter_level = 0;
}

MonsterAttackSystem::CheckStartTimer::CheckStartTimer(void)
{
	this->mattack_system_ = NULL;
}

void MonsterAttackSystem::CheckStartTimer::init(MonsterAttackSystem* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->mattack_system_ = parent;
}

int MonsterAttackSystem::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int MonsterAttackSystem::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->mattack_system_ != NULL, -1);
	return this->mattack_system_->handle_mattack_timeout();
}

MonsterAttackSystem::MonsterAttackSystem() {
	// TODO Auto-generated constructor stub
	this->mattack_scene_package_ = new PoolPackage<MonsterAttackScene>;
	this->player_map_ = new PoolPackage<MAttackInfo, Int64>;
	this->mattack_scene_ = NULL;
	this->mattack_check_timer_.init(this);

	MonsterAttackSystem::reset();
}

MonsterAttackSystem::~MonsterAttackSystem() {
	// TODO Auto-generated destructor stub
}

int MonsterAttackSystem::real_scene()
{
	return this->real_scene_;
}

const Json::Value& MonsterAttackSystem::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->real_scene());
}

void MonsterAttackSystem::reset(void)
{
	this->real_scene_ = 0;

	this->mattack_check_timer_.cancel_timer();
	this->time_info_.reset();
	this->mattack_detail_.reset();
}

void MonsterAttackSystem::recycle_mattack(MonsterAttackScene* scene)
{
	this->mattack_scene_package_->unbind_and_push(scene->space_id(), scene);
}

void MonsterAttackSystem::start_monster_attack(int real_scene)
{
	this->real_scene_ = real_scene;
	const Json::Value& scene_conf = this->scene_conf();
	JUDGE_RETURN(scene_conf != Json::Value::null, ;);

	int activity_id = scene_conf["activity_id"].asInt();
	int enter_level = scene_conf["enter_level"].asInt();
	this->mattack_detail_.__activity_id = activity_id;
	this->mattack_detail_.__enter_level = enter_level;

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(activity_id);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

	const Json::Value& buff_wave = scene_conf["buff_wave"];
	for (uint i = 0; i < buff_wave.size(); ++i)
	{
		int label_id = buff_wave[i][2u].asInt();
		MAttackLabelRecord &label_record = this->label_record_map_[label_id];
		label_record.label_id_ = label_id;

		MMOWorldBoss::load_mattack_info(&label_record);
	}

	GameCommon::cal_activity_info(this->time_info_, activity_conf);
	this->mattack_check_timer_.schedule_timer(this->time_info_.refresh_time_);
	MSG_USER("monster_attack %d %d", this->time_info_.cur_state_, this->time_info_.refresh_time_);
	JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

	this->start_mattack_event();
}

void MonsterAttackSystem::stop_monster_attack()
{
	for (LabelRecordMap::iterator iter = this->label_record_map_.begin();
			iter != this->label_record_map_.end(); ++iter)
	{
		MAttackLabelRecord &label_record = iter->second;
		MMOWorldBoss::update_mattack_info(&label_record);
	}
}

int MonsterAttackSystem::handle_mattack_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_mattack_i(last_state);
}

int MonsterAttackSystem::handle_mattack_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_mattack_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_mattack_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_mattack_event();
		break;
	}
	}

	this->mattack_check_timer_.cancel_timer();
	this->mattack_check_timer_.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("monster attack %d, %d, %d", this->time_info_.time_index_,
			this->time_info_.cur_state_, this->time_info_.refresh_time_);
	return 0;
}

int MonsterAttackSystem::ahead_mattack_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->mattack_detail_.__activity_id,
			this->time_info_.refresh_time_);
	MSG_USER("monster attack ahead %d %d", this->time_info_.time_index_,
			this->time_info_.refresh_time_);

	return 0;
}

int MonsterAttackSystem::start_mattack_event()
{
	this->start_mattack_scene();

	GameCommon::announce(this->scene_conf()["shout_start"].asInt());
	GameCommon::map_sync_activity_tips_start(this->mattack_detail_.__activity_id,
			this->time_info_.refresh_time_);

	MSG_USER("怪物攻城开始");
	return 0;
}

int MonsterAttackSystem::stop_mattack_event()
{
	GameCommon::map_sync_activity_tips_stop(this->mattack_detail_.__activity_id);

	const Json::Value& scene_conf = this->scene_conf();
	GameCommon::announce(scene_conf["shout_finish"].asInt());

	MonsterAttackScene *mattack_scene = this->mattack_scene_;
	mattack_scene->stop_mattack();

	this->mattack_scene_ = NULL;

	MSG_USER("怪物攻城结束");
	return 0;
}

int MonsterAttackSystem::start_mattack_scene()
{
	JUDGE_RETURN(this->mattack_scene_ == NULL, -1);

	this->player_map_->clear();
	this->mattack_scene_package_->clear();

	MonsterAttackScene *mattack_scene = this->mattack_scene_package_->pop_object();
	JUDGE_RETURN(mattack_scene != NULL, -1);

	int space_id = 0;
	mattack_scene->init_mattack_scene(this->real_scene_, space_id);
	mattack_scene->run_scene();

	this->mattack_scene_package_->bind_object(space_id, mattack_scene);
	this->mattack_scene_ = mattack_scene;

	return 0;
}

void MonsterAttackSystem::test_mattack(int id, int set_time)
{
	this->time_info_.cur_state_ = (id + 1) % this->time_info_.time_span_;
	this->time_info_.refresh_time_ = set_time;
	this->handle_mattack_i(id);
}

int MonsterAttackSystem::mattack_left_time()
{
	return this->mattack_check_timer_.left_second();
}

int MonsterAttackSystem::is_activity_time()
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

int MonsterAttackSystem::validate_player_level(int role_level)
{
	return role_level >= this->mattack_detail_.__enter_level;
}

int MonsterAttackSystem::request_enter_mattack(int sid, Proto30400051* request)
{
	JUDGE_RETURN(this->is_activity_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(this->validate_player_level(request->role_level()) == true, ERROR_PLAYER_LEVEL);

	Int64 role_id = request->role_id();
	MonsterAttackScene *mattack_scene = this->mattack_scene_;
	JUDGE_RETURN(mattack_scene != NULL, ERROR_CLIENT_OPERATE);

	MAttackInfo* mattack = this->find_mattack(role_id);
	if (mattack == NULL)
	{
		mattack = this->player_map_->pop_object();
		this->player_map_->bind_object(role_id, mattack);
		mattack->id_ = role_id;
	}
	mattack->camp_index_ = mattack_scene->fetch_player_camp();

	mattack_scene->register_player(mattack);

	Proto30400052 enter_info;
	enter_info.set_space_id(mattack_scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_MONSTER_ATTACK);

	const Json::Value& enter_point = this->scene_conf()["enter_point"];
	enter_info.set_pos_x(enter_point[0u].asInt());
	enter_info.set_pos_y(enter_point[1u].asInt());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int MonsterAttackSystem::update_label_record(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30402301*, request, -1);

	int label_id = request->label_id();

	const Json::Value& scene_conf = this->scene_conf();
	JUDGE_RETURN(scene_conf != Json::Value::null, -1);

	int is_label = false;
	const Json::Value& buff_wave = scene_conf["buff_wave"];
	for (uint i = 0; i < buff_wave.size(); ++i)
	{
		int label = buff_wave[i][2u].asInt();
		JUDGE_CONTINUE(label_id == label);

		is_label = true;
		break;
	}
	JUDGE_RETURN(is_label == true, -1);

	const ProtoRoleInfo& role_info = request->role_info();
	MAttackLabelRecord &label_record = this->label_record_map_[label_id];
	label_record.label_id_ = label_id;
	label_record.role_id_ = role_info.role_id();
	label_record.role_name_ = role_info.role_name();

#ifdef LOCAL_DEBUG
	MMOWorldBoss::update_mattack_info(&label_record);
#endif

	return 0;
}

MAttackInfo* MonsterAttackSystem::find_mattack(Int64 role_id)
{
	return this->player_map_->find_object(role_id);
}

MonsterAttackScene* MonsterAttackSystem::find_mattack_scene(int space_id)
{
	return this->mattack_scene_package_->find_object(space_id);
}

MAttackLabelRecord* MonsterAttackSystem::find_label_record(int label_id)
{
	JUDGE_RETURN(this->label_record_map_.count(label_id) > 0, NULL);

	return &this->label_record_map_[label_id];
}



