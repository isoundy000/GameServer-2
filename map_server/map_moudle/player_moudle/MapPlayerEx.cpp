/*
 * MapPlayerEx.cpp
 *
 * Created on: 2013-03-04 11:09
 *     Author: glendy
 */

#include "MapPlayerEx.h"
#include "ProtoDefine.h"

#include "MapBeast.h"
#include "MongoDataMap.h"
#include "MMORole.h"
//#include "MMOBeast.h"
#include "BaseScript.h"

#include "GameField.h"
#include "AIManager.h"
#include "MapMonitor.h"
#include "SceneLineManager.h"
#include "SessionManager.h"
#include "MongoConnector.h"
#include "CollectChestsScene.h"
#include "AnswerActivityScene.h"

#include <mongo/client/dbclient.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "FloatAI.h"
#include "SMBattleSystem.h"
#include "LeagueWarSystem.h"
#include "MonsterAttackSystem.h"
#include "WorldBossSystem.h"
#include "LeagueMonitor.h"
#include "TrvlBattleScene.h"

#define SCENE_MODE_SWITCH(X) \
{ \
    switch (this->scene_mode()) \
    { \
    case SCENE_MODE_LEAGUE: \
        return MapLeaguer::X; \
    case SCENE_MODE_SCRIPT: \
        return MapPlayerScript::X; \
	case SCENE_MODE_BATTLE_GROUND: \
		return BattleGroundActor::X;\
	case SCENE_MODE_LEAGUE_WAR: \
		return LeagueWarActor::X;\
	case SCENE_MODE_MONSTER_ATTACK: \
		return MonsterAttackActor::X;\
	case SCENE_MODE_WORLD_BOSS: \
		return WorldBossActor::X;\
	case SCENE_MODE_REGION_FIGHT: \
		return LeagueRegionFightActor::X;\
	case SCENE_MODE_TRVL_PEAK: \
		return TrvlPeakActor::X;\
    default: \
        return MapPlayer::X; \
    } \
}

MapPlayerEx::MapPlayerEx(void)
{ /*NULL*/ }

MapPlayerEx::~MapPlayerEx(void)
{
    this->reset();
}

void MapPlayerEx::reset(void)
{
	MapPlayer::reset();
	GameFighter::reset_fighter();
	MapOfflineHook::reset();
	BattleGroundActor::reset();
    MapLeaguer::leaguer_reset();
    MapPlayerScript::reset_script();
    HotspringActor::reset();
    TrvlPeakActor::reset();
}

void MapPlayerEx::reset_everyday()
{
    JUDGE_RETURN(this->blood_container_.everyday_tick_ <= ::time(NULL), );

    MapPlayer::reset_everyday();
    BattleGroundActor::reset_everyday();
    MapPlayerScript::script_reset_everyday();

    this->blood_container_.everyday_tick_ = ::next_day(0, 0).sec();
}

int MapPlayerEx::notify_client_map_offline()
{
	static int drop_reason = CONFIG_INSTANCE->const_set("drop_reason");

    Proto50999999 req;
    req.set_drop_reason(drop_reason);
    FINER_PROCESS_RETURN(RETURN_SERVER_KEEP_ALIVE, &req);
}

int MapPlayerEx::request_enter_scene_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400052*, request, -1);

	const Json::Value& set_conf = CONFIG_INSTANCE->scene_set(
			request->scene_id());
	if (set_conf.isMember("save_recover_scene") == true)
	{
		int scene_id = request->scene_id();
		if (GameCommon::is_world_boss_scene(scene_id))
		{
			this->save_spacial_scene_info();
		}
		else
		{
			this->save_cur_scene_info(0);
		}
	}

	if (set_conf.isMember("quit_team") == true)
	{
		this->notify_quit_trvl_team();
		this->notify_quit_normal_team();
	}

	switch (request->enter_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ENTER_LEAGUE_WAR);
	}
	case GameEnum::ET_SM_BATTLE:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_JOIN_SM_BATTLE);
	}
	case GameEnum::ET_ARENA_FIELD:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ARENA_CHALLENGE);
	}
	case GameEnum::ET_MONSTER_ATTACK:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_MATTACK_REQUEST_ENTER);
	}
	case GameEnum::ET_WORLD_BOSS:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ENTER_WORLD_BOSS);
	}
	case GameEnum::ET_LEAGUE_BOSS:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ENTER_LEAGUE_BOSS);
	}
	case GameEnum::ET_COLLECT_CHESTS:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_COLLECT_CHESTS_ENTER);
	}
	case GameEnum::ET_ANSWER_ACTIVITY:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ANSWER_ACTIVITY_ENTER);
	}
	case GameEnum::ET_HOTSPRING_ACTIVITY:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_HOTSPRING_ACTIVITY_ENTER);
	}
	case GameEnum::ET_TRAVEL_ARENA:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_TRVL_ARENA_ENTER);
	}
	case GameEnum::ET_TM_ARENA:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_TRVL_MARENA_ENTER);
	}
	case GameEnum::ET_LEAGUE_REGION_WAR:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_REQUEST_ENTER_WAR);
	}
	case GameEnum::ET_TRVL_WBOSS:
	{
		return MapPlayer::request_enter_scene_done(request,
				RETURN_ENTER_TRVL_WBOSS);
	}
    case GameEnum::ET_TRVL_BATTLE:
    {
    	int ret = this->validate_player_transfer();
        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRVL_BATTLE_ENTER, ret);
        return MapPlayer::request_enter_scene_done(request, RETURN_TRVL_BATTLE_ENTER);
    }
    case GameEnum::ET_TRVL_PEAK:
    {
    	return MapPlayer::request_enter_scene_done(request,
    			RETURN_TRVL_PEAK_ENTER);
    }
	default:
	{
		MSG_USER("Error Request Enter Scene %d", request->enter_type());
	}
	}

	return 0;
}

int MapPlayerEx::request_exit_cur_system()
{
	switch (this->scene_mode())
	{
	case SCENE_MODE_LEAGUE:
	{
		return this->request_exit_league();
	}
	case SCENE_MODE_SCRIPT:
	{
		return this->request_exit_script(true);
	}
	case SCENE_MODE_BATTLE_GROUND:
	{
		return this->handle_exit_battle_scene();
	}
	case SCENE_MODE_LEAGUE_WAR:
	{
		return this->handle_exit_lwar_scene();
	}
	case SCENE_MODE_MONSTER_ATTACK:
	{
		return this->handle_exit_mattack_scene();
	}
	case SCENE_MODE_WORLD_BOSS:
	{
		return this->handle_exit_wboss_scene();
	}
	case SCENE_MODE_REGION_FIGHT:
	{
		return this->transfer_to_save_scene();
	}
	case SCENE_MODE_TRVL_PEAK:
	{
		return this->handle_exit_trvl_peak_scene();
	}
	default:
	{
		return 0;
	}
	}
}

int MapPlayerEx::request_force_exit_system(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400007*, request, -1);

	JUDGE_RETURN(request->scene_id() == this->scene_id(), -1);
	JUDGE_RETURN(this->scene_mode() != SCENE_MODE_NORMAL, -1);

	switch(this->scene_mode())
	{
	case SCENE_MODE_SCRIPT:
	{
		BaseScript *script = this->fetch_script();
		JUDGE_BREAK(script != NULL);

//		script->set_script_recycle_time(2);
		script->process_scene_tick_failure();
		break;
	}
	}

	return 0;
}

