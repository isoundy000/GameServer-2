/*
 * ArenaLPlayer.cpp
 *
 *  Created on: Aug 18, 2014
 *      Author: peizhibi
 */

#include "ArenaSys.h"
#include "ArenaLPlayer.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

ArenaLPlayer::ArenaLPlayer()
{
	// TODO Auto-generated constructor stub
}

ArenaLPlayer::~ArenaLPlayer()
{
	// TODO Auto-generated destructor stub
}

void ArenaLPlayer::reset_arena()
{
//	this->arena_notify_timer_.cancel_timer();

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, ;);

	arena_role->notify_flag_ = false;
	arena_role->last_view_tick_ = 0;
}

void ArenaLPlayer::reset_arena_everyday()
{
	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(area_role != NULL, ;);
	area_role->reset_everyday();
}

int ArenaLPlayer::fetch_arena_rank_list()
{
	Proto50101009 respond;
	for (int i = 1; i < GameEnum::ARENA_RANK_NUM; ++i)
	{
		ArenaRole* rank_role = ARENA_SYS->arena_rank_role(i);
		JUDGE_CONTINUE(rank_role != NULL);

		ProtoAreaRole* proto_role = respond.add_rank_list();
		JUDGE_CONTINUE(proto_role != NULL);

		proto_role->set_rank(rank_role->rank_);
		proto_role->set_id(rank_role->id_);
		proto_role->set_name(rank_role->name_);
		proto_role->set_sex(rank_role->sex_);
		proto_role->set_force(rank_role->force_);
		proto_role->set_level(rank_role->level_);
		proto_role->set_career(rank_role->career_);
		proto_role->set_wing_level(rank_role->wing_level_);
		proto_role->set_solider_level(rank_role->solider_level_);

		proto_role->set_fashion_clothes(rank_role->fashion_clothes_);
		proto_role->set_fashion_weapon(rank_role->fashion_weapon_);
	}

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	if (arena_role != NULL)
	{
		respond.set_player_rank(arena_role->rank_);
		respond.set_player_force(this->role_detail().__fight_force);
	}
	else
	{
		respond.set_player_rank(-1);
		respond.set_player_force(-1);
	}
	FINER_PROCESS_RETURN(RETURN_ARENA_RANK_LIST, &respond);
}

int ArenaLPlayer::refresh_arena_player()
{
	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, 0);

	ARENA_SYS->set_fighter_set(arena_role);

	Proto50101008 area_info;

	for (IntVec::iterator iter = arena_role->fight_set_.begin();
				iter != arena_role->fight_set_.end(); ++iter)
	{
		ArenaRole* rank_role = ARENA_SYS->arena_rank_role(*iter);
		JUDGE_CONTINUE(rank_role != NULL);

		rank_role->rank_ = *iter;
		ProtoAreaRole* proto_role = area_info.add_role_list();
		JUDGE_CONTINUE(proto_role != NULL);

		proto_role->set_rank(rank_role->rank_);
		proto_role->set_id(rank_role->id_);
		proto_role->set_name(rank_role->name_);
		proto_role->set_sex(rank_role->sex_);
		proto_role->set_force(rank_role->force_);
		proto_role->set_level(rank_role->level_);
		proto_role->set_career(rank_role->career_);

		proto_role->set_fashion_clothes(rank_role->fashion_clothes_);
		proto_role->set_fashion_weapon(rank_role->fashion_weapon_);

//		proto_role->set_weapon(rank_role->weapon_);
//		proto_role->set_clothes(rank_role->clothes_);
//		proto_role->set_fashion_weapon(rank_role->fashion_weapon_);
//		proto_role->set_fashion_clothes(rank_role->fashion_clothes_);
//		proto_role->set_wing_level(rank_role->wing_level_);
//		proto_role->set_solider_level(rank_role->solider_level_);
	}

	FINER_PROCESS_RETURN(RETURN_ARENA_REFRESH_PLAYER, &area_info);
}

