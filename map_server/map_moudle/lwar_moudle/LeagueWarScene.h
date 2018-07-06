/*
 * LeagueWarScene.h
 *
 *  Created on: 2016年9月19日
 *      Author: lyw
 */

#ifndef LEAGUEWARSCENE_H_
#define LEAGUEWARSCENE_H_

#include "Scene.h"
class ProtoLWarInfo;
class Proto80400385;

class LeagueWarScene : public Scene
{
public:
	enum
	{
		TEMP_ATTACK = 0,
		TEMP_DEFENCE,
		TEMP_MIDDLE,	//中立（小怪）

		END
	};

	struct BirthRecord
	{
		MoverCoord __birth_coord;
		void reset(void);
	};
	typedef std::list<BirthRecord> AttackBirthRecordList;
	typedef std::list<BirthRecord> DefenceBirthRecordList;

	struct LWarMonster
	{
		LWarMonster();

		int sort_;			//key
		uint conf_index_;
		int owner_camp_;
		int is_boss_;
		int league_index_;
		int league_name_;

		int attack_player_point_;
		int attack_league_point_;
		int kill_player_point_;
		int kill_league_point_;
		int kill_resource_point_;

		Int64 index_;
		MoverCoord loaction_;
	};
	typedef std::map<Int64, LWarMonster> LWarMonsterMap;

	class LWarTimer: public GameTimer
	{
	public:
		LWarTimer(void);
		~LWarTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);

		void set_script_scene(LeagueWarScene *scene);

		int state_;
		LeagueWarScene* scene_;
	};

	typedef std::map<Int64, LWarLeagueInfo> LWarLeagueMap;
	typedef std::map<Int64, LWarLeagueHurt> LWarLeagueHurtMap;

public:
	LeagueWarScene();
	virtual ~LeagueWarScene();

	virtual void reset(void);
	void run_scene();
	void stop_lwar();
	void init_lwar_scene(int real_scene, int space_id);
	void init_monster_layout(uint type);

	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理
	virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value);//AI伤害处理
	void handle_boss_die(LWarMonster* lwar_monster, LWarRoleInfo* lwar_role);
	void handle_monster_die(LWarMonster* lwar_monster, LWarRoleInfo* lwar_role);
	void handle_player_kill(Int64 attacked_id, Int64 be_killed_id);

	void add_player_score(Int64 role_id, int score);
	void add_league_score(Int64 league_index, string league_name, int score);
	void add_league_hurt(Int64 league_index, string league_name, int hurt_value);

	void enter_player(LWarRoleInfo* lwar_role);
	void exit_player(LWarRoleInfo* lwar_role);
	void update_league_info(Int64 league_id, Int64 leader_id, string leader);

	void register_player(LWarRoleInfo* lwar_role);
	void unregister_player(Int64 role_id);

	int is_player_full();
	int fetch_camp_index(Int64 leauge_index);
	void set_camp_info(Int64 leauge_index, string league_name);
	void change_camp_index(Int64 role_id, int camp_index);

	int fetch_attack_resource();
	int fetch_defence_resource();
	GameAI *find_space_boss();
	int fetch_league_hurt_info(ProtoLWarInfo* lwar_detail);
	int fetch_my_league_hurt(ProtoLWarInfo* lwar_detail, Int64 league_index);
	int fetch_league_score_info(LWarLeagueInfo* all_league);

public:
	void generate_boss(bool shout = false);
	void generate_monster(uint type);
	void send_boss_camp();

	void active_add_buff();
	void active_check_boss_radius();

	int cal_point_radius(MoverCoord coord1, MoverCoord coord2);

	void serialize(LWarRoleInfo* lwar_role, LWarLeagueInfo& lwar_league);

	const Json::Value fetch_layout(uint type);
	bool is_player_enter();
	void test_set_has_enter();

	LWarMonster *find_monster_by_index(Int64 index);
	int erase_monster_map(Int64 index);

	void notify_all_player_info();
	void make_up_player_info(Int64 role_id, Proto80400385* respond); //主推战区信息
	void add_online_score(Int64 now_time);
	void sort_rank();

private:
	int real_scene_;
	int lwar_limit_; 	//最大人数
	int player_count_;	//当前人数
	int max_radius_; 	//大将活动最大半径范围
	int boss_flag_;		//boss刷新标识
	Int64 boss_id_;

	// 积分
	int defence_boss_league_score_; //守住boss获得帮派积分
	int kill_score_;	//杀人积分
	int league_extra_award_; //第一战区获胜额外帮众奖励
	int league_extra_leader_; //第一战区获胜额外帮主奖励
	int label_award_;		//称号奖励
	int first_mail_id_;		//第一战区获胜邮件ID
	int other_mail_id_;		//其他战区获胜邮件ID
	int label_mail_id_;		//称号邮件id

	int defence_resource_;	//守方资源点数
	int attack_resource_;	//攻方资源点数

	// 守方持续增加积分
	int interval_tick_; //间隔时间
	Int64 add_tick_; //积分增加时间
	int add_league_;
	int add_player_;

	Int64 defence_league_; //防守帮派id
	string league_name_; //防守帮派名称
	int has_enter_;		//是否有玩家进入

	int rank_flag_;
	ThreeObjVec league_hurt_rank_; 	//帮派伤害排行榜
	ThreeObjVec league_score_rank_;	//帮派积分排行榜

	LWarMonsterMap lwar_monster_map_;
	LWarTimer lwar_timer_;

	//怪物出生点
	AttackBirthRecordList attack_birth_list_;
	DefenceBirthRecordList defence_birth_list_;

	LWarLeagueMap lwar_league_map_;
	LWarLeagueHurtMap lwar_league_hurt_map_;

};

#endif /* LEAGUEWARSCENE_H_ */
