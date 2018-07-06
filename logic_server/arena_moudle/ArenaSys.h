/*
 * ArenaSys.h
 *
 *  Created on: Aug 15, 2014
 *      Author: peizhibi
 */

#ifndef AREASYS_H_
#define AREASYS_H_

#include "LogicStruct.h"

class ArenaSys
{
public:
	class FightingTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		ArenaSys* arena_sys_;
	};

public:
	ArenaSys(void);
	~ArenaSys();

	void init();
	void fina();

	Int64 last_robot_id(int rank);
	int reward_and_set_next_timeout(int serial);
	int fetch_next_timeout();
	int fetch_buy_money(ArenaRole* area_role);
	int fetch_clear_money(ArenaRole* area_role);

	void save_arena_data();
	void fighting_handle_timeout();
	void midnight_handle_timeout();

	int calc_challenge_result(ArenaRole* rank_role, ArenaRole* player_role);
	void rand_fighter(string name, ArenaRole* area_role);
	void set_fighter_set(ArenaRole* area_role);
	void set_first_fighter_set(ArenaRole* area_role);
	void check_and_set_fighter(ArenaRole* area_role);

	int start_challenge(ArenaRole* area_role, ArenaRole* rank_role, int guide_flag = false,int first = 0);
	int finish_challenge(Message* msg);
	int finish_guide_challenge(Int64 lose_id, AreaFightNode* fight_node);
	int finish_normal_challenge(Int64 lose_id, AreaFightNode* fight_node);
	int challenge_timeout(AreaFightNode* fight_node);

	int validate_register_arena_role(LogicPlayer* player);
	int update_arena_role_shape(Int64 role_id, Message* msg);

	void check_and_register_area(LogicPlayer* player);
	void check_and_update_shape(LogicPlayer* player);

	void notify_challenge_reward(ArenaRole* area_role, int challenge_win,
			int rank_change, int rank_differ);

	ArenaRole* arena_rank_role(int rank);
	ArenaRole* area_role(Int64 role_id);

	void insert_arena_viewer(Int64 role_id);
	void erase_arena_viewer(Int64 role_id);

	void set_arena_role_reward(Int64 role_id);

	AreaSysDetail* fetch_detail();

	void check_area_need_sort(void);

private:
	int open_flag_;

	int cool_time_;
	int need_level_;

	int area_index_;
	int max_finish_time_;

	AreaSysDetail area_detail_;
	FightingTimer fighting_timer_;

	ObjectPoolEx<AreaFightNode>* fight_node_pool_;
};

typedef Singleton<ArenaSys> 	AreaSysSingle;
#define ARENA_SYS           AreaSysSingle::instance()

#endif /* AREASYS_H_ */

