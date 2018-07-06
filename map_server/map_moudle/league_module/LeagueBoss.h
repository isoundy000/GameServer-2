/*
 * LeagueBoss.h
 *
 *  Created on: 2016年8月5日
 *      Author: lyw
 */

#ifndef LEAGUEBOSS_H_
#define LEAGUEBOSS_H_

#include "Scene.h"

class LeagueBoss : public Scene
{
public:
	struct LBossItem
	{
		LBossItem();

		int id_;			//boss表索引
		int sort_;			//key
		int status_;
		int summon_type_;
		int flag_level_;
		int direct_award_;
		Int64 index_;
		Int64 league_index_;
		double total_blood_;
		MoverCoord loaction_;
		IntMap buff_on_;	//已经触发的护盾状态
	};

	struct PlayerHurtInfo
	{
		Int64 __player_id;
		Int64 __tick;
		int __amount_hurt;
		int __rank;
		std::string __player_name;
		PlayerHurtInfo();
	};
	typedef std::map<Int64, PlayerHurtInfo> PlayerHurtMap;

	class BOSSTimer : public GameTimer
	{
	public:
		BOSSTimer(void);
		~BOSSTimer(void);

		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void set_script_scene(LeagueBoss *scene);

		LeagueBoss* scene_;
	};

public:
	LeagueBoss();
	virtual ~LeagueBoss();

	virtual void reset();
	int reset_scene_info();
	void init_league_boss_scene(int flag_lvl);
	void run_boss_scene();
	void refresh_scene();

	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);
	virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value);

	void erase_ai(Int64 ai_id);

	void enter_player(Int64 role_id);
	MoverCoord fetch_enter_pos();

	int fetch_flag_level(Message* msg);
	void summon_league_boss(Message* msg);
	int generate_boss();
	void generate_shield(GameAI* game_ai);
//	int notify_shield_info(BasicStatus* status);

	//掉落宝箱，发送直接奖励等等
	int league_boss_finish(GameAI* game_ai);

	//每10s增加一次场景内人物经验
	int fetch_lboss_status();
	void notify_add_player_exp();
	void send_exp_and_refresh_tick();
	void sort_rank();

private:
	int rank_flag_;
	int refresh_flag_;
	int exp_interval_;
	Int64 refresh_tick_;
	Int64 exp_tick_;

	LBossItem lboss_item_;
	PlayerHurtMap player_hurt_map_;	// 玩家对BOSS的伤害
	ThreeObjVec player_hurt_rank_;
	LongMap player_exp_map_;
	BOSSTimer boss_timer_;
	LongMap chest_map_;
};

#endif /* LEAGUEBOSS_H_ */
