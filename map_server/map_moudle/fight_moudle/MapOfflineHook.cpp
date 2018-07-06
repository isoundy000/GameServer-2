/*
 * MapOfflineHook.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: peizhibi
 */

#include "MapOfflineHook.h"
#include "MapMonitor.h"
#include "BaseScript.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "AreaField.h"
#include "AreaMonitor.h"
#include "AIManager.h"

int MapOfflineHook::RecoverTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int MapOfflineHook::RecoverTimer::handle_timeout(const Time_Value &tv)
{
	return this->offline_hook_->recover_timeout();
}

MapOfflineHook::MapOfflineHook()
{
	// TODO Auto-generated constructor stub
	this->recover_timer_.offline_hook_ = this;
}

MapOfflineHook::~MapOfflineHook()
{
	// TODO Auto-generated destructor stub
}

int MapOfflineHook::sign_in(const int type)
{
	return AutoMapFighter::sign_in();
}

int MapOfflineHook::enter_scene(const int type)
{
	AutoMapFighter::enter_scene(type);
	this->auto_detail_.fetch_skill_mode_ = AutoFighterDetail::SKILL_MODE_COPY;

//	this->mover_detial_.__speed.set_single(110.0, BasicElement::BASIC);
	this->start_auto_action(MAP_MONSTER_INTERVAL);
	this->update_scene_info();
	this->start_recover_timer();

	return 0;
}

int MapOfflineHook::exit_scene(const int type)
{
	AutoMapFighter::exit_scene(type);

	this->set_aim_object(0);
	this->recover_timer_.cancel_timer();
	this->stop_auto_action();

	return 0;
}

int MapOfflineHook::sign_out(const bool is_save_player)
{
	return AutoMapFighter::sign_out();
}

void MapOfflineHook::reset()
{
	this->recover_timer_.cancel_timer();

	this->reset_auto_fighter();
	this->offline_detail_.reset();
}

int MapOfflineHook::die_process(const int64_t fighter_id)
{
	GameFighter::die_process(fighter_id);

	this->fb_die_process(fighter_id);
	this->arena_die_process(fighter_id);

	return 0;
}

int MapOfflineHook::auto_action_timeout(const Time_Value &nowtime)
{
	MapOfflineHook::execute_ai_tree();
	MapOfflineHook::fb_releive();
	MapOfflineHook::arena_field_timeout();
	AutoMapFighter::auto_action_timeout(nowtime);

	return 0;
}

Time_Value MapOfflineHook::calculate_move_tick(void)
{
	double tick = 0.11, sec = 0.0;
	tick *= (100.0 / this->speed_total());
	double usec = ::modf(tick, &sec) * 1e6;
	return Time_Value(sec, usec);
	return AutoMapFighter::calculate_move_tick();
}

int MapOfflineHook::fetch_attack_distance(void)
{
	return GameFighter::fetch_attack_distance();
}

int MapOfflineHook::schedule_move_fighter(void)
{
    JUDGE_RETURN(this->guide_skill() == 0, -1);
	JUDGE_RETURN(this->validate_fighter_movable() == 0, -1);

	// 攻击完首次移动时先停一下，让怪物播放完攻击
	if (this->offline_detail_.last_is_move_ == false)
	{
		this->offline_detail_.last_is_move_ = true;
		this->push_schedule_time(1.7);
	}

	return AutoMapFighter::schedule_move_fighter();
}

int MapOfflineHook::auto_fighter_attack(void)
{
	// 首次攻击时先停一下，让怪物播放完移动
	if (this->offline_detail_.last_is_move_ == true)
	{
		this->offline_detail_.last_is_move_ = false;
		if (this->is_in_script_mode())
		{
			// 副本里停一下让机器人播放完移动再触发触击
			this->push_schedule_time(0.5);
			return 0;
		}
	}

	return AutoMapFighter::auto_fighter_attack();
}

void MapOfflineHook::update_scene_info()
{
	this->update_fb_scene_info();
	this->update_arena_scene_info();
}

void MapOfflineHook::copy_fighter_skill()
{
	/*
	this->clear_skill_queue();
	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);

	const Json::Value &role_json = CONFIG_INSTANCE->role(player->role_detail().__career);
	for (uint i = 0; i < role_json["skill"].size(); ++i)
	{
		if (GameCommon::is_base_skill(role_json["skill"][i].asInt(), player->fetch_mover_type()))
			this->insert_skill(role_json["skill"][i].asInt(), 1);
	}

	IntVec skill_set = player->fetch_cur_scheme_skill();
	for (IntVec::iterator iter = skill_set.begin(); iter != skill_set.end(); ++iter)
	{
		FighterSkill* fighter_skill = NULL;
		JUDGE_CONTINUE(this->find_skill(*iter, fighter_skill) == 0);

		this->insert_skill(fighter_skill->__skill_id, fighter_skill->__level);
	}
	*/
}

void MapOfflineHook::start_recover_timer()
{
	int auto_time = CONFIG_INSTANCE->copy_player("auto_time").asInt();
	this->recover_timer_.schedule_timer(Time_Value(auto_time));
}

void MapOfflineHook::fb_releive()
{
	JUDGE_RETURN(this->is_death() == true, ;);
	JUDGE_RETURN(this->scene_mode() == SCENE_MODE_SCRIPT, ;);

	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);

	MoverCoord mover_coord = this->location();
	player->handle_player_relive(mover_coord);

	this->offline_detail_.die_times_ += 1;
}

