/*
 * ClimbTowerScript.cpp
 *
 * Created on: 2014-02-13 15:35
 *     Author: lyz
 */

#include "ClimbTowerScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"

ClimbTowerScript::ClimbTowerScript(void)
{ /*NULL*/ }

ClimbTowerScript::~ClimbTowerScript(void)
{ /*NULL*/ }

void ClimbTowerScript::reset(void)
{
    BaseScript::reset();
}

void ClimbTowerScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

int ClimbTowerScript::init_script_first_scene(void)
{
    ScriptDetail::Piece &piece_info = this->script_detail_.__piece;
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
    const Json::Value &chapter_scene_json = script_json["prev_condition"]["chapter_scene"];
    const Json::Value &piece_max_json = script_json["prev_condition"]["piece"];
    IntMap scene_index_map;
    for (uint i = 0; i < piece_max_json.size(); ++i)
    {
        piece_info.__piece_chapter_map[i+1] = piece_max_json[i].asInt();
    }
    for (uint i = 0; i < chapter_scene_json.size(); ++i)
    {
        int piece = chapter_scene_json[i][0u].asInt(), 
            begin_chapter = chapter_scene_json[i][1u].asInt(),
            end_chapter = chapter_scene_json[i][2u].asInt(),
            scene_id = chapter_scene_json[i][3u].asInt();
        int monster_level_index = scene_index_map[scene_id] + this->chapter() - begin_chapter;
        scene_index_map[scene_id] += (end_chapter - begin_chapter + 1);
        if (this->piece() != piece)
            continue;
        if (this->chapter() < begin_chapter || end_chapter < this->chapter())
            continue;

        scene_info.__cur_scene = scene_id; 
        scene_info.__monster_level_index = monster_level_index;

        return 0;
    }

    return -1;
}

