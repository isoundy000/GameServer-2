/*
 * WorldBossScene.h
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#ifndef WORLDBOSSSCENE_H_
#define WORLDBOSSSCENE_H_

#include "Scene.h"

class Proto50401023;
class Proto50401024;
class Proto50401025;

class WorldBossScene: public Scene
{
public:
	struct BossItem
	{
		BossItem();
		int sort_;			//key
		int status_;
		int kill_award_;
		int send_pocket_;
		Int64 index_;
		Int64 die_tick_;
		Int64 killer_;
		string killer_name_;
		string name_;
		int level_;
		double total_blood_;
		double cur_blood_;
		MoverCoord loaction_;
		IntMap buff_on_;	//已经触发的护盾状态
	};

	struct PlayerHurtInfo
	{
		Int64 __player_id;
		Int64 __tick;
		Int64 __league_index;
		int __amount_hurt;
		int __rank;
		std::string __player_name;
		PlayerHurtInfo();
	};
	typedef std::map<Int64, PlayerHurtInfo> PlayerHurtMap;

	struct PocketPlayer
	{
		Int64 player_id_;
		Int64 tick_;
		int is_get_;
		int get_num_;
		int get_sort_;	//第几个拿
		PocketPlayer();
	};
	typedef std::map<Int64, PocketPlayer> PocketPlayerMap;

	struct DicePlayer
	{
		Int64 player_id_;
		Int64 tick_;
		std::string name_;
		int num_;
		DicePlayer();
	};
	typedef std::map<Int64, DicePlayer> DicePlayerMap;

	struct BloodLine
	{
		double blood_;
		int shout_id_;
		bool is_shout_;
		BloodLine();
	};
	typedef std::map<int, BloodLine> BloodLineMap;

	class WBossTimer: public GameTimer
	{
	public:
		WBossTimer(void);
		~WBossTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);
		void set_script_scene(WorldBossScene *scene);

		WorldBossScene* scene_;
	};

	class DiceTimer: public GameTimer
	{
	public:
		DiceTimer(void);
		~DiceTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);
		void set_script_scene(WorldBossScene *scene);

		WorldBossScene* scene_;
	};

	class BossTimer: public GameTimer
	{
	public:
		BossTimer(void);
		~BossTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);
		void set_script_scene(WorldBossScene *scene);

		WorldBossScene* scene_;
	};

	typedef std::map<int, ThreeObjMap> ServerMailMap;

public:
	WorldBossScene();
	virtual ~WorldBossScene();

	void init_wboss_scene(int scene_id, int space_id);
	virtual void reset(void);
	void run_scene();

	void enter_player(Int64 role_id, int boss_type);
	void exit_player(Int64 role_id);
	void update_cur_player(Int64 role_id);
	bool is_player_full();

	virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor,
			int hurt_value);	//AI伤害处理
	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理

	void generate_shield(GameAI* game_ai);
//	int notify_shield_info(BasicStatus* status);
	void notify_dice_announce();
	void notify_play_dice();
	int get_dice_num(Int64 role_id, Proto50401024* respond);
	void send_dice_award();

	int get_my_rank_info(Int64 role_id, Proto50401025* respond);

	void check_can_generate();
	void generate_boss();
	void generate_red_pocket();
	int get_red_pocket_award(Int64 role_id, Proto50401023* respond);

	void sort_rank();
	void send_rank_award();
	void request_send_rank();

	const Json::Value& boss_conf_value(const char* item_name);

private:
	int rank_flag_;
	LongMap cur_player_; //场景里的玩家
	string league_name_; //排名第一的帮派名称

	time_t pocket_time_; //抢红包开始时间
	time_t dice_time_;	 //投骰子开始时间

	DicePlayerMap dice_map_; //记录投到的数字，保证数字不一致

	BossItem boss_item_;
	WBossTimer wboss_timer_;
	DiceTimer dice_timer_;
	BossTimer boss_timer_;

	ThreeObjVec player_hurt_rank_;	//玩家伤害排行榜
	PlayerHurtMap player_hurt_map_;	//玩家对BOSS的伤害
	BloodLineMap blood_map_;	 	//血量播报表
	PocketPlayerMap pocket_player_map_;	//抢红包的玩家集合
	ServerMailMap server_mail_map_;	//跨服世界boss发送邮件记录
};

#endif /* WORLDBOSSSCENE_H_ */