int MapPlayerEx::map_test_command(Message *msg)
{
#ifdef TEST_COMMAND
	MSG_DYNAMIC_CAST_NOTIFY(Proto11499999*, request, RETURN_ML_TEST_COMMAND);

	if (request->cmd_name().compare("lvlto") == 0)
	{
		int target_level = request->param1();
        if (target_level < this->level())
        {
            this->fight_detail_.set_level(target_level);
            this->update_fight_property();
        }

        int ret = 0;
		while (target_level > this->level() && ret == 0)
		{
			ret = this->level_upgrade();
		}

		return 0;
	}
	else if (request->cmd_name() == "eventcut")
	{
		return this->check_and_add_event_cut(this->mover_id(), true);
	}
	else if (request->cmd_name() == "tfb_times")
	{
		this->script_detail_.__trvl_total_pass = request->param1();
		return this->request_script_list_info(NULL);
	}
	else if (request->cmd_name() == "tarena_score")
	{
		this->update_tarena_score(request->param1());
		return 0;
	}
	else if (request->cmd_name() == "escort_open")
	{
		return this->open_pescort_car_info();
	}
	else if (request->cmd_name() == "escort_upgrade")
	{
		Proto10401524 test_1;
		test_1.set_cur_level(2);
		test_1.set_target_level(4);
		test_1.set_till(1);
		test_1.set_auto_buy(1);
		return this->upgrade_escort_level(&test_1);
	}
	else if (request->cmd_name() == "escort_start")
	{
		Proto10401522 test_2;
		test_2.set_type(3);
		return this->select_pescort_car_begin(&test_2);
	}
	else if (request->cmd_name() == "escort_end")
	{
		return this->select_pescort_car_done();
	}
	else if (request->cmd_name() == "escort_clear")
	{
		Escort_detail& temp = this->get_escort_detail();
		temp.reset();
		return 0;
	}
	else if (request->cmd_name() == "exit_answer")
	{
		return this->request_exit_cur_system();
	}
	else if (request->cmd_name() == "enter_hotspring")
	{
		return this->request_enter_hotspring_activity();
	}
	else if (request->cmd_name() == "script_info")
	{
		return this->request_script_list_info(NULL);
	}
	else if (request->cmd_name() == "reset_script")
	{
		this->command_reset_script();
		return 0;
	}
	else if (request->cmd_name().compare("get_script") == 0)
	{
		Proto10400914 respond;
		respond.set_script_sort(request->param1());
		this->request_script_player_info(&respond);
		return 0;
	}
	else if (request->cmd_name().compare("special_award") == 0)
	{
		Proto10400939 respond;
		respond.set_script_wave_id(request->param1());
		respond.set_script_sort(request->param2());
		this->request_fetch_special_award(&respond);
		return 0;
	}
	else if (request->cmd_name() == "add_angry")
	{
		this->add_and_notify_angry_value(request->param1());
		return 0;
	}
	else if (request->cmd_name() == "exp")
	{
		int exp = request->param1();
		this->modify_element_experience(exp, ADD_EXP_FROM_TEST);
		return 0;
	}
	else if (request->cmd_name() == "self_mover")
	{
		MSG_USER("Coord X     	= %d", this->mover_detial_.__location.pos_x());
		MSG_USER("Coord Y     	= %d", this->mover_detial_.__location.pos_y());
		MSG_USER("Pixel X     	= %d", this->mover_detial_.__location.pixel_x());
		MSG_USER("Pixel Y     	= %d", this->mover_detial_.__location.pixel_y());
		return 0;
	}
	else if (request->cmd_name().compare("blood") == 0)
    {
        return this->modify_blood_by_levelup(request->param1()); 
    }
	else if (request->cmd_name().compare("spd") == 0)
    {
		return this->update_fighter_speed(GameEnum::SPEED,
				request->param1(), BasicElement::FLY);
    }
	else if (request->cmd_name().compare("magic") == 0)
    {
		int inc_magic = this->fight_detail_.__magic - request->param1();
    	this->modify_magic_by_notify(inc_magic);
    	return 0;
    }
    else if (request->cmd_name().compare("lucky") == 0)
    {
        this->fight_detail_.__lucky.set_single(request->param1(), BasicElement::BASIC);
	    return this->update_fight_property();
    }
	else if (request->cmd_name() == "killing")
	{
		return this->handle_kill_ditheism(this, request->param1());
	}
	else if (request->cmd_name().compare("fight_detail") == 0)
	{
		MSG_USER("fight      	 = %ld", this->fighter_id());
		MSG_USER("level      	 = %d", this->level());
		MSG_USER("experience 	 = %ld", this->fight_detail().__experience);
		MSG_USER("blood      	 = %d", this->fight_detail().__blood_total_i(this));
		MSG_USER("magic      	 = %d", this->fight_detail().__magic_total_i(this));
		MSG_USER("attack_upper   = %.02f", this->fight_detail().__attack_upper_total(this));
		MSG_USER("attack_lower   = %.02f", this->fight_detail().__attack_lower_total(this));
		MSG_USER("defence_upper  = %.02f", this->fight_detail().__defence_upper_total(this));
		MSG_USER("defence_lower  = %.02f", this->fight_detail().__defence_lower_total(this));
		MSG_USER("crit      	 = %.02f", this->fight_detail().__crit_total(this));
		MSG_USER("toughness      = %.02f", this->fight_detail().__toughness_total(this));
		MSG_USER("hit            = %.02f", this->fight_detail().__hit_total(this));
		MSG_USER("avoid          = %.02f", this->fight_detail().__avoid_total(this));
		MSG_USER("lucky          = %.02f", this->fight_detail().__lucky_total(this));
		MSG_USER("force          = %d", this->force_total_i());
		MSG_USER("league         = %d", this->league_id());
		MSG_USER("team           = %d", this->team_id());
		MSG_USER("pk_state       = %d", this->fight_detail().__pk_state);
		MSG_USER("scene_id       = %d", this->scene_id());
		MSG_USER("location       = (%d,%d)", this->location().pixel_x(), this->location().pixel_y());
		MSG_USER("space          = %d", this->space_id());
		MSG_USER("camp           = %d", this->camp_id());
		MSG_USER("speed          = %d", this->speed_total_i());

		return 0;
	}
    else if (request->cmd_name().compare("kill") == 0)
    {
        return this->inc_kill_monster(request->param1(), request->param2());
    }
    else if (request->cmd_name().compare("col") == 0)
    {
        return this->sync_collect_item(request->param1(), request->param2());
    }
    else if (request->cmd_name() == "map_reset_day")
    {
    	this->leaguer_set_nextday();
    	return 0;
    }
    else if (request->cmd_name().compare("transfer") == 0)
    {
		Proto10400111 req;
		req.set_scene_id(request->param1());
		req.set_pixel_x(request->param2());
		req.set_pixel_y(request->param3());
		return this->transfer_to_point(&req, true);
    }
    else if (request->cmd_name() == "birth")
    {
    	return this->transfer_to_born(true);
    }
    else if (request->cmd_name() == "main_town")
    {
    	return this->transfer_to_main_town();
    }
    else if (request->cmd_name() == "relive")
    {
    	Proto10400203 req;
    	req.set_relive(request->param1());
    	req.set_item_id(request->param2());
    	return this->request_relive(&req);
    }
	else if (request->cmd_name() == "fetch_around")
	{
		Scene* scene = this->fetch_scene();
		JUDGE_RETURN(scene != NULL, -1);

		Scene::MoverMap fighter_map;
		scene->fetch_all_around_fighter(this, fighter_map, this->location(), 10);

		MSG_USER("fetch around %d", fighter_map.size());
		return 0;
	}
    else if (request->cmd_name() == "clear_container")
    {
    	this->blood_container_.cur_blood_ = 100;
    	return 0;
    }
    else if (request->cmd_name() == "monster_drop")
    {
    	Scene* scene = this->fetch_scene();
    	JUDGE_RETURN(scene != NULL, -1);

    	AIDropPack* drop_pack = AIDROP_PACKAGE->pop_object();
    	JUDGE_RETURN(drop_pack != NULL, -1);

    	AIDropDetail& drop_detail = drop_pack->drop_detail();
    	drop_detail.ai_sort_ = 0;

    	drop_detail.item_obj_.__id = 211010001;
    	drop_detail.item_obj_.__amount = 1;
    	drop_detail.drop_type_ = GameEnum::DROP_TYPE_PERSON;
    	drop_detail.player_map_[this->role_id()] = this->role_id();

    	MoverCoord ai_coord = scene->rand_coord_by_pixel_radius(this->location(), 6 * 30);
    	return drop_pack->sign_and_enter_scene(ai_coord, this->fetch_scene());
    }
    else if (request->cmd_name() == "clear_drop_limit")
    {
    	MAP_MONITOR->reset_everyday();
    	return 0;
    }
    else if (request->cmd_name().compare("script") == 0)
    {
    	Proto10400901 req;
    	req.set_script_sort(request->param1());
        req.set_chapter(request->param2());
        req.set_piece(request->param3());
    	return this->request_enter_script(&req);
    }
    else if (request->cmd_name().compare("sword_skill") == 0)
    {
    	Proto10400943 req;
    	req.set_index(request->param1());
    	return this->sword_top_select_skill(&req);
    }
    else if (request->cmd_name().compare("buy_script") == 0)
    {
    	Proto10400920 req;
    	req.set_script_sort(request->param1());
    	req.set_inc_times(request->param2());
    	return this->request_script_add_times(&req);
    }
    else if (request->cmd_name().compare("startscript") == 0)
    {
    	Proto10400905 req;
    	req.set_novice_step(request->param1());
    	return this->request_run_script(&req);
    }
    else if (request->cmd_name().compare("firstscript") == 0)
    {
    	return this->request_first_start_script(0);
    }
    else if (request->cmd_name().compare("stopscript") == 0)
    {
    	return this->request_stop_script(0);
    }
    else if (request->cmd_name() == "legend_rank")
    {
    	Proto10400940 respond;
    	respond.set_num1(1);
    	respond.set_num2(10);
    	return this->fetch_legend_top_rank(&respond);
    }
    else if (request->cmd_name().compare("cleanscene") == 0)
    {
    	this->role_detail_.__scene_history.erase(request->param1());
    	return 0;
    }
    else if (request->cmd_name().compare("scripttimes") == 0)
    {
        if (request->param1() <= 0)
        {
            for (ScriptPlayerDetail::ScriptRecordMap::iterator iter = this->script_detail_.__record_map.begin();
                    iter != this->script_detail_.__record_map.end(); ++iter)
            {
            	MSG_DEBUG("script sort:%d",iter->second.__script_sort);
                iter->second.__used_times_tick = Time_Value::zero;
            }
        }
        else
        {
    	    const Json::Value &script_json = CONFIG_INSTANCE->script(request->param1());
    	    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
    	    ScriptPlayerDetail::ScriptRecord *script_rec = this->refresh_script_record(request->param1());
    	    script_rec->__used_times_tick = Time_Value::zero;
    	    const Json::Value &share_finish_json = script_json["finish_condition"]["share_finish_times"];
    	    for (uint i = 0; i < share_finish_json.size(); ++i)
    	    {
    	    	ScriptPlayerDetail::ScriptRecord *script_rec = this->refresh_script_record(share_finish_json[i].asInt());
    	    	script_rec->__used_times_tick = Time_Value::zero;
    	    }
        }
    	return 0;
    }
    else if (request->cmd_name().compare("area") == 0)
    {
    	return this->obtain_area_info(true);
    }
    else if (request->cmd_name().compare("fight") == 0)
    {
    	Proto10400201 req;
    	req.set_skill_id(request->param1());
    	GameFighter *fighter = 0;
    	int low_id = 0, high_id = 0;
    	split_long_to_int(this->role_id(), low_id, high_id);
    	Int64 fighter_id = merge_int_to_long(request->param2(), high_id);
    	if (this->find_fighter(fighter_id, fighter) == 0)
    	{
    		req.set_skill_pixel_x(fighter->location().pixel_x());
    		req.set_skill_pixel_y(fighter->location().pixel_y());
    		req.add_target_list()->set_target_id(fighter->fighter_id());
    	}
    	else
    	{
    		req.set_skill_pixel_x(this->location().pixel_x());
    		req.set_skill_pixel_y(this->location().pixel_y());
    	}
    	return this->prepare_fight_skill(&req);
    }
    else if (request->cmd_name().compare("pinfo") == 0)
    {
    	this->debug_output_fight_info(request->param1());
		return 0;
    }
    else if (request->cmd_name().compare("skillup") == 0)
    {
    	Proto10400711 req;
    	req.set_skill_id(request->param1());
    	return this->request_skill_level_up(&req);
    }
    else if (request->cmd_name().compare("stop") == 0)
    {
    	GameAI *game_ai = AIMANAGER->ai_package()->find_object(request->param1());
        if (game_ai != 0)
        	game_ai->stop_auto_action();
    }
    else if (request->cmd_name().compare("start") == 0)
    {
        GameAI *game_ai = AIMANAGER->ai_package()->find_object(request->param1());
        if (game_ai != 0)
        	game_ai->start_auto_action(1);
    }
    else if (request->cmd_name().compare("logobj") == 0)
    {
    	Scene *scene = this->fetch_scene();
    	JUDGE_RETURN(scene != NULL, -1);

    	GameFighter *fighter = 0;
    	int id = request->param1();
    	int type = GameCommon::fetch_mover_type(id);
    	switch (type)
    	{
    	case MOVER_TYPE_PLAYER:
    	{
    		Int64 role_id = merge_int_to_long(id, this->role_id_high());
    		MapPlayerEx *player = 0;
    		if (this->monitor()->find_player(role_id, player) != 0)
    			return -1;
    		fighter = player;
    		break;
    	}
    	case MOVER_TYPE_BEAST:
    	{
    		Int64 beast_id = merge_int_to_long(id, this->role_id_high());
    		MapBeast *beast = this->find_beast(beast_id);
    		if (beast == 0)
    			return -1;
    		fighter = beast;
    		break;
    	}
    	case MOVER_TYPE_MONSTER:
    	{
    		GameAI *ai = scene->find_ai(id);
    		if (ai == 0)
    			return -1;
    		fighter = ai;
    		break;
    	}
    	default:
    		return -1;
    	}

    	MSG_USER("==========fighter:%d=====================", id);
		MSG_USER("fight      	 = %d", fighter->fighter_id());
		MSG_USER("level      	 = %d", fighter->level());
		MSG_USER("experience 	 = %d", fighter->fight_detail().__experience);
		MSG_USER("blood      	 = %d", fighter->fight_detail().__blood_total_i(fighter));
		MSG_USER("magic      	 = %d", fighter->fight_detail().__magic_total_i(fighter));
		MSG_USER("attack_upper   = %.02f", fighter->fight_detail().__attack_upper_total(fighter));
		MSG_USER("attack_lower   = %.02f", fighter->fight_detail().__attack_lower_total(fighter));
		MSG_USER("defence_upper  = %.02f", fighter->fight_detail().__defence_upper_total(fighter));
		MSG_USER("defence_lower  = %.02f", fighter->fight_detail().__defence_lower_total(fighter));
		MSG_USER("crit      	 = %d", fighter->fight_detail().__crit_total_i(fighter));
		MSG_USER("toughness      = %d", fighter->fight_detail().__toughness_total_i(fighter));
		MSG_USER("lucky          = %.02f", fighter->fight_detail().__lucky_total(fighter));
		MSG_USER("force          = %d", fighter->force_total_i());
		MSG_USER("league         = %d", fighter->league_id());
		MSG_USER("team           = %d", fighter->team_id());
		MSG_USER("pk_state       = %d", fighter->fight_detail().__pk_state);
		MSG_USER("scene_id       = %d", fighter->scene_id());
		MSG_USER("location       = (%d,%d)", fighter->location().pixel_x(), fighter->location().pixel_y());
		MSG_USER("space          = %d", fighter->space_id());
		MSG_USER("camp           = %d", fighter->camp_id());
		GameAI *game_ai = dynamic_cast<GameAI *>(fighter);
		if (game_ai != 0)
			MSG_USER("birth          = (%d,%d)", game_ai->birth_coord().pixel_x(), game_ai->birth_coord().pixel_y());
		MSG_USER("========================================");

		return 0;
    }
    else if (request->cmd_name().compare("monster") == 0)
    {
    	MoverCoord coord;
    	coord.set_pixel(request->param1(), request->param2());
    	AIMANAGER->generate_monster_by_sort(500000002, coord, this->fetch_scene());
    }
    else if (request->cmd_name() == "fetch_scene")
    {
    	MSG_USER("scene_id: %d, space_id: %d", this->scene_id(), this->space_id());
    	return 0;
    }
    else if (request->cmd_name() == "start_escort")
    {
    	Proto10401522 info;
    	info.set_type(0);
    	return this->select_pescort_car_begin(&info);
    }
    else if(request->cmd_name().substr(0, 6).compare("mattr_") == 0)
    {
    	return this->modify_role_attr_for_test(request);
    }
	else if(request->cmd_name().compare("max") == 0)
	{
		this->fight_detail_.__blood_max.set_single(9e5, BasicElement::FLY);
		this->modify_blood_by_levelup(9e5);
		this->fight_detail_.__magic_max.set_single(9e5, BasicElement::FLY);
		this->modify_magic_by_notify(-9e5);
		this->fight_detail_.__attack_lower.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__attack_upper.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__defence_lower.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__defence_upper.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__hit.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__avoid.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__crit.set_single(9e5, BasicElement::FLY);
		this->fight_detail_.__toughness.set_single(9e5, BasicElement::FLY);
		this->update_fight_property();
	}
	else if(request->cmd_name().compare("reset_attr") == 0)
    {
		this->fight_detail().__level = this->level() - 1;
		this->level_upgrade();
    }
    else if (request->cmd_name().compare("sline") == 0)
    {
    	std::ostringstream msg_stream;
    	this->monitor()->scene_line_manager()->report_line_info(msg_stream);
    	MSG_USER("%s", msg_stream.str().c_str());
    }
    else if (request->cmd_name() == "exit")
    {
    	return this->request_exit_cur_system();
    }
    else if(request->cmd_name().compare("send_msg") == 0)
    {
    	return this->monitor()->process_inner_map_request(this->role_id(), request->param1());
    }
    else if (request->cmd_name() == "stopcruise")
    {
        Scene *scene = this->fetch_scene();
        if (scene == NULL)
            return 0;

        FloatAI *float_ai = scene->find_float_ai(this->role_id());
        if (float_ai != NULL)
        {
            float_ai->exit_scene();
            float_ai->sign_out();
        }
        return 0;
    }
    else if (request->cmd_name().compare("checkrect") == 0)
    {
//        MoverCoord pointA, pointB, pointC, pointD;
//        center_to_rect(pointA, pointB, pointC, pointD, this->fight_skill_.__skill_coord,
//                this->fight_skill_.__angle, GameCommon::json_by_level(skill_json["width"], skill_level).asInt() * 30, GameCommon::json_by_level(skill_json["height"], skill_level).asInt() * 30);
    	MoverCoord target;
    	target.set_pixel(4905,2745);
    	this->generate_move_path(target);
    	return 0;
    }
    else if (request->cmd_name() == "killvalue")
    {
    	int prev_color = this->fetch_name_color();
		this->inc_kill_value(this, request->param1());
		if (prev_color != this->fetch_name_color())
		{
			this->remove_last_color_info(prev_color);
			this->nofity_name_color_change();
		}
		return 0;
    }
    else if (request->cmd_name() == "minimap")
    {
    	if (request->param1() == 1)
    		return this->request_open_mini_map_pannel();
    	else
    		return this->request_close_mini_map_pannel();
    }
    else if (request->cmd_name() == "pickup")
    {
    	Proto10400002 req;
    	req.set_drop_id(request->param1() + BASE_OFFSET_AIDROP);
    	return this->pick_up_drop_goods_begin(&req);
    }
    else if (request->cmd_name() == "buff")
    {
    	return this->insert_defender_status(this, request->param1(), 0,
    			request->param2(), 0, request->param3(), request->param4());
    }
    else if (request->cmd_name() == "removestone")
    {
        Int64 role_id = merge_int_to_long(request->param1(), this->role_id_high());
        Proto10400937 req;
        req.set_mover_id(role_id);
        return this->request_remove_stone_state(&req);
    }
    else if (request->cmd_name() == "enter_SMBattle")
    {
    	return this->request_join_sm_battle();
    }
    else if (request->cmd_name() == "enter_tmarena")
    {
    	return this->request_enter_travel_marena();
    }
    else if (request->cmd_name() == "month_tarena")
    {
    	this->tarena_detail_.adjust_tick_ = 0;
    	this->BattleGroundActor::reset_everyday();
    	return 0;
    }
    else if (request->cmd_name() == "fetch_wb")
    {
        return this->request_wboss_info();
    }
    else if (request->cmd_name().compare("pocket") == 0)
    {
    	Proto10401023 respond;
        respond.set_scene_id(request->param1());
        this->request_get_wboss_pocket_award(&respond);
    }
    else if (request->cmd_name().compare("dice") == 0)
    {
    	Proto10401024 respond;
    	respond.set_scene_id(request->param1());
    	this->request_create_dice_num(&respond);
    }
    else if (request->cmd_name().compare("my_rank") == 0)
	{
    	Proto10401025 respond;
		respond.set_scene_id(request->param1());
		this->request_my_rank_info(&respond);
	}
    else if (request->cmd_name() == "enter_lw")
    {
    	return this->request_join_league_war();
    }
    else if (request->cmd_name() == "lwar_info")
    {
    	return this->request_league_war_info();
    }
    else if (request->cmd_name().compare("change_space") == 0)
    {
    	Proto10400352 respond;
    	respond.set_space_id(request->param1());
    	return this->request_change_space(&respond);
    }
    else if (request->cmd_name().compare("lwar_score") == 0)
    {
    	return this->request_league_war_score();
    }
    else if (request->cmd_name() == "enter_ma")
    {
    	return this->request_enter_player();
    }
    else if(request->cmd_name() == "transfer_lscort")
    {
    	Proto10400118 req;
    	req.set_pixel_x(request->param1());
    	req.set_pixel_y(request->param2());
    	return this->transfer_to_escort_npc(&req);
    }
    else if (request->cmd_name() == "tbattle")
    {
    	if (request->param1() == 1)
    	{
    		Proto30402503 inner_req;
    		inner_req.set_type(request->param1());
    		inner_req.set_ahead(request->param2());
    		inner_req.set_last(request->param3());
    		return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_BATTLE_SCENE_ID, &inner_req);
    	}
    	else if (request->param1() == 2)
    	{
    		return this->request_enter_travel_battle();
    	}
    	else if (request->param1() == 3)
    	{
    		return this->request_exit_cur_system();
    	}
        else if (request->param1() == 4)
        {
            TrvlBattleScene *scene = dynamic_cast<TrvlBattleScene *>(this->fetch_scene());
            if (scene != NULL)
                scene->handle_player_change_floor(this, this->space_id(), request->param2());
        }
    }
    else
    {
    	if (request->cmd_name() == "open_act")
    	{
    		CONFIG_INSTANCE->test_client_set_open_day(request->param1());
    	}
    	MAP_MONITOR->dispatch_to_logic(this, msg);
    }
