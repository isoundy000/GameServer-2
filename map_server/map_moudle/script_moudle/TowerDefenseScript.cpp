/*
 * TowerDefenseScript.cpp
 *
 * Created on: 2014-02-13 15:42
 *     Author: lyz
 */

#include "TowerDefenseScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ScriptScene.h"
#include "GameFighter.h"

TowerDefenseScript::TowerDefenseScript(void)
{ /*NULL*/ }

TowerDefenseScript::~TowerDefenseScript(void)
{ /*NULL*/ }

void TowerDefenseScript::reset(void)
{
    BaseScript::reset();
}

void TowerDefenseScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

void TowerDefenseScript::set_spirit_value(const int value)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    wave_info.__spirit_value = value;
}

int TowerDefenseScript::spirit_value(void)
{
    return this->script_detail_.__wave.__spirit_value;
}

void TowerDefenseScript::update_matrix_spirit_level(const int matrix_id, const int level)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    wave_info.__matrix_level_map[matrix_id] = level;

    this->notify_update_spirit();
    this->check_area_matrix();
}

int TowerDefenseScript::matrix_spirit_level(const int matrix_id)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    return wave_info.__matrix_level_map[matrix_id];
}

int TowerDefenseScript::check_matrix_status(GameFighter *fighter)
{
    ScriptScene *scene = this->fetch_current_scene();
    JUDGE_RETURN(scene != 0, -1);

    GameFighter *attackor = 0;
    BasicStatus *status = 0;
    IntMap &matrix_level_map = this->script_detail_.__wave.__matrix_level_map;
    for (IntMap::iterator iter = matrix_level_map.begin();
            iter != matrix_level_map.end(); ++iter)
    {
        int matrix_id = iter->first;
        if (fighter->find_status(matrix_id, status) != 0)
            continue;
        if (scene->find_fighter(status->__attacker, attackor) != 0 || 
                coord_offset_grid(fighter->location(), attackor->location()) >= status->__value3)
        {
            fighter->remove_status(status);
        }
    }
    return 0;
}

int TowerDefenseScript::check_call_puppet(const int puppet, const int scene_id)
{
    JUDGE_RETURN(scene_id == this->current_script_scene(), ERROR_CLIENT_OPERATE);

    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(scene_id);
    const Json::Value &puppet_json = scene_json["exec"]["puppet"];
    JUDGE_RETURN(puppet_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
    JUDGE_RETURN(puppet <= int(puppet_json.size()), ERROR_CLIENT_OPERATE);

    int puppet_sort = puppet_json[puppet - 1][1u].asInt();
    if (wave_info.__active_puppet.count(puppet_sort) == 0)
        return ERROR_CLIENT_OPERATE;

    int call_flag = wave_info.__active_puppet[puppet_sort];
    if (call_flag > 0)
        return ERROR_CLIENT_OPERATE;

    return puppet_sort;
}

int TowerDefenseScript::update_puppet_call_flag(const int puppet)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->current_script_scene());
    const Json::Value &puppet_json = scene_json["exec"]["puppet"];
    JUDGE_RETURN(puppet_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
    JUDGE_RETURN(puppet <= int(puppet_json.size()), ERROR_CLIENT_OPERATE);

    int puppet_sort = puppet_json[puppet - 1][1u].asInt();
    wave_info.__active_puppet[puppet_sort] = 1;
    if (wave_info.__active_puppet_flag.size() > 0 && puppet <= int(wave_info.__active_puppet_flag.size()))
        wave_info.__active_puppet_flag[puppet - 1] = 2;

    this->notify_update_spirit();

    return 0;
}

