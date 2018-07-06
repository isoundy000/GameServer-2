/*
 * CollectChestsScene.h
 *
 *  Created on: 2016年8月9日
 *      Author: lzy0927
 */

#ifndef COLLECTCHESTSSCENE_H_
#define COLLECTCHESTSSCENE_H_


#include "Scene.h"

enum ChestInfo{
	MAX_NUM = 20,

	CHEST_INFO_END
};
class CollectChestsScene : public Scene
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

	class RefreshTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);
	};

	class BroadcastTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);
	};

	class CountDownTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);
	};

	typedef std::map<int, CheckStartTimer>CheckStartTimerMap;

public:
	CollectChestsScene();
	virtual ~CollectChestsScene();
	static CollectChestsScene* instance();

	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);
	virtual int handle_ai_alive_recycle(GameAI* game_ai);
	virtual IntPair fetch_addition_exp();

	int erase_notify_alive_info(GameAI* game_ai);
	int notify_refresh_chests();
	int request_enter_collect_chests(int gate_id, Proto30400051* request);

	int handle_broadcast_timer();
	int check_and_start_broadcast(int start_tick);

	int midnight_refresh(time_t now_tick);
	int handle_refresh_chests(time_t now_tick);

	int handle_refresh_chests(CollectChestsDetail::ChestsItem& item);

	void init_collect_chests(Scene* scene);
	void new_start();
	void new_stop();
	int handle_chests_timeout(int type);
	int handle_chests_i(int state);
	int ahead_chests_event();
	ActivityTimeInfo &get_time_info();
	int get_cycle_id();

	int is_player_have_join(Int64 role_id);

	void handle_activity_ai_die(GameAI* game_ai, Int64 killer_id);
	void handle_activity_ai_die(Int64 killer_id);
	void check_generate_chests(CollectChestsDetail::ChestsItem& item);

	void test_chests(int id, int set_time);
private:
	CollectChestsDetail chests_detail_;
	RefreshTimer refresh_timer_;
	BroadcastTimer broad_timer_;
	ActivityStartTimer start_timer_;
	CountDownTimer count_down_timer_;
	Scene* scene_;

	CheckStartTimerMap check_start_timer_map;
	TimeInfoMap time_info_map;
};

#define COLLECTCHESTS_INSTANCE		CollectChestsScene::instance()

#endif /* COLLECTCHESTSSCENE_H_ */
