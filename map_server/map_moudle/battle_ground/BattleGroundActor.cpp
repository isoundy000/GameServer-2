/*
 * BattleGroundActor.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: lijin
 */

#include "BattleGroundActor.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "SMBattleSystem.h"
#include "SMBattleScene.h"
#include "TrvlArenaMonitor.h"
#include "TrvlArenaScene.h"
#include "TArenaPrepScene.h"
#include "TMArenaPrep.h"
#include "TMArenaScene.h"
#include "TMarenaMonitor.h"
#include "TrvlBattleMonitor.h"
#include "TrvlBattleScene.h"

void TArenaDetail::reset()
{
	this->stage_ = 1;
	this->score_ = 0;
	this->adjust_tick_ 	= 0;

	this->win_times_	= 0;
	this->draw_flag_ 	= 0;
	this->get_exploit_	= 0;
	this->attend_times_ = 0;

	this->draw_win_.clear();
}

int TArenaDetail::update(int add)
{
	static int START_INDEX 	= 1;
	static int END_INDEX 	= CONFIG_INSTANCE->tarena_stage_size();

	this->score_ += add;
	this->score_ = std::max<int>(0, this->score_);

	int last_stage = this->stage_;
	for (int i = START_INDEX; i <= END_INDEX; ++i)
	{
		const Json::Value& conf = CONFIG_INSTANCE->tarena_stage(i);
		JUDGE_BREAK(this->score_ >= conf["need_point"].asInt());
		this->stage_ = i + 1;
	}

	this->stage_ = std::min<int>(END_INDEX, this->stage_);
	JUDGE_RETURN(this->stage_ > last_stage, false);

	return true;
}

const Json::Value& TArenaDetail::conf()
{
	return CONFIG_INSTANCE->tarena_stage(this->stage_);
}

void TMarenaDetail::reset()
{
	this->score_	= 0;
	this->win_times_ = 0;
	this->attend_times_ = 0;
}

BattleGroundActor::BattleGroundActor()
{
	// TODO Auto-generated constructor stub
}

BattleGroundActor::~BattleGroundActor()
{
	// TODO Auto-generated destructor stub
}

void BattleGroundActor::reset()
{
	this->tarena_detail_.reset();
	this->tmarena_detail_.reset();
    this->tbattle_timeout_tick_ = Time_Value::zero;
    this->tbattle_score_timeout_ = Time_Value::zero;
    this->tbattle_treasure_buff_set_.clear();
}

void BattleGroundActor::reset_everyday()
{
	this->tarena_detail_.draw_flag_ = 0;
	this->tarena_detail_.win_times_ = 0;
	this->tarena_detail_.get_exploit_ 	= 0;
	this->tarena_detail_.attend_times_ 	= 0;
	this->tarena_detail_.draw_win_.clear();
	this->tarena_month_settle();
}

int BattleGroundActor::read_transfer_sm_battler(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400113*, request, -1);

	this->tarena_detail_.stage_ = request->stage();
	this->tarena_detail_.score_ = request->score();
	this->tarena_detail_.draw_flag_ = request->draw_flag();
	this->tarena_detail_.win_times_ = request->win_times();
	this->tarena_detail_.get_exploit_ = request->get_exploit();
	this->tarena_detail_.attend_times_ = request->attend_times();
	this->tarena_detail_.adjust_tick_ = request->adjust_tick();

	GameCommon::proto_to_map(this->tarena_detail_.draw_win_,
			request->draw_win());
	return 0;
}

int BattleGroundActor::sync_transfer_sm_battler(void)
{
	Proto30400113 sync_info;
	sync_info.set_stage(this->tarena_detail_.stage_);
	sync_info.set_score(this->tarena_detail_.score_);
	sync_info.set_draw_flag(this->tarena_detail_.draw_flag_);
	sync_info.set_win_times(this->tarena_detail_.win_times_);
	sync_info.set_get_exploit(this->tarena_detail_.get_exploit_);
	sync_info.set_attend_times(this->tarena_detail_.attend_times_);
	sync_info.set_adjust_tick(this->tarena_detail_.adjust_tick_);

	GameCommon::map_to_proto(sync_info.mutable_draw_win(),
			this->tarena_detail_.draw_win_);
	return this->send_to_other_scene(this->scene_id(), sync_info);
}

int BattleGroundActor::enter_scene(const int type)
{
	switch (this->scene_id())
	{
	case GameEnum::SUN_MOON_BATTLE_ID:
	{
		return this->on_enter_battle_scene(type);
	}
	case GameEnum::TRVL_ARENA_SCENE_ID:
	{
		return this->on_enter_travel_arena_scene(type);
	}
	case GameEnum::TRVL_ARENA_PREP_SCENE_ID:
	{
		return this->on_enter_prep_tarena_scene(type);
	}
	case GameEnum::TRVL_MARENA_PREP_SCENE_ID:
	{
		return this->on_enter_prep_tmarena_scene(type);
	}
	case GameEnum::TRVL_MARENA_SCENE_ID:
	{
		return this->on_enter_travel_marena_scene(type);
	}
	case GameEnum::TRVL_BATTLE_SCENE_ID:
	{
		return this->on_enter_tbattle_scene(type);
	}
	}

	return 0;
}

int BattleGroundActor::exit_scene(const int type)
{
	switch (this->scene_id())
	{
	case GameEnum::SUN_MOON_BATTLE_ID:
	{
		this->on_exit_battle_scene(type);
		break;
	}
	case GameEnum::TRVL_ARENA_SCENE_ID:
	{
		this->on_exit_travel_arena_scene(type);
		break;
	}
	case GameEnum::TRVL_ARENA_PREP_SCENE_ID:
	{
		this->on_exit_prep_tarena_scene(type);
		break;
	}
	case GameEnum::TRVL_MARENA_SCENE_ID:
	{
		this->on_exit_travel_marena_scene(type);
		break;
	}
    case GameEnum::TRVL_BATTLE_SCENE_ID:
    {
        this->on_exit_tbattle_scene(type);
        break;
    }
	}

	return MapPlayer::exit_scene(type);
}