int ArenaLPlayer::fetch_arena_info()
{
	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_ARENA_FETCH_INFO,
			ERROR_CLIENT_OPERATE);

	arena_role->notify_flag_ = false;
	arena_role->last_view_tick_ = ::time(NULL);

	if (arena_role->open_flag_ == false)
	{
		return this->fetch_guide_arena_info(arena_role, this->vip_type());
	}
	else
	{
		return this->fetch_normal_arena_info(arena_role, arena_role->open_flag_);
	}
}

int ArenaLPlayer::fetch_guide_arena_info(ArenaRole* arena_role, const int vip_type)
{
//	int rank = arena_role->rank_ + 1;

	Proto50101001 area_info;
	area_info.set_rank(-1);
	area_info.set_next_time(ARENA_SYS->fetch_next_timeout());

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank(arena_role->rank_);
	JUDGE_RETURN(rank_json != Json::Value::null, 0);

	area_info.set_reward_id(rank_json["reward_id"].asInt());
	int i = 0;
	for (i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank = CONFIG_INSTANCE->athletics_rank_by_id(i);
		if (rank["rank_num"].asInt() >= arena_role->rank_) continue;

		area_info.set_next_reward_id(rank["reward_id"].asInt());
		break;
	}
	if (arena_role->rank_ == 1) area_info.set_next_reward_id(rank_json["reward_id"].asInt());
	area_info.set_is_skip(arena_role->is_skip_);
	area_info.set_sex(arena_role->sex_);
	area_info.set_force(this->role_detail().__fight_force);
	area_info.set_career(arena_role->career_);

	area_info.set_left_times(arena_role->left_times_);
	area_info.set_have_reward(arena_role->have_reward());

	area_info.set_left_cool_time(arena_role->left_cool_time());

	int rank_num = arena_role->rank_;
	for (int i = 0 ; i < 3; ++i)
	{
		if (i != 0 ) rank_num += ::rand() % 100 + 1;
		Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(rank_num)["rpm_id"].asInt();
		Int64 robot_id = ARENA_SYS->fetch_detail()->robot_id_map[rpm_id];
		ArenaRole& rank_role = ARENA_SYS->fetch_detail()->arena_role_set_[robot_id];

		ProtoAreaRole* proto_role = area_info.add_area_role();
		JUDGE_CONTINUE(proto_role != NULL);

		proto_role->set_rank(rank_num);
		proto_role->set_id(rank_role.id_);
		proto_role->set_name(rank_role.name_);
		proto_role->set_sex(rank_role.sex_);
		proto_role->set_force(rank_role.force_);
		proto_role->set_level(rank_role.level_);
		proto_role->set_career(rank_role.career_);
		proto_role->set_wing_level(rank_role.wing_level_);
		proto_role->set_solider_level(rank_role.solider_level_);

		proto_role->set_fashion_clothes(rank_role.fashion_clothes_);
		proto_role->set_fashion_weapon(rank_role.fashion_weapon_);
	}

	FINER_PROCESS_RETURN(RETURN_ARENA_FETCH_INFO, &area_info);
}

