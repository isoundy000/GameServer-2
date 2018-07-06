/*
 * WorldBossSystem.h
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#ifndef WORLDBOSSSYSTEM_H_
#define WORLDBOSSSYSTEM_H_

#include "PubStruct.h"
#include "ProtoDefine.h"
#include "MapMapStruct.h"

class WorldBossScene;

class WorldBossSystem
{
public:
	typedef std::vector<WorldBossScene*> WorldBossSceneSet;
	typedef std::map<int, WorldBossInfo> WorldBossInfoMap;
	typedef std::map<int, ActivityTimeInfo> ActivityTimeInfoMap;
	typedef std::map<uint, int> WBossSysInfoMap;

	class CheckStartTimer : public GameTimer
	{
	public:
		CheckStartTimer(void);
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void init(WorldBossSystem* parent);
		void set_activity_id(int activity_id);
		int activity_id();

	private:
		WorldBossSystem* wboss_system_;
		int activity_id_;
	};
	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;

public:
	WorldBossSystem();
	virtual ~WorldBossSystem();
	void reset(void);

	int handle_wboss_timeout(int activity_id);
	int handle_wboss_i(int state, int activity_id);

	int ahead_wboss_event(int activity_id);
	int start_wboss_event(int activity_id);
	int stop_wboss_event(int activity_id);

	void start_world_boss();
	void stop_world_boss();
	int start_wboss_scene(int scene_id);
	void check_generate_boss();

	int request_enter_wboss(int sid, Proto30400051* request);
	int request_fetch_wboss_info(int gate_id, Int64 role_id, Message* msg);
	int login_send_red_point(int gate_id, Int64 role_id, Message* msg);
	int notify_all_player(int value);
	bool is_wboss_scene(int scene_id);

	int fetch_refresh_tick();	//配置里的刷新时间
	int real_refresh_tick();	//刷新时间倒计时
	const Json::Value& scene_conf(int scene_id);
	WorldBossScene* fetch_wboss_scene(int scene_id);
	void set_boss_status(int scene_id, int status);

	void test_relive_boss(int scene_id);
	void test_open_wboss(int time, int id, int set_time);

private:
	int server_flag_; //场景标识

	WorldBossSceneSet wboss_scene_set_;
	WorldBossInfoMap wboss_info_map_;

	WBossSysInfoMap wboss_sys_map_;
	ActivityTimeInfoMap time_info_map_;
	CheckStartTimerMap wboss_timer_map_;

	PoolPackage<WorldBossScene>* wboss_scene_package_;
};

typedef Singleton<WorldBossSystem> WorldBossSystemSingleton;
#define WORLD_BOSS_SYSTEM  WorldBossSystemSingleton::instance()

#endif /* WORLDBOSSSYSTEM_H_ */