int BattleGroundActor::die_process(const int64_t fighter_id)
{
	if (this->scene_id() == GameEnum::TRVL_BATTLE_SCENE_ID)
	{
		this->travel_battle_die_process(fighter_id);
	}

	int ret = MapPlayer::die_process(fighter_id);
	JUDGE_RETURN(fighter_id > 0, ret);

	switch (this->scene_id())
	{
	case GameEnum::SUN_MOON_BATTLE_ID:
	{
		this->sm_battler_die_process(fighter_id);
		break;
	}
	case GameEnum::TRVL_ARENA_SCENE_ID:
	{
		this->travel_arena_die_process(fighter_id);
		break;
	}
	case GameEnum::TRVL_MARENA_SCENE_ID:
	{
		this->travel_marena_die_process(fighter_id);
		break;
	}
	}

	return ret;
}

int BattleGroundActor::check_travel_timeout(void)
{
    switch (this->scene_id())
    {
    case GameEnum::TRVL_BATTLE_SCENE_ID:
        return this->check_tbattle_timeout();
    default:
        break;
    }
    return 0;
}

int BattleGroundActor::process_relive(const int relive_mode, MoverCoord &relive_coord)
{
    switch (this->scene_id())
    {
    case GameEnum::TRVL_BATTLE_SCENE_ID:
        return this->process_tbattle_relive(relive_mode, relive_coord);
    default:
        return MapPlayer::process_relive(relive_mode, relive_coord);
    }
    return 0;
}

int BattleGroundActor::obtain_area_info(int request_flag)
{
    MapPlayer::obtain_area_info(request_flag);
    switch (this->scene_id())
    {
    case GameEnum::TRVL_BATTLE_SCENE_ID:
        return this->process_tbattle_obtain_area_info(request_flag);
    default:
        break;
    }
    return 0;
}

int BattleGroundActor::sm_battle_scene()
{
	return GameEnum::SUN_MOON_BATTLE_ID;
}

int BattleGroundActor::sm_enter_scene_type()
{
	switch (this->scene_id())
	{
	case GameEnum::SUN_MOON_BATTLE_ID:
	{
		return GameEnum::ET_SM_BATTLE;
	}
	case GameEnum::TRVL_ARENA_SCENE_ID:
	{
		return GameEnum::ET_TRAVEL_ARENA;
	}
    case GameEnum::TRVL_BATTLE_SCENE_ID:
    {
        return GameEnum::ET_TRVL_BATTLE;
    }
	}
	return -1;
}

int BattleGroundActor::request_join_sm_battle(void)
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_JOIN_SM_BATTLE, ERROR_NORMAL_SCENE);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_SM_BATTLE);
	enter_info.set_force(this->force_total_i());

	int ret = this->send_request_enter_info(this->sm_battle_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_JOIN_SM_BATTLE, ret);

	return 0;
}

int BattleGroundActor::handle_exit_battle_scene(void)
{
	switch (this->scene_id())
	{
	case GameEnum::TRVL_ARENA_SCENE_ID:
	case GameEnum::TRVL_MARENA_SCENE_ID:
	{
		return this->transfer_to_prev_scene();
	}
	default:
	{
		return this->transfer_to_save_scene();
	}
	}
}

int BattleGroundActor::scan_sm_battle_rank(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400803*, reqeust, RETURN_SCAN_SM_BATTLE_RANK);

	SMBattleScene* scene = SM_BATTLE_SYSTEM->find_battle_scene(this->space_id());
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_SCAN_SM_BATTLE_RANK, ERROR_SERVER_INNER);

	Proto50400803 rank_info;
	scene->make_up_rank_info(this->role_id(), &rank_info);
	FINER_PROCESS_RETURN(RETURN_SCAN_SM_BATTLE_RANK, &rank_info);
}


int BattleGroundActor::request_init_sm_battle_info()
{
	SMBattleScene* scene = SM_BATTLE_SYSTEM->find_battle_scene(this->space_id());
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_REQUEST_INIT_SM_INFO, ERROR_SERVER_INNER);

	Proto80100501 respond;
	scene->make_up_self_info(this->role_id(), &respond);
	scene->make_up_other_info(&respond);
	FINER_PROCESS_RETURN(ACTIVE_SM_BATTLE_SCENE_INFO, &respond);
}

int BattleGroundActor::on_enter_battle_scene(int type)
{
	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(this->role_id());
	JUDGE_RETURN(battler != NULL, ERROR_SCENE_NO_EXISTS);

	SMBattleScene* scene = SM_BATTLE_SYSTEM->find_battle_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(scene);
	MapPlayer::enter_scene(type);

	battler->name_ 	= this->role_name();
	battler->sex_  	= this->fight_sex();
	this->set_camp_id(battler->camp_id());

	scene->enter_player(battler);
	this->update_labour_task_info();
	return 0;
}

int BattleGroundActor::on_exit_battle_scene(int type)
{
	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(this->role_id());
	JUDGE_RETURN(battler != NULL, -1);

	SMBattleScene* battle = SM_BATTLE_SYSTEM->find_battle_scene(this->space_id());
	JUDGE_RETURN(battle != NULL, -1);

	battle->exit_player(battler);
	return 0;
}

void BattleGroundActor::notify_quit_trvl()
{
	this->notify_quit_trvl_team(true);
	this->travel_arena_unsign();
	this->travel_marena_unsign();
}

int BattleGroundActor::trvl_script_scene()
{
	return GameEnum::TRVL_SCRIPT_SCENE_ID;
}

