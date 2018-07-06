/*
 * TrvlArenaScene.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: peizhibi
 */

#include "TrvlArenaScene.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "TrvlArenaMonitor.h"

int TrvlArenaScene::ArenaTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlArenaScene::ArenaTimer::handle_timeout(const Time_Value &tv)
{
	switch (this->state_)
	{
	case TrvlArenaScene::PREP:
	{
		this->scene_->handle_prep_timeout();
		this->scene_->notify_arena_fight();
		break;
	}
	case TrvlArenaScene::FIGHTING:
	{
		this->scene_->handle_fighting_timeout();
		this->scene_->check_and_handle_offline();
		this->scene_->notify_arena_fight();
		break;
	}
	case TrvlArenaScene::FINISH:
	{
		this->scene_->handle_finish_timeout();
		break;
	}
	}

	return 0;
}

TrvlArenaScene::TrvlArenaScene()
{
	// TODO Auto-generated constructor stub
	this->arena_timer_.scene_ = this;
}

TrvlArenaScene::~TrvlArenaScene()
{
	// TODO Auto-generated destructor stub
}

void TrvlArenaScene::reset()
{
	Scene::reset();

	this->arena_timer_.state_ = TrvlArenaScene::PREP;
	this->arena_timer_.cancel_timer();

	this->prep_time_ = 0;
	this->left_time_ = 0;
	this->finish_flag_ = 0;

	for (int i = 0; i < TOTAL_ROLE; ++i)
	{
		this->role_id_[i] = 0;
	}
}

int TrvlArenaScene::is_fighting_time()
{
	return this->arena_timer_.state_ == TrvlArenaScene::FIGHTING;
}

void TrvlArenaScene::init_arena_scene(int space_id, FourObj& obj)
{
	int scene_id = GameEnum::TRVL_ARENA_SCENE_ID;

	this->init_scene(space_id, scene_id);
	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, scene_id, this);

	this->role_type_ = obj.id_;
	this->role_id_[0] = obj.first_id_;
	this->role_id_[1] = obj.second_id_;

	const Json::Value& conf = this->conf();
	this->prep_time_ = conf["prep_time"].asInt();
	this->left_time_ = conf["fighting_time"].asInt();

	this->arena_timer_.state_ = TrvlArenaScene::PREP;
	this->arena_timer_.schedule_timer(1);
}

void TrvlArenaScene::notify_arena_fight(Int64 role)
{
	Proto80100504 respond;
	respond.set_left_time(this->left_time());
	this->make_up_role_info(respond.mutable_first(), 0);
	this->make_up_role_info(respond.mutable_second(), 1);
	this->notify_all_player_msg(ACTIVE_TARENA_FIGHT_INFO, &respond);
}

void TrvlArenaScene::check_and_handle_offline()
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);
	JUDGE_RETURN(this->left_time_ > 0, ;);

	MapPlayerEx* first = this->find_player(this->role_id_[0]);
	MapPlayerEx* second = this->find_player(this->role_id_[1]);

	if (first == NULL)
	{
		this->handle_arena_finish(this->role_id_[0]);
	}
	else if (second == NULL)
	{
		this->handle_arena_finish(this->role_id_[1]);
	}
}

void TrvlArenaScene::make_up_role_info(ProtoLMRole* role, int index)
{
	TrvlArenaRole* arena = TRVL_ARENA_MONITOR->find_role(this->role_id_[index]);
	JUDGE_RETURN(arena != NULL, ;);

	role->set_role_id(arena->id_);
	role->set_role_name(arena->name_);
	role->set_role_force(arena->force_);
	role->set_sex(arena->sex_);
	role->set_fight_score(100);
	role->set_rank_index(arena->stage_);

	MapPlayerEx* player = this->find_player(arena->id_);
	JUDGE_RETURN(player != NULL, ;);
	JUDGE_RETURN(player->is_enter_scene() == true, ;);

	role->set_fight_score(player->cur_blood_percent(100));
}

void TrvlArenaScene::handle_prep_timeout()
{
	this->prep_time_ -= 1;
	JUDGE_RETURN(this->prep_time_ <= 0, ;);

	this->arena_timer_.cancel_timer();
	this->arena_timer_.state_ = TrvlArenaScene::FIGHTING;
	this->arena_timer_.schedule_timer(1);
}

void TrvlArenaScene::handle_fighting_timeout()
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	this->left_time_ -= 1;
	JUDGE_RETURN(this->left_time_ <= 0, ;);

	Int64 lose_id = this->fetch_timeout_lose();
	this->handle_arena_finish(lose_id);
}

void TrvlArenaScene::handle_finish_timeout()
{
	this->arena_timer_.cancel_timer();
//	this->restore_all_player_blood_full();
	this->notify_all_player_exit(GameEnum::EXIT_TYPE_PREVE);

	MAP_MONITOR->unbind_scene(this->space_id(), this->scene_id());
	TRVL_ARENA_MONITOR->recycle_scene(this);
}