int ArenaLPlayer::fetch_normal_arena_info(ArenaRole* arena_role, int open_flag)
{
	JUDGE_RETURN(arena_role != NULL, 0);

//	if (open_flag > 0)
	ARENA_SYS->set_fighter_set(arena_role);
//	else
//		ARENA_SYS->set_first_fighter_set(arena_role);

	Proto50101001 area_info;
	area_info.set_rank(arena_role->rank_);
	area_info.set_next_time(ARENA_SYS->fetch_next_timeout());

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank(arena_role->rank_);
	JUDGE_RETURN(rank_json != Json::Value::null, 0);

	area_info.set_reward_id(rank_json["reward_id"].asInt());
	for (int i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank = CONFIG_INSTANCE->athletics_rank_by_id(i);
		if (rank["rank_num"].asInt() >= arena_role->rank_) continue;

		area_info.set_next_reward_id(rank["reward_id"].asInt());
		break;
	}

	if (arena_role->rank_ == 1) area_info.set_next_reward_id(rank_json["reward_id"].asInt());
	area_info.set_is_skip(arena_role->is_skip_);
	area_info.set_sex(arena_role->sex_);
	area_info.set_force(this->role_detail().__fight_force);
	area_info.set_career(arena_role->career_);

	area_info.set_left_times(arena_role->left_times_);
	area_info.set_buy_times(arena_role->buy_times_);
	area_info.set_have_reward(arena_role->have_reward());

	area_info.set_left_cool_time(arena_role->left_cool_time());
	area_info.set_is_over_limit(arena_role->is_over_limit_);

	for (IntVec::iterator iter = arena_role->fight_set_.begin();
			iter != arena_role->fight_set_.end(); ++iter)
	{
		ArenaRole* rank_role = ARENA_SYS->arena_rank_role(*iter);
		JUDGE_CONTINUE(rank_role != NULL);

		rank_role->rank_ = *iter;

		ProtoAreaRole* proto_role = area_info.add_area_role();
		JUDGE_CONTINUE(proto_role != NULL);

		proto_role->set_rank(rank_role->rank_);
		proto_role->set_id(rank_role->id_);
		proto_role->set_name(rank_role->name_);
		proto_role->set_sex(rank_role->sex_);
		proto_role->set_force(rank_role->force_);
		proto_role->set_level(rank_role->level_);
		proto_role->set_career(rank_role->career_);
		proto_role->set_wing_level(rank_role->wing_level_);
		proto_role->set_solider_level(rank_role->solider_level_);
		proto_role->set_fashion_clothes(rank_role->fashion_clothes_);
		proto_role->set_fashion_weapon(rank_role->fashion_weapon_);
	}

	Int64 now_tick = ::time(NULL);
	for (std::list<ArenaRole::Record>::reverse_iterator
			iter = arena_role->his_record_.rbegin();
			iter != arena_role->his_record_.rend(); ++iter)
	{
		ProtoAreaRecord* proto_record = area_info.add_area_record();
		JUDGE_CONTINUE(proto_record != NULL);

		proto_record->set_pass_time(now_tick - iter->tick_);
		proto_record->set_type(iter->fight_type_);
		proto_record->set_state(iter->fight_state_);

		proto_record->set_name(iter->name_);
		proto_record->set_rank(iter->rank_);
		proto_record->set_rank_change(iter->rank_change_);
	}

	ARENA_SYS->insert_arena_viewer(this->role_id());
	FINER_PROCESS_RETURN(RETURN_ARENA_FETCH_INFO, &area_info);
}

int ArenaLPlayer::validate_has_area_reward(void)
{
	ArenaRole *arena_role = ARENA_SYS->area_role(this->role_id());

	JUDGE_RETURN(arena_role != NULL, ERROR_CLIENT_OPERATE);
	JUDGE_RETURN(arena_role->have_reward() == true, ERROR_CLIENT_OPERATE);

	return 0;
}

int ArenaLPlayer::draw_area_reward()
{
	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_ARENA_DRAW_REWARD,
			ERROR_CLIENT_OPERATE);

	int ret = this->validate_has_area_reward();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ARENA_DRAW_REWARD, ret);

	Proto50101002 reward_info;
	reward_info.set_last_rank(arena_role->last_rank_);
	reward_info.set_add_anima(arena_role->reward_id_);

	this->request_add_reward(arena_role->reward_id_, ADD_FROM_ARENA);
	arena_role->reward_id_ = 0;

	//资源找回
	this->check_arena_pa_event();
	this->sync_restore_event_info(GameEnum::ES_ACT_ARENA, arena_role->rank_, 1);

	FINER_PROCESS_RETURN(RETURN_ARENA_DRAW_REWARD, &reward_info);
}

