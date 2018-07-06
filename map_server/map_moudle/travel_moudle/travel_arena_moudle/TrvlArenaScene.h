/*
 * TrvlArenaScene.h
 *
 *  Created on: Sep 27, 2016
 *      Author: peizhibi
 */

#ifndef TRVLARENASCENE_H_
#define TRVLARENASCENE_H_

#include "Scene.h"

class ProtoLMRole;
class TrvlArenaRole;

class TrvlArenaScene : public Scene
{
public:
	enum
	{
		PREP = 0,	//准备
		FIGHTING,	//战斗
		FINISH,		//结束

		TOTAL_ROLE = 2,
		END
	};

	class ArenaTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		int state_;
		TrvlArenaScene* scene_;
	};

public:
	TrvlArenaScene();
	virtual ~TrvlArenaScene();

	virtual void reset();
	virtual int is_fighting_time();

public:
	void init_arena_scene(int space_id, FourObj& obj);
	void notify_arena_fight(Int64 role = 0);
	void check_and_handle_offline();
	void make_up_role_info(ProtoLMRole* role, int index);

	void handle_prep_timeout();
	void handle_fighting_timeout();
	void handle_finish_timeout();

	void handle_arena_finish(Int64 lose_id);
	void handle_arena_kill(Int64 lose_id);
	void handle_arena_kill_robot(Int64 lose_id);
	void handle_arena_kill_rivial(Int64 lose_id);
	void handle_arena_reward(TrvlArenaRole* self, TrvlArenaRole* rival, int flag);

	int left_time();

	Int64 fetch_win(Int64 lose_id);
	Int64 fetch_timeout_lose();

private:
	int role_type_;
	int prep_time_;
	int left_time_;
	int finish_flag_;
	Int64 role_id_[TOTAL_ROLE];
	ArenaTimer arena_timer_;
};

#endif /* TRVLARENASCENE_H_ */