#endif
	return 0;
}

int MapPlayerEx::start_map_sync_transfer()
{
	Time_Value nowtime = Time_Value::gettimeofday();
	if (this->transfer_timeout_tick() < nowtime)
	{
		MSG_USER("transfer timeout for logout %ld %s", this->role_id(), this->role_name().c_str());

		Proto31400005 req;
		req.set_role_id(this->role_id());
		this->send_to_logic_thread(req);
		this->sign_out(false);
		return 0;
	}

	// start map sync
	{
		this->sync_transfer_base();
		this->sync_transfer_move();
		this->sync_transfer_fight();
		this->sync_transfer_online();
        this->sync_transfer_script();
        this->sync_transfer_vip();
        this->sync_transfer_leaguer();
        this->sync_transfer_tiny();
        this->sync_transfer_escort();
        this->sync_transfer_team();
        this->sync_transfer_killer();
        this->sync_transfer_shape_and_label();
        this->sync_transfer_sm_battler();
		this->finish_sync_transfer_scene();
	}

	MSG_USER("log transfer 2 %ld %d %d %d", this->role_id(), this->scene_id(),
			this->location().pixel_x(), this->location().pixel_y());

	// sign out
	{
		this->sign_out();
	}

	return 0;
}

//在场景中，退出游戏处理
int MapPlayerEx::request_logout_game()
{
	this->notify_quit_trvl();

	switch (this->scene_id())
	{
	case GameEnum::TRVL_ARENA_SCENE_ID:
	{
		this->travel_arena_die_process(0);
		break;
	}
	default:
	{
	    break;
	}
	}

    this->exit_scene(EXIT_SCENE_LOGOUT);
    this->sign_out();

    return 0;
}

