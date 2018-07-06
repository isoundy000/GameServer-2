/*
 * MapTeamer.cpp
 *
 *  Created on: Jul 29, 2013
 *      Author: peizhibi
 */

#include "MapTeamer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "Scene.h"

MapTeamer::MapTeamer()
{
	// TODO Auto-generated constructor stub
}

MapTeamer::~MapTeamer()
{
	// TODO Auto-generated destructor stub
}

MapTeamInfo& MapTeamer::team_info(int type)
{
	return this->team_info_[type];
}

void MapTeamer::reset()
{
	for (int i = 0; i < GameEnum::TOTAL_TEAM; ++i)
	{
		this->team_info_[i].reset();
	}
}

int MapTeamer::teamer_state(int type)
{
	JUDGE_RETURN(this->team_info_[type].team_index_ > 0, GameEnum::TEAMER_STATE_NONE);
	return this->team_info_[type].leader_id_ == this->fighter_id() ? GameEnum::TEAMER_STATE_LEADER
			: GameEnum::TEAMER_STATE_TEAMER;
}

int MapTeamer::team_index(int type)
{
	return this->team_info_[type].team_index();
}

int MapTeamer::team_empty(int type)
{
	return this->team_info(type).teamer_set_.empty();
}

int MapTeamer::read_team_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400701*, request, -1);

	int type = GameEnum::NORMAL_TEAM;
	if (request->travel_team() == true)
	{
		type = GameEnum::TRAVEL_TEAM;
	}

	MapTeamInfo& team_info = this->team_info(type);
	int prev_team_id = team_info.team_index();

	team_info.reset();
	team_info.team_index_ = request->team_index();
	team_info.leader_id_ = request->leader_id();

	for(int i = 0; i < request->teamer_set_size(); ++i)
	{
		int64_t teamer_id = request->teamer_set(i);
		team_info.teamer_set_[teamer_id] = true;
	}

	for(int i = 0; i < request->replacement_set_size(); ++i)
	{
		int64_t replacement_id = request->replacement_set(i);
		team_info.replacement_set_[replacement_id] = true;
	}

	this->update_team_blood_info(type);

	MapPlayerEx *player = this->self_player();
	Scene *scene = this->fetch_scene();
	if (scene != NULL && player != NULL)
	{
		if (prev_team_id > 0)
		{
			scene->unbind_team_player(prev_team_id, player);
		}

		if (team_info.team_index() > 0)
		{
			scene->bind_team_player(team_info.team_index(), player);
		}
	}

	return 0;
}

int MapTeamer::req_team_enter_script(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400703*, request, -1);

	// TODO 队长请求进入副本 条件判断
	request->set_req_result(0);
	return this->monitor()->dispatch_to_logic(this, request);
}

int MapTeamer::req_team_get_ready(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400704*, request, -1);

	// TODO 队员请求进入准备完毕状态 条件判断
	request->set_req_result(0);
	return this->monitor()->dispatch_to_logic(this, request);
}

int MapTeamer::update_team_blood_info(int type)
{
	JUDGE_RETURN(this->team_info(type).teamer_set_.empty() == false, -1);

	this->team_notify_teamer_blood(type);
	this->team_fetch_teamer_blood(type);

	return 0;
}

int MapTeamer::team_notify_teamer_blood(int type)
{
	JUDGE_RETURN(this->team_empty(type) == false, -1);

	Proto80400401 blood_info;
	blood_info.set_fighter_id(this->fighter_id());
	blood_info.set_blood(this->fight_detail().__blood);
	blood_info.set_max_blood(this->fight_detail().__blood_total_i(this));

	MapPlayerEx* player = this->self_player();
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		ProtoPairObj* info = blood_info.add_mount_info();
		MountDetail& mount_detail = player->mount_detail(i);
		info->set_obj_id(i);
		info->set_obj_value(mount_detail.mount_grade_);
	}

	return this->notify_all_teamer_info(type, ACTIVE_NOTIFY_TEAMER_BLOOD,  &blood_info);
}

int MapTeamer::team_fetch_teamer_blood(int type)
{
	JUDGE_RETURN(this->team_empty(type) == false, -1);

	MapTeamInfo& team_info = this->team_info(type);
	for(LongMap::iterator iter = team_info.teamer_set_.begin();
			iter != team_info.teamer_set_.end(); ++iter)
	{
//		JUDGE_CONTINUE(this->fighter_id() != iter->first);

		Proto80400401 blood_info;
		MapPlayerEx* player = NULL;
		if(0 == MAP_MONITOR->find_player(iter->first, player))
		{
			blood_info.set_fighter_id(player->fighter_id());
			blood_info.set_blood(player->fight_detail().__blood);
			blood_info.set_max_blood(player->fight_detail().__blood_total_i(player));

			for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
			{
				ProtoPairObj* info = blood_info.add_mount_info();
				MountDetail& mount_detail = player->mount_detail(i);
				info->set_obj_id(i);
				info->set_obj_value(mount_detail.mount_grade_);
			}
		}
		else
		{
			blood_info.set_fighter_id(iter->first);
			blood_info.set_blood(10000);
			blood_info.set_max_blood(10000);

			for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
			{
				ProtoPairObj* info = blood_info.add_mount_info();

				info->set_obj_id(i);
				info->set_obj_value(1);
			}

		}

		MSG_USER(%d / %d, blood_info.blood(), blood_info.max_blood());
		this->respond_to_client(ACTIVE_NOTIFY_TEAMER_BLOOD, &blood_info);
	}

	return 0;
}

