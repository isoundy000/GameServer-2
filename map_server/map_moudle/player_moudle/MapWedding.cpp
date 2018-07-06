/*
 * MapWedding.cpp
 *
 * Created on: 2015-06-10 16:44
 *     Author: lyz
 */

#include "MapWedding.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "FloatAI.h"
#include "Scene.h"
#include "MapMonitor.h"

MapWedding::~MapWedding(void)
{ /*NULL*/ }

MapPlayer *MapWedding::wedding_player(void)
{
    return dynamic_cast<MapPlayer *>(this);
}

void MapWedding::reset_wedding(void)
{
    this->wedding_id_ = 0;
    this->partner_id_ = 0;
    this->float_ai_id_ = 0;
}

void MapWedding::set_wedding_id(const Int64 id)
{
    this->wedding_id_ = id;
}

Int64 MapWedding::wedding_id(void)
{
    return this->wedding_id_;
}

void MapWedding::set_partner_id(const Int64 id)
{
    this->partner_id_ = id;
}

Int64 MapWedding::partner_id(void)
{
    return this->partner_id_;
}

void MapWedding::set_float_ai_id(const Int64 id)
{
    this->float_ai_id_ = id;
}

Int64 MapWedding::float_ai_id(void)
{
    return this->float_ai_id_;
}

int MapWedding::process_get_wedding_role_coord(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30101601 *, request, msg, -1);

    MapPlayer *player = this->wedding_player();
//
//    MoverCoord role_coord, npc_coord;
//    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
//    int scene_id = wedding_json["npc_locate"][0u].asInt(),
//        npc_sort = wedding_json["npc_locate"][1u].asInt();
//    const Json::Value &npc_json = CONFIG_INSTANCE->npc(scene_id, npc_sort);
//    if (npc_json.isMember("pos"))
//    {
//        npc_coord.set_pixel(npc_json["pos"][0u].asInt(), npc_json["pos"][1u].asInt());
//    }
//    else
//    {
//        npc_coord.set_pixel(npc_json["posX"].asInt(), npc_json["posY"].asInt());
//    }
//
//
    Proto31101602 inner_req;
    inner_req.set_res_recogn(request->res_recogn());
    inner_req.set_wedding_type(request->type());
//
//    MapPlayer *other_player = NULL;
    for (int i = 0; i < request->role_list_size(); ++i)
    {
        Int64 role_id = request->role_list(i);
//        other_player = player->find_player(role_id);
//        if (other_player == NULL)
//            return player->respond_to_client_error(request->res_recogn(), ERROR_COORD_OFFSET);

//        if (coord_offset_grid(other_player->location(), npc_coord) > 5)
//           return player->respond_to_client_error(request->res_recogn(), ERROR_TASK_NPC_DISTANCE);
//
        inner_req.add_role_list(role_id);
    }
//
    return player->send_to_logic_thread(inner_req);
//	return 0;
}

int MapWedding::sync_update_intimacy_by_kill(const Int64 fighter_id)
{
    MapPlayer *player = this->wedding_player();
    Int64 benefited_id = player->fetch_benefited_attackor_id(fighter_id);

    JUDGE_RETURN(GameCommon::fetch_mover_type(benefited_id) == MOVER_TYPE_PLAYER, 0);
    JUDGE_RETURN(benefited_id != player->role_id(), 0);

    const int /*TYPE_FINISH_SCRIPT = 1, */TYPE_KILL_PLAYER = 2;

    Proto31101607 inner_req;
    inner_req.set_type(TYPE_KILL_PLAYER);
    inner_req.set_scene_id(player->scene_id());
    inner_req.add_role_list(player->role_id());
    inner_req.add_role_list(benefited_id);

    return player->monitor()->dispatch_to_logic(player, &inner_req);
}

int MapWedding::enter_float_cruise_state(const bool is_notify/* = false*/)
{
    MapPlayer *player = this->wedding_player();
    Scene *scene = NULL;
    if (player->monitor()->find_scene(player->space_id(), player->scene_id(), scene) != 0 || scene == NULL)
        return -1;
    
    FloatAI *float_ai = scene->find_float_ai(player->role_id());
    if (float_ai == NULL)
        return 0;
    
    this->set_float_ai_id(float_ai->ai_id());

    if (is_notify == true)
    {
        Proto80400226 respond;
        respond.set_fighter_id(player->role_id());
        respond.set_float_id(this->float_ai_id());
        player->respond_to_broad_area(&respond);
    }
    return 0;
}

int MapWedding::exit_float_cruise_state(void)
{
    MapPlayer *player = this->wedding_player();
    this->set_float_ai_id(0);

    Proto80400228 respond;
    respond.set_mover_id(player->role_id());
    return player->respond_to_broad_area(&respond);
}

int MapWedding::correct_player_coord_when_float(void)
{
    MapPlayer *player = this->wedding_player();
    Scene *scene = NULL;
    if (player->monitor()->find_scene(player->space_id(), player->scene_id(), scene) != 0 || scene == NULL)
        return -1;

    FloatAI *float_ai = scene->find_float_ai(player->role_id());
    JUDGE_RETURN(float_ai != NULL, 0);
    JUDGE_RETURN(float_ai->is_float_owner(player->role_id()) == true, 0);

    player->mover_detail().__location = float_ai->location();
    this->enter_float_cruise_state(false);

    return 0;
}

bool MapWedding::is_float_cruise(void)
{
    if (this->float_ai_id() > 0)
        return true;
    return false;
}

int MapWedding::follow_float_move_action(std::vector<MoverCoord> &step_list)
{
    JUDGE_RETURN(step_list.size() > 0, 0);

    MapPlayer *player = this->wedding_player();

    MoverCoord latest_step;
    player->mover_detail().__step_list.clear();
    for (std::vector<MoverCoord>::iterator iter = step_list.begin();
            iter != step_list.end(); ++iter)
    {
        player->mover_detail().__step_list.push_back(*iter);
        latest_step = *iter;
    }

    player->schedule_move(latest_step);

    return 0; 
}

int MapWedding::process_sync_wedding_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30101610 *, request, -1);

    MapPlayer* player = this->wedding_player();
    return player->update_wedding_info(request->partner_id(), request->partner_name(),
    			request->wedding_id(), request->wedding_type());
}


int MapWedding::check_wedding_giftbox_times(const int wedding_type)
{
	MapPlayer *player = this->wedding_player();
    Time_Value nowtime = Time_Value::gettimeofday();
    if (player->role_detail().__wedding_giftbox_tick <= nowtime)
    {
    	player->role_detail().__wedding_giftbox_tick = next_day(0, 0, nowtime);
    	player->role_detail().__wedding_giftbox_times = 0;
    }

    const Json::Value &wedding_type_json = CONFIG_INSTANCE->wedding()["wedding"]["wedding_type"][wedding_type - 1];
    int max_collect_times = wedding_type_json["person_max_giftbox"].asInt();
    if (max_collect_times <= 0)
        max_collect_times = 15;
    if (player->role_detail().__wedding_giftbox_times < max_collect_times)
        return 0;
    return ERROR_WEDDING_GIFTBOX_AMOUNT;
}

int MapWedding::notify_send_flower_effect(Message *msg)
{
    return this->wedding_player()->respond_to_broad_area(msg);
}

