/*
 * LeagueRegionFightScene.h
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#ifndef LEAGUEREGIONFIGHTSCENE_H_
#define LEAGUEREGIONFIGHTSCENE_H_

#include "Scene.h"

class LRFLeagueInfo;

struct LRFPlayer
{
	Int64 playerid; 				//玩家id
	Int64 league_id;

	int flag_reward_;
	IntMap history_reward_;

	int camp_id_;
	int fight_score; 				//战斗积分
	int resource_score;             //玩家资源点积分

	int hickty_index_;
	int hickty_id_;                     //当前器械
	int attack_;
	int defence_;
	int health_;
	int force_hurt_;

	LRFPlayer();
	void reset();
	void set_hickty_info(const Json::Value& weapon_conf);
};

struct RegionWarMonster
{
	RegionWarMonster();

	int sort_;
	int conf_index_;
	int owner_camp_;
	int position_index;

	Int64 index;
	int kill_player_point_;
	int kill_league_point_;
	int kill_player_res_score;
	int attack_player_point;
	int attack_league_point;
	IntVec flag_per_ten_add_;
};

class LeagueRegionFightScene: public Scene
{
public:
	class RegionWarTimer : public GameTimer
	{
	public:
		RegionWarTimer();

		int type();
		int handle_timeout(const Time_Value &tv);

		void set_script_scene(LeagueRegionFightScene *scene);

		int state_;
		LeagueRegionFightScene* scene_;
	};

	//参与战斗所有玩家
	typedef std::map<Int64, LRFPlayer> LRFPlayerMap;
	typedef std::map<Int64, RegionWarMonster> WarMonsterMap;

public:
	void reset();

	void init_region_scene(int real_scene, LRFLeagueInfo& first, LRFLeagueInfo& second);
	void init_flags(int type, int camp_id);
	void generate_tower(int type, int camp_id);
	void generate_res(int type, int camp_id);
	void generate_res_exchange(int type);

	void run_scene();
	void stop_region_war();

	virtual int is_fighting_time();
	virtual int is_validate_attack_player(GameFighter* attacker, GameFighter* target);
	virtual int is_validate_attack_monster(GameFighter* attacker, GameFighter* target);
	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理
	virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value);//AI伤害处理
	virtual int modify_ai_hurt_value(GameAI* game_ai, int src_value, Int64 attackor_id);

	void handle_league_flag_die(GameAI* game_ai, Int64 benefited_attackor);	//旗帜被杀
	void handle_league_tower_die(GameAI* game_ai, Int64 benefited_attackor);
	void handle_league_res_die(GameAI* game_ai, Int64 benefited_attackor);
	void check_handle_region_timeout();	//超时结束
	void handle_region_finish(Int64 win_league, int timeout);
	void handle_region_flag_reward(Int64 win_league, int timeout);
	void handle_recycle_region();
	void handle_add_all_leaguer(const RegionWarMonster& monster, Int64 role);
	void update_player_hickty_level(Int64 league_id);

	void shout_lrf_win_by_flag(Int64 win_league);
	void shout_lrf_tower(int die_camp, int amount);
	void shout_lrf_center(int win_camp);

	void handle_player_kill(Int64 attacked_id, Int64 be_killed_id);
	void add_lrf_player_score(LRFPlayer& lrf_player, int score);
	void add_lrf_league_score(Int64 league_id, int score);
	void check_reward_lrf_player(LRFPlayer& lrf_player);
	void restore_center_flag_blood(int camp);

	void enter_player(MapPlayer* player);
	void exit_player();

	void add_lrf_info_by_flag();
	void add_lrf_info_by_flag(const RegionWarMonster& monster);

	void notify_lrf_fight_info();
	void notify_lrf_fight_info(GameMover* mover, int left_time);
	void notify_lrf_finish_info(Int64 win_league, int timeout);
	void set_lrf_result_info(Int64 win_league);
	void set_lrf_result_reward(LRFLeagueInfo* league);

	LRFPlayer* fetch_lrf_player(Int64 id);
	LRFLeagueInfo* fetch_lrf_league(Int64 id);

	int fetch_camp_id(Int64 league_id);
	int fetch_lrf_weapon_level(Int64 league_id);

	IntPair fetch_lrf_center_flag(LRFLeagueInfo* lrf_league);
	IntPair fetch_lrf_league_flag(LRFLeagueInfo* lrf_league);
	IntPair fetch_fight_score_reward(LRFPlayer& lrf_player);

	int is_lrf_finish();
	int is_in_center_relive(int camp_id);
	int validate_sub_resource(Int64 role, int need_resource);
	int lrf_modify_player_blood(LRFPlayer* self, GameFighter* player);

public:
	static int caculate_lrf_hurt(int attack, int defence);

private:
	int real_scene;
	int total_tower_;
	int get_flag_reward_;
	int init_resource_;
	int kill_player_score_[2];
	PairObjVec score_reward_;

	int ten_second_;
	int finish_flag_;
	Int64 center_monster_;

	LRFLeagueInfo* win_;
	LRFLeagueInfo* lose_;

	int first_rank_;
	int second_rank_;
	LRFLeagueInfo* first_info_;
	LRFLeagueInfo* second_info_;

	LRFPlayerMap lrf_player_map_;
	RegionWarTimer region_timer;

	WarMonsterMap region_monster_map_;	//帮旗
	WarMonsterMap tower_monster_map_;	//箭塔
	WarMonsterMap res_monster_map_;		//资源怪
	WarMonsterMap exchange_point_map_;	//兑换点
};

#endif /* LEAGUEREGIONFIGHTSCENE_H_ */
