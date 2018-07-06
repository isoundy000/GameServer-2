/*
 * TrvlArenaMonitor.h
 *
 *  Created on: Sep 26, 2016
 *      Author: peizhibi
 */

#ifndef TRVLARENAMONITOR_H_
#define TRVLARENAMONITOR_H_

#include "PubStruct.h"

class ProtoLMRole;
class Proto30400051;
class TrvlArenaScene;
class BattleGroundActor;

struct TrvlArenaRole : public BaseServerInfo, public BaseMember
{
	enum
	{
		STATE_NONE 		= 0,	//空闲
		STATE_ATTEND	= 1,	//参与战斗
		END
	};

	TrvlArenaRole();
	void reset();
	void reset_everyday();
	void serialize_rank(ProtoLMRole* proto);

	int index();
	int validate();
	const Json::Value& conf();

	int sid_;
	int rank_;			//排名
	int sync_;			//跟单服数据同步
	int stage_;			//段位
	int score_;			//积分
	Int64 update_tick_;	//时间
	int win_times_;		//赢次数
	int con_lose_times_;//连输次数
	Int64 last_rival_;	//上次对手

	int init_;
	int state_;			//状态
	int space_id_;
	int pos_index_;
	Int64 start_tick_;
};

class TrvlArenaMonitor
{
public:
	enum
	{
		RIVAL_ROLE		= 1,		//对手
		RIVAL_ROBOT_A	= 2,		//机器A
		RIVAL_ROBOT_B	= 3,		//机器B

		MAX_ROLE		= 2,
		PAGE_SIZE		= 10,
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

	typedef std::list<FourObj> FourObjList;

public:
	TrvlArenaMonitor();
	~TrvlArenaMonitor();

	const Json::Value& scene_conf();
	const Json::Value& scene_set_conf();

	TrvlArenaRole* find_role(Int64 role);
	TrvlArenaRole* find_and_pop(Int64 role);
	TrvlArenaScene* find_scene(int space_id);

	int scene_id();
	int prep_scene_id();

	int left_prep_time();
	int is_enter_time();
	int is_fighting_time();
	IntPair fighting_state();

	void clear_history_db(int force = false);
	void recycle_scene(TrvlArenaScene* scene);
	void notify_recogn(Int64 role, int recogn);

	void start();
	void stop();
	void test_arena(int id, int last);
	void handle_monitor_timeout();
	void handle_monitor_timeout_i(int state);
	void arrange_rival_timeout();

	void arrange_rival();
	void start_transfer();

	void ahead_event();
	void start_event();
	void stop_event();

	int sign(int sid, Int64 role, Message* msg);	//报名
	int unsign(int sid, Int64 role); //取消
	int operate(int sid, Int64 role, Message* msg);
	int fetch_info(int sid, Message* msg);
	int fetch_rank(int sid, Int64 role, Message* msg);
	int fetch_arrange_wait();	//当期等待时间
	int send_offline_score(TrvlArenaRole* role);

	int register_tarena_role(BattleGroundActor* player);
	int request_enter_tarena(int sid, Proto30400051* request);

private:
	IntPair find_rival(int start_index);

	void load();
	void save();
	void arrange_arena(FourObj& obj);
	void sort_history_rank(int check = true);	//排行榜

private:
	int monitor_init_;

	int TOTAL_ARRANGE_TIME;	//等待时间
	int ROBOT_B_LOSE_TIMES;	//输4次，打机器人B
	int PREP_WAITING_TIME;	//预热时间
	int TRANSFER_OUT_TIME;	//传出时间
	MoverCoord enter_pos_[MAX_ROLE];	//配置进入点

	int check_tick_;
	int enter_level_;
	int activity_id_;
	int same_rivial_;

	int arena_id_;		//空间ID，自增
	int arrange_time_;	//安排对手时间
	int transfer_time_;	//准备时间
	int left_prep_time_;//剩余预热时间
	FourObjVec arrange_vec_;
	FourObjList fight_list_;	//对手列表

	LongMap sign_map_;		//报名
	LongMap fight_map_;		//准备传送进入战斗的玩家

	ThreeObjVec his_rank_vec_;	//排行榜
	ActivityTimeInfo time_info_;
	MonitorTimer monitor_timer_;
	ArrangeTimer arrange_timer_;
	TransferTimer transfer_timer_;

	PoolPackage<TrvlArenaScene>* scene_package_;
	PoolPackage<TrvlArenaRole, Int64>* role_package_;
};

typedef Singleton<TrvlArenaMonitor> TrvlArenaMonitorSingle;
#define TRVL_ARENA_MONITOR   (TrvlArenaMonitorSingle::instance())

#endif /* TRVLARENAMONITOR_H_ */
