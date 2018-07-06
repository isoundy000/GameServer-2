/*
 * TrvlWbossMonitor.cpp
 *
 *  Created on: 2017年4月26日
 *      Author: lyw
 */

#include "TrvlWbossMonitor.h"
#include "MapMonitor.h"
#include "MMOTravel.h"
#include "MapPlayerEx.h"
#include "WorldBossScene.h"
#include "TrvlWbossPreScene.h"

TrvlWbossMonitor::CheckStartTimer::CheckStartTimer(void)
{
	this->wboss_system_ = NULL;
	this->activity_id_ = 0;
}

int TrvlWbossMonitor::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

void TrvlWbossMonitor::CheckStartTimer::init(TrvlWbossMonitor* parent)
{
	JUDGE_RETURN(NULL != parent, ;);
	this->wboss_system_ = parent;
}

void TrvlWbossMonitor::CheckStartTimer::set_activity_id(int activity_id)
{
	this->activity_id_ = activity_id;
}

int TrvlWbossMonitor::CheckStartTimer::activity_id()
{
	return this->activity_id_;
}

int TrvlWbossMonitor::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->wboss_system_ != NULL, -1);
	return this->wboss_system_->handle_wboss_timeout(this->activity_id());
}

TrvlWbossMonitor::TrvlWbossMonitor() {
	// TODO Auto-generated constructor stub
	this->wboss_scene_package_ = new PoolPackage<WorldBossScene>;
	this->wboss_pre_scene_ = new TrvlWbossPreScene();
	TrvlWbossMonitor::reset();
}

TrvlWbossMonitor::~TrvlWbossMonitor() {
	// TODO Auto-generated destructor stub
}

void TrvlWbossMonitor::reset(void)
{
	this->player_map_.clear();
	this->server_map_.clear();
	this->wboss_info_map_.clear();
	this->time_info_map_.clear();
	this->wboss_timer_map_.clear();
	this->wboss_sys_map_.clear();
}

int TrvlWbossMonitor::handle_wboss_timeout(int activity_id)
{
	JUDGE_RETURN(activity_id > 0, 0);

	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	int last_state = time_info.cur_state_;
	time_info.set_next_stage();
	this->handle_wboss_i(last_state, activity_id);

	return 0;
}

int TrvlWbossMonitor::handle_wboss_i(int state, int activity_id)
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

	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->trvl_wboss("activity_id_and_tick");
	for (uint i = 0; i < activity_id_and_tick.size(); ++i)
	{
		int id = activity_id_and_tick[i][0u].asInt();
		JUDGE_CONTINUE(activity_id == id);

		CheckStartTimer &check_timer = this->wboss_timer_map_[i];
		ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
		check_timer.cancel_timer();
		check_timer.schedule_timer(time_info.refresh_time_);

		MSG_USER("trvl world boss %d, %d, %d", time_info.time_index_,
				time_info.cur_state_, time_info.refresh_time_);
	}

	return 0;
}

int TrvlWbossMonitor::ahead_wboss_event(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	GameCommon::map_sync_activity_tips_ahead(PairObj(activity_id, true), time_info.refresh_time_);
	MSG_USER("world_boss ahead %d %d", time_info.time_index_, time_info.refresh_time_);

	for (WorldBossSceneSet::iterator iter = this->wboss_scene_set_.begin();
			iter != this->wboss_scene_set_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = *iter;
		wboss_scene->request_send_rank();
	}

	return 0;
}

int TrvlWbossMonitor::start_wboss_event(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	GameCommon::map_sync_activity_tips_start(PairObj(activity_id, true), time_info.refresh_time_);

	for (WorldBossSceneSet::iterator iter = this->wboss_scene_set_.begin();
			iter != this->wboss_scene_set_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = *iter;
		wboss_scene->check_can_generate();
	}

//	this->notify_all_player(1);

	int shout_id = CONFIG_INSTANCE->trvl_wboss("refresh_shout").asInt();
	GameCommon::trvl_announce(shout_id);
	MSG_USER("trvl world boss start %d %d", time_info.time_index_, time_info.refresh_time_);

	return 0;
}

int TrvlWbossMonitor::stop_wboss_event(int activity_id)
{
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];
	GameCommon::map_sync_activity_tips_stop(PairObj(activity_id, true), time_info.refresh_time_);

	return 0;
}

void TrvlWbossMonitor::init_trvl_wboss_pre()
{
	this->wboss_pre_scene_->init_wboss_pre();
}