int MapPlayerEx::logout_and_use_offline()
{
	this->online_exit_scene();
	this->online_sign_out();

	/*玩家ID不变*/
	this->start_offline_ = true;

	this->copy_fighter_skill();
	this->start_auto_action();

	return 0;
}

int MapPlayerEx::logout_and_left_copy()
{
//	this->beast_unrecycle_ = true;
//
//	this->online_exit_scene();
//	this->online_sign_out();
//	this->monitor()->unbind_player(this->role_id());
//
//	/*玩家ID变化*/
//	this->copy_offline_ = true;
//	this->beast_unrecycle_ = false;
//
//	this->update_copy_role_id();
//	MAP_MONITOR->bind_player(this->role_id(), this);
//
//	this->set_cur_beast_offline();
//	this->update_scene_info();
//
//	this->copy_fighter_skill();
//	this->start_auto_action();

	return 0;
}

int MapPlayerEx::recycle_copy_and_sign_in(int sid, Message* msg)
{
	this->stop_auto_action();
	this->start_offline_ = false;
	this->resign(sid, msg);

	this->exit_scene();
    // 玩家移动坐标修正
    {
		if (this->is_movable_coord(this->location()) == true)
		{
			MoverCoord move_coord;
			move_coord.set_pos(this->location().pos_x(), this->location().pos_y());
			this->mover_detial_.__location = move_coord;
		}
		else
		{
			MoverCoord move_coord;
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					if (i == 0 && j == 0)
						continue;

					move_coord.set_pos(this->location().pos_x() + i, this->location().pos_y() + j);
					if (this->is_movable_coord(move_coord) == true)
						break;
				}
			}
			if (this->is_movable_coord(move_coord) == true)
				this->mover_detial_.__location = move_coord;
		}
    }

	this->notify_player_login();
	this->enter_scene();
	return 0;
}

