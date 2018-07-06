/*
 * FourAltarScript.cpp
 *
 * Created on: 2015-03-17 21:26
 *     Author: lyz
 */

#include "FourAltarScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ScriptAI.h"
#include "FightField.h"
#include "ScriptScene.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"

FourAltarScript::~FourAltarScript(void)
{ /*NULL*/ }

void FourAltarScript::reset(void)
{
    this->four_altar_detail_.reset();
    BaseScript::reset();
}

int FourAltarScript::sync_increase_monster(ScriptAI *script_ai)
{
    int ret = BaseScript::sync_increase_monster(script_ai);

    const Json::Value &monster_json = CONFIG_INSTANCE->monster(script_ai->ai_sort());
    if (monster_json.isMember("boss") && monster_json["boss"].asInt() == 1)
    {
        this->four_altar_detail_.__boss_id_set.insert(script_ai->ai_id());
    }
    return ret;
}

int FourAltarScript::sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id)
{
    int ret = BaseScript::sync_kill_monster(script_ai, fighter_id);
    LongSet::iterator iter = this->four_altar_detail_.__boss_id_set.find(script_ai->ai_id());
    if (iter != this->four_altar_detail_.__boss_id_set.end())
    {
        this->active_four_altar(script_ai);
        this->four_altar_detail_.__boss_id_set.erase(iter);
    }

    return ret;
}

int FourAltarScript::init_scene_exec_condition(const Json::Value &scene_json)
{
    int ret = BaseScript::init_scene_exec_condition(scene_json);

    FourAltarDetail::EffectDetail effect_detail;
    const Json::Value &four_altar_json = scene_json["exec"]["four_altar"];
    for (uint altar_index = 0; altar_index < four_altar_json.size(); ++altar_index)
    {
        const Json::Value &altar_json = four_altar_json[altar_index];
        int sort = altar_json["sort"].asInt();
        FourAltarDetail::SceneSkillDetail &scene_skill_detail = this->four_altar_detail_.__scene_skill_map[sort];
        scene_skill_detail.reset();
        
        scene_skill_detail.__tip_id = altar_json["top_id"].asInt();
        for (uint oncebuff_index = 0; oncebuff_index < altar_json["once_buff"].size(); ++oncebuff_index)
        {
            const Json::Value &oncebuff_json = altar_json["once_buff"][oncebuff_index];
            this->init_effect_detail(effect_detail, oncebuff_json);
            scene_skill_detail.__once_buff_list.push_back(effect_detail);
        }
        this->init_effect_detail(scene_skill_detail.__scene_skill, altar_json["scene_skill"]);
    }
    return ret;
}

int FourAltarScript::init_effect_detail(FourAltarDetail::EffectDetail &effect_detail, const Json::Value &json)
{
    effect_detail.reset();
    effect_detail.__target = json["target"].asInt();
    effect_detail.__effect_type = this->convert_effect_type(json["effect"].asString());
    effect_detail.__effect_value = json["effect_value"].asDouble();
    effect_detail.__effect_percent = json["effect_percent"].asDouble();
    effect_detail.__effect_interval = json["effect_interval"].asDouble();
    effect_detail.__effect_last = json["effect_last"].asDouble();
    effect_detail.__skill_interval = json["skill_interval"].asDouble();
    if (json.isMember("text_id") && json["text_id"].size() > 0)
        effect_detail.__text_id = json["text_id"][0u].asInt();
    return 0;
}

int FourAltarScript::convert_effect_type(const std::string &effect_name)
{
    if (effect_name == Effect::ATTACK)
        return FourAltarScript::SET_ATTACK;
    else if (effect_name == Effect::DEFENCE)
        return FourAltarScript::SET_DEFENCE;
    else if (effect_name == Effect::AVOID)
        return FourAltarScript::SET_AVOID;
    else if (effect_name == Effect::SPEED)
        return FourAltarScript::SET_SPEED;
    else if (effect_name == Effect::DIZZY)
        return FourAltarScript::SET_DIZZY;
    else if (effect_name == Effect::BLOOD)
        return FourAltarScript::SET_BLOOD;
    else if (effect_name == Effect::METEOR)
        return FourAltarScript::SET_METEOR;
    else if (effect_name == Effect::PROP)
        return FourAltarScript::SET_PROP;
    else if (effect_name == Effect::RECOVERT_MAGIC)
        return FourAltarScript::SET_RECOVERT_MAGIC;
    return 0;
}

int FourAltarScript::active_four_altar(ScriptAI *script_ai)
{
    this->four_altar_detail_.__active_boss_set.insert(script_ai->ai_sort());
    FourAltarDetail::BossSceneSkillMap::iterator iter = this->four_altar_detail_.__scene_skill_map.find(script_ai->ai_sort());
    JUDGE_RETURN(iter != this->four_altar_detail_.__scene_skill_map.end(), 0);

    FourAltarDetail::SceneSkillDetail &sceneskill_detail = iter->second;
    this->process_oncebuff_active(sceneskill_detail);
    this->process_scene_skill_launch(iter->first, sceneskill_detail);

    return 0;
}

