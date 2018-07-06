/*
 * TMArenaScene.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#include "TMArenaScene.h"
#include "TMarenaMonitor.h"
#include "MapMonitor.h"
#include "TMArenaPrep.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"

TMSideInfo::TMSideInfo()
{
	TMSideInfo::reset();
}

void TMSideInfo::reset()
{
	this->side_ = 0;
	this->die_times_ = 0;
	this->role_vec_.clear();
	this->role_map_.clear();
}

int TMArenaScene::ArenaTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int TMArenaScene::ArenaTimer::handle_timeout(const Time_Value &tv)
{
	switch (this->state_)
	{
	case TMArenaScene::PREP:
	{
		this->scene_->handle_prep_timeout();
		this->scene_->notify_tmarena_fight_info(0);
		break;
	}
	case TMArenaScene::FIGHTING:
	{
		this->scene_->handle_fighting_timeout();
		this->scene_->check_and_handle_offline();
		this->scene_->notify_tmarena_fight_info(1);
		break;
	}
	case TMArenaScene::FINISH:
	{
		this->scene_->handle_finish_timeout();
		break;
	}
	}

	return 0;
}

TMArenaScene::TMArenaScene()
{
	// TODO Auto-generated constructor stub
	this->arena_timer_.scene_ = this;
}

TMArenaScene::~TMArenaScene()
{
	// TODO Auto-generated destructor stub
}

void TMArenaScene::reset(void)
{
	Scene::reset();

	for (int i = 0; i < TOTAL_SIDE; ++i)
	{
		this->coord_[i].reset();
		this->protected_time_[i] = 0;
	}

	this->prep_time_ = 0;
	this->fight_time_ = 0;
	this->finish_flag_ = 0;
	this->max_die_times_ = 0;

	this->first_.reset();
	this->second_.reset();
	this->rank_vec_.clear();
	this->arena_timer_.cancel_timer();

	this->first_.side_ = GameEnum::CAMP_ONE;
	this->second_.side_ = GameEnum::CAMP_TWO;
}

int TMArenaScene::fetch_relive_protected_time(Int64 role)
{
	int side = this->fetch_camp_id(role);
	return this->protected_time_[side - 1];
}

int TMArenaScene::fetch_enter_buff(IntVec& buff_vec, Int64 role)
{
	int side = this->fetch_camp_id(role);

	const Json::Value& buff_conf = this->conf()["buff_list"][side - 1];
	GameCommon::json_to_t_vec(buff_vec, buff_conf);

	return 0;
}

void TMArenaScene::init_tmarena_scene(const IntPair& key, const LongVec& attend_vec)
{
	int cur_index = key.second * TMarenaMonitor::MIN_FIGHTER;
	for (int i = 0; i < TMarenaMonitor::MIN_FIGHTER; ++i)
	{
		Int64 role = attend_vec[cur_index++];

		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(role);
		JUDGE_CONTINUE(arena_role != NULL);

		PairObj obj;
		obj.id_ = role;
		obj.value_ = arena_role->force_;
		this->rank_vec_.push_back(obj);
	}

	std::sort(this->rank_vec_.begin(), this->rank_vec_.end(), GameCommon::pair_comp_by_desc);
	JUDGE_RETURN(this->rank_vec_.size() == TMarenaMonitor::MIN_FIGHTER, ;);

	this->init_scene(key.first, TRVL_MARENA_MONITOR->fight_scene());
	this->start_scene();

	if (TMarenaMonitor::MIN_FIGHTER == 6)
	{
		this->first_.role_map_[this->rank_vec_[0].id_] = 0;
		this->first_.role_map_[this->rank_vec_[3].id_] = 0;
		this->second_.role_map_[this->rank_vec_[1].id_] = 0;
		this->second_.role_map_[this->rank_vec_[2].id_] = 0;
		this->second_.role_map_[this->rank_vec_[4].id_] = 0;
		this->second_.role_map_[this->rank_vec_[5].id_] = 0;

		this->first_.role_vec_.push_back(this->rank_vec_[0].id_);
		this->first_.role_vec_.push_back(this->rank_vec_[3].id_);
		this->second_.role_vec_.push_back(this->rank_vec_[1].id_);
		this->second_.role_vec_.push_back(this->rank_vec_[2].id_);
		this->second_.role_vec_.push_back(this->rank_vec_[4].id_);
		this->second_.role_vec_.push_back(this->rank_vec_[5].id_);
	}
	else
	{
		this->first_.role_map_[this->rank_vec_[0].id_] = 0;
		this->second_.role_map_[this->rank_vec_[1].id_] = 0;

		this->first_.role_vec_.push_back(this->rank_vec_[0].id_);
		this->second_.role_vec_.push_back(this->rank_vec_[1].id_);
	}

	for (PairObjVec::iterator iter = this->rank_vec_.begin();
			iter != this->rank_vec_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(iter->id_);
		JUDGE_CONTINUE(arena_role != NULL);

		arena_role->fight_state_ = 1;
		arena_role->drop_reward_ = 0;
		arena_role->cur_score_ = 0;
		arena_role->kill_times_ = 0;
		arena_role->die_times_ = 0;
		arena_role->is_mvp_ = false;
	}

	const Json::Value& conf = this->conf();
	this->prep_time_ 		= conf["max_transfer_time"].asInt() + 2;
	this->fight_time_ 		= conf["fighting_time"].asInt();
	this->max_die_times_ 	= conf["max_die_times"].asInt();

	const Json::Value& enter_pos = conf["enter_pos"];
	const Json::Value& relive_protected = conf["relive_protected"];
	for (int i = 0; i < TOTAL_SIDE; ++i)
	{
		this->coord_[i].set_pixel(enter_pos[i][0u].asInt(),
				enter_pos[i][1u].asInt());
		this->protected_time_[i] = relive_protected[i].asInt();
	}

	MAP_MONITOR->bind_scene(key.first, TRVL_MARENA_MONITOR->fight_scene(), this);
}

void TMArenaScene::notify_find_rivial()
{
	for (PairObjVec::iterator iter = this->rank_vec_.begin();
			iter != this->rank_vec_.end(); ++iter)
	{
		MapPlayerEx* player = TMARENA_PREP->find_player(iter->id_);
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(ACTIVE_TMARENA_FIND_RIVAL);
	}
}

void TMArenaScene::notify_tmarena_fight_info(int state)
{
	Proto80100508 respond;
	respond.set_state(state);
	respond.set_left_time(this->fight_time_);
	respond.set_first_die_times(this->first_.die_times_);
	respond.set_second_die_times(this->second_.die_times_);

	for (LongVec::iterator iter = this->first_.role_vec_.begin();
			iter != this->first_.role_vec_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(*iter);
		JUDGE_CONTINUE(arena_role != NULL);

		ProtoLMRole* proto = respond.add_first();
		proto->set_fight_score(0);
		arena_role->serialize(proto);

		MapPlayerEx* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		proto->set_fight_score(player->cur_blood_percent(100));
	}

	for (LongVec::iterator iter = this->second_.role_vec_.begin();
			iter != this->second_.role_vec_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(*iter);
		JUDGE_CONTINUE(arena_role != NULL);

		ProtoLMRole* proto = respond.add_second();
		proto->set_fight_score(0);
		arena_role->serialize(proto);

		MapPlayerEx* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		proto->set_fight_score(player->cur_blood_percent(100));
	}

	for (PairObjVec::iterator iter = this->rank_vec_.begin();
			iter != this->rank_vec_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->id_);
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(ACTIVE_TMARENA_FIGHT_INFO, &respond);
	}
}

void TMArenaScene::notify_transfer_enter()
{
	for (PairObjVec::iterator iter = this->rank_vec_.begin();
			iter != this->rank_vec_.end(); ++iter)
	{
		MapPlayerEx* player = TMARENA_PREP->find_player(iter->id_);
		JUDGE_CONTINUE(player != NULL);

		MoverCoord coord = this->fetch_enter_pos(iter->id_);
		player->transfer_dispatcher(this->scene_id(), coord,
				SCENE_MODE_BATTLE_GROUND, this->space_id());
	}
}

void TMArenaScene::start_prep_time()
{
	this->arena_timer_.state_ = TMArenaScene::PREP;
	this->arena_timer_.cancel_timer();
	this->arena_timer_.schedule_timer(1);
}

void TMArenaScene::handle_prep_timeout()
{
	this->prep_time_ -= 1;
	JUDGE_RETURN(this->prep_time_ <= 0, ;);

	this->arena_timer_.state_ = TMArenaScene::FIGHTING;
	this->arena_timer_.cancel_timer();
	this->arena_timer_.schedule_timer(1);
}

void TMArenaScene::handle_fighting_timeout(int force)
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	if (force == false)
	{
		this->fight_time_ -= 1;
		JUDGE_RETURN(this->fight_time_ <= 0, ;);
	}

	int win_side = this->fetch_win_side();
	this->handle_marena_finish(win_side);
}

void TMArenaScene::handle_marena_finish(int win_side)
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	this->finish_flag_ = true;
	this->handle_marena_result(win_side);
	TRVL_MARENA_MONITOR->update_rank();

	this->arena_timer_.state_ = TMArenaScene::FINISH;
	this->arena_timer_.cancel_timer();
	this->arena_timer_.schedule_timer(this->conf()["quit_time"].asInt());
}

void TMArenaScene::handle_marena_result(int win_side)
{
	TMSideInfo* win = NULL;
	TMSideInfo* lose = NULL;

	if (win_side == GameEnum::CAMP_ONE)
	{
		win = &this->first_;
		lose = &this->second_;
	}
	else
	{
		win = &this->second_;
		lose = &this->first_;
	}

	this->handle_marena_result(false, lose);
	this->handle_marena_result(true, win);

	Proto80100509 respond;
	for (LongVec::iterator iter = win->role_vec_.begin();
			iter != win->role_vec_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(*iter);
		JUDGE_CONTINUE(arena_role != NULL);

		ProtoLMRole* proto = respond.add_win();
		arena_role->serialize_b(proto);
	}

	for (LongVec::iterator iter = lose->role_vec_.begin();
			iter != lose->role_vec_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(*iter);
		JUDGE_CONTINUE(arena_role != NULL);

		ProtoLMRole* proto = respond.add_lose();
		arena_role->serialize_b(proto);
	}

//	MSG_USER("TMArenaScene %s", respond.Utf8DebugString().c_str());

	respond.set_result(false);
	this->notify_marena_result(&respond, lose);

	respond.set_result(true);
	this->notify_marena_result(&respond, win);
}

void TMArenaScene::handle_marena_result(int flag, TMSideInfo* side_info)
{
	LongPair pair;
	for (LongMap::iterator iter = side_info->role_map_.begin();
			iter != side_info->role_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(flag == true);

		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(iter->first);
		JUDGE_CONTINUE(arena_role != NULL);

		if (pair.first != 0)
		{
			JUDGE_CONTINUE(arena_role->kill_times_ > pair.second);
		}

		pair.first = iter->first;
		pair.second = arena_role->kill_times_;
	}

	for (LongMap::iterator iter = side_info->role_map_.begin();
			iter != side_info->role_map_.end(); ++iter)
	{
		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(iter->first);
		JUDGE_CONTINUE(arena_role != NULL);

		arena_role->fight_state_ = 0;
		arena_role->attend_times_ += 1;
		arena_role->start_tick_ = ::time(NULL) + 10;

		int add_score = TRVL_MARENA_MONITOR->point_reward_[flag];
		if (pair.first == iter->first)
		{
			add_score += TRVL_MARENA_MONITOR->win_mpv_;
			arena_role->is_mvp_ = true;
		}

		arena_role->score_ += add_score;
		arena_role->update_tick_ = ::time(NULL);

		arena_role->cur_score_ = add_score;
		arena_role->win_times_ += flag;
		JUDGE_CONTINUE(arena_role->attend_times_ <= TRVL_MARENA_MONITOR->drop_reward_times_);

		MapPlayerEx* player = MAP_MONITOR->find_map_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		arena_role->drop_reward_ = TRVL_MARENA_MONITOR->drop_reward_[flag];
		player->request_add_reward(arena_role->drop_reward_, ADD_FROM_TMARENA_FINISH);
	}
}

void TMArenaScene::notify_marena_result(Proto80100509* respond, TMSideInfo* side_info)
{
	for (LongMap::iterator iter = side_info->role_map_.begin();
			iter != side_info->role_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(iter->first);
		JUDGE_CONTINUE(arena_role != NULL);

		respond->set_reward(arena_role->drop_reward_);
		player->respond_to_client(ACTIVE_TMARENA_FIGHT_FINISH, respond);
	}
}

void TMArenaScene::check_and_handle_offline()
{
	int first_flag = this->is_player_online(GameEnum::CAMP_ONE);
	int second_flag = this->is_player_online(GameEnum::CAMP_TWO);

	if (first_flag == false)
	{
		this->handle_marena_finish(GameEnum::CAMP_TWO);
	}
	else if (second_flag == false)
	{
		this->handle_marena_finish(GameEnum::CAMP_ONE);
	}
}

void TMArenaScene::handle_finish_timeout()
{
	this->arena_timer_.cancel_timer();
	this->notify_all_player_exit(GameEnum::EXIT_TYPE_PREVE);

	MAP_MONITOR->unbind_scene(this->space_id(), this->scene_id());
	TRVL_MARENA_MONITOR->recycle_scene(this);
}

void TMArenaScene::handle_marena_player_die(Int64 killer_id, Int64 killed_id)
{
	TMArenaRole* arena_killer = TRVL_MARENA_MONITOR->find_role(killer_id);
	TMArenaRole* arena_killed = TRVL_MARENA_MONITOR->find_role(killed_id);
	JUDGE_RETURN(arena_killer != NULL && arena_killed != NULL, ;);

	TMSideInfo* kill_side = NULL;
	TMSideInfo* die_side = NULL;
	if (this->first_.role_map_.count(killer_id) > 0)
	{
		kill_side = &this->first_;
		die_side = &this->second_;
	}
	else
	{
		kill_side = &this->second_;
		die_side = &this->first_;
	}

	arena_killer->kill_times_ += 1;
	arena_killed->die_times_ += 1;
	die_side->die_times_ += 1;
	JUDGE_RETURN(die_side->die_times_ >= this->max_die_times_, ;);

	this->notify_tmarena_fight_info(1);
	this->handle_marena_finish(kill_side->side_);
}

int TMArenaScene::fetch_win_side()
{
	if (this->first_.die_times_ > this->second_.die_times_)
	{
		return this->second_.side_;
	}
	else
	{
		return this->first_.side_;
	}
}

int TMArenaScene::is_player_online(int side)
{
	TMSideInfo* side_info = NULL;
	if (side == GameEnum::CAMP_ONE)
	{
		side_info = &this->first_;
	}
	else
	{
		side_info = &this->second_;
	}

	for (LongMap::iterator iter = side_info->role_map_.begin();
			iter != side_info->role_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->find_player(iter->first) != NULL);
		return true;
	}

	return false;
}

int TMArenaScene::fetch_camp_id(Int64 role)
{
	if (this->first_.role_map_.count(role) > 0)
	{
		return GameEnum::CAMP_ONE;
	}
	else
	{
		return GameEnum::CAMP_TWO;
	}
}

MoverCoord TMArenaScene::fetch_enter_pos(Int64 role)
{
	if (this->first_.role_map_.count(role) > 0)
	{
		return this->coord_[0];
	}
	else
	{
		return this->coord_[1];
	}
}

