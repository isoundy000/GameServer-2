/*
 * WorldBossActor.cpp
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#include "WorldBossActor.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "WorldBossSystem.h"
#include "WorldBossScene.h"
#include "TrvlWbossPreScene.h"
#include "MMOWorldBoss.h"
#include "MapLogicStruct.h"
#include "TrvlWbossMonitor.h"

void SaveInfo::reset(void)
{
	this->scene_id_ = 0;
	this->coord_.reset();
}

WorldBossActor::WorldBossActor() {
	// TODO Auto-generated constructor stub

}

WorldBossActor::~WorldBossActor() {
	// TODO Auto-generated destructor stub
}

void WorldBossActor::reset()
{
	this->save_info_.reset();
}

int WorldBossActor::enter_scene(const int type)
{
	switch (this->wboss_enter_scene_type())
	{
	case GameEnum::ET_WORLD_BOSS:
	{
		return this->on_enter_wboss_scene(type);
	}
	case GameEnum::ET_TRVL_WBOSS:
	{
		if (this->scene_id() == this->trvl_wboss_pre_scene())
		{
			return this->on_enter_trvl_wboss_pre_scene(type);
		}
		else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
		{
			return this->on_enter_trvl_wboss_scene(type);
		}
	}
	}

	return 0;
}

int WorldBossActor::exit_scene(const int type)
{
	switch (this->wboss_enter_scene_type())
	{
	case GameEnum::ET_WORLD_BOSS:
	{
		this->on_exit_wboss_scene(type);
		break;
	}
	case GameEnum::ET_TRVL_WBOSS:
	{
		this->on_exit_trvl_wboss_scene(type);
		break;
	}
	}

	return MapPlayer::exit_scene(type);
}

int WorldBossActor::request_join_wboss(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401021*, reqeust, RETURN_ENTER_WORLD_BOSS);
	int request_scene = reqeust->boss_scene_id();

	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ENTER_WORLD_BOSS, ERROR_NORMAL_SCENE);
	CONDITION_NOTIFY_RETURN(GameCommon::is_world_boss_scene(request_scene) == true,
			RETURN_ENTER_WORLD_BOSS, ERROR_NOT_WORLD_BOSS_SCENE);


	Proto30402203 inner_res;
	inner_res.set_boss_scene_id(request_scene);
	inner_res.set_scene_id(reqeust->scene_id());
	inner_res.set_pos_x(reqeust->pos_x());
	inner_res.set_pos_y(reqeust->pos_y());
	inner_res.set_use_fly(reqeust->use_fly());

	int need_vip = CONFIG_INSTANCE->world_boss("vip_level").asInt();
	if (reqeust->use_fly() == 1)
	{
		if (this->vip_detail().__vip_level >= need_vip)
		{
			this->request_join_wboss_done(&inner_res);
		}
		else
		{
			this->send_to_logic_thread(inner_res);
		}
	}
	else
	{
		this->request_join_wboss_done(&inner_res);
	}

	return 0;
}

int WorldBossActor::request_join_wboss_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30402203*, reqeust, RETURN_ENTER_WORLD_BOSS);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_WORLD_BOSS);

	int request_scene = reqeust->boss_scene_id();
	int ret = this->send_request_enter_info(request_scene, enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ENTER_WORLD_BOSS, ret);

	MoverCoord coord;
	int scene_id = 0;
	if (reqeust->scene_id() <= 0)
	{
		scene_id = this->scene_id();
	}
	else
	{
		scene_id = reqeust->scene_id();
	}

	if (reqeust->pos_x() <= 0 || reqeust->pos_y() <= 0)
	{
		coord = this->location();
	}
	else
	{
		coord.set_pixel(reqeust->pos_x(), reqeust->pos_y());
	}
	this->save_info_.scene_id_ = scene_id;
	this->save_info_.coord_ = coord;

	return 0;
}

int WorldBossActor::handle_exit_wboss_scene(void)
{
	switch (this->wboss_enter_scene_type())
	{
	case GameEnum::ET_WORLD_BOSS:
	{
		this->transfer_to_save_scene();
		break;
	}
	case GameEnum::ET_TRVL_WBOSS:
	{
		this->handle_exit_trvl_wboss();
		break;
	}
	}

	return 0;
}

int WorldBossActor::request_wboss_info()
{
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::WORLD_BOSS_SCENE_ID_1,
			INNER_REQUEST_WORLD_BOSS_INFO);
}

int WorldBossActor::request_wboss_scene_info()
{
	WorldBossScene* wboss_scene = NULL;
	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	}
	JUDGE_RETURN(wboss_scene != NULL, 0);

	wboss_scene->enter_player(this->role_id(), GameEnum::ET_WORLD_BOSS);

	return 0;
}

int WorldBossActor::request_get_wboss_pocket_award(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401023*, reqeust, RETURN_GET_POCKET_AWARD);

	int scene_id = reqeust->scene_id();
	CONDITION_NOTIFY_RETURN(this->scene_id() == scene_id, RETURN_GET_POCKET_AWARD, ERROR_USE_SCENE_LIMIT);

	Proto50401023 respond;

	WorldBossScene* wboss_scene = NULL;
	if (GameCommon::is_world_boss_scene(scene_id) == true)
	{
		wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	}
	else if (GameCommon::is_trvl_wboss_scene(scene_id) == true)
	{
		wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	}
	CONDITION_NOTIFY_RETURN(wboss_scene != NULL, RETURN_GET_POCKET_AWARD, ERROR_USE_SCENE_LIMIT);

	int ret = wboss_scene->get_red_pocket_award(this->role_id(), &respond);

	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_GET_POCKET_AWARD, ret);

	const Json::Value& conf = CONFIG_INSTANCE->scene(scene_id);
	int pocket_award_id = conf["pocket_award"].asInt();
	const Json::Value& item_json = CONFIG_INSTANCE->item(pocket_award_id);
	this->request_add_single_item(pocket_award_id, respond.get_money(),
			item_json["bind"].asInt(), ADD_FROM_FROM_WB_POCKET);

	return this->respond_to_client(RETURN_GET_POCKET_AWARD, &respond);
}

int WorldBossActor::request_create_dice_num(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401024*, reqeust, RETURN_CREATE_DICE_NUM);

	int scene_id = reqeust->scene_id();
	CONDITION_NOTIFY_RETURN(this->scene_id() == scene_id, RETURN_CREATE_DICE_NUM, ERROR_USE_SCENE_LIMIT);

	Proto50401024 respond;

	WorldBossScene* wboss_scene = NULL;
	if (GameCommon::is_world_boss_scene(scene_id) == true)
	{
		wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	}
	else if (GameCommon::is_trvl_wboss_scene(scene_id) == true)
	{
		wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	}
	CONDITION_NOTIFY_RETURN(wboss_scene != NULL, RETURN_CREATE_DICE_NUM, ERROR_USE_SCENE_LIMIT);

	int ret = wboss_scene->get_dice_num(this->role_id(), &respond);

	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_CREATE_DICE_NUM, ret);

	return this->respond_to_client(RETURN_CREATE_DICE_NUM, &respond);
}

int WorldBossActor::request_my_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401025*, reqeust, RETURN_MY_RANK_INFO);

	int scene_id = reqeust->scene_id();
	CONDITION_NOTIFY_RETURN(this->scene_id() == scene_id, RETURN_MY_RANK_INFO, ERROR_USE_SCENE_LIMIT);

	Proto50401025 respond;

	WorldBossScene* wboss_scene = NULL;
	if (GameCommon::is_world_boss_scene(scene_id) == true)
	{
		wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	}
	else if (GameCommon::is_trvl_wboss_scene(scene_id) == true)
	{
		wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	}
	CONDITION_NOTIFY_RETURN(wboss_scene != NULL, RETURN_MY_RANK_INFO, ERROR_USE_SCENE_LIMIT);

	wboss_scene->get_my_rank_info(this->role_id(), &respond);

	return this->respond_to_client(RETURN_MY_RANK_INFO, &respond);
}

int WorldBossActor::request_wboss_red_point()
{
	Proto30402202 inner;
	inner.set_level(this->level());
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::WORLD_BOSS_SCENE_ID_1, &inner);
}

int WorldBossActor::on_enter_wboss_scene(const int type)
{
	WorldBossScene* wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	JUDGE_RETURN(wboss_scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(wboss_scene);
	MapPlayer::enter_scene(type);

	wboss_scene->enter_player(this->role_id(), GameEnum::ET_WORLD_BOSS);

	this->update_sword_pool_info();
	this->update_cornucopia_task_info();
	this->update_labour_task_info();

	return 0;
}

int WorldBossActor::on_exit_wboss_scene(const int type)
{
	WorldBossScene* wboss_scene = NULL;
	wboss_scene = WORLD_BOSS_SYSTEM->fetch_wboss_scene(this->scene_id());
	JUDGE_RETURN(wboss_scene != NULL, ERROR_SCENE_NO_EXISTS);

	wboss_scene->exit_player(this->role_id());
	return 0;
}

int WorldBossActor::wboss_enter_scene_type()
{
	if (GameCommon::is_world_boss_scene(this->scene_id()) == true)
	{
		return GameEnum::ET_WORLD_BOSS;
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()) == true
			|| this->scene_id() == this->trvl_wboss_pre_scene())
	{
		return GameEnum::ET_TRVL_WBOSS;
	}

	return -1;
}

int WorldBossActor::update_sword_pool_info()
{
	Proto31402901 inner_res;
	inner_res.set_left_add_flag(1);
	inner_res.set_left_add_num(1);
	int task_id = GameEnum::SPOOL_TASK_WBOSS;
	inner_res.set_task_id(task_id);

	MSG_USER("WorldBossActor, update_sword_pool_info, Proto31402901: %s", inner_res.Utf8DebugString().c_str());

	this->send_to_logic_thread(inner_res);
	return 0;
}

SaveInfo &WorldBossActor::fetch_save_info()
{
	return this->save_info_;
}

int WorldBossActor::update_cornucopia_task_info()
{
	Proto31403200 task_info;
	task_info.set_task_id(GameEnum::CORNUCOPIA_TASK_WBOSS);
	task_info.set_task_finish_count(1);
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}

int WorldBossActor::update_labour_task_info()
{
	Proto31403201 task_info;
	task_info.set_task_id(GameEnum::LABOUR_TASK_WBOSS);
	task_info.set_task_finish_count(1);
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}

int WorldBossActor::request_join_trvl_pre_scene()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ENTER_TRVL_WBOSS_PRE, ERROR_NORMAL_SCENE);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_TRVL_WBOSS);
	enter_info.set_server_flag(CONFIG_INSTANCE->server_flag());
	enter_info.set_main_version(CONFIG_INSTANCE->main_version());

	int ret = this->send_request_enter_info(this->trvl_wboss_pre_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ENTER_TRVL_WBOSS_PRE, ret);

	return 0;
}

int WorldBossActor::request_join_trvl_wboss(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401030*, request, RETURN_ENTER_TRVL_WBOSS);

	CONDITION_NOTIFY_RETURN(this->scene_id() == this->trvl_wboss_pre_scene(),
			RETURN_ENTER_TRVL_WBOSS, ERROR_CLIENT_OPERATE);

	int scene_id = request->boss_scene_id();
	CONDITION_NOTIFY_RETURN(GameCommon::is_trvl_wboss_scene(scene_id) == true,
			RETURN_ENTER_TRVL_WBOSS, ERROR_USE_SCENE_LIMIT);

	const Json::Value& scene_conf = CONFIG_INSTANCE->scene(scene_id);
	CONDITION_NOTIFY_RETURN(scene_conf.empty() == false, RETURN_ENTER_TRVL_WBOSS,
			ERROR_CONFIG_NOT_EXIST);

	int level_limit = scene_conf["level_limit"].asInt();
	CONDITION_NOTIFY_RETURN(this->role_detail().__level >= level_limit,
			RETURN_ENTER_TRVL_WBOSS, ERROR_PLAYER_LEVEL);

	WorldBossScene* wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(scene_id);
	CONDITION_NOTIFY_RETURN(wboss_scene != NULL, RETURN_ENTER_TRVL_WBOSS,
			ERROR_SCENE_NO_EXISTS);
	CONDITION_NOTIFY_RETURN(wboss_scene->is_player_full() == false,
			RETURN_ENTER_TRVL_WBOSS, ERROR_PLAYER_FULL);

	const Json::Value& enter_point = scene_conf["relive"];
	MoverCoord coord;
	coord.set_pixel(enter_point["posX"].asInt(), enter_point["posY"].asInt());

	this->transfer_dispatcher(scene_id, coord, SCENE_MODE_WORLD_BOSS, wboss_scene->space_id());
	return 0;
}

int WorldBossActor::request_exit_trvl_wboss()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_trvl_wboss_scene(this->scene_id()) == true,
			RETURN_LEAVE_TRVL_WBOSS, ERROR_USE_SCENE_LIMIT);

	TrvlWbossPreScene* wboss_pre_scene = TRVL_WBOSS_MONITOR->fetch_pre_scene();
	CONDITION_NOTIFY_RETURN(wboss_pre_scene != NULL, RETURN_LEAVE_TRVL_WBOSS,
			ERROR_SCENE_NO_EXISTS);

	WorldBossScene* wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	CONDITION_NOTIFY_RETURN(wboss_scene != NULL, RETURN_LEAVE_TRVL_WBOSS,
			ERROR_SCENE_NO_EXISTS);

	wboss_scene->exit_player(this->role_id());

	const Json::Value& scene_conf = CONFIG_INSTANCE->scene(this->trvl_wboss_pre_scene());
	CONDITION_NOTIFY_RETURN(scene_conf.empty() == false, RETURN_LEAVE_TRVL_WBOSS,
			ERROR_CONFIG_NOT_EXIST);

	const Json::Value& enter_point = scene_conf["relive"];
	MoverCoord coord;
	coord.set_pixel(enter_point["posX"].asInt(), enter_point["posY"].asInt());

	this->transfer_dispatcher(this->trvl_wboss_pre_scene(), coord,
			SCENE_MODE_WORLD_BOSS, wboss_pre_scene->space_id());

	return 0;
}

int WorldBossActor::request_trvl_wboss_info()
{
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_WBOSS_SCENE_ID_1,
			INNER_REQUEST_TRVL_WBOSS_INFO);
}

int WorldBossActor::login_enter_trvl_wboss(int scene_id)
{
	JUDGE_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true, 0);
	JUDGE_RETURN(GameCommon::is_trvl_wboss_scene(scene_id) == true, 0);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_TRVL_WBOSS);
	enter_info.set_server_flag(CONFIG_INSTANCE->server_flag());
	enter_info.set_main_version(CONFIG_INSTANCE->main_version());

	int ret = this->send_request_enter_info(scene_id, enter_info);
	JUDGE_RETURN(ret == 0, 0);

	return 0;
}

int WorldBossActor::on_enter_trvl_wboss_pre_scene(const int type)
{
	TrvlWbossPreScene* wboss_pre_scene = TRVL_WBOSS_MONITOR->fetch_pre_scene();
	JUDGE_RETURN(wboss_pre_scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(wboss_pre_scene);
	MapPlayer::enter_scene(type);

	return 0;
}

int WorldBossActor::on_enter_trvl_wboss_scene(const int type)
{
	WorldBossScene* wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	JUDGE_RETURN(wboss_scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(wboss_scene);
	MapPlayer::enter_scene(type);

	int camp_id = TRVL_WBOSS_MONITOR->fetch_player_camp(this->role_id());
	this->set_camp_id(camp_id);
	wboss_scene->enter_player(this->role_id(), GameEnum::ET_TRVL_WBOSS);

	return 0;
}

int WorldBossActor::on_exit_trvl_wboss_scene(const int type)
{
	JUDGE_RETURN(GameCommon::is_trvl_wboss_scene(this->scene_id()) == true, 0);

	WorldBossScene* wboss_scene = TRVL_WBOSS_MONITOR->fetch_wboss_scene(this->scene_id());
	JUDGE_RETURN(wboss_scene != NULL, 0);

	wboss_scene->exit_player(this->role_id());

	return 0;
}

int WorldBossActor::handle_exit_trvl_wboss()
{
	if (GameCommon::is_trvl_wboss_scene(this->scene_id()) == true)
	{
		this->request_exit_trvl_wboss();
	}
	else if (this->scene_id() == this->trvl_wboss_pre_scene())
	{
		this->transfer_to_save_scene();
	}

	return 0;
}

int WorldBossActor::trvl_wboss_pre_scene()
{
	return GameEnum::TRVL_WBOSS_SCENE_ID_READY;
}