int ArenaLPlayer::close_arena_info()
{
	ARENA_SYS->erase_arena_viewer(this->role_id());
	FINER_PROCESS_NOTIFY(RETURN_ARENA_CLOSE_INFO);
}

int ArenaLPlayer::validate_can_area_challenge(void)
{
	ArenaRole *arena_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, ERROR_CLIENT_OPERATE);

	JUDGE_RETURN(arena_role->left_times_ > 0, ERROR_ARENA_FIGHT_TIMES);

	if (arena_role->left_cool_time() >= CONFIG_INSTANCE->athletics_base()["left_time_limit"].asInt() * 60)
	{
		return ERROR_ARENA_COOL_TIME;
	}
	if (arena_role->left_cool_time() > 0 && arena_role->is_over_limit_ == 1)
	{
		return ERROR_ARENA_COOL_TIME;
	}

	return 0;
}

int ArenaLPlayer::change_arena_skip(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10101010*, request, RETURN_ARENA_CHANGE_SKIP);

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_ARENA_CHANGE_SKIP,
			ERROR_CLIENT_OPERATE);

	arena_role->is_skip_ = request->status();

	Proto50101010 respond;
	respond.set_cur_status(arena_role->is_skip_);
	FINER_PROCESS_RETURN(RETURN_ARENA_CHANGE_SKIP, &respond);
}

int ArenaLPlayer::start_area_challenge(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10101003*, request, RETURN_ARENA_CHALLENGE);

	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ARENA_CHALLENGE, ERROR_NORMAL_SCENE);

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_ARENA_CHALLENGE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(arena_role->is_fighting_ == false, RETURN_ARENA_CHALLENGE,
			ERROR_ARENA_LAST_CHALLENGE);

	int ret = this->validate_can_area_challenge();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ARENA_CHALLENGE, ret);

	//挑战排名检测
	if (request->rank() <= 5)
	{
		CONDITION_NOTIFY_RETURN(arena_role->rank_ <= 100, RETURN_ARENA_CHALLENGE, ERROR_CLIENT_OPERATE);
	}

//    LOGIC_MONITOR->inner_notify_player_assist_event(this->role_id(), GameEnum::PA_EVENT_ARENA_CHALLENGE, 0);

	arena_role->level_ = this->role_level();
	arena_role->force_ = this->role_detail().__fight_force;
	if (arena_role->open_flag_ == false)
	{
		return this->start_guide_arena_challenge(request->rank(), arena_role);
	}
	else
	{
		return this->start_normal_arena_challenge(request->rank(), arena_role);
	}
}

int ArenaLPlayer::start_guide_arena_challenge(int rank, ArenaRole* arena_role)
{
	JUDGE_RETURN(rank > 0, -1);

	Int64 rpm_id = CONFIG_INSTANCE->athletics_rank(rank)["rpm_id"].asInt();
	rpm_id += (::std::rand() % GameEnum::ARENA_BASE_RANK_NUM) * GameEnum::ARENA_BASE_ID_NUM;

	Int64 robot_id = ARENA_SYS->fetch_detail()->robot_id_map[rpm_id];
	ArenaRole* rank_role = ARENA_SYS->area_role(robot_id);

	rank_role->rank_ = rank;
	ARENA_SYS->start_challenge(arena_role, rank_role, true, 1);

	if (arena_role->is_skip_) return 0;
	this->transfer_to_arena_field(arena_role);

	return 0;
}