void TrvlArenaScene::handle_arena_finish(Int64 lose_id)
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	this->arena_timer_.cancel_timer();
	this->arena_timer_.state_ = TrvlArenaScene::FINISH;

	this->finish_flag_ = true;
	this->handle_arena_kill(lose_id);

	int quit_time = this->conf()["quit_time"].asInt();
	this->arena_timer_.schedule_timer(quit_time);
}

void TrvlArenaScene::handle_arena_kill(Int64 lose_id)
{
	if (this->role_type_ != TrvlArenaMonitor::RIVAL_ROLE)
	{
		this->handle_arena_kill_robot(lose_id);
	}
	else
	{
		this->handle_arena_kill_rivial(lose_id);
	}
}

void TrvlArenaScene::handle_arena_kill_robot(Int64 lose_id)	//机器人
{

}

void TrvlArenaScene::handle_arena_kill_rivial(Int64 lose_id)	//人与人
{
	Int64 win_id = this->fetch_win(lose_id);
	JUDGE_RETURN(win_id > 0, ;);

	TrvlArenaRole* win = TRVL_ARENA_MONITOR->find_role(win_id);
	TrvlArenaRole* lose = TRVL_ARENA_MONITOR->find_role(lose_id);
	JUDGE_RETURN(win != NULL && lose != NULL, ;);

	win->win_times_ += 1;
	win->con_lose_times_ = 0;
	lose->con_lose_times_ += 1;

	this->handle_arena_reward(win, lose, 1);
	this->handle_arena_reward(lose, win, 0);

	int quit_time = this->conf()["quit_time"].asInt();
	win->start_tick_ = ::time(NULL) + quit_time + 1;
	lose->start_tick_ = ::time(NULL) + quit_time + 1;

	win->state_ = TrvlArenaRole::STATE_NONE;
	lose->state_ = TrvlArenaRole::STATE_NONE;
}

void TrvlArenaScene::handle_arena_reward(TrvlArenaRole* self,
		TrvlArenaRole* rivial, int flag)
{
	static int ADJUST_FLAG[2] = { -1, 1 };
	static int SERIAL_LIST[2] = { ADD_FROM_TRAVEL_ARENA_LOSE,
			ADD_FROM_TRAVEL_ARENA_WIN };

	static string POINT_NAME[2] = { "lose_point", "win_point" };
	static string REWARD_NAME[2] = { "lose_reward", "win_reward" };

	int adjust_flag = ADJUST_FLAG[flag];
	string poin_name = POINT_NAME[flag];
	string reward_name = REWARD_NAME[flag];

	const Json::Value& conf = CONFIG_INSTANCE->tarena_fight(
			self->index(), rivial->index());

	int reward_id = conf[reward_name].asInt();
	int score = adjust_flag * conf[poin_name].asInt();

	RewardInfo reward_info;
	GameCommon::make_up_reward_items(reward_info, reward_id);

	MapPlayerEx* player = this->find_player(self->id_);
	if (player != NULL)
	{
		Proto80100505 respond;
		respond.set_flag(flag);
		respond.set_score(score);
		respond.set_name(rivial->name_);
		respond.set_stage(self->stage_);
		respond.set_reward_id(reward_id);
		player->respond_to_client(ACTIVE_TARENA_FIGHT_FINISH, &respond);
		player->sync_restore_info(GameEnum::ES_ACT_TRVL_ARENA, self->stage_, 0);

		int exploit = reward_info.resource_map_[GameEnum::ITEM_ID_EXPLOIT];
		player->update_tarena_score(score, exploit, self);
		player->request_add_reward(reward_id, SERIAL_LIST[flag]);
	}
	else
	{
		TRVL_ARENA_MONITOR->send_offline_score(self);
	}
}

int TrvlArenaScene::left_time()
{
	int total = this->left_time_ + this->prep_time_;
	return std::max<int>(total, 0);
}

Int64 TrvlArenaScene::fetch_win(Int64 lose_id)
{
	if (this->role_id_[0] == lose_id)
	{
		return this->role_id_[1];
	}
	else
	{
		return this->role_id_[0];
	}
}

Int64 TrvlArenaScene::fetch_timeout_lose()
{
	MapPlayerEx* first = this->find_player(this->role_id_[0]);
	MapPlayerEx* second = this->find_player(this->role_id_[1]);

	if (first == NULL)
	{
		return this->role_id_[0];
	}
	else if (second == NULL)
	{
		return this->role_id_[1];
	}

	if (first->cur_blood_percent() > second->cur_blood_percent())
	{
		return this->role_id_[1];
	}
	else
	{
		return this->role_id_[0];
	}
}
