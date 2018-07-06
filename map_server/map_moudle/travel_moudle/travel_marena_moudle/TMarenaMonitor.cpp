/*
 * TMarenaMonitor.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#include "TMarenaMonitor.h"
#include "MapMonitor.h"
#include "TMArenaPrep.h"
#include "TMArenaScene.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"

TMArenaRole::TMArenaRole()
{
	TMArenaRole::reset();
}

void TMArenaRole::reset()
{
	BaseServerInfo::reset();
	BaseMember::reset();
	this->reset_everyday();
}

void TMArenaRole::reset_everyday()
{
	this->init_ = false;
	this->is_mvp_ = false;

	this->sid_ = 0;
	this->rank_ = 0;
	this->score_ = 0;
	this->die_times_ = 0;
	this->kill_times_ = 0;

	this->win_times_ = 0;
	this->attend_times_ = 0;
	this->start_tick_ = 0;
	this->cur_score_ = 0;
	this->drop_reward_ = 0;
	this->update_tick_ = 0;
	this->fight_state_ = 0;
}

void TMArenaRole::serialize(ProtoLMRole* proto)
{
	proto->set_role_name(this->name_);
	proto->set_sex(this->sex_);
	proto->set_role_force(this->force_);
	proto->set_rank_index(this->kill_times_);
	proto->set_level(this->die_times_);
}

void TMArenaRole::serialize_b(ProtoLMRole* proto)
{
	proto->set_role_name(this->name_);
	proto->set_sex(this->is_mvp_);
	proto->set_rank_index(this->kill_times_);
	proto->set_fight_score(this->cur_score_);
}

void TMArenaRole::serialize_c(ProtoLMRole* proto)
{
	proto->set_rank_index(this->rank_);
	proto->set_role_name(this->name_);
	proto->set_sex(this->attend_times_);
	proto->set_fight_score(this->score_);
}

void TMArenaRole::serialize_d(ProtoRoleInfo* proto)
{
	proto->set_role_name(this->name_);
	proto->set_role_sex(this->sex_);
	proto->set_role_force(this->force_);
	proto->set_role_career(this->fashion_);
	proto->set_role_wing(this->wing_);
	proto->set_role_solider(this->solider_);
	proto->set_vip_type(this->vip_type_);
}

int TMarenaMonitor::MonitorTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TMarenaMonitor::MonitorTimer::handle_timeout(const Time_Value &tv)
{
	return TRVL_MARENA_MONITOR->handle_mointor_timeout();
}

int TMarenaMonitor::ArrangeTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int TMarenaMonitor::ArrangeTimer::handle_timeout(const Time_Value &tv)
{
	return TRVL_MARENA_MONITOR->handle_arragne_timeout();
}

int TMarenaMonitor::TransferTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TMarenaMonitor::TransferTimer::handle_timeout(const Time_Value &tv)
{
	TMARENA_PREP->notify_all_player_exit();
	return this->cancel_timer();
}

TMarenaMonitor::TMarenaMonitor()
{
	// TODO Auto-generated constructor stub
	this->init_ = false;
	this->update_rank_ = false;
	this->activity_id_ = 0;
	this->enter_level_ = 0;

	this->arena_id_ = 1;
	this->win_mpv_ = 0;
	this->arrange_time_ = 0;
	this->drop_reward_times_ = 0;
	this->max_arrange_time_ = 0;
	this->max_transfer_time_ = 0;

	this->scene_package_ = new PoolPackage<TMArenaScene>;
	this->role_package_ = new PoolPackage<TMArenaRole, Int64>;
}

TMarenaMonitor::~TMarenaMonitor()
{
	// TODO Auto-generated destructor stub
}

const Json::Value& TMarenaMonitor::scene_set_conf()
{
	return CONFIG_INSTANCE->scene_set(this->prep_scene());
}

const Json::Value& TMarenaMonitor::fight_scene_conf()
{
	return CONFIG_INSTANCE->scene(this->fight_scene());
}

int TMarenaMonitor::prep_scene()
{
	return GameEnum::TRVL_MARENA_PREP_SCENE_ID;
}

int TMarenaMonitor::fight_scene()
{
	return GameEnum::TRVL_MARENA_SCENE_ID;
}

int TMarenaMonitor::attend_amount()
{
	return this->sign_map_.size() + this->last_sign_map_.size();
}

int TMarenaMonitor::is_enter_time()
{
	return this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START;
}

int TMarenaMonitor::is_fighting_time()
{
	return this->left_prep_time_.is_zero();
}

IntPair TMarenaMonitor::fighting_state()
{
	IntPair pair;
	JUDGE_RETURN(this->init_ == true, pair);

	if (this->is_enter_time() == false)
	{
		pair.first = 1;
		pair.second = 0;
	}
	else if (this->is_fighting_time() == true)
	{
		pair.first = 1;
		pair.second = this->monitor_timer_.left_second();
	}
	else
	{
		pair.first = 0;
		pair.second = this->left_prep_time_.left_time_;
	}

	return pair;
}

int TMarenaMonitor::request_enter_tmarena(int sid, Proto30400051* request)
{
	string main_version = request->main_version();
	JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true,
			ERROR_TRVL_SAME_VERSION);

	JUDGE_RETURN(this->is_enter_time() == true, ERROR_GUARD_TIME);
	JUDGE_RETURN(request->role_level() >= this->enter_level_, ERROR_PLAYER_LEVEL);

	Proto30400052 enter_info;
	enter_info.set_space_id(0);
	enter_info.set_scene_mode(SCENE_MODE_BATTLE_GROUND);

	MoverCoord enter_point = TMARENA_PREP->fetch_enter_pos();
	enter_info.set_pos_x(enter_point.pixel_x());
	enter_info.set_pos_y(enter_point.pixel_y());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int TMarenaMonitor::fetch_rank_info(int sid, Int64 role)
{
	Proto50400835 respond;

	int max_amount = std::min<int>(100, this->his_rank_vec_.size());
	for (int i = 0; i < max_amount; ++i)
	{
		TMArenaRole* marena_role = this->find_role(this->his_rank_vec_[i].id_);
		JUDGE_CONTINUE(marena_role != NULL);

		ProtoLMRole* proto = respond.add_rank_list();
		marena_role->serialize_c(proto);
	}

	TMArenaRole* self_role = this->find_role(role);
	if (self_role != NULL)
	{
		self_role->serialize_c(respond.mutable_self_info());
	}

	TMArenaRole* first_role = this->first_role();
	if (first_role != NULL)
	{
		first_role->serialize_d(respond.mutable_first_info());
	}

	return MAP_MONITOR->dispatch_to_client_from_gate(sid, role, &respond);
}

int TMarenaMonitor::handle_mointor_timeout()
{
	int state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_mointor_timeout_i(state);
}

int TMarenaMonitor::handle_arragne_timeout()
{
	this->left_prep_time_.reduce_time();
	JUDGE_RETURN(this->left_prep_time_.is_zero() == true, -1);

	this->arrange_rivial();
	this->start_transfer();
	this->sort_rank();
	return 0;
}

int TMarenaMonitor::handle_mointor_timeout_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
#ifdef TEST_COMMAND
		int left_time = 30;
#else
		int left_time = this->fight_scene_conf()["prep_time"].asInt();
#endif
		this->left_prep_time_.set_time(left_time);
		this->start_event();
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_event();
		break;
	}
	}

	this->monitor_timer_.cancel_timer();
	this->monitor_timer_.schedule_timer(this->time_info_.refresh_time_);

	MSG_USER("TMarenaMonitor %d, %d, %d", this->time_info_.time_index_,
			this->time_info_.cur_state_, this->time_info_.refresh_time_);
	return 0;
}

void TMarenaMonitor::start()
{
	this->init_ = true;
	TMARENA_PREP->tmarena_prep_init();

	const Json::Value& set_conf = this->scene_set_conf();
	this->enter_level_ = set_conf["enter_level"].asInt();
	this->activity_id_ = set_conf["activity_id"].asInt();

	const Json::Value& fight_conf = this->fight_scene_conf();
	this->max_arrange_time_ = fight_conf["max_arrange_time"].asInt();
	this->max_transfer_time_ = fight_conf["max_transfer_time"].asInt();
	this->win_mpv_ = fight_conf["win_mpv"].asInt();
	this->drop_reward_times_ = fight_conf["drop_reward"].asInt();
	this->point_reward_[0] = fight_conf["lose_point_reward"].asInt();
	this->point_reward_[1] = fight_conf["win_point_reward"].asInt();
	this->drop_reward_[0] = fight_conf["lose_reward"].asInt();
	this->drop_reward_[1] = fight_conf["win_reward"].asInt();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->activity_id_);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_, activity_conf);
    this->monitor_timer_.schedule_timer(this->time_info_.refresh_time_);

    MSG_USER("TMarenaMonitor %d...", this->time_info_.refresh_time_);
    JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

    this->start_event();
}

void TMarenaMonitor::stop()
{

}

void TMarenaMonitor::test_marena(int id, int last)
{
	this->time_info_.cur_state_ = 2;
	this->time_info_.refresh_time_ = last;
	this->handle_mointor_timeout_i(0);
}

void TMarenaMonitor::start_event()
{
	this->arena_id_ = 1;
	this->sign_map_.clear();
	this->last_sign_map_.clear();
	this->transfer_scene_.clear();

	this->role_package_->clear();
	this->scene_package_->clear();

	this->transfer_timer_.cancel_timer();
	this->arrange_timer_.schedule_timer(1);

	GameCommon::map_sync_activity_tips_start(PairObj(this->activity_id_, true),
			this->time_info_.refresh_time_);
	MSG_USER("TMarenaMonitor start");
}

void TMarenaMonitor::stop_event()
{
	for (IntMap::iterator iter = this->transfer_scene_.begin();
			iter != this->transfer_scene_.end(); ++iter)
	{
		TMArenaScene* scene = this->find_scene(iter->first);
		JUDGE_CONTINUE(scene != NULL);
		this->recycle_scene(scene);
	}

	BIntMap fighting_map = this->scene_package_->fetch_index_map();
	for (BIntMap::iterator iter = fighting_map.begin();
			iter != fighting_map.end(); ++iter)
	{
		TMArenaScene* scene = this->find_scene(iter->first);
		JUDGE_CONTINUE(scene != NULL);
		scene->handle_fighting_timeout(true);
	}

	this->sign_map_.clear();
	this->last_sign_map_.clear();
	this->transfer_scene_.clear();

	this->arrange_timer_.cancel_timer();
#ifdef TEST_COMMAND
	this->transfer_timer_.schedule_timer(Time_Value::MINUTE);
#else
	this->transfer_timer_.schedule_timer(5 * Time_Value::MINUTE);
#endif

	this->update_rank();
	this->sort_rank();
	this->send_rank_reward();

	GameCommon::map_sync_activity_tips_stop(PairObj(this->activity_id_, true));
	MSG_USER("TMarenaMonitor stop");
}

void TMarenaMonitor::arrange_rivial()
{
	JUDGE_RETURN(this->monitor_timer_.left_second() > 30, ;);

	this->arrange_time_ += 1;
	JUDGE_RETURN(this->arrange_time_ >= this->max_arrange_time_, ;);

	this->arrange_time_ = 0;

	int total_scene = this->attend_amount() / TMarenaMonitor::MIN_FIGHTER;
	int total_amount = total_scene * TMarenaMonitor::MIN_FIGHTER;
	JUDGE_RETURN(total_scene > 0, ;);

	//上一轮剩余
	LongVec attend_vec;
	attend_vec.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	GameCommon::t_map_to_vec(attend_vec, this->last_sign_map_);

	//本轮
	LongVec cur_attend_vec;
	cur_attend_vec.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	GameCommon::t_map_to_vec(cur_attend_vec, this->sign_map_);
	GameCommon::shuffle_t_vec(cur_attend_vec);

	//加入本轮
	int left_amount = total_amount - attend_vec.size();
	for (int i = 0; i < left_amount; ++i)
	{
		attend_vec.push_back(cur_attend_vec[i]);
	}

	//保存到下一轮
	this->last_sign_map_.clear();
	int cur_amount = cur_attend_vec.size();
	for (int i = left_amount; i < cur_amount; ++i)
	{
		this->last_sign_map_[cur_attend_vec[i]] = true;
	}

	//安排比赛
	for (int i = 0; i < total_scene; ++i)
	{
		this->arena_id_ += 1;
		this->transfer_scene_[this->arena_id_] = true;

		TMArenaScene* scene = this->scene_package_->pop_object();
		scene->init_tmarena_scene(IntPair(this->arena_id_, i), attend_vec);
		scene->notify_find_rivial();
		this->scene_package_->bind_object(scene->space_id(), scene);
	}

	this->sign_map_.clear();
	this->transfer_time_.set_time(this->max_transfer_time_);
}

void TMarenaMonitor::start_transfer()
{
	JUDGE_RETURN(this->transfer_time_.is_zero() == false, ;);

	this->transfer_time_.reduce_time();
	JUDGE_RETURN(this->transfer_time_.is_zero() == true, ;);

	for (IntMap::iterator iter = this->transfer_scene_.begin();
			iter != this->transfer_scene_.end(); ++iter)
	{
		TMArenaScene* scene = this->find_scene(iter->first);
		JUDGE_CONTINUE(scene != NULL);
		scene->notify_transfer_enter();
		scene->start_prep_time();
	}

	this->transfer_scene_.clear();
}

void TMarenaMonitor::sort_rank()
{
	JUDGE_RETURN(this->update_rank_ == true, ;);

	this->update_rank_ = false;
	this->his_rank_vec_.clear();

	BLongMap index_map = this->role_package_->fetch_index_map();
	this->his_rank_vec_.reserve(index_map.size());

	for (BLongMap::iterator iter = index_map.begin(); iter != index_map.end(); ++iter)
	{
		TMArenaRole* role = this->find_role(iter->first);
		JUDGE_CONTINUE(role != NULL);

		ThreeObj obj;
		obj.id_ = role->id_;
		obj.value_ = role->score_;
		obj.tick_ = role->update_tick_;
		this->his_rank_vec_.push_back(obj);
	}

	std::sort(this->his_rank_vec_.begin(), this->his_rank_vec_.end(),
			GameCommon::three_comp_by_desc);

	int rank = 1;
	for (ThreeObjVec::iterator iter = this->his_rank_vec_.begin();
			iter != this->his_rank_vec_.end(); ++iter)
	{
		TMArenaRole* role = this->find_role(iter->id_);
		JUDGE_CONTINUE(role != NULL);

		role->rank_ = rank;
		++rank;
	}
}

void TMarenaMonitor::update_rank()
{
	this->update_rank_ = true;
}

void TMarenaMonitor::send_rank_reward()
{
	Proto30100232 inner;
	for (ThreeObjVec::iterator iter = this->his_rank_vec_.begin();
			iter != this->his_rank_vec_.end(); ++iter)
	{
		TMArenaRole* role = this->find_role(iter->id_);
		JUDGE_CONTINUE(role != NULL);

		inner.set_rank(role->rank_);
		inner.set_role(role->id_);
		MAP_MONITOR->dispatch_to_logic(role->sid_, &inner);
	}
}

void TMarenaMonitor::register_role(BattleGroundActor* player)
{
	TMArenaRole* arena_role = this->find_role(player->role_id());
	JUDGE_RETURN(arena_role != NULL, ;);

	if (arena_role->init_ == false)
	{
		player->set_base_member_info(*arena_role);
		arena_role->sid_ = player->gate_sid();
		arena_role->init_ = true;
	}

	if (this->last_sign_map_.count(player->role_id()) == 0)
	{
		this->sign_map_[player->role_id()] = true;
	}
}

void TMarenaMonitor::unsign(Int64 role)
{
	this->sign_map_.erase(role);
	this->last_sign_map_.erase(role);
}

void TMarenaMonitor::recycle_scene(TMArenaScene* scene)
{
	this->scene_package_->unbind_and_push(scene->space_id(), scene);
}

TMArenaRole* TMarenaMonitor::first_role()
{
	JUDGE_RETURN(this->his_rank_vec_.empty() == false, NULL);

	Int64 role = this->his_rank_vec_[0].id_;
	return this->find_role(role);
}

TMArenaRole* TMarenaMonitor::find_role(Int64 role)
{
	return this->role_package_->find_object(role);
}

TMArenaRole* TMarenaMonitor::find_and_pop(Int64 role)
{
	TMArenaRole* arena_role = this->role_package_->find_object(role);
	JUDGE_RETURN(arena_role == NULL, arena_role);

	arena_role = this->role_package_->pop_object();
	arena_role->id_ = role;

	this->role_package_->bind_object(role, arena_role);
	return arena_role;
}

TMArenaScene* TMarenaMonitor::find_scene(int space)
{
	return this->scene_package_->find_object(space);
}