void MapOfflineHook::update_fb_scene_info()
{
	JUDGE_RETURN(this->scene_mode() == SCENE_MODE_SCRIPT, ;);
	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);

	BaseScript* script = player->fetch_script();
	JUDGE_RETURN(script != NULL, ;);

	ScriptDetail& script_detial = script->script_detail();
	JUDGE_RETURN(script_detial.replacements_set_.count(player->role_id()) == 0, ;);

	MapTeamInfo& team_info = player->team_info();
	team_info.teamer_set_ = script_detial.teamer_set_;
	script_detial.replacements_set_[player->role_id()] = true;

	Proto30100226 id_info;
	id_info.set_team_id(script->team_id());
	id_info.set_new_id(player->role_id());
	id_info.set_src_id(player->src_role_id());
	MAP_MONITOR->dispatch_to_logic(&id_info);
}

void MapOfflineHook::update_arena_scene_info()
{
	JUDGE_RETURN(this->scene_id() == GameEnum::ARENA_SCENE_ID, ;);

	AreaField* arena_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(arena_field != NULL, ;);

	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);
	arena_field->update_arena_fighter(player->role_id(), player->src_role_id());

//	this->update_copy_player_speed(1.6);
	this->set_camp_id(MAP_MONITOR->generate_camp_id());
	this->mover_detial_.__toward = MOVE_TOWARD_LEFT;
}


void MapOfflineHook::arena_field_timeout()
{
	JUDGE_RETURN(this->is_death() == false, ;);
	JUDGE_RETURN(this->scene_id() == GameEnum::ARENA_SCENE_ID, ;);

	AreaField* arena_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(arena_field != NULL, ;);

	MapPlayerEx* player = arena_field->find_rivial(this->fighter_id());
	JUDGE_RETURN(player != NULL, ;);

	this->set_aim_object(player->fighter_id());
	this->chase_or_attack_fighter();
}

void MapOfflineHook::fb_die_process(const int64_t fighter_id)
{
	JUDGE_RETURN(this->scene_mode() == SCENE_MODE_SCRIPT, ;);
	this->set_aim_object(0);
}

void MapOfflineHook::arena_die_process(const int64_t fighter_id)
{
	JUDGE_RETURN(this->scene_id() == GameEnum::ARENA_SCENE_ID, ;);

	AreaField* arena_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(arena_field != NULL, ;);

	MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(this);
	arena_field->area_field_finish(player->src_role_id());
}

int MapOfflineHook::execute_ai_tree(void)
{
	//副本
	JUDGE_RETURN(this->is_death() == false, -1);
	JUDGE_RETURN(this->scene_mode() == SCENE_MODE_SCRIPT, -1);

	this->fetch_fb_aim_object();
	this->chase_or_attack_fighter();

	return 0;
}

int MapOfflineHook::fetch_fb_aim_object(void)
{
	JUDGE_RETURN(this->fetch_aim_object() == NULL, -1);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	//周围查找
	{
		Scene::MoverMap fighter_map;
		scene->fetch_all_around_fighter(this, fighter_map, this->location(),
				GameEnum::DEFAULT_COPY_PLAYER_RADIUS);

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			GameFighter* fighter = dynamic_cast<GameFighter *>(iter->second);
			JUDGE_CONTINUE(MapOfflineHook::validate_fb_aim_object(fighter) == true);

			this->set_aim_object(iter->first);
			return 0;
		}
	}

	//全屏查找
	{
		Scene::FighterMap& fighter_map = scene->fighter_map();
		for (Scene::FighterMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			JUDGE_CONTINUE(MapOfflineHook::validate_fb_aim_object(iter->second) == true);
			this->set_aim_object(iter->first);
			return 0;
		}
	}

	return 0;
}

int MapOfflineHook::recover_timeout(void)
{
//	JUDGE_RETURN(this->is_blood_full() == false, -1);
//	JUDGE_RETURN(GameCommon::is_script_scene(this->scene_id()), -1);
//
//	static int add_value = CONFIG_INSTANCE->copy_player("auto_blood").asInt();
//	static int level_coeffi = CONFIG_INSTANCE->copy_player("level_coeffi").asInt();
//
//	int add_blood = -1 * (add_value + level_coeffi * this->level());
//	return this->modify_blood_by_fight(add_blood, FIGHT_TIPS_SYSTEM_AUTO);
	return 0;
}

int MapOfflineHook::update_copy_player_speed(double add_times)
{
	BasicElement& speed = this->mover_detial_.__speed;
	return this->update_fighter_speed(GameEnum::SPEED, speed.basic() * add_times);
}

bool MapOfflineHook::validate_fb_aim_object(GameFighter* fighter)
{
	JUDGE_RETURN(fighter != NULL, false);
	JUDGE_RETURN(std::rand() % 2 == 0, false);

	JUDGE_RETURN(fighter->is_monster() == true, false);
	JUDGE_RETURN(fighter->camp_id() != GameEnum::MONSTER_CAMP_NPC, false);

	return true;
}

int MapOfflineHook::update_blood_in_offline(const double value, const int fight_tips,
			const int64_t attackor, const int skill_id)
{
	return 0;
}