int BattleGroundActor::create_trvl_script_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400811*, request, RETURN_TRVL_SCRIPT_CREATE_TEAM);

	int ret = this->validate_player_transfer();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_SCRIPT_CREATE_TEAM, ret);

	Scene* scene = this->fetch_scene();
	CONDITION_NOTIFY_RETURN(scene != NULL && scene->has_quit_team() == false,
			RETURN_TRVL_SCRIPT_CREATE_TEAM, ERROR_SCENE_CREATE_TEAM);

	int scene_id = request->scene_id();
	CONDITION_NOTIFY_RETURN(this->validate_scene_level(scene_id) == true,
			RETURN_TRVL_SCRIPT_CREATE_TEAM, ERROR_PLAYER_LEVEL);

	this->role_detail_.sign_trvl_team_ = true;
	this->notify_quit_normal_team();

	Proto30400502 inner;
	inner.set_type(0);
	inner.set_scene_id(scene_id);
	inner.set_sceret(request->sceret());
	inner.set_limit_force(request->limit_force());
	inner.set_auto_start(request->auto_start());
	inner.set_main_version(CONFIG_INSTANCE->main_version());

	ProtoServer* server = inner.mutable_server_info();
	ProtoTeamer* teamer = inner.mutable_self_info();
	this->role_detail_.make_up_teamer_info(this->role_id(), teamer);
	this->role_detail_.serialize(server);
	return MAP_MONITOR->dispatch_to_scene(this, scene_id, &inner);
}

int BattleGroundActor::add_trvl_script_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400814*, request, RETRUN_TRVL_SCRIPT_ADD_TEAM);

	int ret = this->validate_player_transfer();
	CONDITION_NOTIFY_RETURN(ret == 0, RETRUN_TRVL_SCRIPT_ADD_TEAM, ret);

	Scene* scene = this->fetch_scene();
	CONDITION_NOTIFY_RETURN(scene != NULL && scene->has_quit_team() == false,
			RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_SCENE_CREATE_TEAM);

	int scene_id = request->scene_id();
	CONDITION_NOTIFY_RETURN(this->validate_scene_level(scene_id) == true,
			RETRUN_TRVL_SCRIPT_ADD_TEAM, ERROR_PLAYER_LEVEL);

	this->role_detail_.sign_trvl_team_ = true;
	this->notify_quit_normal_team();

	Proto30400502 inner;
	inner.set_type(1);
	inner.set_scene_id(scene_id);
	inner.set_index(request->index());
	inner.set_sceret(request->screte());
	inner.set_auto_start(request->auto_start());	//自动准备
	inner.set_main_version(CONFIG_INSTANCE->main_version());

	ProtoServer* server = inner.mutable_server_info();
	ProtoTeamer* teamer = inner.mutable_self_info();
	this->role_detail_.make_up_teamer_info(this->role_id(), teamer);
	this->role_detail_.serialize(server);
	return MAP_MONITOR->dispatch_to_scene(this, scene_id, &inner);
}

int BattleGroundActor::fetch_trvl_script_team_list(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400812*, request, RETURN_TRVL_SCRIPT_FETCH_LIST);

	Proto30400503 inner;
	inner.set_type(0);
	inner.set_scene_id(request->scene_id());
	return MAP_MONITOR->dispatch_to_scene(this, this->trvl_script_scene(), &inner);
}

int BattleGroundActor::quit_trvl_script_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400813*, request, RETURN_TRVL_SCRIPT_QUIT_TEAM);
	return this->notify_quit_trvl_team(true);
}

int BattleGroundActor::fetch_trvl_script_team_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400815*, request, RETURN_TRVL_SCRIPT_TEAM_INFO);

	int scene_id = this->trvl_script_scene();

	Proto30400503 inner;
	inner.set_type(2);
	inner.set_scene_id(scene_id);
	return MAP_MONITOR->dispatch_to_scene(this, scene_id, &inner);
}

int BattleGroundActor::start_trvl_script_team(Message* msg)
{
	Proto30400503 inner;
	inner.set_type(3);
	return MAP_MONITOR->dispatch_to_scene(this, this->trvl_script_scene(), &inner);
}

int BattleGroundActor::prep_trvl_script_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400817*, request, RETURN_TRVL_SCRIPT_PREV_OPER);

	Proto30400503 inner;
	inner.set_type(4);
	inner.set_index(request->operate());
	return MAP_MONITOR->dispatch_to_scene(this, this->trvl_script_scene(), &inner);
}

int BattleGroundActor::kick_trvl_script_team(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400818*, request, RETURN_TRVL_SCRIPT_KICK);

	Proto30400503 inner;
	inner.set_type(5);
	inner.set_role(request->role());
	return MAP_MONITOR->dispatch_to_scene(this, this->trvl_script_scene(), &inner);
}

int BattleGroundActor::travel_arena_scene()
{
	return GameEnum::TRVL_ARENA_SCENE_ID;
}

int BattleGroundActor::travel_arena_prep_scene()
{
	return GameEnum::TRVL_ARENA_PREP_SCENE_ID;
}

int BattleGroundActor::fetch_travel_arena_info()
{
//	CONDITION_NOTIFY_RETURN(MAP_MONITOR->is_has_travel_scene() == true,
//			RETURN_TRVL_ARENA_INFO, ERROR_CLIENT_OPERATE);

	Proto50400821 respond;
	respond.set_stage(this->tarena_detail_.stage_);
	respond.set_score(this->tarena_detail_.score_);
	respond.set_get_expolit(this->tarena_detail_.get_exploit_);
	respond.set_attend_times(this->tarena_detail_.attend_times_);
	respond.set_draw_flag(this->tarena_detail_.draw_flag_);
	respond.set_win_times(this->tarena_detail_.win_times_);

	IntPair pair = TRVL_ARENA_MONITOR->fighting_state();
	respond.set_state(pair.first);
	respond.set_left_time(pair.second);

	int left_time = this->tarena_detail_.adjust_tick_ - ::time(NULL);
	respond.set_left_day(std::max<int>(0, left_time));

	for (IntMap::iterator iter = this->tarena_detail_.draw_win_.begin();
			iter != this->tarena_detail_.draw_win_.end(); ++iter)
	{
		respond.add_draw_win(iter->first);
	}

	this->sync_restore_info(GameEnum::ES_ACT_TRVL_ARENA, this->tarena_detail_.stage_, 0);
	FINER_PROCESS_RETURN(RETURN_TRVL_ARENA_INFO, &respond);
}