void TrvlWbossMonitor::start_world_boss()
{
	this->wboss_scene_set_.clear();
	this->wboss_scene_package_->clear();
	this->wboss_timer_map_.clear();

	for (int scene_id = (GameEnum::TRVL_WBOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::TRVL_WBOSS_SCENE_ID_END; ++scene_id)
	{
		JUDGE_CONTINUE(GameCommon::is_trvl_wboss_scene(scene_id) == true);
		JUDGE_CONTINUE(MAP_MONITOR->is_current_server_scene(scene_id) == true);

		const Json::Value& scene_conf = this->scene_conf(scene_id);
		JUDGE_CONTINUE(scene_conf != Json::Value::null);

		this->start_wboss_scene(scene_id);
	}

	this->init_trvl_wboss_pre();

	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->trvl_wboss("activity_id_and_tick");
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
		MSG_USER("trvl world boss %d %d", time_info.cur_state_, time_info.refresh_time_);

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

void TrvlWbossMonitor::stop_world_boss()
{
	for (int scene_id = (GameEnum::TRVL_WBOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::TRVL_WBOSS_SCENE_ID_END; ++scene_id)
	{
		WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
		MMOTravel::update_wboss_info(&wboss_info, true);
	}
}

int TrvlWbossMonitor::start_wboss_scene(int scene_id)
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

void TrvlWbossMonitor::check_generate_boss()
{
	for (int scene_id = (GameEnum::TRVL_WBOSS_SCENE_ID_READY + 1);
			scene_id < GameEnum::TRVL_WBOSS_SCENE_ID_END; ++scene_id)
	{
		WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
		wboss_info.scene_id_ = scene_id;

		MMOTravel::load_wboss_info(&wboss_info);
		JUDGE_CONTINUE(wboss_info.status_ == 1);

		WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
		JUDGE_CONTINUE(wboss_scene != NULL);

		wboss_scene->generate_boss();
	}
}

int TrvlWbossMonitor::request_enter_trvl_wboss(int sid, Proto30400051* request)
{
	int scene_id = request->request_scene();
	if (scene_id == GameEnum::TRVL_WBOSS_SCENE_ID_READY)
	{
		return this->request_enter_wboss_pre(sid, request);
	}
	else
	{
		return this->request_enter_wboss_scene(sid, request);
	}
}

int TrvlWbossMonitor::request_enter_wboss_scene(int sid, Proto30400051* request)
{
	int scene_id = request->request_scene();
	JUDGE_RETURN(GameCommon::is_trvl_wboss_scene(scene_id) == true, ERROR_USE_SCENE_LIMIT);

	string main_version = request->main_version();
	if (main_version.empty() == false)
	{
		JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true,
				ERROR_TRVL_SAME_VERSION);
	}

	const Json::Value& scene_conf = this->scene_conf(scene_id);
	JUDGE_RETURN(scene_conf != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

	int level_limit = scene_conf["level_limit"].asInt();
	JUDGE_RETURN(request->role_level() >= level_limit, ERROR_PLAYER_LEVEL);

	WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
	JUDGE_RETURN(wboss_scene != NULL, ERROR_CLIENT_OPERATE);

	//设置玩家阵营
	this->set_server_player(sid, request->role_id());

	// 判断房间人数
	JUDGE_RETURN(wboss_scene->is_player_full() == false, ERROR_PLAYER_FULL);

	Proto30400052 enter_info;
	enter_info.set_space_id(wboss_scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_WORLD_BOSS);

	const Json::Value& enter_point = scene_conf["relive"];
	enter_info.set_pos_x(enter_point["posX"].asInt());
	enter_info.set_pos_y(enter_point["posY"].asInt());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int TrvlWbossMonitor::request_enter_wboss_pre(int sid, Proto30400051* request)
{
	int scene_id = request->request_scene();
	JUDGE_RETURN(scene_id == GameEnum::TRVL_WBOSS_SCENE_ID_READY, ERROR_USE_SCENE_LIMIT);

	string main_version = request->main_version();
	if (main_version.empty() == false)
	{
		JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true,
				ERROR_TRVL_SAME_VERSION);
	}

	const Json::Value& scene_conf = this->scene_conf(scene_id);
	JUDGE_RETURN(scene_conf != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

	int level_limit = scene_conf["level_limit"].asInt();
	JUDGE_RETURN(request->role_level() >= level_limit, ERROR_PLAYER_LEVEL);

	TrvlWbossPreScene* wboss_pre_scene = this->fetch_pre_scene();
	JUDGE_RETURN(wboss_pre_scene != NULL, ERROR_CLIENT_OPERATE);

	//设置玩家阵营
	this->set_server_player(sid, request->role_id());

	Proto30400052 enter_info;
	enter_info.set_space_id(wboss_pre_scene->space_id());
	enter_info.set_scene_mode(SCENE_MODE_WORLD_BOSS);

	const Json::Value& enter_point = scene_conf["relive"];
	enter_info.set_pos_x(enter_point["posX"].asInt());
	enter_info.set_pos_y(enter_point["posY"].asInt());

	return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

int TrvlWbossMonitor::request_fetch_wboss_info(int gate_id, Int64 role_id, Message* msg)
{
	Proto50401028 respond;
	for (WorldBossInfoMap::iterator iter = this->wboss_info_map_.begin();
			iter != this->wboss_info_map_.end(); ++iter)
	{
		WorldBossScene* wboss_scene = this->fetch_wboss_scene(iter->first);
		JUDGE_CONTINUE(wboss_scene != NULL);

		WorldBossInfo& wboss_info = iter->second;
		ProtoWorldBossInfo *wb_info = respond.add_wb_info();
		wb_info->set_boss_scene_id(wboss_info.scene_id_);
		wb_info->set_boss_status(wboss_info.status_);
		wb_info->set_blood(wboss_info.cur_blood_);
		wb_info->set_killer(wboss_info.killer_name_);
		wb_info->set_is_full(wboss_scene->is_player_full());
	}
	int refresh_tick = this->fetch_refresh_tick();
	respond.set_refresh_tick(refresh_tick);

	return MAP_MONITOR->dispatch_to_client_from_gate(gate_id, role_id, &respond);
}

const Json::Value& TrvlWbossMonitor::scene_conf(int scene_id)
{
	return CONFIG_INSTANCE->scene(scene_id);
}

WorldBossScene* TrvlWbossMonitor::fetch_wboss_scene(int scene_id)
{
	return this->wboss_scene_package_->find_object(scene_id);
}

TrvlWbossPreScene* TrvlWbossMonitor::fetch_pre_scene()
{
	return this->wboss_pre_scene_;
}

void TrvlWbossMonitor::set_boss_status(int scene_id, int status,
		Int64 killer, string killer_name, double cur_blood)
{
	WorldBossInfo& wboss_info = this->wboss_info_map_[scene_id];
	wboss_info.status_ = status;
	wboss_info.killer_ = killer;
	wboss_info.killer_name_ = killer_name;
	wboss_info.cur_blood_ = cur_blood;

	MMOTravel::update_wboss_info(&wboss_info, false);
}

int TrvlWbossMonitor::fetch_refresh_tick()
{
	int now_tick = ::time(NULL) - GameCommon::today_zero();
	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->trvl_wboss("activity_id_and_tick");
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

void TrvlWbossMonitor::set_server_player(int sid, Int64 role_id)
{
	if (this->server_map_.count(sid) <= 0)
	{
		int camp_id = this->server_map_.size() + 1;
		this->server_map_[sid] = camp_id;
	}

	this->player_map_[role_id] = sid;
}

int TrvlWbossMonitor::fetch_player_camp(Int64 role_id)
{
	JUDGE_RETURN(this->player_map_.count(role_id) > 0, 0);

	int sid = this->player_map_[role_id];
	JUDGE_RETURN(this->server_map_.count(sid) > 0, 0);

	return this->server_map_[sid];
}

int TrvlWbossMonitor::fetch_server_sid(Int64 role_id)
{
	JUDGE_RETURN(this->player_map_.count(role_id) > 0, -1);
	return this->player_map_[role_id];
}

void TrvlWbossMonitor::test_trvl_wboss(int time, int id, int set_time)
{
	const Json::Value& activity_id_and_tick = CONFIG_INSTANCE->trvl_wboss("activity_id_and_tick");
	int activity_id = activity_id_and_tick[time][0u].asInt();
	ActivityTimeInfo &time_info = this->time_info_map_[activity_id];

	time_info.cur_state_ = (id + 1) % time_info.time_span_;
	time_info.refresh_time_ = set_time;
	this->handle_wboss_i(id, activity_id);
}

void TrvlWbossMonitor::test_relive_boss(int scene_id)
{
	JUDGE_RETURN(GameCommon::is_trvl_wboss_scene(scene_id) == true, ;);

	WorldBossScene* wboss_scene = this->fetch_wboss_scene(scene_id);
	JUDGE_RETURN(wboss_scene != NULL, ;);

	wboss_scene->generate_boss();
	this->set_boss_status(scene_id, 1, 0, "", 1);
}






