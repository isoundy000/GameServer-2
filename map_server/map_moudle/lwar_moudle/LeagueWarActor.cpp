/*
 * LeagueWarActor.cpp
 *
 *  Created on: 2016年9月20日
 *      Author: lyw
 */

#include "LeagueWarActor.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "LeagueWarSystem.h"
#include "LeagueWarScene.h"
#include "GameAI.h"

LeagueWarActor::LeagueWarActor() {
	// TODO Auto-generated constructor stub
}

LeagueWarActor::~LeagueWarActor() {
	// TODO Auto-generated destructor stub
}

void LeagueWarActor::reset()
{

}

int LeagueWarActor::enter_scene(const int type)
{
	switch (this->lwar_enter_scene_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		return this->on_enter_lwar_scene(type);
	}
	}

	return 0;
}

int LeagueWarActor::exit_scene(const int type)
{
	switch (this->lwar_enter_scene_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		this->on_exit_lwar_scene(type);
		break;
	}
	}

	return MapPlayer::exit_scene(type);
}

int LeagueWarActor::die_process(const int64_t fighter_id)
{
	int ret = MapPlayer::die_process(fighter_id);
	JUDGE_RETURN(fighter_id > 0, ret);

	switch (this->lwar_enter_scene_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		this->lwar_die_process(fighter_id);
		break;
	}
	}

	return ret;
}

int LeagueWarActor::request_relive(Message* msg)
{
	switch (this->lwar_enter_scene_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		this->on_lwar_relive(msg);
		break;
	}
	}
	return 0;
}

int LeagueWarActor::league_war_scene()
{
	return GameEnum::LWAR_SCENE_ID;
}

int LeagueWarActor::lwar_enter_scene_type()
{
	switch (this->scene_id())
	{
	case GameEnum::LWAR_SCENE_ID:
	{
		return GameEnum::ET_LEAGUE_WAR;
	}
	}
	return -1;
}

int LeagueWarActor::request_join_league_war()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ENTER_LEAGUE_WAR, ERROR_NORMAL_SCENE);

	CONDITION_NOTIFY_RETURN(this->league_id() > 0, RETURN_ENTER_LEAGUE_WAR, ERROR_LEAGUE_NO_EXIST);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_LEAGUE_WAR);
	enter_info.set_league_index(this->league_id());
	enter_info.set_league_name(this->role_detail().__league_name);

	int ret = this->send_request_enter_info(this->league_war_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ENTER_LEAGUE_WAR, ret);

	return 0;
}

int LeagueWarActor::handle_exit_lwar_scene()
{
	switch (this->lwar_enter_scene_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		this->transfer_to_save_scene();
		break;
	}
	}

	return 0;
}

int LeagueWarActor::on_enter_lwar_scene(const int type)
{
	LWarRoleInfo* lwar_role = LEAGUE_WAR_SYSTEM->find_lwar(this->role_id());
	JUDGE_RETURN(lwar_role != NULL, ERROR_ENTER_FAILURE);
	LeagueWarScene* lwar_scene = LEAGUE_WAR_SYSTEM->find_lwar_scene(this->space_id());
	JUDGE_RETURN(lwar_scene != NULL, ERROR_NOT_HAS_THIS_SPACE);

	this->init_mover_scene(lwar_scene);
	MapPlayer::enter_scene(type);

	//断线重连进入时检测阵营
	lwar_role->camp_index_ = lwar_scene->fetch_camp_index(lwar_role->league_index_);

	this->set_camp_id(lwar_role->camp_id());
	lwar_role->name_ = this->role_name();
	lwar_role->sex_ = this->fight_sex();
	lwar_role->league_name_ = this->role_detail().__league_name;
	lwar_scene->enter_player(lwar_role);

	return 0;
}

int LeagueWarActor::on_exit_lwar_scene(const int type)
{
	LWarRoleInfo* lwar_role = LEAGUE_WAR_SYSTEM->find_lwar(this->role_id());
	JUDGE_RETURN(lwar_role != NULL, -1);

	LEAGUE_WAR_SYSTEM->request_exit_lwar(lwar_role->league_index_, this->role_id());

	LeagueWarScene* lwar_scene = LEAGUE_WAR_SYSTEM->find_lwar_scene(this->space_id());
	JUDGE_RETURN(lwar_scene != NULL, -1);

	lwar_scene->exit_player(lwar_role);
	return 0;
}

int LeagueWarActor::on_lwar_relive(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400203 *, request, RETURN_RELIVE);

	JUDGE_RETURN(this->scene_id() == GameEnum::LWAR_SCENE_ID, ERROR_USE_SCENE_LIMIT);

	const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
	JUDGE_RETURN(scene_json.isMember("relive") == true, ERROR_NO_RELIVE_POINT);

	MapPlayer::request_relive(msg);

	return 0;
}