int BattleGroundActor::travel_arena_sign()
{
	CONDITION_NOTIFY_RETURN(TRVL_ARENA_MONITOR->is_fighting_time() == true,
			RETURN_TRVL_ARENA_SIGN, ERROR_GUARD_TIME);

	TrvlArenaRole* tarena_role = TRVL_ARENA_MONITOR->find_and_pop(this->role_id());
	CONDITION_NOTIFY_RETURN(tarena_role != NULL, RETURN_TRVL_ARENA_SIGN,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(tarena_role->start_tick_ <= ::time(NULL),
			RETURN_TRVL_ARENA_SIGN, ERROR_OPERATE_TOO_FAST);

	CONDITION_NOTIFY_RETURN(tarena_role->state_ == TrvlArenaRole::STATE_NONE,
			RETURN_TRVL_ARENA_SIGN, ERROR_TARENA_FIGHTING);

	TRVL_ARENA_MONITOR->register_tarena_role(this);
	FINER_PROCESS_NOTIFY(RETURN_TRVL_ARENA_SIGN);
}

int BattleGroundActor::travel_arena_rank(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400823*, request, RETURN_TRVL_ARENA_RANK);

	Proto30400505 inner;
	inner.set_type(1);
	inner.set_value(request->page());
	return MAP_MONITOR->dispatch_to_scene(this, this->travel_arena_prep_scene(), &inner);
}

int BattleGroundActor::travel_arena_draw()
{
	CONDITION_NOTIFY_RETURN(this->tarena_detail_.draw_flag_ == false,
			RETURN_TRVL_ARENA_DRAW, ERROR_CLIENT_OPERATE);

	const Json::Value& conf = this->tarena_detail_.conf();
	this->request_add_reward(conf["reward"].asInt(), SerialObj(
			ADD_FROM_TRAVEL_ARENA_DAY, this->tarena_detail_.stage_));

	this->tarena_detail_.draw_flag_ = true;
	this->check_tarena_pa_event(0);
	this->sync_restore_info(GameEnum::ES_ACT_TRVL_ARENA, this->tarena_detail_.stage_, 1);

	FINER_PROCESS_NOTIFY(RETURN_TRVL_ARENA_DRAW);
}

int BattleGroundActor::travel_arena_unsign()
{
	JUDGE_RETURN(this->travel_arena_prep_scene() == this->scene_id(), -1);

	TRVL_ARENA_MONITOR->unsign(0, this->role_id());
	FINER_PROCESS_NOTIFY(RETURN_TRVL_ARENA_UNSIGN);
}

int BattleGroundActor::request_enter_travel_arena()
{
	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_TRAVEL_ARENA);
	enter_info.set_main_version(CONFIG_INSTANCE->main_version());

	int ret = this->send_request_enter_info(this->travel_arena_prep_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_ARENA_ENTER, ret);

	return 0;
}

int BattleGroundActor::travel_arena_draw_win(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400827*, request, RETURN_TRVL_ARENA_WIN_DRAW);

	const Json::Value& win_reward = CONFIG_INSTANCE->scene(this->travel_arena_scene()
			)["win_times_reward"];
	CONDITION_NOTIFY_RETURN(win_reward.empty() == false, RETURN_TRVL_ARENA_WIN_DRAW,
			ERROR_CLIENT_OPERATE);

	uint index = request->index();
	CONDITION_NOTIFY_RETURN(index < win_reward.size(), RETURN_TRVL_ARENA_WIN_DRAW,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->tarena_detail_.draw_win_.count(index) == 0,
			RETURN_TRVL_ARENA_WIN_DRAW, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->tarena_detail_.win_times_ >= win_reward[index][0u].asInt(),
			RETURN_TRVL_ARENA_WIN_DRAW, ERROR_CLIENT_OPERATE);

	this->tarena_detail_.draw_win_[index] = true;
	this->request_add_reward(win_reward[index][1u].asInt(), ADD_FROM_TARENA_WIN_TIMES);
	this->check_tarena_pa_event(1);

	Proto50400827 respond;
	respond.set_index(index);
	FINER_PROCESS_RETURN(RETURN_TRVL_ARENA_WIN_DRAW, &respond);
}

int BattleGroundActor::on_enter_prep_tarena_scene(int type)
{
	this->init_mover_scene(TARENA_PREP_SCENE);
	return MapPlayer::enter_scene(type);
}

int BattleGroundActor::on_exit_prep_tarena_scene(int type)
{
	return 0;
}

int BattleGroundActor::on_enter_travel_arena_scene(int type)
{
	TrvlArenaRole* role = TRVL_ARENA_MONITOR->find_role(this->role_id());
	JUDGE_RETURN(role != NULL, ERROR_SCENE_NO_EXISTS);

	TrvlArenaScene* scene = TRVL_ARENA_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(scene);
	MapPlayer::enter_scene(type);

	this->tarena_detail_.attend_times_ += 1;
	this->set_camp_id(role->pos_index_ + 1);
	return 0;
}

int BattleGroundActor::on_exit_travel_arena_scene(int type)
{
	TrvlArenaRole* role = TRVL_ARENA_MONITOR->find_role(this->role_id());
	JUDGE_RETURN(role != NULL, -1);

	role->start_tick_ = ::time(NULL);
	return 0;
}

