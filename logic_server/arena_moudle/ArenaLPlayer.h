/*
 * ArenaLPlayer.h
 *
 *  Created on: Aug 18, 2014
 *      Author: peizhibi
 */

#ifndef AREALPLAYER_H_
#define AREALPLAYER_H_

#include "BaseLogicPlayer.h"

class ArenaLPlayer : virtual public BaseLogicPlayer
{
public:
	ArenaLPlayer();
	virtual ~ArenaLPlayer();

	void reset_arena();
	void reset_arena_everyday();

	int fetch_arena_info();
	int fetch_guide_arena_info(ArenaRole* arena_role, const int vip_type);
	int fetch_normal_arena_info(ArenaRole* arena_role, int open_flag);

	int change_arena_skip(Message* msg);
	int fetch_arena_rank_list();
	int refresh_arena_player();

	int validate_has_area_reward(void);
	int draw_area_reward();
	int close_arena_info();

	int validate_can_area_challenge(void);
	int start_area_challenge(Message* msg);
	int start_guide_arena_challenge(int rank, ArenaRole* arena_role);
	int start_normal_arena_challenge(int rank, ArenaRole* arena_role);
	int transfer_to_arena_field(ArenaRole* area_role);

	int fetch_arena_times_money();
	int buy_arena_times_begin();
	int buy_arena_times_done(Message* msg);

	int clear_arena_cool_begin();
	int clear_arena_cool_done(Message* msg);

	int check_and_notity_arena_tips();
	int start_next_arena_timer(int cool_time);

	int check_arena_pa_event(void);

	void update_arena_exp_restore();
	void update_sword_pool_info(int num, int flag);
	void update_cornucopia_task_info();
};

#endif /* AREALPLAYER_H_ */

