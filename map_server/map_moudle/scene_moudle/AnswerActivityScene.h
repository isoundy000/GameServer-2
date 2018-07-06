/*
 * AnswerActivityScene.h
 *
 *  Created on: 2016年8月13日
 *      Author: lzy
 */

#ifndef ANSWERACTIVITYSCENE_H_
#define ANSWERACTIVITYSCENE_H_


#include "Scene.h"

    enum
	{
    	ANSWER_ACTIVITY_ID = 20031,
    	ANSWER_ACTIVITY_RANK_NUM = 5,
    	ANSWER_ACTIVITY_END
    };

class AnswerActivityScene : public Scene
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
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

	class RefreshTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);
	};

	class SuperviseTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);
	};

	class TopPlayerCoordTimer : public GameTimer
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

public:
	AnswerActivityScene();
	virtual ~AnswerActivityScene();
	static AnswerActivityScene* instance();

	int handle_answer_question(Int64 player_id, Int64 time);

	virtual IntPair fetch_addition_exp();

	int erase_notify_alive_info(GameAI* game_ai);
	int notify_refresh_topic();
	int notify_right_answer();
	int answer_done_work();
	int calc_player_score(const int time_num);

	int request_enter_answer_activity(int gate_id, Proto30400051* request);
	AnswerActivityDetail &get_AnswerActivityDetail();
	int handle_broadcast_timer();
	int check_and_start_broadcast(int start_tick);

	int handle_refresh_topic(time_t now_tick);
	int handle_wait_time(time_t now_tick);

	void refresh_rank_list(void);
	void notify_player_get_award(void);
	void notify_answer_activity_info(void);
	void notify_top_player_num(void);
	void notify_top_player_coord(void);
	void new_start();
	void init_answer_scene();
	void new_stop();
	int player_wait_time(Int64 player_id);
	int handle_answer_timeout();
	int handle_answer_i(int state);
	int ahead_answer_event();

	void test_answer(int id, int set_time);
	void wait_timeout();
	void refresh_timeout();
	void refresh_player_info();

	int is_on_activity(int cycle_id, int year, int mouth, int day);
	int next_start_times();

	ActivityTimeInfo &get_time_info();

private:
	AnswerActivityDetail answer_detail_;
	WaitTimer wait_timer_;
	RefreshTimer refresh_timer_;
	BroadcastTimer broad_timer_;
	ActivityStartTimer start_timer_;
	SuperviseTimer supervise_timer_;
	TopPlayerCoordTimer top_player_coord_timer_;

	ActivityTimeInfo time_info_;
	CheckStartTimer check_start_timer_;
};
typedef AnswerActivityDetail::PlayerInfoMap AnswerMap;

#define ANSWERACTIVITY_INSTANCE		AnswerActivityScene::instance()


#endif /* ANSWERACTIVITYSCENE_H_ */
