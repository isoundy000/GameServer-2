/*
 * ScriptAI.cpp
 *
 * Created on: 2014-01-03 10:25
 *     Author: lyz
 */

#include "ScriptAI.h"
#include "GameConfig.h"
#include "BaseScript.h"
#include "ScriptScene.h"
#include "TowerDefenseScript.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"

ScriptAI::ScriptAI(void)
{ /*NULL*/}

ScriptAI::~ScriptAI(void)
{ /*NULL*/ }

int ScriptAI::auto_action_timeout(const Time_Value &nowtime)
{
    ScriptScene *scene = dynamic_cast<ScriptScene *>(this->fetch_scene());
    JUDGE_RETURN(scene != NULL, 0);

    if (this->is_death() == false)
    {
		if (scene->is_running() == false)
			return 0;

		BaseScript *script = this->fetch_script();
		if (script == 0 || script->is_holdup() == true)
			return 0;
    }

    return GameAI::auto_action_timeout(nowtime);
}

void ScriptAI::reset(void)
{
    this->script_ai_detail_.reset();

    GameAI::reset();
}

int ScriptAI::sign_out(void)
{
    ScriptScene *scene = dynamic_cast<ScriptScene *>(this->fetch_scene());
    if (scene != 0)
    {
        scene->recycle_monster(this);
    }

    return GameAI::sign_out();
}

int ScriptAI::enter_scene(const int type/*=ENTER_SCENE_LOGIN*/)
{
	return GameAI::enter_scene(type);
}

ScriptAIDetail &ScriptAI::script_ai_detail(void)
{
    return this->script_ai_detail_;
}

void ScriptAI::set_script_sort(const int sort)
{
    this->script_ai_detail_.__script_sort = sort;
}

int ScriptAI::script_sort(void)
{
    return this->script_ai_detail_.__script_sort;
}

void ScriptAI::set_scene_config_index(const int index)
{
    this->script_ai_detail_.__scene_config_index = index;
}

int ScriptAI::scene_config_index(void)
{
    return this->script_ai_detail_.__scene_config_index;
}

int ScriptAI::record_index(void)
{
	return this->script_ai_detail_.__record_index;
}

void ScriptAI::set_record_index(const int index)
{
	this->script_ai_detail_.__record_index = index;
}

void ScriptAI::set_level_index(const int index)
{
    this->script_ai_detail_.__level_index = index;
}

int ScriptAI::level_index(void)
{
    return this->script_ai_detail_.__level_index;
}

void ScriptAI::set_wave_id(const int wave)
{
    this->script_ai_detail_.__wave_id = wave;
}

int ScriptAI::wave_id(void)
{
    return this->script_ai_detail_.__wave_id;
}

const Json::Value &ScriptAI::fetch_layout_item(void)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    if (this->scene_config_index() < 0 || int(scene_json["layout"].size()) <= this->scene_config_index())
        return Json::Value::null;

    return scene_json["layout"][this->scene_config_index()];
}

void ScriptAI::init_base_property(void)
{
	GameAI::init_base_property();
}

int ScriptAI::recycle_self(void)
{
    return this->monitor()->script_ai_pool()->push(this);
}

int ScriptAI::die_process(const int64_t fighter_id)
{
    GameAI::die_process(fighter_id);

    BaseScript *script = this->monitor()->find_script(this->space_id());
    JUDGE_RETURN(script != NULL, -1);

    return script->sync_kill_monster(this, fighter_id);
}

BaseScript *ScriptAI::fetch_script(void)
{
    return this->monitor()->find_script(this->space_id());
}

void ScriptAI::produce_drop_items(const Json::Value &monster_drop,
    		LongMap& player_map, const ItemObj& item_obj, LongSet &coord_set, int *drop_size)
{
}

bool ScriptAI::is_movable_coord(const MoverCoord &coord)
{
    BaseScript *script = this->monitor()->find_script(this->space_id());
    if (script == 0 || script->current_script_scene() != this->scene_id())
        return GameAI::is_movable_coord(coord);

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    if (scene_json["exec"].isMember("wave_spt_map") == false)
        return GameAI::is_movable_coord(coord);

    const Json::Value &wave_spt_json = scene_json["exec"]["wave_spt_map"];
    int current_wave = script->finish_wave() + 1, spt_id = this->scene_id();
    if (current_wave > script->total_wave())
        current_wave = script->total_wave();
    for (uint i = 0; i < wave_spt_json.size(); ++i)
    {
        if (wave_spt_json[i][0u].asInt() == current_wave)
        {
            spt_id = wave_spt_json[i][1u].asInt();
            break;
        }
    }

    return GameCommon::is_movable_coord(spt_id, coord);
}