int MapPlayerEx::set_offline_player_info(Proto30400432* request, Int64 src_role_id, Int64 machine_role_id)
{
    if (src_role_id > 0)
    {
        this->src_role_id_ = src_role_id;
    }

    if (machine_role_id > 0)
    {
        this->role_id_ = machine_role_id;
    }

    if (this->src_role_id_ == this->role_id_)
    {
    	this->start_offline_ = true;
    }
    else
    {
    	this->copy_offline_ = true;
    }

	this->mover_detial_.__scene_mode = request->scene_mode();
	this->mover_detial_.__location.set_pos(request->pos_x(), request->pos_y());

	const ProtoRoleInfo& role_info = request->role_info();
	this->role_detail_.__sex = role_info.role_sex();
	this->role_detail_.__career = role_info.role_career();
	this->role_detail_.__level = role_info.role_level();
	this->fight_detail_.__level = role_info.role_level();
	this->role_detail_.__name = role_info.role_name();

	const ProtoFightPro& fight_prop = request->fight_prop();
	this->fight_detail_.__attack_upper.set_single(fight_prop.attack());
	this->fight_detail_.__defence_upper.set_single(fight_prop.defence());
	this->fight_detail_.__avoid.set_single(fight_prop.avoid());
	this->fight_detail_.__crit.set_single(fight_prop.crit());
	this->fight_detail_.__toughness.set_single(fight_prop.toughness());
	this->fight_detail_.__hit.set_single(fight_prop.hit());
	this->mover_detial_.__speed.set_single(100.0);
    this->mover_detial_.__speed_multi.set_single(1, BasicElement::BASIC);

	this->fight_detail_.__blood = fight_prop.blood();
	this->fight_detail_.__magic = fight_prop.magic();
	this->fight_detail_.__blood_max.set_single(fight_prop.blood());
	this->fight_detail_.__magic_max.set_single(fight_prop.magic());

	for (int i = 0; i < request->skill_set_size(); ++i)
	{
		const ProtoPairObj& pair = request->skill_set(i);
		this->insert_skill(pair.obj_id(), pair.obj_value());
	}

	ShapeDetail& shape_detial = this->shape_detail();
	for (int i = 0; i < request->shape_set_size(); ++i)
	{
		const ProtoPairObj& pair = request->shape_set(i);
		shape_detial[pair.obj_id()] = pair.obj_value();
	}

    this->killed_info_.kill_num_ = request->kill_num();
    this->killed_info_.killing_value_ = request->killing_value();
    this->set_magic_weapon_id(request->weapon_id());
    this->set_magic_weapon_lvl(request->weapon_level());
    this->set_equip_refine_lvl(request->equip_refine_lvl());

    if (request->cur_label() > 0)
    	this->label_info()[request->cur_label()] = 9999;

    this->mount_detail(GameEnum::FUN_XIAN_WING).mount_grade_ = request->wing_level();
    this->mount_detail(GameEnum::FUN_XIAN_WING).mount_shape_ = request->wing_level();
    this->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_grade_ = request->solider_level();
    this->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_shape_ = request->solider_level();
    this->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_grade_ = request->magic_level();
    this->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_shape_ = request->magic_level();
    this->mount_detail(GameEnum::FUN_LING_BEAST).mount_grade_ = request->beast_level();
    this->mount_detail(GameEnum::FUN_LING_BEAST).mount_shape_ = request->beast_level();
    this->mount_detail(GameEnum::FUN_MOUNT).mount_grade_ = request->mount_level();
    this->mount_detail(GameEnum::FUN_MOUNT).mount_shape_ = request->mount_level();

    this->fashion_detail().select_id_ = request->fashion_id();
    this->fashion_detail().sel_color_id_ = request->fashion_color();

	return 0;
}

int MapPlayerEx::start_offline_copy(Int64 machine_role_id, Proto30400432* request)
{
	Scene* scene = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_scene(request->space_id(),
			request->scene_id(), scene) == 0, -1);

	this->role_id_ = machine_role_id;
	this->src_role_id_ = request->role_info().role_id();

	this->copy_offline_ = true;

	this->init_mover_scene(scene);
	this->set_offline_player_info(request);

	this->copy_offline_ = true;

	MapOfflineHook::sign_in();
	MapOfflineHook::enter_scene();

	return 0;
}

int MapPlayerEx::start_offline_copy(const BSONObj& res)
{
	JUDGE_RETURN(res.isEmpty() == false, -1);

	Int64 role_id = MAP_MONITOR->generate_role_copy_id();
    MAP_MONITOR->bind_player(role_id, this);
	JUDGE_RETURN(this->set_offline_copy_by_bson(res, role_id) == 0, -1);

	MapOfflineHook::sign_in();
	MapOfflineHook::enter_scene();

	MSG_USER("scene_id %d, %d, role_name %s", this->scene_id(), this->space_id(),
			this->role_name().c_str());

	this->refresh_mount_shape(this->mount_detail(GameEnum::FUN_MAGIC_EQUIP));
	this->refresh_mount_shape(this->mount_detail(GameEnum::FUN_LING_BEAST));
	this->refresh_mount_shape(this->mount_detail(GameEnum::FUN_MOUNT));

	return 0;
}