int BattleGroundActor::have_tarena_win_reward()
{
	JUDGE_RETURN(this->tarena_detail_.win_times_ > 0, false);

	const Json::Value& win_reward = CONFIG_INSTANCE->scene(
			this->travel_arena_scene())["win_times_reward"];

	for (uint i = 0; i < win_reward.size(); ++i)
	{
		int need_times = win_reward[i][0u].asInt();
		JUDGE_RETURN(this->tarena_detail_.win_times_ >= need_times, false);
		JUDGE_CONTINUE(this->tarena_detail_.draw_win_.count(i) == 0);
		return true;
	}

	return false;
}



void BattleGroundActor::check_tarena_pa_event(int type)
{
	static int enter_level = CONFIG_INSTANCE->scene_set(
			this->travel_arena_scene())["enter_level"].asInt();
	JUDGE_RETURN(this->level() >= enter_level, ;);

	if (type == 0)
	{
		MAP_MONITOR->inner_notify_player_assist_event(this,
				GameEnum::PA_EVENT_TARENA_STAGE_REWARD,
				this->tarena_detail_.draw_flag_ == false);
	}
	else if (type == 1)
	{
		int ret = this->have_tarena_win_reward();
		MAP_MONITOR->inner_notify_player_assist_event(this,
				GameEnum::PA_EVENT_TARENA_WIN_REWARD, ret);
	}
}

void BattleGroundActor::update_tarena_score(int score, int exploit, TrvlArenaRole* arena)
{
	this->update_tarena_score_i(score);
	this->tarena_detail_.get_exploit_ += exploit;

	this->record_other_serial(MAIN_TARENA_STAGE, 0, score,
			this->tarena_detail_.stage_, this->tarena_detail_.score_);
	JUDGE_RETURN(arena != NULL, ;);

	arena->stage_ = this->tarena_detail_.stage_;
	arena->score_ = this->tarena_detail_.score_;
	arena->update_tick_ = ::time(NULL);
	JUDGE_RETURN(this->tarena_detail_.win_times_ != arena->win_times_, ;);

	this->tarena_detail_.win_times_ = arena->win_times_;
	this->check_tarena_pa_event(1);
}

void BattleGroundActor::update_tarena_score_i(int score)
{
	int flag = this->tarena_detail_.update(score);
	JUDGE_RETURN(flag == true, ;);
	this->update_tarena_open_activity();
}

void BattleGroundActor::update_tarena_open_activity()
{
	int TYPE = 1;	//BackSetActDetail::F_ACT_AIM_CHASE
	int OPEN_ACTIVITY_DAY = 6;

	SubObj sub(OPEN_ACTIVITY_DAY, this->tarena_detail_.stage_);
	this->sync_open_activity_info(TYPE, sub);
}

void BattleGroundActor::update_labour_task_info()
{
	Proto31403201 task_info;
	task_info.set_task_id(GameEnum::LABOUR_TASK_BROCAST_BATTLEGROUND);
	task_info.set_task_finish_count(1);
	MAP_MONITOR->dispatch_to_logic(this, &task_info);
}

void BattleGroundActor::tarena_month_settle()
{
	JUDGE_RETURN(this->tarena_detail_.adjust_tick_ <= ::time(NULL), ;);

	this->tarena_detail_.adjust_tick_ = GameCommon::next_month_start_zero();
	JUDGE_RETURN(this->tarena_detail_.score_ > 0, ;);

	const Json::Value& cur_conf = this->tarena_detail_.conf();
	JUDGE_RETURN(cur_conf.empty() == false, ;);

	int adjust_point = cur_conf["adjust_point"].asInt();
	JUDGE_RETURN(adjust_point > 0, ;);

	int last_score = this->tarena_detail_.score_;
	int season_reward = cur_conf["season_reward"].asInt();

	this->tarena_detail_.score_ = adjust_point;
	this->update_tarena_score(0);

	int mail_id = CONFIG_INSTANCE->const_set("tarena_season_mail");
	MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);

	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), cur_conf["name"].asCString(), last_score,
			CONFIG_INSTANCE->tarena_state_name(this->tarena_detail_.stage_).c_str(),
			this->tarena_detail_.score_);

	mail_info->add_goods(season_reward);
	GameCommon::request_save_mail_content(this->role_id(), mail_info);
}

int BattleGroundActor::request_enter_travel_marena()
{
	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_TM_ARENA);
	enter_info.set_main_version(CONFIG_INSTANCE->main_version());

	int ret = this->send_request_enter_info(GameEnum::TRVL_MARENA_PREP_SCENE_ID, enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_MARENA_ENTER, ret);

	return 0;
}