int ArenaLPlayer::start_normal_arena_challenge(int rank, ArenaRole* arena_role)
{
	ArenaRole* rank_role = ARENA_SYS->arena_rank_role(rank);
	CONDITION_NOTIFY_RETURN(rank_role != NULL, RETURN_ARENA_CHALLENGE,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(rank_role->id_ != this->role_id(),
			RETURN_ARENA_CHALLENGE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(rank_role->is_fighting_ == false,
			RETURN_ARENA_CHALLENGE, ERROR_ARENA_IS_FIGHTING);

//	this->sync_to_ml_activity_finish(GameEnum::ACTIVITY_ARENA);
	rank_role->rank_ = rank;
	ARENA_SYS->start_challenge(arena_role, rank_role, false, 0);
	if (arena_role->is_skip_) return 0;
	this->transfer_to_arena_field(arena_role);

	return 0;
}

int ArenaLPlayer::transfer_to_arena_field(ArenaRole* area_role)
{
//	JUDGE_RETURN(area_role->is_fighting_ == true, -1);

	Proto30400052 enter_info;
	enter_info.set_scene_id(GameEnum::ARENA_SCENE_ID);
	enter_info.set_space_id(area_role->area_index_);

	enter_info.set_scene_mode(SCENE_MODE_LEAGUE);
	enter_info.set_enter_type(GameEnum::ET_ARENA_FIELD);

	MoverCoord coord;
	coord.set_pixel(CONFIG_INSTANCE->athletics_base()["first_loc"][0u].asInt(),
			CONFIG_INSTANCE->athletics_base()["first_loc"][1u].asInt());

	enter_info.set_pos_x(coord.pixel_x());
	enter_info.set_pos_y(coord.pixel_y());

	return LOGIC_MONITOR->dispatch_to_scene(this, &enter_info);
}

int ArenaLPlayer::fetch_arena_times_money()
{
	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(area_role != NULL, RETURN_ARENA_BUY_INFO,
			ERROR_CLIENT_OPERATE);

	int buy_money = ARENA_SYS->fetch_buy_money(area_role);
	CONDITION_NOTIFY_RETURN(buy_money > 0, RETURN_ARENA_BUY_INFO,
			ERROR_ARENA_MAX_BUY_TIMES);

	Proto50101006 money_info;
	money_info.set_money(buy_money);
	FINER_PROCESS_RETURN(RETURN_ARENA_BUY_INFO, &money_info);
}

int ArenaLPlayer::buy_arena_times_begin()
{
	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(area_role != NULL, RETURN_ARENA_BUY_TIMES,
			ERROR_CLIENT_OPERATE);

	int buy_limit = CONFIG_INSTANCE->athletics_base()["buy_limit"].asInt();
	CONDITION_NOTIFY_RETURN(area_role->buy_times_ < buy_limit, RETURN_ARENA_BUY_TIMES,
				ERROR_ARENA_BUY_LIMIT);

	int buy_money = ARENA_SYS->fetch_buy_money(area_role);
	CONDITION_NOTIFY_RETURN(buy_money > 0, RETURN_ARENA_BUY_TIMES,
			ERROR_CLIENT_OPERATE);

	Proto31400308 buy_info;
	buy_info.set_need_money(buy_money);

	area_role->buy_times_ += 1;
	return LOGIC_MONITOR->dispatch_to_scene(this, &buy_info);
}

int ArenaLPlayer::buy_arena_times_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400308*, request, RETURN_ARENA_BUY_TIMES);

	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(area_role != NULL, RETURN_ARENA_BUY_TIMES,
			ERROR_CLIENT_OPERATE);

	if (request->oper_result() == 0)
	{
		this->update_sword_pool_info(1,2);
		area_role->left_times_ ++;
		Proto50101004 times_info;
		times_info.set_left_times(area_role->left_times_);
		times_info.set_buy_times(area_role->buy_times_);
		FINER_PROCESS_RETURN(RETURN_ARENA_BUY_TIMES, &times_info);
	}
	else
	{
		area_role->buy_times_ -= 1;
		area_role->buy_times_ = std::max(0, area_role->buy_times_);
		return this->respond_to_client_error(RETURN_ARENA_BUY_TIMES,
				request->oper_result());
	}
}

