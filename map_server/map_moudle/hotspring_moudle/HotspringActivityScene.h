/*
 * HotspringActivityScene.h
 *
 *  Created on: 2016年9月18日
 *      Author: lzy
 */

#ifndef HOTSPRINGACTIVITYSCENE_H_
#define HOTSPRINGACTIVITYSCENE_H_

#include "Scene.h"

    //温泉
    enum
    {
    	HOTSPRING_ACTIVITY_FIRST_ID = 20001,
    	HOTSPRING_ACTIVITY_SECOND_ID = 20011,
    	HOTSPRING_ACTIVITY_EXP_INTERVAL = 5,
    	HOTSPRING_END
    };

class HotspringActivityScene : public Scene
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_act_type(int type);

		int act_type_;
	};

	class ActivityStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	class WaitTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
	};

	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;
public:
	HotspringActivityScene();
	virtual ~HotspringActivityScene();
	static HotspringActivityScene* instance();

	virtual int makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info);

	ActivityTimeInfo &get_time_info();
	int request_enter_hotspring_activity(int gate_id, Proto30400051* request);
	void new_start();
	void init_hotspring_scene();
	void new_stop();
	int handle_hotspring_timeout(int type);
	int handle_hotspring_i(int state);
	int ahead_hotspring_event();
	void wait_timeout();
	void start_timeout();
	void notify_player_wait_timeout();
	void notify_player_start_timeout();
	void notify_player_get_award();

	int bind_double_major(Int64 player_a, Int64 player_b);
	int unbind_double_major(Int64 player_a, Int64 player_b);
	int is_player_double_major(Int64 id);

	Int64 find_double_player(Int64 role_id);
	int is_player_has_choose(Int64 id);
	int player_choose_answer(Int64 id, int answer);

	int get_refresh_time();
	int refresh_player_info(Int64 id);
	int get_cur_status();
	int get_cycle_id();
	int announce_work(const int type, const int ext_int = 0, const string ext_str = "");
	HotspringActivityDetail &get_hotspring_detail();

	int get_player_is_right(Int64 player_id);
	Int64 &get_player_exp(Int64 player_id);
	void test_hotspring(int id, int set_time);

	int find_near_player(Int64 role_id, MoverCoord& coord, string& name, Int64& aim_id);
private:
	HotspringActivityDetail hotspring_detail_;
	TimeInfoMap time_info_map_;
	WaitTimer wait_timer_;
	ActivityStartTimer start_timer_;
	CheckStartTimerMap check_start_timer_map;
};


#define HOTSPRING_INSTANCE		HotspringActivityScene::instance()
#endif /* HOTSPRINGACTIVITYSCENE_H_ */
