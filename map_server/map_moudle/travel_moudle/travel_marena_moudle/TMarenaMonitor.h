/*
 * TMarenaMonitor.h
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#ifndef TMARENAMONITOR_H_
#define TMARENAMONITOR_H_

#include "PubStruct.h"

class ProtoLMRole;
class Proto30400051;
class TMArenaScene;
class BattleGroundActor;

struct TMArenaRole : public BaseServerInfo, public BaseMember
{
	int sid_;
	int init_;
	int fight_state_;	//0:空闲

	int rank_;
	int score_;
	int win_times_;
	int attend_times_;
	int die_times_;
	int kill_times_;

	int is_mvp_;
	int cur_score_;
	int drop_reward_;
	Int64 start_tick_;
	Int64 update_tick_;

	TMArenaRole();
	void reset();
	void reset_everyday();

	void serialize(ProtoLMRole* proto);
	void serialize_b(ProtoLMRole* proto);
	void serialize_c(ProtoLMRole* proto);
	void serialize_d(ProtoRoleInfo* proto);
};

class TMarenaMonitor
{
public:
	enum
	{
#ifdef LOCAL_DEBUG
		MIN_FIGHTER		= 2,
#else
		MIN_FIGHTER		= 6,
#endif
		END
	};

	class MonitorTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	class ArrangeTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	class TransferTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

public:
	TMarenaMonitor();
	virtual ~TMarenaMonitor();

	const Json::Value& scene_set_conf();
	const Json::Value& fight_scene_conf();

	int prep_scene();
	int fight_scene();
	int attend_amount();

	int is_enter_time();
	int is_fighting_time();
	IntPair fighting_state();

	int request_enter_tmarena(int sid, Proto30400051* request);
	int fetch_rank_info(int sid, Int64 role);
	int handle_mointor_timeout();
	int handle_arragne_timeout();
	int handle_mointor_timeout_i(int state);

	void start();
	void stop();

	void test_marena(int id, int last);
	void start_event();
	void stop_event();
	void arrange_rivial();
	void start_transfer();

	void sort_rank();
	void update_rank();
	void send_rank_reward();

	void register_role(BattleGroundActor* player);
	void unsign(Int64 role);
	void recycle_scene(TMArenaScene* scene);

	TMArenaRole* first_role();
	TMArenaRole* find_role(Int64 role);
	TMArenaRole* find_and_pop(Int64 role);
	TMArenaScene* find_scene(int space);

public:
	int win_mpv_;
	int drop_reward_times_;
	int point_reward_[2];
	int drop_reward_[2];

private:
	int init_;
	int update_rank_;

	int activity_id_;
	int enter_level_;
	int arena_id_;
	int max_arrange_time_;
	int max_transfer_time_;

	int arrange_time_;	//安排对手时间
	LeftTimeOut transfer_time_;	//准备时间
	IntMap transfer_scene_;

	LeftTimeOut left_prep_time_;
	ActivityTimeInfo time_info_;

	MonitorTimer monitor_timer_;
	ArrangeTimer arrange_timer_;
	TransferTimer transfer_timer_;

	LongMap sign_map_;		//报名
	LongMap last_sign_map_;	//上次报名
	LongMap prep_fight_map_;//准备传送进入战斗的玩家
	ThreeObjVec his_rank_vec_;	//排行榜

	PoolPackage<TMArenaScene>* scene_package_;
	PoolPackage<TMArenaRole, Int64>* role_package_;
};

typedef Singleton<TMarenaMonitor> TMArenaMonitorSingle;
#define TRVL_MARENA_MONITOR   (TMArenaMonitorSingle::instance())

#endif /* TMARENAMONITOR_H_ */
