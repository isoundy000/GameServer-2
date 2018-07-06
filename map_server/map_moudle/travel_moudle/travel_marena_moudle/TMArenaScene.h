/*
 * TMArenaScene.h
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#ifndef TMARENASCENE_H_
#define TMARENASCENE_H_

#include "Scene.h"

class Proto80100509;

struct TMSideInfo
{
	TMSideInfo();
	void reset();

	int side_;
	int die_times_;
	LongVec role_vec_;
	LongMap role_map_;
};

class TMArenaScene : public Scene
{
public:
	enum
	{
		PREP = 0,	//准备
		FIGHTING,	//战斗
		FINISH,		//结束

		TOTAL_SIDE = 2,
		END
	};

	class ArenaTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		int state_;
		TMArenaScene* scene_;
	};

public:
	TMArenaScene();
	virtual ~TMArenaScene();
	virtual void reset(void);
	virtual int fetch_relive_protected_time(Int64 role);
	virtual int fetch_enter_buff(IntVec& buff_vec, Int64 role);

	void init_tmarena_scene(const IntPair& key, const LongVec& attend_vec);
	void notify_find_rivial();
	void notify_transfer_enter();
	void notify_tmarena_fight_info(int state);

	void start_prep_time();
	void handle_prep_timeout();
	void handle_fighting_timeout(int force = false);
	void check_and_handle_offline();
	void handle_marena_finish(int win_side);
	void handle_marena_result(int win_side);
	void handle_marena_result(int flag, TMSideInfo* side_info);
	void notify_marena_result(Proto80100509* respond, TMSideInfo* side_info);
	void handle_finish_timeout();
	void handle_marena_player_die(Int64 killer_id, Int64 killed_id);

	int fetch_camp_id(Int64 role);
	int fetch_win_side();
	int is_player_online(int side);
	MoverCoord fetch_enter_pos(Int64 role);

private:
	int prep_time_;
	int fight_time_;
	int finish_flag_;
	int max_die_times_;
	int protected_time_[TOTAL_SIDE];

	TMSideInfo first_;
	TMSideInfo second_;

	MoverCoord coord_[TOTAL_SIDE];
	PairObjVec rank_vec_;
	ArenaTimer arena_timer_;
};

#endif /* TMARENASCENE_H_ */