int ArenaLPlayer::clear_arena_cool_begin()
{
	ArenaRole* area_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(area_role != NULL, RETURN_ARENA_CLEAR_COOL,
			ERROR_CLIENT_OPERATE);

//	CONDITION_NOTIFY_RETURN(area_role->left_cool_time() > 0 && this->vip_type() <= 0,
//			RETURN_AREA_CLEAR_COOL, ERROR_CLIENT_OPERATE);
	int clear_money = ARENA_SYS->fetch_clear_money(area_role);
	Proto31400309 money_info;
	money_info.set_need_money(clear_money);

	return LOGIC_MONITOR->dispatch_to_scene(this, &money_info);
}

int ArenaLPlayer::clear_arena_cool_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400309*, request, RETURN_ARENA_CLEAR_COOL);

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	CONDITION_NOTIFY_RETURN(arena_role != NULL, RETURN_ARENA_CLEAR_COOL,
			ERROR_CLIENT_OPERATE);

	arena_role->next_fight_tick_ = 0;
//	this->arena_notify_timer_.cancel_timer();

//	if (arena_role->left_times_ > 0)
//	{
//		LOGIC_MONITOR->inner_notify_player_assist_event(this->role_id(),
//				GameEnum::PA_EVENT_ARENA_CHALLENGE, 1);
//	}

	this->check_arena_pa_event();
	FINER_PROCESS_NOTIFY(RETURN_ARENA_CLEAR_COOL);
}

int ArenaLPlayer::check_and_notity_arena_tips()
{
	JUDGE_RETURN(this->notify_msg_flag() == true, -1);

	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, -1);
	JUDGE_RETURN(arena_role->notify_flag_ == false, -1);

	JUDGE_RETURN(arena_role->left_times_ > 0, -1);
	JUDGE_RETURN(arena_role->left_cool_time() <= 0, -1);

	static int view_span_time = CONFIG_INSTANCE->arena("view_span_time").asInt();
	JUDGE_RETURN(::time(NULL) - arena_role->last_view_tick_ > view_span_time, -1);

//    LOGIC_MONITOR->inner_notify_player_assist_event(this->role_id(),
//            GameEnum::PA_EVENT_ARENA_CHALLENGE, 1);

	arena_role->notify_flag_ = true;
	FINER_PROCESS_NOTIFY(ACTIVE_ARENA_TIPS);
}

int ArenaLPlayer::start_next_arena_timer(int cool_time)
{
//	this->arena_notify_timer_.schedule_timer(Time_Value(cool_time));
	return 0;
}

int ArenaLPlayer::check_arena_pa_event(void)
{
	if (this->validate_can_area_challenge() == 0)
	{
	    this->inner_notify_assist_event(GameEnum::PA_EVENT_ARENA_CHALLENGE_NEW, 1);
	}
	else
	{
	    this->inner_notify_assist_event(GameEnum::PA_EVENT_ARENA_CHALLENGE_NEW, 0);
	}

	if (this->validate_has_area_reward() == 0)
	{
	    this->inner_notify_assist_event(GameEnum::PA_EVENT_ARENA_AWARD, 1);
	}
	else
	{
	    this->inner_notify_assist_event(GameEnum::PA_EVENT_ARENA_AWARD, 0);
	}

	return 0;
}

void ArenaLPlayer::update_arena_exp_restore()
{
	ArenaRole* arena_role = ARENA_SYS->area_role(this->role_id());
	JUDGE_RETURN(arena_role != NULL, ;);

	this->sync_restore_event_info(GameEnum::ES_ACT_ARENA, arena_role->rank_, 0);
}

void ArenaLPlayer::update_sword_pool_info(int num, int flag)
{
	Proto31402901 inner_res;
	inner_res.set_left_add_flag(flag);
	inner_res.set_left_add_num(num);

	int task_id = GameEnum::SPOOL_TASK_LEGEND;
	inner_res.set_task_id(task_id);

	LOGIC_MONITOR->dispatch_to_scene(this, &inner_res);
}

void ArenaLPlayer::update_cornucopia_task_info()
{
	LogicPlayer *player = this->logic_player();
	player->update_cornucopia_activity_value(GameEnum::CORNUCOPIA_TASK_LEGEND, 1);
}