int LeagueWarActor::request_league_war_info(void)
{
	CONDITION_NOTIFY_RETURN(LEAGUE_WAR_SYSTEM->is_activity_time() == true,
			RETUEN_FETCH_LEAGUE_WAR_INFO, ERROR_NOT_JOIN_TIME);

	CONDITION_NOTIFY_RETURN(this->scene_id() == GameEnum::LWAR_SCENE_ID,
				RETUEN_FETCH_LEAGUE_WAR_INFO, ERROR_USE_SCENE_LIMIT);

	int space_count = LEAGUE_WAR_SYSTEM->find_open_space_num();

	Proto50400351 respond;
	for (int space_id = 0; space_id < space_count; ++space_id)
	{
		LeagueWarScene* lwar_scene = LEAGUE_WAR_SYSTEM->find_lwar_scene(space_id);
		JUDGE_CONTINUE(lwar_scene != NULL);

		if (lwar_scene->is_player_enter() == false)
			break;

		ProtoLWarInfo *lwar_detail = respond.add_lwar_detail();
		lwar_detail->set_space_id(space_id);

		int attack_resource = lwar_scene->fetch_attack_resource();
		int defence_resource = lwar_scene->fetch_defence_resource();
		lwar_detail->set_attack_resource(attack_resource);
		lwar_detail->set_defence_resource(defence_resource);

		GameAI* game_ai = lwar_scene->find_space_boss();
		if (game_ai != NULL)
		{
			lwar_detail->set_boss_name(game_ai->ai_detail().__name);
			double left_percent = game_ai->fight_detail().blood_percent(game_ai, 1);
			lwar_detail->set_boss_blood(left_percent);
		}

		lwar_scene->fetch_league_hurt_info(lwar_detail);
		lwar_scene->fetch_my_league_hurt(lwar_detail, this->league_id());
	}
	// 获取全服帮派积分
	LEAGUE_WAR_SYSTEM->find_league_rank(&respond);
	// 获取个人帮派积分
	LEAGUE_WAR_SYSTEM->find_player_league_rank(&respond, this->league_id());

	FINER_PROCESS_RETURN(RETUEN_FETCH_LEAGUE_WAR_INFO, &respond);
}

int LeagueWarActor::request_change_space(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400352*, reqeust, RETURN_CHANGE_SPACE);

	CONDITION_NOTIFY_RETURN(this->scene_id() == GameEnum::LWAR_SCENE_ID,
			RETURN_CHANGE_SPACE, ERROR_USE_SCENE_LIMIT);
	CONDITION_NOTIFY_RETURN(LEAGUE_WAR_SYSTEM->is_activity_time() == true,
			RETURN_CHANGE_SPACE, ERROR_GUARD_TIME);

	int space_id = reqeust->space_id();
	CONDITION_NOTIFY_RETURN(this->space_id() != space_id,
			RETURN_CHANGE_SPACE, ERROR_IN_SAME_SPACE);

	LeagueWarScene* lwar_scene = LEAGUE_WAR_SYSTEM->find_lwar_scene(space_id);
	CONDITION_NOTIFY_RETURN(lwar_scene != NULL, RETURN_CHANGE_SPACE, ERROR_NOT_HAS_THIS_SPACE);

	CONDITION_NOTIFY_RETURN(lwar_scene->is_player_full() == false,
			RETURN_CHANGE_SPACE, ERROR_SPACE_IS_FULL);

	LWarRoleInfo* lwar_role = LEAGUE_WAR_SYSTEM->find_lwar(this->role_id());
	CONDITION_NOTIFY_RETURN(lwar_role != NULL, RETURN_CHANGE_SPACE, ERROR_ENTER_FAILURE);
	lwar_role->space_ = space_id;
	lwar_role->camp_index_ = lwar_scene->fetch_camp_index(this->league_id());
	lwar_scene->register_player(lwar_role);

	LEAGUE_WAR_SYSTEM->init_league_map(this->role_id(), this->league_id(), this->role_detail().__league_name, space_id);

	int ret = this->validate_player_change();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_CHANGE_SPACE, ret);

	const Json::Value& enter_point = lwar_role->camp_index_ == 0 ?
			CONFIG_INSTANCE->league_war("attack_pos") :
			CONFIG_INSTANCE->league_war("defend_pos");

	MoverCoord coord;
	coord.set_pixel(enter_point[0u].asInt(), enter_point[1u].asInt());
	return this->transfer_dispatcher(this->scene_id(), coord, SCENE_MODE_LEAGUE_WAR, space_id);
}

int LeagueWarActor::request_league_war_score()
{
	CONDITION_NOTIFY_RETURN(LEAGUE_WAR_SYSTEM->is_activity_time() == true,
			RETURN_FETCH_ALL_LWAR_SCORE, ERROR_NOT_JOIN_TIME);

	CONDITION_NOTIFY_RETURN(this->scene_id() == GameEnum::LWAR_SCENE_ID,
			RETURN_FETCH_ALL_LWAR_SCORE, ERROR_USE_SCENE_LIMIT);

	Proto50400353 respond;
	LEAGUE_WAR_SYSTEM->find_all_league_rank(&respond);
	FINER_PROCESS_RETURN(RETURN_FETCH_ALL_LWAR_SCORE, &respond);
}

int LeagueWarActor::validate_player_change()
{
	if (this->is_death() == true)
	{
		return ERROR_PLAYER_DEATH;
	}

	if (this->is_jumping() == true)
	{
		return ERROR_USE_SKILL_JUMPING;
	}

	if (this->is_float_cruise() == true)
	{
		return ERROR_PLAYER_CRUISE;
	}

	return 0;
}

int LeagueWarActor::lwar_die_process(Int64 fighter_id)
{
	Int64 real_fighter = this->fetch_benefited_attackor_id(fighter_id);
	JUDGE_RETURN(real_fighter != this->role_id(), -1);

	LeagueWarScene* lwar_scene = LEAGUE_WAR_SYSTEM->find_lwar_scene(this->space_id());
	JUDGE_RETURN(lwar_scene != NULL, -1);

	lwar_scene->handle_player_kill(real_fighter, this->role_id());
	return 0;
}



