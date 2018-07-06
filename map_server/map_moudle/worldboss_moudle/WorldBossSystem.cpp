/*
 * WorldBossSystem.cpp
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#include "WorldBossSystem.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "WorldBossScene.h"
#include "MMOWorldBoss.h"

WorldBossSystem::CheckStartTimer::CheckStartTimer(void)
{
	this->wboss_system_ = NULL;
	this->activity_id_ = 0;
}

void WorldBossSystem::CheckStartTimer::init(WorldBossSystem* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->wboss_system_ = parent;
}

void WorldBossSystem::CheckStartTimer::set_activity_id(int activity_id)
{
	this->activity_id_ = activity_id;
}

int WorldBossSystem::CheckStartTimer::activity_id()
{
	return this->activity_id_;
}

int WorldBossSystem::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int WorldBossSystem::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->wboss_system_ != NULL, -1);
	return this->wboss_system_->handle_wboss_timeout(this->activity_id());
}

WorldBossSystem::WorldBossSystem() {
	// TODO Auto-generated constructor stub
	this->wboss_scene_package_ = new PoolPackage<WorldBossScene>;
	WorldBossSystem::reset();
}

WorldBossSystem::~WorldBossSystem() {
	// TODO Auto-generated destructor stub
}

void WorldBossSystem::reset(void)
{
	this->server_flag_ = false;
	this->wboss_info_map_.clear();
	this->wboss_sys_map_.clear();
	this->time_info_map_.clear();
}

int WorldBossSystem::handle_wboss_timeout(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	int last_state = time_info.cur_state_;
	time_info.set_next_stage();
	this->handle_wboss_i(last_state, activity_id);

	return 0;
}

int WorldBossSystem::handle_wboss_i(int state, int activity_id)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_wboss_event(activity_id);
		break;
	}
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->start_wboss_event(activity_id);
		break;
	}
	case GameEnum::ACTIVITY_STATE_START:
	{
		this->stop_wboss_event(activity_id);
		break;
	}
	}

	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->world_boss("activity_id_and_tick");
	for (uint i = 0; i < activity_id_and_tick.size(); ++i)
	{
		int id = activity_id_and_tick[i][0u].asInt();
		JUDGE_CONTINUE(activity_id == id);

		CheckStartTimer &check_timer = this->wboss_timer_map_[i];
		ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
		check_timer.cancel_timer();
		check_timer.schedule_timer(time_info.refresh_time_);

		MSG_USER("world_boss %d, %d, %d", time_info.time_index_,
				time_info.cur_state_, time_info.refresh_time_);
	}
	return 0;
}

int WorldBossSystem::ahead_wboss_event(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	GameCommon::map_sync_activity_tips_ahead(activity_id, time_info.refresh_time_);
	MSG_USER("world_boss ahead %d %d", time_info.time_index_, time_info.refresh_time_);

	for (WorldBossSceneSet::iterator iter = this->wboss_scene_set_.begin();
			iter != this->wboss_scene_set_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = *iter;
		wboss_scene->request_send_rank();
	}

	return 0;
}

int WorldBossSystem::start_wboss_event(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	GameCommon::map_sync_activity_tips_start(activity_id, time_info.refresh_time_);

	for (WorldBossSceneSet::iterator iter = this->wboss_scene_set_.begin();
			iter != this->wboss_scene_set_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = *iter;
		wboss_scene->check_can_generate();
	}

	this->notify_all_player(1);

	int shout_id = CONFIG_INSTANCE->world_boss("refresh_shout").asInt();
	GameCommon::announce(shout_id);
	MSG_USER("world_boss start %d %d", time_info.time_index_, time_info.refresh_time_);
	return 0;
}

int WorldBossSystem::stop_wboss_event(int activity_id)
{
	GameCommon::map_sync_activity_tips_stop(activity_id);
	return 0;
}

void WorldBossSystem::start_world_boss()
{
	this->wboss_scene_set_.clear();
	this->wboss_scene_package_->clear();
	this->wboss_timer_map_.clear();

	for (int scene_id = (GameEnum::WORLD_BOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END; ++scene_id)
	{
		JUDGE_CONTINUE(GameCommon::is_world_boss_scene(scene_id) == true);
		JUDGE_CONTINUE(MAP_MONITOR->is_current_server_scene(scene_id) == true);

		const Json::Value& scene_conf = this->scene_conf(scene_id);
		JUDGE_CONTINUE(scene_conf != Json::Value::null);

		this->start_wboss_scene(scene_id);
		this->server_flag_ = true;
	}
	JUDGE_RETURN(this->server_flag_ == true, ;);

	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->world_boss("activity_id_and_tick");
	for (uint i = 0; i < activity_id_and_tick.size(); ++i)
	{
		int activity_id = activity_id_and_tick[i][0u].asInt();
		this->wboss_sys_map_[i] = activity_id;

		const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(activity_id);
		JUDGE_CONTINUE(activity_conf.empty() == false);

		ActivityTimeInfo time_info;
		GameCommon::cal_activity_info(time_info, activity_conf);
		this->time_info_map_[activity_id] = time_info;

		CheckStartTimer &check_timer = this->wboss_timer_map_[i];
		check_timer.init(this);
		check_timer.set_activity_id(activity_id);

		check_timer.schedule_timer(time_info.refresh_time_);
		MSG_USER("world_boss %d %d", time_info.cur_state_, time_info.refresh_time_);

		if (time_info.cur_state_ == GameEnum::ACTIVITY_STATE_START)
		{
			this->start_wboss_event(activity_id);
		}
		else
		{
			this->check_generate_boss();
		}
	}
}

void WorldBossSystem::stop_world_boss()
{
	JUDGE_RETURN(this->server_flag_ == true, ;);

	for (int scene_id = (GameEnum::WORLD_BOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END; ++scene_id)
	{
		WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
		MMOWorldBoss::update_wboss_info(&wboss_info, true);
	}
}

int WorldBossSystem::start_wboss_scene(int scene_id)
{
	for (WorldBossSceneSet::iterator iter = this->wboss_scene_set_.begin();
			iter != this->wboss_scene_set_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = *iter;
		JUDGE_RETURN(wboss_scene->scene_id() != scene_id, -1);
	}

	WorldBossScene* wboss_scene = this->wboss_scene_package_->pop_object();
	JUDGE_RETURN(wboss_scene != NULL, NULL);

	int space_id = 0;
	wboss_scene->init_wboss_scene(scene_id, space_id);
	wboss_scene->run_scene();

	this->wboss_scene_package_->bind_object(scene_id, wboss_scene);
	this->wboss_scene_set_.push_back(wboss_scene);

	return 0;
}

void WorldBossSystem::check_generate_boss()
{
	for (int scene_id = (GameEnum::WORLD_BOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END; ++scene_id)
	{
		WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
		wboss_info.scene_id_ = scene_id;

		MMOWorldBoss::load_wboss_info(&wboss_info);
		JUDGE_CONTINUE(wboss_info.status_ == 1);

		WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
		JUDGE_CONTINUE(wboss_scene != NULL);

		wboss_scene->generate_boss();
	}
}

int WorldBossSystem::request_enter_wboss(int sid, Proto30400051* request)
{
	int scene_id = request->request_scene();
	JUDGE_RETURN(this->is_wboss_scene(scene_id) == true, ERROR_USE_SCENE_LIMIT);

	const Json::Value& scene_conf = this->scene_conf(scene_id);
	JUDGE_RETURN(scene_conf != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

	int level_limit = scene_conf["level_limit"].asInt();
	JUDGE_RETURN(request->role_level() >= level_limit, ERROR_PLAYER_LEVEL);

	WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
	JUDGE_RETURN(wboss_scene != NULL, ERROR_CLIENT_OPERATE);

	Proto30400052 enter_info;
	enter_info.set_space_id(wboss_scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_WORLD_BOSS);

	const Json::Value& enter_point = scene_conf["relive"];
	enter_info.set_pos_x(enter_point["posX"].asInt());
	enter_info.set_pos_y(enter_point["posY"].asInt());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int WorldBossSystem::request_fetch_wboss_info(int gate_id, Int64 role_id, Message* msg)
{
	Proto50401022 respond;
	for (WorldBossInfoMap::iterator iter = this->wboss_info_map_.begin();
			iter != this->wboss_info_map_.end(); ++iter)
	{
		WorldBossInfo& wboss_info = iter->second;
		ProtoWorldBossInfo *wb_info = respond.add_wb_info();
		wb_info->set_boss_scene_id(wboss_info.scene_id_);
		wb_info->set_boss_status(wboss_info.status_);
	}
	int refresh_tick = this->fetch_refresh_tick();
	respond.set_refresh_tick(refresh_tick);

	return MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
}

int WorldBossSystem::login_send_red_point(int gate_id, Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30402202*, request, -1);

	int level = request->level();
	int red_flag = false;
	for (WorldBossInfoMap::iterator iter = this->wboss_info_map_.begin();
			iter != this->wboss_info_map_.end(); ++iter)
	{
		WorldBossInfo& wboss = iter->second;
		const Json::Value& scene_conf = this->scene_conf(wboss.scene_id_);
		JUDGE_CONTINUE(scene_conf != Json::Value::null);

		int level_limit = scene_conf["level_limit"].asInt();
		JUDGE_CONTINUE(wboss.status_ == 1 && level >= level_limit);

		red_flag = true;
		break;
	}

	int event_id = GameEnum::PA_EVENT_WORLD_BOSS;
	if (red_flag == true)
	{
		Proto81401703 respond;
		respond.set_even_id(event_id);
		respond.set_even_value(1);
		MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
	}
	else
	{
		Proto81401705 respond;
		respond.set_even_id(event_id);
		MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
	}
	return MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, RETURN_FETCH_RED_POINT);
}

int WorldBossSystem::notify_all_player(int value)
{
	Proto80401026 active_res;
	active_res.set_value(value);
	MAP_MONITOR->notify_all_player_info(ACTIVE_SEND_WBOSS_CHANGE, &active_res);

	return 0;
}

bool WorldBossSystem::is_wboss_scene(int scene_id)
{
	return (scene_id > GameEnum::WORLD_BOSS_SCENE_ID_READY && scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END);
}

int WorldBossSystem::fetch_refresh_tick()
{
	int now_tick = ::time(NULL) - GameCommon::today_zero();
	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->world_boss("activity_id_and_tick");
	for (uint i = 0; i < activity_id_and_tick.size(); ++i)
	{
		int hour = activity_id_and_tick[i][1u].asInt();
		int min = activity_id_and_tick[i][2u].asInt();
		int refresh_tick = hour*3600 + min*60;
		JUDGE_CONTINUE(refresh_tick > now_tick);

		return refresh_tick;
	}

	int next_hour = activity_id_and_tick[0u][1u].asInt();
	int next_min = activity_id_and_tick[0u][2u].asInt();
	return next_hour*3600 + next_min*60;
}

int WorldBossSystem::real_refresh_tick()
{
	int start_time = 0;
	int cur_tick = GameCommon::fetch_cur_day_sec();
	for (ActivityTimeInfoMap::iterator iter = this->time_info_map_.begin();
			iter != this->time_info_map_.end(); ++iter)
	{
		ActivityTimeInfo& time_info = iter->second;
		JUDGE_CONTINUE(cur_tick <= time_info.fetch_start_time());

		start_time = time_info.fetch_start_time();
		break;
	}

	if (start_time == 0)
	{
		ActivityTimeInfoMap::iterator iter = this->time_info_map_.begin();
		start_time = iter->second.fetch_start_time();
	}

	if (start_time >= cur_tick)
		return start_time - cur_tick;
	else
		return start_time + Time_Value::DAY - cur_tick;

}

const Json::Value& WorldBossSystem::scene_conf(int scene_id)
{
	return CONFIG_INSTANCE->scene(scene_id);
}

WorldBossScene* WorldBossSystem::fetch_wboss_scene(int scene_id)
{
	return this->wboss_scene_package_->find_object(scene_id);
}

void WorldBossSystem::set_boss_status(int scene_id, int status)
{
	WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
	wboss_info.status_ = status;

	if (status == 0)
		this->notify_all_player(0);

	//保存boss状态
	MMOWorldBoss::update_wboss_info(&wboss_info, false);
}

void WorldBossSystem::test_relive_boss(int scene_id)
{
	JUDGE_RETURN(GameCommon::is_world_boss_scene(scene_id) == true, ;);

	WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
	JUDGE_RETURN(wboss_scene != NULL, ;);

	wboss_scene->generate_boss();
	this->set_boss_status(scene_id, 1);
}

void WorldBossSystem::test_open_wboss(int time, int id, int set_time)
{
	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->world_boss("activity_id_and_tick");
	int activity_id = activity_id_and_tick[time][0u].asInt();
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];

	time_info.cur_state_ = (id + 1) % time_info.time_span_;
	time_info.refresh_time_ = set_time;
	this->handle_wboss_i(id, activity_id);
}