int ScriptAI::validate_movable(const MoverCoord &step)
{
    BaseScript *script = this->monitor()->find_script(this->space_id());
    if (script == 0 || script->current_script_scene() != this->scene_id())
        return GameAI::validate_movable(step);

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    if (scene_json["exec"].isMember("wave_spt_map") == false)
        return GameAI::validate_movable(step);

    int ret = this->validate_fighter_movable();
    JUDGE_RETURN(ret == 0, ret);

    const Json::Value &wave_spt_json = scene_json["exec"]["wave_spt_map"];
    int current_wave = script->finish_wave() + 1, spt_id = this->scene_id();
    if (current_wave > script->total_wave())
        current_wave = script->total_wave();
    for (uint i = 0; i < wave_spt_json.size(); ++i)
    {
        if (wave_spt_json[i][0u].asInt() == current_wave)
        {
            spt_id = wave_spt_json[i][1u].asInt();
            break;
        }
    }

    if (GameCommon::is_movable_coord(spt_id, step) == false)
        return ERROR_COORD_ILLEGAL;
    
    return 0;
}

int ScriptAI::modify_blood_by_fight(const double org_inc_val, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != 0 && script->is_holdup() == false
    		&& script->is_finish_script() == false
    		&& script->is_failure_script() == false, 0);

    double inc_val = script->recalc_ai_modify_blood(org_inc_val);

    int real_value = GameAI::modify_blood_by_fight(inc_val, fight_tips, attackor, skill_id);
    JUDGE_RETURN(real_value > 0 && attackor > 0, real_value);

    Int64 benefited_attackor_id = this->fetch_benefited_attackor_id(attackor);
    script->record_player_hurt(benefited_attackor_id, real_value);

	if (this->is_boss())
	{
		script->update_boss_hurt(this->ai_sort(), real_value);
	}

	if (this->camp_id() == GameEnum::MONSTER_CAMP_NPC)
	{
		ScriptScene *scene = script->fetch_current_scene();
		if (scene != 0 && scene->protect_npc_id() == this->ai_id())
			script->notify_npc_blood(this->ai_id(), this->fight_detail().__blood);
	}

    return real_value;
}

int ScriptAI::schedule_move_fighter(void)
{
    TowerDefenseScript *script = dynamic_cast<TowerDefenseScript *>(this->monitor()->find_script(this->space_id()));
    if (script != 0)
    {
        script->check_matrix_status(this);
    }
    return GameAI::schedule_move_fighter();
}


int ScriptAI::modify_player_experience(int64_t fighter_id, int inc_exp)
{
    MapPlayerEx *player = this->find_player(fighter_id);
    if(NULL == player)
    {
    	player = this->find_player_with_offline(fighter_id);
    	JUDGE_RETURN(player != 0, -1);
    	JUDGE_RETURN(player->copy_offline() == true, -1);
    }

    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != 0, -1);

    player->modify_element_experience(inc_exp , SerialObj(EXP_FROM_SCRIPT_MONSTER, this->ai_sort()));
    script->update_player_exp(player, inc_exp);

    return 0;
}

int ScriptAI::process_push_away(const int range)
{
	return GameAI::process_push_away(range);
}

void ScriptAI::fetch_drop_money_amount(int *money_type, int *money_amount, const Json::Value &monster_drop)
{
    GameAI::fetch_drop_money_amount(money_type, money_amount, monster_drop);
}

void ScriptAI::produce_drop_items(const Json::Value &monster_drop, LongMap& player_map, LongSet &coord_set, int *drop_size)
{
	MapPlayerEx* player = this->find_player_with_offline(this->fetch_killed_id());
	if (player != NULL)
	{
		ScriptPlayerDetail::ScriptRecord *script_rec = player->refresh_script_record(this->script_sort());
		if (script_rec->__is_first_pass == 0)
		{
			const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
			if (scene_json["exec"]["first_monster_no_drop"].asInt() == 1)
				return ;
		}
	}
	return GameAI::produce_drop_items(monster_drop, player_map, coord_set, drop_size);
}