int MapTeamer::notify_all_teamer_info(int type, int recogn, Message* msg, bool with_self)
{
	MapTeamInfo& team_info = this->team_info(type);
	for(LongMap::iterator iter = team_info.teamer_set_.begin();
			iter != team_info.teamer_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->fighter_id() != iter->first || with_self == true);

		MapPlayerEx* player = NULL;
		JUDGE_CONTINUE(0 == MAP_MONITOR->find_player(iter->first, player));

		player->respond_to_client(recogn, msg);
	}

	return 0;
}

int MapTeamer::sync_transfer_team(void)
{
	Proto30400109 request;
	for (int i = 0; i < GameEnum::TOTAL_TEAM; ++i)
	{
		MapTeamInfo& team_info = this->team_info(i);
		team_info.serialize(request.add_team_info());
	}

	return this->send_to_other_scene(this->scene_id(), request);
}

int MapTeamer::read_transfer_team(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400109 *, request, -1);

	for (int i = 0; i < request->team_info_size(); ++i)
	{
		MapTeamInfo& team_info = this->team_info(i);
		team_info.unserialize(request->team_info(i));
	}

	return 0;
}

// include self
int MapTeamer::teamate_count()
{
	return this->team_info().teamer_set_.size();
}

int MapTeamer::replacement_count()
{
	return this->team_info().replacement_set_.size();
}

int MapTeamer::make_up_teamer_move(Block_Buffer *buff)
{
    Proto80400406 respond;
    respond.set_role_id(this->mover_id());
    respond.set_pixel_x(this->location().pixel_x());
    respond.set_pixel_y(this->location().pixel_y());

    ProtoClientHead head;
    head.__recogn = ACTIVE_TEAMER_MOVE;
    return this->make_up_client_block(buff, &head, &respond);
}

int MapTeamer::make_up_teamer_appear(Block_Buffer *buff)
{
//    MapPlayer *player = this->self_player();
//    JUDGE_RETURN(player != NULL, 0);
//
//    Proto80400404 respond;
//    respond.set_role_id(this->mover_id());
//    respond.set_team_id(this->team_id());
//    respond.set_role_name(player->role_name());
//    respond.set_career(player->role_detail().__career);
//    respond.set_sex(player->role_detail().__sex);
//    respond.set_pixel_x(player->location().pixel_x());
//    respond.set_pixel_y(player->location().pixel_y());
//
//    ProtoClientHead head;
//    head.__recogn = ACTIVE_TEAMER_APPEAR;
//    return this->make_up_client_block(buff, &head, &respond);
	return 0;
}

int MapTeamer::notify_teamer_move(void)
{
//	JUDGE_RETURN(GameCommon::is_mini_script_scene(this->scene_id()) == false, 0);
//
//    Scene *scene = this->fetch_scene();
//    JUDGE_RETURN(scene != NULL, 0);
//
//    Block_Buffer buff;
//    this->make_up_teamer_move(&buff);
//    return scene->push_team_player_data(this->team_id(), buff);
	return 0;
}

int MapTeamer::notify_teamer_disappear(Scene *scene)
{
//    JUDGE_RETURN(scene != NULL && this->team_id() > 0, 0);
//
//    Proto80400405 respond;
//    respond.set_role_id(this->mover_id());
//
//    Block_Buffer buff;
//    ProtoClientHead head;
//    head.__recogn = ACTIVE_TEAMER_DISAPPEAR;
//    this->make_up_client_block(&buff, &head, &respond);
//    return scene->push_team_player_data(this->team_id(), buff);
	return 0;
}

int MapTeamer::request_open_mini_map_pannel(void)
{
//	JUDGE_RETURN(GameCommon::is_mini_script_scene(this->scene_id()) == false, 0);
//
//    Scene *scene = this->fetch_scene();
//    JUDGE_RETURN(scene != NULL, 0);
//
//    scene->open_mini_map_pannel(this->team_id(), this->mover_id());
//    this->set_open_mini_map(true);
//
//    if (this->team_id() > 0)
//    {
//        Block_Buffer buff;
//        scene->make_up_all_teamer_appear_info(this->team_id(), buff);
//        this->respond_from_broad_client(&buff);
//    }

    return 0;
}

int MapTeamer::request_close_mini_map_pannel(void)
{
//    this->set_open_mini_map(false);
//
//    Scene *scene = this->fetch_scene();
//    JUDGE_RETURN(scene != NULL, 0);
//
//    scene->close_mini_map_pannel(this->team_id(), this->mover_id());
    return 0;
}

int MapTeamer::fetch_teamer_appear_info(void)
{
//	JUDGE_RETURN(GameCommon::is_mini_script_scene(this->scene_id()) == false, 0);
//    JUDGE_RETURN(this->team_id() > 0, 0);
//
//    Scene *scene = this->fetch_scene();
//    JUDGE_RETURN(scene != NULL, 0);
//
//    Block_Buffer buff;
//    scene->make_up_all_teamer_appear_info(this->team_id(), buff);
//    return this->respond_from_broad_client(&buff);
	return 0;
}

int MapTeamer::fetch_teamer_near_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400705 *, request, -1);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

    Proto30100704 near_info;
    near_info.set_role_id(request->role_id());

    IntSet team_list;
    for (MoverMap::iterator player_iter = scene->player_map().begin();
            player_iter != scene->player_map().end(); ++player_iter)
    {
        MapPlayer *player = dynamic_cast<MapPlayer *>(player_iter->second);
        JUDGE_CONTINUE(player != NULL && player->is_enter_scene() == true);

        int team_id = player->team_id();
        JUDGE_CONTINUE(team_id > 0);
		JUDGE_CONTINUE(team_list.count(team_id) == 0);

		near_info.add_leader_list(player->role_id());
		near_info.add_team_list(team_id);
		team_list.insert(team_id);
    }

    return this->monitor()->dispatch_to_logic(this, &near_info);
}