int MapPlayerEx::set_offline_copy_by_bson(const BSONObj& res, Int64 role_id /*=0*/)
{
	JUDGE_RETURN(res.isEmpty() == false, -1);

	// base info
	this->copy_offline_ = true;
	if(role_id == 0)
	{
		role_id = res[DBCopyPlayer::ID].numberLong();
	}
	this->role_id_ = role_id;
	this->src_role_id_ = res[DBCopyPlayer::ID].numberLong();

	// role info
	this->role_detail_.__sex = res[DBCopyPlayer::SEX].numberInt();
	this->role_detail_.__level = res[DBCopyPlayer::LEVEL].numberInt();
	this->role_detail_.__career = res[DBCopyPlayer::CAREER].numberInt();
	this->fight_detail_.__level = res[DBCopyPlayer::LEVEL].numberInt();
	this->role_detail_.__name = res[DBCopyPlayer::NAME].str();
	this->role_detail_.__league_id = res[DBCopyPlayer::LEAGUE_ID].numberLong();
	this->role_detail_.__league_name = res[DBCopyPlayer::LEAGUE_NAME].str();
	this->role_detail_.__league_pos = res[DBCopyPlayer::LEAGUE_POS].numberInt();
	this->role_detail_.__partner_id = res[DBCopyPlayer::PARTNER_ID].numberLong();
	this->role_detail_.__partner_name = res[DBCopyPlayer::PARTNER_NAME].str();
	this->role_detail_.__wedding_id = res[DBCopyPlayer::WEDDING_ID].numberLong();
	this->role_detail_.__wedding_type = res[DBCopyPlayer::WEDDING_TYPE].numberInt();

	// fight info
	this->fight_detail_.__attack_lower.set_single(res[DBCopyPlayer::ATTACK_LOWER].numberDouble());
	this->fight_detail_.__attack_upper.set_single(res[DBCopyPlayer::ATTACK_UPPER].numberDouble());
	this->fight_detail_.__defence_lower.set_single(res[DBCopyPlayer::DEFENCE_LOWER].numberDouble());
	this->fight_detail_.__defence_upper.set_single(res[DBCopyPlayer::DEFENCE_UPPER].numberDouble());
	this->fight_detail_.__avoid.set_single(res[DBCopyPlayer::DODGE].numberDouble());
	this->fight_detail_.__crit.set_single(res[DBCopyPlayer::CRIT].numberDouble());
	this->fight_detail_.__toughness.set_single(res[DBCopyPlayer::TOUGHNESS].numberDouble());
	this->fight_detail_.__hit.set_single(res[DBCopyPlayer::HIT].numberDouble());
	this->fight_detail_.__lucky.set_single(res[DBCopyPlayer::LUCKY].numberDouble());
	this->mover_detial_.__speed.reset();
	this->mover_detial_.__speed_multi.reset();
	this->mover_detial_.__speed.set_single(110.0);
    this->mover_detial_.__speed_multi.set_single(1, BasicElement::BASIC);

	this->fight_detail_.__blood = res[DBCopyPlayer::BLOOD].numberInt();
	this->fight_detail_.__magic = res[DBCopyPlayer::MAGIC].numberInt();
	this->fight_detail_.__blood_max.set_single(res[DBCopyPlayer::BLOOD].numberInt());
	this->fight_detail_.__magic_max.set_single(res[DBCopyPlayer::MAGIC].numberInt());
	this->fight_detail_.__damage.set_single(res[DBCopyPlayer::DAMAGE].numberDouble());
	this->fight_detail_.__reduction.set_single(res[DBCopyPlayer::REDUCTION].numberDouble());
	this->mount_detail(GameEnum::FUN_XIAN_WING).mount_grade_ = res[DBCopyPlayer::WING_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_XIAN_WING).mount_shape_ = res[DBCopyPlayer::WING_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_grade_ = res[DBCopyPlayer::SOLIDER_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_shape_ = res[DBCopyPlayer::SOLIDER_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_grade_ = res[DBCopyPlayer::MAGIC_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_shape_ = res[DBCopyPlayer::MAGIC_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_LING_BEAST).mount_grade_ = res[DBCopyPlayer::BEAST_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_LING_BEAST).mount_shape_ = res[DBCopyPlayer::BEAST_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_MOUNT).mount_grade_ = res[DBCopyPlayer::MOUNT_LEVEL].numberInt();
	this->mount_detail(GameEnum::FUN_MOUNT).mount_shape_ = res[DBCopyPlayer::MOUNT_LEVEL].numberInt();

	this->fashion_detail().select_id_ = res[DBCopyPlayer::FASHION_ID].numberInt();
	this->fashion_detail().sel_color_id_ = res[DBCopyPlayer::FASHION_COLOR].numberInt();

	this->set_magic_weapon_id(res[DBCopyPlayer::WEAPON_LEVEL].numberInt() / 1000);
	this->set_magic_weapon_lvl(res[DBCopyPlayer::WEAPON_LEVEL].numberInt() % 1000);
	this->set_equip_refine_lvl(res[DBCopyPlayer::EQUIP_REFINE_LVL].numberInt());

	this->caculate_total_force();

	int skill_id = 400011001 + 1000 * (this->role_detail().__sex - 1);

    for (int i = 0; i < 4; ++i)
    {
//        if (GameCommon::is_base_skill(role_json["skill"][i].asInt(), this->fetch_mover_type()))
//    		this->insert_auto_skill(role_json["skill"][i].asInt(), 1);
    	this->insert_skill(skill_id + i, 1);
    }

	// skill info
	{
		BSONObjIterator iter(res.getObjectField(DBCopyPlayer::SKILL_SET.c_str()));
		while (iter.more())
		{
			BSONObj skill_obj = iter.next().embeddedObject();
			BSONObj tick_obj = skill_obj.getObjectField(Skill::SSkill::USETICK.c_str());

			Time_Value tick(tick_obj[Skill::SSkill::Tick::SEC].numberInt(),
					tick_obj[Skill::SSkill::Tick::USEC].numberInt());
			int skill_id = skill_obj[Skill::SSkill::SKILL_ID].numberInt();
			if (skill_id <= 0)
				continue;
			this->insert_skill(skill_id, skill_obj[Skill::SSkill::LEVEL].numberInt());
		}
	}

	// shape info
	GameCommon::bson_to_map(this->shape_detail(), res.getObjectField(
			DBCopyPlayer::SHAPE_SET.c_str()));

	return 0;
}

int MapPlayerEx::start_offline_player(Proto30400432* request)
{
	Scene* scene = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_scene(request->space_id(),
			request->scene_id(), scene) == 0, -1);

	this->role_id_ = request->role_info().role_id();
	this->start_offline_ = true;

    MAP_MONITOR->bind_player(this->role_id_, this); 

	this->init_mover_scene(scene);
	this->set_offline_player_info(request);

	MapOfflineHook::sign_in();
	MapOfflineHook::enter_scene();

	return 0;
}

int MapPlayerEx::start_offline_beast(Proto30400432* request)
{
//	this->set_beast_offline_info(request->mutable_offline_beast());
//
//	MapBeast *beast = this->fetch_cur_beast();
//	if (beast != NULL)
//		this->beast_enter_scene(beast);

	return 0;
}

int MapPlayerEx::online_exit_scene(void)
{
	SCENE_MODE_SWITCH(exit_scene(EXIT_SCENE_LOGOUT));
    return 0;
}

int MapPlayerEx::offline_exit_scene(void)
{
	MapMaster::master_exit_scene();
	MapOfflineHook::exit_scene();
	return 0;
}

int MapPlayerEx::online_sign_out()
{
	MapPlayer::sign_out(true);
	MapPlayerScript::sign_out(true);
	return 0;
}

int MapPlayerEx::offline_sign_out()
{
	MapMaster::master_sign_out();
	MapOfflineHook::sign_out();

	MSG_USER("unbind player %ld", this->role_id());
    this->monitor()->unbind_player(this->role_id());
    this->monitor()->player_pool()->push(this);

    return 0;
}

int MapPlayerEx::sign_in(const int type)
{
	SCENE_MODE_SWITCH(sign_in(type));
    return 0;
}

int MapPlayerEx::enter_scene(const int type)
{
	if (this->is_online_player() == false)
	{
		return MapOfflineHook::enter_scene(type);
	}
	else
	{
		this->fighter_timer_.schedule_timer(0.1);
		SCENE_MODE_SWITCH(enter_scene(type));
		return 0;
	}
}

int MapPlayerEx::exit_scene(const int type)
{
	this->fighter_timer_.cancel_timer();

	this->exit_scene_i(type);
	GameFighter::exit_scene(type);

    return 0;
}

int MapPlayerEx::sign_out(const bool is_save_player)
{
	GameFighter::sign_out();
#ifndef LOCAL_DEBUG
    SESSION_MANAGER->unbind_and_push(this->role_detail_.__account);
#endif

    MapPlayerScript::sign_out(is_save_player);
    MapLeaguer::sign_out(is_save_player);
    MapPlayer::sign_out(is_save_player);
    HotspringActor::sign_out(is_save_player);

    MAP_MONITOR->unbind_player(this->role_id());
    MAP_MONITOR->player_pool()->push(this);

	return 0;
}

int MapPlayerEx::exit_scene_i(int type)
{
	SCENE_MODE_SWITCH(exit_scene(type));
	return 0;
}

int MapPlayerEx::transfer_to_other_scene(Message *msg)
{
//    SCENE_MODE_SWITCH(transfer_to_other_scene(msg));
    return 0;
}

int MapPlayerEx::prepare_fight_skill(Message *msg)
{
    // 防止发包刷攻击
	Time_Value nowtime = Time_Value::gettimeofday();
	JUDGE_RETURN(this->fight_frequency_tick_ < nowtime, 0);

	this->fight_frequency_tick_ = nowtime + Time_Value(0, 100000);
    SCENE_MODE_SWITCH(prepare_fight_skill(msg));
    return 0;
}

int MapPlayerEx::request_relive(Message *msg)
{
	if (this->is_death() == false)
	{
		return this->notify_mover_cur_location();
	}
	else
	{
		JUDGE_RETURN(this->add_validate_operate_tick() == true, 0);

	    const Json::Value &scene_json = this->scene_conf();
	    CONDITION_NOTIFY_RETURN(scene_json["relive_limit"].asInt() == 0,
	    		RETURN_RELIVE, ERROR_CLIENT_OPERATE);

	    SCENE_MODE_SWITCH(request_relive(msg));
	}

    return 0;
}

int MapPlayerEx::process_relive_after_used_item(void)
{
    SCENE_MODE_SWITCH(process_relive_after_used_item());
    return 0;
}

int MapPlayerEx::modify_blood_by_fight(const double inc_value, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	SCENE_MODE_SWITCH(modify_blood_by_fight(inc_value, fight_tips, attackor, skill_id));
	return 0;
}

int MapPlayerEx::modify_magic_by_notify(const double value, const int fight_tips)
{
	SCENE_MODE_SWITCH(modify_magic_by_notify(value, fight_tips));
    return 0;
}

int MapPlayerEx::notify_client_enter_info()
{
	SCENE_MODE_SWITCH(notify_client_enter_info());
    return 0;
}

double MapPlayerEx::fetch_addition_exp_percent(void)
{
	SCENE_MODE_SWITCH(fetch_addition_exp_percent());
    return 0;
}

int MapPlayerEx::schedule_move(const MoverCoord &step, const int toward, const Time_Value &arrive_tick)
{
    SCENE_MODE_SWITCH(schedule_move(step, toward, arrive_tick));
    return 0;
}

int MapPlayerEx::client_obtain_area_info()
{
	int ret = this->loign_enter_scene();

	JUDGE_RETURN(ret == 0, ret);
	JUDGE_RETURN(this->is_enter_scene() == true, -1);

	return this->obtain_area_info(true);
}

int MapPlayerEx::loign_enter_scene()
{
	JUDGE_RETURN(this->is_enter_scene() == false, 0);

	int ret = this->enter_scene(ENTER_SCENE_TRANSFER);
	JUDGE_RETURN(ret != 0, 0);

	MSG_USER("Error Map player enter scene %ld, %d, %d",
			this->mover_id(), ret, this->scene_id());

	if (GameCommon::is_normal_scene(this->role_detail_.__save_scene.scene_id_))
	{
		//传回上次保存场景
		this->transfer_to_save_scene();
	}
	else
	{
		//传送回出生点
		this->transfer_to_born(true);
	}

	return ret;
}

int MapPlayerEx::obtain_area_info(int request_flag)
{
//	if (this->is_login() == true)
//	{
//		this->check_pa_event_climb_tower_script();
//	}

	SCENE_MODE_SWITCH(obtain_area_info(request_flag));
    return 0;
}

int MapPlayerEx::die_process(int64_t fighter_id)
{
	if (!this->is_online_player())
	{
		MapMaster::master_exit_scene();
		return MapOfflineHook::die_process(fighter_id);
	}
	else
	{
		SCENE_MODE_SWITCH(die_process(fighter_id));
        return 0;
	}
}

bool MapPlayerEx::is_movable_coord(const MoverCoord &coord)
{
	SCENE_MODE_SWITCH(is_movable_coord(coord));
	return false;
}

int MapPlayerEx::validate_movable(const MoverCoord &step)
{
    SCENE_MODE_SWITCH(validate_movable(step));
    return 0;
}

int MapPlayerEx::validate_relive_point(int check_type)
{
    SCENE_MODE_SWITCH(validate_relive_point(check_type));
    return 0;
}

int MapPlayerEx::validate_relive_locate(const int item_id)
{
    SCENE_MODE_SWITCH(validate_relive_locate(item_id));
    return 0;
}

int MapPlayerEx::process_relive(const int relive_mode, MoverCoord &relive_coord)
{
	SCENE_MODE_SWITCH(process_relive(relive_mode, relive_coord));
    return 0;
}

int MapPlayerEx::modify_role_attr_for_test(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11499999*, request, RETURN_ML_TEST_COMMAND);

	double value = request->param1();
	if(request->cmd_name().compare("mattr_attack") == 0)
	{
    	this->fight_detail_.__attack_lower.set_single(value, BasicElement::BASIC);
    	this->fight_detail_.__attack_upper.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_defence") == 0)
	{
    	this->fight_detail_.__defence_lower.set_single(value, BasicElement::BASIC);
    	this->fight_detail_.__defence_upper.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_hit") == 0)
	{
    	this->fight_detail_.__hit.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_avoid") == 0)
	{
    	this->fight_detail_.__avoid.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_crit") == 0)
	{
    	this->fight_detail_.__crit.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_toughness") == 0)
	{
    	this->fight_detail_.__toughness.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_blood") == 0)
	{
		this->modify_blood_by_levelup(value);
	}
	else if(request->cmd_name().compare("mattr_blood_max") == 0)
	{
    	this->fight_detail_.__blood_max.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_magic") == 0)
	{
		this->modify_magic_by_notify(this->fight_detail_.__magic - value);
	}
	else if(request->cmd_name().compare("mattr_magic_max") == 0)
	{
    	this->fight_detail_.__magic_max.set_single(value, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_damage_multi") == 0)
	{
    	this->fight_detail_.__damage_multi.set_single(
    			value / GameEnum::DAMAGE_ATTR_PERCENT, BasicElement::BASIC);
	}
	else if(request->cmd_name().compare("mattr_reduction_multi") == 0)
	{
    	this->fight_detail_.__reduction_multi.set_single(
    			value / GameEnum::DAMAGE_ATTR_PERCENT, BasicElement::BASIC);
	}
	else
	{
		return this->respond_to_client_error(RETURN_ML_TEST_COMMAND, ERROR_CLIENT_OPERATE);
	}

    this->update_fight_property(BasicElement::BASIC);
    return 0;
}

int MapPlayerEx::fetch_relive_data(const int relive_mode, int &blood_percent, int &item_id, int &item_amount)
{
    SCENE_MODE_SWITCH(fetch_relive_data(relive_mode, blood_percent, item_id, item_amount));
    return 0;
}

int MapPlayerEx::recovert_magic(const Time_Value &nowtime)
{
    SCENE_MODE_SWITCH(recovert_magic(nowtime));
    return 0;
}

int MapPlayerEx::recovert_blood(const Time_Value &nowtime)
{
    SCENE_MODE_SWITCH(recovert_blood(nowtime));
    return 0;
}

int MapPlayerEx::validate_prepare_attack_target(void)
{
	SCENE_MODE_SWITCH(validate_prepare_attack_target());
	return 0;
}

int MapPlayerEx::validate_launch_attack_target(void)
{
	SCENE_MODE_SWITCH(validate_launch_attack_target());
	return 0;
}


int MapPlayerEx::cached_timeout(const Time_Value &nowtime)
{
    SCENE_MODE_SWITCH(cached_timeout(nowtime));
    return 0;
}

double MapPlayerEx::fetch_reduce_hurt_rate(void)
{
    SCENE_MODE_SWITCH(fetch_reduce_hurt_rate());
    return 0.0;
}

int MapPlayerEx::gather_state_begin(Message* msg)
{
	SCENE_MODE_SWITCH(gather_state_begin(msg));
	return 0;
}

int MapPlayerEx::gather_state_end()
{
	SCENE_MODE_SWITCH(gather_state_end());
	return 0;
}

int MapPlayerEx::gather_goods_begin(Message* msg)
{
	SCENE_MODE_SWITCH(gather_goods_begin(msg));
    return 0;
}

int MapPlayerEx::gather_goods_done(Message* msg)
{
	SCENE_MODE_SWITCH(gather_goods_done(msg));
	return 0;
}

int MapPlayerEx::process_pick_up_suceess(AIDropPack *drop_pack)
{
    SCENE_MODE_SWITCH(process_pick_up_suceess(drop_pack));
    return 0;
}

int MapPlayerEx::pick_up_drop_goods_begin(Message* msg)
{
    SCENE_MODE_SWITCH(pick_up_drop_goods_begin(msg));
    return 0;
}

int MapPlayerEx::set_pk_state(const int state)
{
    SCENE_MODE_SWITCH(set_pk_state(state));
    return 0;
}


void MapPlayerEx::record_exception_skill_info()
{
	int role_level = this->level();
	JUDGE_RETURN(role_level >= 65, ;);

	int total_count = 0;
	FightDetail& fight_detail = this->fight_detail();

	for (SkillMap::iterator iter = fight_detail.__skill_map.begin();
			iter != fight_detail.__skill_map.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		JUDGE_CONTINUE(skill->__check_flag == 1);

		total_count++;
		JUDGE_CONTINUE(skill->__level_type > 0);
		JUDGE_CONTINUE(skill->__level <= 1);

		this->record_other_serial(MAIN_EXECEPTION_SERIAL, 4, skill->__skill_id, role_level, 1);
	}

	if (total_count < 10)
	{
		this->record_other_serial(MAIN_EXECEPTION_SERIAL, 4, total_count, role_level, 2);
	}
}

void MapPlayerEx::debug_output_fight_info(const int type)
{
	std::stringstream ss;
	BasicElement *base_elem = NULL, *multi_elem = NULL;
	switch (type)
	{
		case GameEnum::ATTACK_UPPER:
		{
			ss << "attack_upper: " << this->fight_detail_.__attack_upper_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__attack_upper);
			multi_elem = &(this->fight_detail_.__attack_upper_multi);
			break;
		}
		case GameEnum::ATTACK_LOWER:
		{
			ss << "attack_lower: " << this->fight_detail_.__attack_lower_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__attack_lower);
			multi_elem = &(this->fight_detail_.__attack_lower_multi);
			break;
		}
		case GameEnum::DEFENCE_UPPER:
		{
			ss << "defence_upper: " << this->fight_detail_.__defence_upper_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__defence_upper);
			multi_elem = &(this->fight_detail_.__defence_upper_multi);
			break;
		}
		case GameEnum::DEFENCE_LOWER:
		{
			ss << "defence_lower: " << this->fight_detail_.__defence_lower_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__defence_lower);
			multi_elem = &(this->fight_detail_.__defence_lower_multi);
			break;
		}
		case GameEnum::HIT:
		{
			ss << "hit: " << this->fight_detail_.__hit_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__hit);
			multi_elem = &(this->fight_detail_.__hit_multi);
			break;
		}
		case GameEnum::AVOID:
		{
			ss << "avoid: " << this->fight_detail_.__avoid_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__avoid);
			multi_elem = &(this->fight_detail_.__avoid_multi);
			break;
		}
		case GameEnum::CRIT:
		{
			ss << "crit: " << this->fight_detail_.__crit_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__crit);
			multi_elem = &(this->fight_detail_.__crit_hurt_multi);
			break;
		}
		case GameEnum::TOUGHNESS:
		{
			ss << "toughness: " << this->fight_detail_.__toughness_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__toughness);
			multi_elem = &(this->fight_detail_.__toughness_multi);
			break;
		}
		case GameEnum::LUCKY:
		{
			ss << "lucky: " << this->fight_detail_.__lucky_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__lucky);
			multi_elem = &(this->fight_detail_.__lucky_multi);
			break;
		}
		case GameEnum::DAMAGE:
		{
			ss << "add hurt: " << this->fight_detail_.__damage_total(this) << ", " << this->fight_detail_.__damage_rate_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__damage);
			multi_elem = &(this->fight_detail_.__damage_multi);
			break;
		}
		case GameEnum::REDUCTION:
		{
			ss << "reduce hurt: " << this->fight_detail_.__reduction_total(this) << ", " << this->fight_detail_.__reduction_rate_total(this) << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__reduction);
			multi_elem = &(this->fight_detail_.__reduction_multi);
			break;
		}
		case GameEnum::BLOOD_MAX:
		{
			ss << "blood: " << this->fight_detail_.__blood_total(this) << ", " << this->fight_detail_.__blood << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__blood_max);
			multi_elem = &(this->fight_detail_.__blood_multi);
			break;
		}
		case GameEnum::MAGIC_MAX:
		{
			ss << "magic: " << this->fight_detail_.__magic_total(this) << ", " << this->fight_detail_.__magic << ", name_color: " << this->fight_detail_.__color_all_per << std::endl;
			base_elem = &(this->fight_detail_.__magic_max);
			multi_elem = &(this->fight_detail_.__magic_multi);
			break;
		}
		default:
			return;
	}

	if (base_elem != NULL)
	{
		ss << "base_prop:" << std::endl;
		for (int i = BasicElement::BASIC; i < BasicElement::OFFSET_END; ++i)
			ss << i << ": " << base_elem->single(i) << ", ";
	}
	if (multi_elem != NULL)
	{
		ss << "multi_prop:" << std::endl;
		for (int i = BasicElement::BASIC; i < BasicElement::OFFSET_END; ++i)
			ss << i << ": " << multi_elem->single(i) << ", ";
	}
	MSG_USER("%s", ss.str().c_str());
}

