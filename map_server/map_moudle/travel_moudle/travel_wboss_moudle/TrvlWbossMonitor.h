/*
 * TrvlWbossMonitor.h
 *
 *  Created on: 2017年4月26日
 *      Author: lyw
 */

#ifndef TRVLWBOSSMONITOR_H_
#define TRVLWBOSSMONITOR_H_

#include "PubStruct.h"
#include "ProtoDefine.h"
#include "MapMapStruct.h"
class WorldBossScene;
class TrvlWbossPreScene;

class TrvlWbossMonitor
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

		void init(TrvlWbossMonitor* parent);
		void set_activity_id(int activity_id);
		int activity_id();

	private:
		TrvlWbossMonitor* wboss_system_;
		int activity_id_;
	};
	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;

public:
	TrvlWbossMonitor();
	virtual ~TrvlWbossMonitor();
	void reset(void);

	int handle_wboss_timeout(int activity_id);
	int handle_wboss_i(int state, int activity_id);

	int ahead_wboss_event(int activity_id);
	int start_wboss_event(int activity_id);
	int stop_wboss_event(int activity_id);

	void init_trvl_wboss_pre();

	void start_world_boss();
	void stop_world_boss();
	int start_wboss_scene(int scene_id);
	void check_generate_boss();

	int request_enter_trvl_wboss(int sid, Proto30400051* request);
	int request_enter_wboss_scene(int sid, Proto30400051* request);
	int request_enter_wboss_pre(int sid, Proto30400051* request);
	int request_fetch_wboss_info(int gate_id, Int64 role_id, Message* msg);

public:
	const Json::Value& scene_conf(int scene_id);

	WorldBossScene* fetch_wboss_scene(int scene_id);
	TrvlWbossPreScene* fetch_pre_scene();
	void set_boss_status(int scene_id, int status, Int64 killer, string killer_name, double cur_blood);
	int fetch_refresh_tick();	//配置里的刷新时间

	void set_server_player(int sid, Int64 role_id);
	int fetch_player_camp(Int64 role_id);
	int fetch_server_sid(Int64 role_id);

	void test_trvl_wboss(int time, int id, int set_time);
	void test_relive_boss(int scene_id);

private:
	LongMap player_map_; 	//玩家标识
	IntMap server_map_; 	//服务器阵营

	ActivityTimeInfoMap time_info_map_;
	CheckStartTimerMap wboss_timer_map_;
	WBossSysInfoMap wboss_sys_map_;

	WorldBossInfoMap wboss_info_map_;
	WorldBossSceneSet wboss_scene_set_;
	PoolPackage<WorldBossScene>* wboss_scene_package_;
	TrvlWbossPreScene* wboss_pre_scene_;

};

typedef Singleton<TrvlWbossMonitor> TrvlWbossMonitorSingle;
#define TRVL_WBOSS_MONITOR   (TrvlWbossMonitorSingle::instance())

#endif /* TRVLWBOSSMONITOR_H_ */
