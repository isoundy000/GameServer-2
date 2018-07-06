/*
 * LeagueRegionFightPlayer.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#include "LeagueRegionFightActor.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "LeagueReginFightSystem.h"
#include "LeagueRegionFightScene.h"

LeagueRegionFightActor::LeagueRegionFightActor()
{
}

LeagueRegionFightActor::~LeagueRegionFightActor()
{
}

//帮派领地
int LeagueRegionFightActor::get_last_region_info(Message *msg)
{
	Proto30400057 inner_info;
	inner_info.set_type(0);
	inner_info.set_league_id(this->league_id());
	return MAP_MONITOR->dispatch_to_scene(this, this->lrf_scene(), &inner_info);
}

int LeagueRegionFightActor::get_region_bet_info(Message *msg)
{
	Proto30400057 inner_info;
	inner_info.set_type(1);
	return MAP_MONITOR->dispatch_to_scene(this, this->lrf_scene(), &inner_info);
}

int LeagueRegionFightActor::get_bet_support_apply(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10404003*, request, -1);

	Proto30400057 inner_info;
	inner_info.set_type(2);
	inner_info.set_league_id(request->support_league_id());
	return MAP_MONITOR->dispatch_to_scene(this, this->lrf_scene(), &inner_info);
}

int LeagueRegionFightActor::player_request_join_war(Message *msg)
{
	CONDITION_NOTIFY_RETURN(this->league_id() > 0, RETURN_REQUEST_ENTER_WAR,
			ERROR_LEAGUE_NO_EXIST);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_LEAGUE_REGION_WAR);
	enter_info.set_league_index(this->league_id());
	enter_info.set_league_name(this->role_detail().__league_name);

	int ret = this->send_request_enter_info(this->lrf_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REQUEST_ENTER_WAR, ret);

	return 0;
}

int LeagueRegionFightActor::player_request_change_mode(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10404005*, request, -1);

	CONDITION_NOTIFY_RETURN(this->is_in_league_region() == true,
			RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->check_is_in_transfer_time() == false,
			RETURN_LRF_CHANGE_MODE, ERROR_IN_TRANSFER);

	CONDITION_NOTIFY_RETURN(LRF_MONITOR->is_activity_time() == true,
			RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->is_lrf_change_mode() == false,
			RETURN_LRF_CHANGE_MODE, ERROR_HAVE_TRANSF);

	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	int level = scene->fetch_lrf_weapon_level(this->league_id());
	const Json::Value& weapon_conf = CONFIG_INSTANCE->lrf_weapon(request->hickty_id(), level);
	CONDITION_NOTIFY_RETURN(weapon_conf != Json::Value::null, RETURN_LRF_CHANGE_MODE,
			ERROR_CLIENT_OPERATE);

	Proto31400057 inner;
	inner.set_hickty_id(request->hickty_id());

	switch(request->buy_type())
	{
	case 1:
	{
		int need_amount = weapon_conf["resources_exchange"].asInt();
		CONDITION_NOTIFY_RETURN(scene->validate_sub_resource(this->role_id(),
				need_amount) == true, RETURN_LRF_CHANGE_MODE, ERROR_LRF_NO_RESOURCE);
		return this->lrf_request_change_mode_done(&inner);
	}
	case 2:
	{
		inner.set_buy_type(GameEnum::ITEM_MONEY_UNBIND_GOLD);
		inner.set_amount(weapon_conf["money_exchange"].asInt());
		return this->send_to_logic_thread(inner);
	}
	default:
	{
		inner.set_buy_type(GameEnum::ITEM_MONEY_BIND_GOLD);
		inner.set_amount(weapon_conf["bind_money_exchange"].asInt());
		return this->send_to_logic_thread(inner);
	}
	}
}

int LeagueRegionFightActor::lrf_request_change_mode_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400057*, request, -1);

	CONDITION_NOTIFY_RETURN(this->is_lrf_change_mode() == false,
			RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(this->check_is_in_transfer_time() == false,
			RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_LRF_CHANGE_MODE, ERROR_CLIENT_OPERATE);

	int level = scene->fetch_lrf_weapon_level(this->league_id());
	const Json::Value& weapon_conf = CONFIG_INSTANCE->lrf_weapon(request->hickty_id(), level);
	CONDITION_NOTIFY_RETURN(weapon_conf != Json::Value::null, RETURN_LRF_CHANGE_MODE,
			ERROR_CLIENT_OPERATE);

	this->use_transfer(request->hickty_id());

	this->update_hickty_info(weapon_conf);
	this->modify_blood_by_levelup(this->role_detail_.health_);

	this->set_cur_beast_aim(0);
	this->all_beast_exit_scene();

	FINER_PROCESS_NOTIFY(RETURN_LRF_CHANGE_MODE);
}

int LeagueRegionFightActor::quit_lrf_change_mode()
{
	JUDGE_RETURN(this->is_lrf_change_mode() == true, -1);

	this->remove_transfer(this->role_detail_.hickty_id_);
	this->notify_fight_property(0);
	this->all_beast_enter_scene();

	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	LRFPlayer* lrf_player = scene->fetch_lrf_player(this->role_id());
	JUDGE_RETURN(lrf_player != NULL, -1);

	lrf_player->hickty_id_ = 0;
	lrf_player->hickty_index_ = 0;
	return 0;
}

int LeagueRegionFightActor::fetch_lrf_war_rank()
{
	Proto30400057 inner_info;
	inner_info.set_type(3);
	return MAP_MONITOR->dispatch_to_scene(this, this->lrf_scene(), &inner_info);
}

int LeagueRegionFightActor::enter_scene(const int type)
{
	LeagueRegionFightScene* lrf_scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	JUDGE_RETURN(lrf_scene != NULL, ERROR_NOT_HAS_THIS_SPACE);

	this->init_mover_scene(lrf_scene);
	MapPlayer::enter_scene(type);

	int camp_id = lrf_scene->fetch_camp_id(this->league_id());
	this->set_camp_id(camp_id);
	lrf_scene->enter_player(this);

	return 0;
}

int LeagueRegionFightActor::exit_scene(const int type)
{
	this->quit_lrf_change_mode();
	return MapPlayer::exit_scene(type);
}

int LeagueRegionFightActor::die_process(const int64_t fighter_id)
{
	this->quit_lrf_change_mode();

	int ret = MapPlayer::die_process(fighter_id);
	JUDGE_RETURN(fighter_id > 0, ret);

	this->lrf_die_process(fighter_id);
	return ret;
}

int LeagueRegionFightActor::request_relive(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400203 *, request, RETURN_RELIVE);
	CONDITION_NOTIFY_RETURN(this->is_in_league_region() == true,
			RETURN_RELIVE, ERROR_CLIENT_OPERATE);

	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	CONDITION_NOTIFY_RETURN(scene != NULL, RETURN_RELIVE, ERROR_CLIENT_OPERATE);

	const Json::Value& conf = scene->conf();
	CONDITION_NOTIFY_RETURN(conf != Json::Value::null, RETURN_RELIVE, ERROR_CLIENT_OPERATE);

	switch (request->relive())
	{
	case GameEnum::RELIVE_SPECIAL:
	{
		CONDITION_NOTIFY_RETURN(scene->is_in_center_relive(this->camp_id()) == true,
				RETURN_RELIVE, ERROR_CLIENT_OPERATE);

		MoverCoord relive_coord;
		relive_coord.set_pixel(conf["center_relive_point"][0u].asInt(),
				conf["center_relive_point"][1u].asInt());
	    return this->handle_player_relive(relive_coord, 100);
	}
	default:
	{
		return MapPlayer::request_relive(msg);
	}
	}
}

int LeagueRegionFightActor::modify_blood_by_fight(const double value, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	if (this->is_lrf_change_mode() == true && value > 0)
	{
		return this->lrf_modify_blood(value, attackor);
	}
	else
	{
		return MapPlayer::modify_blood_by_fight(value, fight_tips, attackor, skill_id);
	}
}

int LeagueRegionFightActor::lrf_die_process(Int64 fighter_id)
{
	Int64 real_fighter = this->fetch_benefited_attackor_id(fighter_id);
	JUDGE_RETURN(real_fighter != this->role_id(), -1);

	LeagueRegionFightScene* lrf_scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	JUDGE_RETURN(lrf_scene != NULL, -1);

	lrf_scene->handle_player_kill(real_fighter, this->role_id());
	return 0;
}

int LeagueRegionFightActor::lrf_modify_blood(double value, Int64 attackor)
{
	JUDGE_RETURN(value > 0, -1);

	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	GameFighter* attackor_fighter = scene->find_fighter(attackor);
	JUDGE_RETURN(attackor_fighter != NULL, -1);

	LRFPlayer* lrf_player = scene->fetch_lrf_player(this->role_id());
	JUDGE_RETURN(lrf_player != NULL, -1);

    double buff_exempt = this->find_status_value_by_type(
    		BasicStatus::EXEMPT, BasicStatus::VALUE2);

	if (attackor_fighter->is_player())
	{
		int value = scene->lrf_modify_player_blood(lrf_player, attackor_fighter->self_player())
				* (1 - GameCommon::div_percent(buff_exempt));
		return MapPlayer::modify_blood_by_fight(value, FIGHT_TIPS_NORMAL, attackor, 0);
	}
	else
	{
		int value = lrf_player->force_hurt_ * (1 - GameCommon::div_percent(buff_exempt));
		return MapPlayer::modify_blood_by_fight(value, FIGHT_TIPS_NORMAL, attackor, 0);
	}
}

int LeagueRegionFightActor::lrf_scene()
{
	return GameEnum::LEAGUE_REGION_FIGHT_ID;
}

int LeagueRegionFightActor::update_hickty_info(const Json::Value& weapon_conf)
{
	LeagueRegionFightScene* scene = LRF_MONITOR->find_lrf_scene(this->space_id());
	JUDGE_RETURN(scene != NULL, -1);

	LRFPlayer* lrf_player = scene->fetch_lrf_player(this->role_id());
	JUDGE_RETURN(lrf_player != NULL, -1);

	lrf_player->set_hickty_info(weapon_conf);
	this->role_detail_.health_ = lrf_player->health_;
	this->notify_fight_property(0);

	return 0;
}