int MapPlayerEx::handle_inner_trade_distance(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400242 *, request, msg, -1);
	int recogn  = request->cur_recogn();
	Int64 role_id = request->role_id();
	MapPlayerEx *des_player =NULL;
	this->monitor()->find_player(role_id,des_player);

	if(des_player == NULL)
	{
		//到逻辑服判断玩家是否真的离线
		this->trade_fetch_player_online_state(role_id,recogn);
		return 0;
	}
	if( (this->scene_id() != des_player->scene_id()) || (this->scene_mode() != des_player->scene_mode())
			|| (this->space_id() != des_player->space_id()))
	{
		if(recogn == RETURN_TRADE_INVITE_RESPOND)
		{
			des_player->respond_to_client_error(recogn, ERROR_TRADE_REQUEST_IGNORE);
		}
		this->respond_to_client_error(recogn, ERROR_TRADE_NO_IN_CURRSCENE);
		MAP_MONITOR->process_inner_logic_request(this->role_id_,INNER_NOTIFY_TRADE_CANCEL);
		return 0;
	}
	int length = CONFIG_INSTANCE->tiny("length").asInt();
	if( length <= 0 )
		length = 1140;
	const int block_with = length / 30;//1屏宽度
	if(coord_offset_grid(this->location(), des_player->location()) > block_with)
	{
		if(recogn == RETURN_TRADE_INVITE_RESPOND)
		{
			des_player->respond_to_client_error(recogn, ERROR_TRADE_REQUEST_IGNORE);
		}
		this->respond_to_client_error(recogn, ERROR_TRADE_OVER_ONE_SCREEN);
		MAP_MONITOR->process_inner_logic_request(this->role_id_,INNER_NOTIFY_TRADE_CANCEL);
		return 0;
	}
	else
	if(coord_offset_grid(this->location(), des_player->location()) > 8)
	{
		if(recogn == RETURN_TRADE_INVITE_RESPOND)
		{
			des_player->respond_to_client_error(recogn, ERROR_TRADE_REQUEST_IGNORE);
		}
		this->respond_to_client_error(recogn, ERROR_TRADE_OVER_DISTANCE);
		MAP_MONITOR->process_inner_logic_request(this->role_id_,INNER_NOTIFY_TRADE_CANCEL);
		return 0;
	}
	Proto31400242  respon;
	respon.set_role_id(role_id);
	respon.set_cur_recogn(recogn);
	MAP_MONITOR->process_inner_logic_request(this->role_id_,respon);
	return 0;
}

int MapPlayerEx::trade_fetch_player_online_state(Int64 role_id,const int recogn)
{
	Proto30101607 inner_req;
	inner_req.set_role_id(role_id);
	inner_req.set_recogn(recogn);
	return MAP_MONITOR->dispatch_to_logic(this,&inner_req);
}

int MapPlayerEx::trade_get_player_online_state(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30101607 *, request, -1);
	if(request->on_line())
	{
		this->respond_to_client_error(request->recogn(),ERROR_TRADE_NO_IN_CURRSCENE);
	}
	else
	{
		this->respond_to_client_error(request->recogn(),ERROR_PLAYER_OFFLINE);
	}
	MAP_MONITOR->process_inner_logic_request(this->role_id_,INNER_NOTIFY_TRADE_CANCEL);
	return 0;
}

int MapPlayerEx::check_travel_timeout(void)
{
    SCENE_MODE_SWITCH(check_travel_timeout());
    return 0;
}