int BattleGroundActor::travel_marena_sign()
{
	CONDITION_NOTIFY_RETURN(this->scene_id() == GameEnum::TRVL_MARENA_PREP_SCENE_ID,
			RETURN_TRVL_MARENA_SIGN, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(TRVL_MARENA_MONITOR->is_fighting_time() == true
			&& TRVL_MARENA_MONITOR->is_enter_time() == true,
			RETURN_TRVL_MARENA_SIGN, ERROR_GUARD_TIME);

	TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_and_pop(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_TRVL_MARENA_SIGN,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(arena_role->start_tick_ <= ::time(NULL),
			RETURN_TRVL_ARENA_SIGN, ERROR_OPERATE_TOO_FAST);

	CONDITION_NOTIFY_RETURN(arena_role->fight_state_ == 0,
			RETURN_TRVL_MARENA_SIGN, ERROR_TARENA_FIGHTING);

	TRVL_MARENA_MONITOR->register_role(this);
	FINER_PROCESS_NOTIFY(RETURN_TRVL_MARENA_SIGN);
}

int BattleGroundActor::travel_marena_unsign()
{
	JUDGE_RETURN(this->scene_id() == GameEnum::TRVL_MARENA_PREP_SCENE_ID, -1);

	TRVL_MARENA_MONITOR->unsign(this->role_id());
	FINER_PROCESS_NOTIFY(RETURN_TRVL_MARENA_UNGISN);
}

int BattleGroundActor::travel_marena_info()
{
	CONDITION_NOTIFY_RETURN(this->scene_id() == GameEnum::TRVL_MARENA_PREP_SCENE_ID,
			RETURN_TRVL_MAREAN_INFO, ERROR_CLIENT_OPERATE);

	Proto50400834 respond;
	respond.set_win_reward(TRVL_MARENA_MONITOR->drop_reward_[1]);
	respond.set_lose_reward(TRVL_MARENA_MONITOR->drop_reward_[0]);

	IntPair pair = TRVL_MARENA_MONITOR->fighting_state();
	respond.set_state(pair.first);
	respond.set_left_time(pair.second);

	TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(this->role_id());
	if (arena_role != NULL)
	{
		respond.set_score(arena_role->score_);
		respond.set_attends(arena_role->attend_times_);
	}

	TMArenaRole* first_role = TRVL_MARENA_MONITOR->first_role();
	if (first_role != NULL)
	{
		respond.set_max_name(first_role->name_);
		respond.set_max_score(first_role->score_);
	}

	FINER_PROCESS_RETURN(RETURN_TRVL_MAREAN_INFO, &respond);
}

int BattleGroundActor::travel_marena_rank_begin()
{
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_MARENA_PREP_SCENE_ID,
			INNER_MAP_TMARENA_RANK_INFO);
}

int BattleGroundActor::on_enter_prep_tmarena_scene(int type)
{
	this->init_mover_scene(TMARENA_PREP);
	return MapPlayer::enter_scene(type);
}

int BattleGroundActor::on_enter_travel_marena_scene(int type)
{
	TMArenaScene* scene = TRVL_MARENA_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(scene);
	MapPlayer::enter_scene(type);

	this->set_camp_id(scene->fetch_camp_id(this->role_id()));
	return 0;
}

int BattleGroundActor::on_exit_travel_marena_scene(int type)
{
	TMArenaRole* arena_role = TRVL_MARENA_MONITOR->find_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, -1);

	arena_role->start_tick_ = ::time(NULL);
	return 0;
}

void BattleGroundActor::update_tmarena_score(int score, int flag, TMArenaRole* arena_role)
{
//	arena_role->score_ += score;
//	arena_role->update_tick_ = ::time(NULL);
//
//	arena_role->cur_score_ += score;
//	arena_role->win_times_ += flag;
//	JUDGE_RETURN(arena_role->attend_times_ <= TRVL_MARENA_MONITOR->drop_reward_times_, ;);
//
//	arena_role->drop_reward_ = TRVL_MARENA_MONITOR->drop_reward_[flag];
//	this->request_add_reward(arena_role->drop_reward_, ADD_FROM_TMARENA_FINISH);
}

TArenaDetail& BattleGroundActor::tarena_detail()
{
	return this->tarena_detail_;
}

TMarenaDetail& BattleGroundActor::tmarena_detail()
{
	return this->tmarena_detail_;
}

int BattleGroundActor::travel_battle_scene(void)
{
    return GameEnum::TRVL_BATTLE_SCENE_ID;
}

int BattleGroundActor::request_enter_travel_battle(void)
{
    Proto30400051 enter_info;
    enter_info.set_enter_type(GameEnum::ET_TRVL_BATTLE);
    enter_info.set_main_version(CONFIG_INSTANCE->main_version());

    int ret = this->send_request_enter_info(this->travel_battle_scene(), enter_info);
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_BATTLE_ENTER, ret);

    return 0;
}