int FourAltarScript::process_oncebuff_active(FourAltarDetail::SceneSkillDetail &sceneskill_detail)
{
    for (FourAltarDetail::EffectList::iterator iter = sceneskill_detail.__once_buff_list.begin();
            iter != sceneskill_detail.__once_buff_list.end(); ++iter)
    {
        this->launch_scene_skill_effect(*iter);
    }
    return 0;
}

int FourAltarScript::process_scene_skill_launch(const int boss_sort, FourAltarDetail::SceneSkillDetail &sceneskill_detail)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->four_altar_detail_.__min_interval_check_tick <= nowtime, -1);
    JUDGE_RETURN(sceneskill_detail.__skill_check_tick <= nowtime, -1);

    {
        Proto80400917 respond;
        respond.set_sort(boss_sort);
        respond.set_tip_id(sceneskill_detail.__tip_id);
        this->notify_all_player(&respond);

        if (sceneskill_detail.__scene_skill.__text_id > 0)
        {
			Proto80400219 effect_res;
			effect_res.set_text_id(sceneskill_detail.__scene_skill.__text_id);
			this->notify_all_player(&effect_res);
        }
    }

    this->four_altar_detail_.__min_interval_check_tick = nowtime + Time_Value(10);
    sceneskill_detail.__skill_check_tick = nowtime + GameCommon::fetch_time_value(sceneskill_detail.__scene_skill.__skill_interval);

    return this->launch_scene_skill_effect(sceneskill_detail.__scene_skill);
}

int FourAltarScript::other_script_timeout(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->four_altar_detail_.__min_interval_check_tick != Time_Value::zero, 0);
    JUDGE_RETURN(this->four_altar_detail_.__min_interval_check_tick <= nowtime, 0);

    for (LongSet::iterator iter = this->four_altar_detail_.__active_boss_set.begin();
            iter != this->four_altar_detail_.__active_boss_set.end(); ++iter)
    {
        int sort = *iter;
        FourAltarDetail::BossSceneSkillMap::iterator sceneskill_iter = this->four_altar_detail_.__scene_skill_map.find(sort);
        if (sceneskill_iter == this->four_altar_detail_.__scene_skill_map.end())
            continue;

        if (this->process_scene_skill_launch(sort, sceneskill_iter->second) == 0)
            break;
    }
    return 0;
}

void FourAltarScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

int FourAltarScript::launch_scene_skill_effect(FourAltarDetail::EffectDetail &effect_detail)
{
    ScriptScene *scene = this->fetch_current_scene();
    JUDGE_RETURN(scene != 0, -1);

    if (effect_detail.__target == ST_PLAYER)
    {
        MapPlayerEx *player = 0;
        for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
                iter != this->script_detail_.__player_set.end(); ++iter)
        {
            if (this->monitor()->find_player(*iter, player) != 0)
                continue;

            this->process_effect_to_defender(effect_detail, player);
        }
    }
    else if (effect_detail.__target == ST_BOSS)
    {
        GameAI *script_ai = 0;
        for (LongSet::iterator iter = this->four_altar_detail_.__boss_id_set.begin();
                iter != this->four_altar_detail_.__boss_id_set.end(); ++iter)
        {
            script_ai = scene->find_ai(*iter);
            if (script_ai == NULL || script_ai->is_death())
                continue;
            this->process_effect_to_defender(effect_detail, script_ai);
        }
    }
    else if (effect_detail.__target == ST_ALL_MONSTER)
    {
        ScriptAI *script_ai = 0;
        ScriptScene::ScriptAIMap &script_ai_map = scene->script_ai_map();
        for (ScriptScene::ScriptAIMap::iterator iter = script_ai_map.begin(); 
                iter != script_ai_map.end(); ++iter)
        {
            script_ai = iter->second;
            if (script_ai == NULL || script_ai->is_death())
                continue;

            this->process_effect_to_defender(effect_detail, script_ai);
        }
    }
    return 0;
}

int FourAltarScript::process_effect_to_defender(FourAltarDetail::EffectDetail &effect_detail, GameFighter *defender)
{
//    switch (effect_detail.__effect_type)
//    {
//        case SET_ATTACK:
//        {
//            defender->insert_defender_status(defender, BasicStatus::SCENE_ATTACK,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_DEFENCE:
//        {
//            defender->insert_defender_status(defender, BasicStatus::SCENE_DEFENCE,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_AVOID:
//        {
//            defender->insert_defender_status(defender, BasicStatus::SCENE_AVOID,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_DIZZY:
//        {
//            defender->insert_defender_status(defender, BasicStatus::DIZZY,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_BLOOD:
//        {
//             defender->insert_defender_status(defender, BasicStatus::DIRECTMAXBLOOD,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_PROP:
//        {
//             defender->insert_defender_status(defender, BasicStatus::SCENE_PROP,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        case SET_RECOVERT_MAGIC:
//        {
//             defender->insert_defender_status(defender, BasicStatus::SCENE_RECOVERT_MAGIC,
//                    effect_detail.__effect_interval, effect_detail.__effect_last, 0,
//                    effect_detail.__effect_value, effect_detail.__effect_percent);
//            break;
//        }
//        default:
//            break;
//    }
    return 0;
}

