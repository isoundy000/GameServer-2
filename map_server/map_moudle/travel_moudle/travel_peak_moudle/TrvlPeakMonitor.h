/*
 * TrvlPeakMonitor.h
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#ifndef TRVLPEAKMONITOR_H_
#define TRVLPEAKMONITOR_H_

#include "PubStruct.h"
#include "MapMapStruct.h"

template<class Key, class Value, class HSMUTEX>
class HashMap;
class TrvlPeakPreScene;
class TrvlPeakScene;

struct TrvlPeakSysInfo
{
	TrvlPeakSysInfo(void);
	void reset(void);

	int __activity_id;
	int __enter_level;
};

class TrvlPeakMonitor
{
	friend class MMOTravTeam;

public:
	enum
	{
		TIMER_QUALITY 	= 1,
		TIMER_KNOCKOUT 	= 2,
		TIMER_SIGNUP 	= 3,

		RANK_PAGE_SIZE 	= 10,

		END
	};

	typedef HashMap<Int64, TravPeakTeam *, NULL_MUTEX> TravPeakTeamMap;
	typedef HashMap<Int64, Int64, NULL_MUTEX> TravRoleTeamMap;

	typedef ObjectPoolEx<TravPeakTeam> TravPeakTeamPool;

	class StartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
		void set_timer_type(int timer_type);

	private:
		int timer_type_;	//1:peak quality 2:peak knockout
	};

	class ArrangeTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	// one hour
	class SaveTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	// one day
	class DayTimer : public GameTimer
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
	TrvlPeakMonitor();
	virtual ~TrvlPeakMonitor();
	void reset(void);

	void start(void);
	void stop(void);

	Int64 find_team_id(const Int64 role_id);
	TravPeakTeam *find_travel_team(const Int64 team_id);
	TravPeakTeam *find_travel_team_by_role_id(const Int64 role_id);
	TrvlPeakPreScene *fetch_trvl_peak_pre_scene();

	TrvlPeakScene* find_scene(int space_id);
	void recycle_scene(TrvlPeakScene* scene);
	void push_scene(TrvlPeakScene* scene);
	void notify_recogn(Int64 role, int recogn);

	void arrange_rival_timeout();
	void arrange_rival();
	void start_transfer();

	bool is_activity_time();
	int left_prep_time();
	IntPair fetch_act_left_tick();
	int is_fighting_time();

	int check_teamer_can_enter(Int64 team_id);
	const Json::Value& scene_conf();

	//报名
	void handle_peak_signup_timeout();
	void handle_peak_signup_i(int state);
	void init_signup_act_info();

	//资格赛
	void handle_peak_quality_timeout();
	void handle_peak_quality_i(int state);
	void init_quality_act_info();
	bool is_quality_act_time();

	TravPeakQualityInfo &quality_info();

	//淘汰赛
	void handle_peak_knockout_timeout();
	void handle_peak_knockout_i(int state);
	void init_knockout_act_info();
	bool is_knockout_act_time();

	void init_pre_scene();
	void start_event(int timer_type);
	void stop_event(int timer_type);

	void set_fight_type(int type);
	int fight_type();
	int player_exp(Int64 role_id);

	void update_fight_after_team_record(TravPeakTeam *win_team, TravPeakTeam *lost_team);

public:
	int request_enter_travel_peak(int sid, Proto30400051* request);
	int sync_travel_team_info(int sid, Int64 role_id, Message *msg);
	int process_signup_travel_team(int sid, Int64 role_id, Message *msg);
	int process_fetch_travel_peak_tick(int sid, Int64 role_id, Message *msg);
	int process_fetch_trvl_team_detail(int sid, Int64 role_id, Message *msg);
	int sync_update_trvl_teamer_force_info(int sid, Int64 role_id, Message *msg);
	int sync_offline_player_info(int sid, Int64 role_id, Message *msg);
	int process_fetch_trvl_peak_rank(int sid, Int64 role_id, Message *msg);

	int register_travel_peak_team(Int64 team_id);
	int unregister_travel_peak_team(Int64 team_id); //取消

	void test_trvl_peak(int type, int last);

protected:
	void init_conf_info();
	void reset_day_info();
	void cal_activity_time();
	int peak_scene_id();
	int peak_pre_scene_id();
	int fetch_arrange_wait();	//当期等待时间

	int fetch_conti_or_break_win_score(int continue_win, const Json::Value& conf);
	int fetch_win_or_lost_reward(int times, int result, const Json::Value& conf);

	void arrange_peak(FourObj& obj);		//安排房间战斗
	IntPair find_rival(int start_index);	//查找对手

	int notify_match_knock_fight_team(Int64 team_id);

	void load_trvl_peak();
	void save_trvl_peak();
	void save_remote_travel_team(TravPeakTeam *travel_team);
	void load_peak_act_info();
	void save_peak_act_info();

	void start_send_signup_state();
	void send_act_time_to_logic();
	void active_add_player_exp();
	void cal_quality_rank_info();
	void sort_quality_score_rank();

private:
	TravPeakTeamMap *trav_peak_team_map_;       // key: team_id
	TravRoleTeamMap *trav_role_team_map_;       // key: role_id
	TravPeakTeamPool *trav_peak_team_pool_;

	PoolPackage<TrvlPeakScene>* scene_package_;

	TravPeakQualityInfo quality_info_;			// 资格赛信息
	TravPeakKnockoutInfo knockout_info_;		// 淘汰赛信息

	ActivityTimeInfo signup_time_;
	ActivityTimeInfo quality_time_;
	ActivityTimeInfo knockout_time_;
	TrvlPeakSysInfo signup_act_;
	TrvlPeakSysInfo quality_act_;
	TrvlPeakSysInfo knockout_act_;

	StartTimer signup_timer_;
	StartTimer quality_timer_;
	StartTimer knockout_timer_;
	ArrangeTimer arrange_timer_;
	SaveTimer save_timer_;
	DayTimer day_timer_;
	TransferTimer transfer_timer_;

	TrvlPeakPreScene *peak_pre_scene_;

	FourObjVec arrange_vec_;
	FourObjList fight_list_;//对手列表
	LongMap sign_map_;		//匹配战斗的队伍
	LongMap fight_map_;		//准备传送进入战斗的队伍
	LongMap player_exp_map_;//角色经验

	int monitor_init_;
	int arena_id_;			//空间ID，自增
	int arrange_time_;		//安排对手时间
	int transfer_pre_time_;	//准备时间
	int left_prep_time_;	//剩余预热时间
	int fight_type_;		//1:资格赛 2:淘汰赛
	Int64 add_exp_time_;

	bool is_update_quality_state_;	//需要更新资格赛活动状态
	bool is_update_knockout_state_;	//需要更新淘汰赛活动状态
	bool is_update_quality_rank_;	//需要更新资格赛积分信息

	//配置的数据
	int prep_waiting_time_;	//预热时间
	int wait_tick_;			//每局相隔的战斗时间
	int fighting_time_;		//战斗时间
	int quit_time_;			//退出时间
	int prep_time_;			//等待传送时间+开始倒计时
	int wait_time_;			//排队时间
	int transfer_out_time_;	//传出时间
	int exp_time_;			//经验增加间隔
	MoverCoord enter_pos_[2];//配置进入点

	int is_test_;	//本地/内网测试状态

};

typedef Singleton<TrvlPeakMonitor> TrvlPeakMonitorSingle;
#define TRVL_PEAK_MONITOR   (TrvlPeakMonitorSingle::instance())

#endif /* TRVLPEAKMONITOR_H_ */