int BattleGroundActor::request_tbattle_last_rank_list(Message* msg)
{
    DYNAMIC_CAST_RETURN(Proto10401103 *, request, msg, -1);

    Proto30402501 inner_req;
    inner_req.set_type(1);
    inner_req.set_refresh(request->refresh());
    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int BattleGroundActor::request_tbattle_history_top_list(Message* msg)
{
    DYNAMIC_CAST_RETURN(Proto10401104 *, request, msg, -1);

    Proto30402501 inner_req;
    inner_req.set_type(2);
    inner_req.set_refresh(request->refresh());
    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int BattleGroundActor::request_tbattle_view_player_info(Message* msg)
{
    DYNAMIC_CAST_RETURN(Proto10401105 *, request, msg, -1);

    Proto30402502 inner_req;
    inner_req.set_role_id(request->role_id());
    inner_req.set_type(request->type());
    inner_req.set_rank(request->rank());
    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int BattleGroundActor::request_tbattle_cur_rank_list(Message* msg)
{
    DYNAMIC_CAST_RETURN(Proto10401106 *, request, msg, -1);

    Proto30402501 inner_req;
    inner_req.set_type(3);
    inner_req.set_refresh(request->refresh());
    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int BattleGroundActor::request_tbattle_indiana_list(Message* msg)
{
    DYNAMIC_CAST_RETURN(Proto10401107 *, request, msg, -1);

    Proto30402501 inner_req;
    inner_req.set_type(4);
    inner_req.set_refresh(request->refresh());
    return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
}

int BattleGroundActor::on_enter_tbattle_scene(int type)
{
	TrvlBattleScene* scene = TRVL_BATTLE_MONITOR->trvl_battle_scene_by_floor(this->space_id());
	JUDGE_RETURN(scene != NULL, ERROR_SCENE_NO_EXISTS);

    this->notify_tbattle_left_pannel();

    this->tbattle_timeout_tick_ = Time_Value::zero;
    this->tbattle_score_timeout_ = Time_Value::zero;
//    {
//        IntPair value_pair = this->interval_score_by_floor();
//        if (value_pair.first <= 0)
//            value_pair.first = 10;
//        this->tbattle_score_timeout_ = Time_Value::gettimeofday() + Time_Value(value_pair.first);
//    }
	this->init_mover_scene(scene);
	return MapPlayer::enter_scene(type);
}

int BattleGroundActor::on_exit_tbattle_scene(int type)
{
    // 夺宝持有者下线时马上生成新的秘宝
    BasicStatus *status = NULL;
    if (this->find_status(BasicStatus::TBATTLE_TREASURE, status) == 0)
    {
    	this->remove_status(status);
        IntSet remove_set = this->tbattle_treasure_buff_set();
        this->tbattle_treasure_buff_set().clear();
        for (IntSet::iterator iter = remove_set.begin();
                iter != remove_set.end(); ++iter)
        {
            this->remove_status(*iter);
        }

        TRVL_BATTLE_MONITOR->set_treasure_info(0, "", Time_Value::zero);

        TrvlBattleScene *scene = dynamic_cast<TrvlBattleScene *>(this->fetch_scene());
        if (scene != NULL)
        {
            scene->generate_treasure_sort_immediate();
        }
    }
    return 0;
}

int BattleGroundActor::need_kill_amount_to_next_floor(const int floor)
{
    const Json::Value &killamount_up_json = CONFIG_INSTANCE->tbattle_reward()["killamount_up"];
    if (0 < floor && floor <= int(killamount_up_json.size()))
        return killamount_up_json[floor - 1].asInt();
    return 0;
}

int BattleGroundActor::notify_tbattle_left_pannel(void)
{
    JUDGE_RETURN(this->scene_id() == GameEnum::TRVL_BATTLE_SCENE_ID, 0);

    TrvlBattleRole *tb_role = TRVL_BATTLE_MONITOR->find_tb_role(this->role_id());
    TrvlBattleRanker *tb_ranker = TRVL_BATTLE_MONITOR->find_tb_ranker(this->role_id());

    Proto80401101 respond;
    respond.set_left_sec(TRVL_BATTLE_MONITOR->left_actvity_sec());
    respond.set_cur_floor(this->space_id());
    respond.set_max_floor(this->space_id());
    respond.set_need_kill_amount(this->need_kill_amount_to_next_floor(this->space_id()));
    if (tb_role != NULL)
    {
        respond.set_kill_amount(tb_role->__cur_floor_kill_amount);
        respond.set_award_score(tb_role->next_award_score());
        respond.set_max_floor(tb_role->__max_floor);
    }
    if (tb_ranker != NULL)
    {
        respond.set_score(tb_ranker->__score);
    }

    TrvlBattleMonitor *monitor = TRVL_BATTLE_MONITOR;
    respond.set_first_top_id(monitor->first_top_id());
    respond.set_first_top_name(monitor->first_top_name());
    respond.set_treasure_owner_id(monitor->treasure_owner_id());
    respond.set_treasure_owner_name(monitor->treasure_owner_name());
    respond.set_treasure_left_sec(monitor->treasure_left_sec());

    return this->respond_to_client(ACTIVE_TRVL_BATTLE_LEFT_PANNEL, &respond);
}

int BattleGroundActor::notify_tbattle_enter_next_floor(void)
{
    Proto80401102 respond;
    respond.set_cur_floor(this->space_id());
    return this->respond_to_client(ACTIVE_TRVL_BATTLE_NEXT_FLOOR, &respond);
}

int BattleGroundActor::notify_tbattle_fallback_floor(void)
{
    Proto80401103 respond;
    respond.set_cur_floor(this->space_id());
    return this->respond_to_client(ACTIVE_TRVL_BATTLE_BACK_FLOOR, &respond);
}

int BattleGroundActor::notify_tbattle_finish_info(void)
{
    Proto80401104 respond;

    TrvlBattleRanker *ranker_info = NULL;
    for (int i = 1; i <= 3; ++i)
    {
        ranker_info = TRVL_BATTLE_MONITOR->find_tb_ranker_by_rank(i);
        if (ranker_info != NULL)
        {
            ProtoTrvlBattleRank *proto_rank = respond.add_rank_list();
            ranker_info->serialize(proto_rank);
        }
    }

    ranker_info = TRVL_BATTLE_MONITOR->find_tb_ranker(this->role_id());
    if (ranker_info != NULL)
    {
        respond.set_self_rank(ranker_info->__rank);
        respond.set_self_kill(ranker_info->__total_kill_amount);
        respond.set_self_score(ranker_info->__score);
    }
    return this->respond_to_client(ACTIVE_TRVL_BATTLE_RESULT, &respond);
}

int BattleGroundActor::check_tbattle_timeout(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();

    if (this->tbattle_score_timeout_ == Time_Value::zero)
    {
        IntPair value_pair = this->interval_score_by_floor();
        if (value_pair.first <= 0)
            value_pair.first = 10;
        this->tbattle_score_timeout_ = Time_Value::gettimeofday() + Time_Value(value_pair.first);
        //this->notify_tbattle_left_pannel();
        return 0;
    }

    if (this->tbattle_score_timeout_ <= nowtime)
    {
        // 此处一定要10秒
        this->tbattle_interval_inc_score();
        this->notify_tbattle_left_pannel();
    }

    return 0;
}

int BattleGroundActor::process_tbattle_relive(const int relive_mode, MoverCoord &relive_coord)
{
    if (relive_mode == GameEnum::RELIVE_CONFIG_POINT)
    {
    	const Json::Value &relive_json = this->scene_conf()["relive"];
    	double percent = relive_json["state"][0u].asInt() / 100.0;
        this->fighter_restore_all(FIGHT_TIPS_RELIVE, 0, percent);
        this->team_notify_teamer_blood();

        TrvlBattleScene *scene = dynamic_cast<TrvlBattleScene *>(this->fetch_scene());
        if (scene != NULL)
        {
            scene->handle_back_floor_when_die(this);
            this->insert_protect_buff();
        }
        else
        	MapPlayer::process_relive(relive_mode, relive_coord);
        return 0;
    }
    
    return MapPlayer::process_relive(relive_mode, relive_coord);
}

IntSet &BattleGroundActor::tbattle_treasure_buff_set(void)
{
    return this->tbattle_treasure_buff_set_;
}

void BattleGroundActor::refresh_treasure_prop_buff(const double interval, const double last)
{
	JUDGE_RETURN(last > 0.000001, ;);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, ;);

    IntSet remove_set = this->tbattle_treasure_buff_set();
    this->tbattle_treasure_buff_set().clear();
    for (IntSet::iterator iter = remove_set.begin();
            iter != remove_set.end(); ++iter)
    {
    	JUDGE_CONTINUE(*iter != BasicStatus::TBATTLE_TREASURE);
        this->remove_status(*iter);
    }
    
    int buff_idx = CONFIG_INSTANCE->tbattle_player_amount_to_buff_idx(scene->player_map().size());
    const Json::Value &buff_effect_json = CONFIG_INSTANCE->tbattle_buff(buff_idx);
    if (buff_effect_json != Json::Value::null)
    {
        const Json::Value &buff_id_list_json = buff_effect_json["buff_id"];
        for (uint i = 0; i < buff_id_list_json.size(); ++i)
        {
            int buff_id = buff_id_list_json[i].asInt();
            JUDGE_CONTINUE(buff_id != BasicStatus::TBATTLE_TREASURE);

            const Json::Value &buff_json = CONFIG_INSTANCE->buff(buff_id);
            JUDGE_CONTINUE(buff_json != Json::Value::null);

            this->tbattle_treasure_buff_set().insert(buff_id);

            this->insert_defender_status(this, buff_id, 0, last, 0.0,
                    buff_json["value"].asDouble(), buff_json["percent"].asDouble());
        }
    }
}

int BattleGroundActor::inc_tbattle_treasure_status_effect(BasicStatus *status, const int enter_type, const int refresh_type)
{
    this->refresh_treasure_prop_buff(status->__interval.sec(), status->__last_tick.sec());
    return 0;
}

int BattleGroundActor::process_tbattle_treasure_status_timeout(BasicStatus *status)
{
	JUDGE_RETURN(TRVL_BATTLE_MONITOR->is_started_battle_activity() == true, 0);
    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    int treasure_reward_score = tbattle_reward_json["treasure_reward_score"].asInt();
    TRVL_BATTLE_MONITOR->inc_score_only(this, treasure_reward_score);
    TRVL_BATTLE_MONITOR->send_treasure_timeout_reward(this);
    TRVL_BATTLE_MONITOR->set_treasure_info(0, "", Time_Value::zero);

    TrvlBattleScene *scene = dynamic_cast<TrvlBattleScene *>(this->fetch_scene());
    if (scene != NULL)
    {
        scene->generate_treasure_sort_immediate();
    }
    return 0;
}

int BattleGroundActor::send_trvl_reward_id(const int reward_id, const SerialObj &serial_obj, const int mail_id)
{
    Proto30402505 inner_req;
    inner_req.set_reward_id(reward_id);
    serial_obj.serialize(inner_req.mutable_serial());
    inner_req.set_mail_id(mail_id);

    return this->send_to_logic_thread(inner_req);
}

IntPair BattleGroundActor::interval_score_by_floor(void)
{
    IntPair value_pair;
    int floor = this->space_id();
    const Json::Value &interval_reward_score_json = CONFIG_INSTANCE->tbattle_reward()["interval_reward_score"];
    if (0 < floor && floor <= int(interval_reward_score_json.size()))
    {
        value_pair.first = interval_reward_score_json[floor - 1][0u].asInt();
        value_pair.second = interval_reward_score_json[floor - 1][1u].asInt();
    }
    return value_pair;
}

void BattleGroundActor::tbattle_interval_inc_score(void)
{
	JUDGE_RETURN(TRVL_BATTLE_MONITOR->is_started_battle_activity(), ;);
    IntPair value_pair = this->interval_score_by_floor();

    TRVL_BATTLE_MONITOR->inc_score_only(this, value_pair.second);

    this->tbattle_score_timeout_ = Time_Value::gettimeofday() + Time_Value(value_pair.first);
}

int BattleGroundActor::process_tbattle_obtain_area_info(const int request_flag)
{
    return this->notify_tbattle_left_pannel();

    return 0;
}

int BattleGroundActor::sm_battler_die_process(Int64 fighter_id)
{
	Int64 real_fighter = this->fetch_benefited_attackor_id(fighter_id);
    JUDGE_RETURN(real_fighter != this->role_id(), -1);

	SMBattleScene* scene = SM_BATTLE_SYSTEM->find_battle_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	scene->handle_sm_kill(real_fighter, this->role_id());
	return 0;
}

int BattleGroundActor::travel_arena_die_process(Int64 fighter_id)
{
	TrvlArenaScene* scene = TRVL_ARENA_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	scene->handle_arena_finish(this->role_id());
	return 0;
}


int BattleGroundActor::travel_marena_die_process(Int64 fighter_id)
{
	TMArenaScene* scene = TRVL_MARENA_MONITOR->find_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	Int64 killer_id = this->fetch_benefited_attackor_id(fighter_id);
	scene->handle_marena_player_die(killer_id, this->mover_id());
	return 0;
}

int BattleGroundActor::travel_battle_die_process(Int64 fighter_id)
{
    TrvlBattleScene* scene = TRVL_BATTLE_MONITOR->trvl_battle_scene_by_floor(this->space_id());
    JUDGE_RETURN(scene != NULL, -1);

    Int64 killer_player_id = this->fetch_benefited_attackor_id(fighter_id);
    scene->handle_trvl_battle_die_player(this->role_id(), killer_player_id);
    return 0;
}
