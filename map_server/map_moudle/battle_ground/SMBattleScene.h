/*
 * SMBattleScene.h
 *
 *  Created on: Mar 17, 2014
 *      Author: lijin
 */

#ifndef SMBATTLESCENE_H_
#define SMBATTLESCENE_H_

#include "Scene.h"

class Proto50400803;
class Proto80100501;

class SMBattleScene :public Scene
{
public:
	enum
	{
		TOTAL_CAMP		= 3,
		END
	};

	struct TemItem
	{
		TemItem();

		int sort_;			//key
		int conf_index_;
		int owner_camp_;

		Int64 index_;
		MoverCoord loaction_;

		int reward_id_;
		int attack_point_;
		int kill_point_;
		int shout_refresh_;
		int refresh_time_;
	};

	typedef std::map<int, TemItem> TemItemMap;

	class BattleTimer: public GameTimer
	{
	public:
		BattleTimer(void);
		~BattleTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);

		void set_script_scene(SMBattleScene *scene);

		int state_;
		SMBattleScene* scene_;
	};

public:
	SMBattleScene();
	virtual ~SMBattleScene();

    virtual int handle_player_hurt(MapPlayer* player, Int64 benefited_attackor, int hurt_value);
    virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value);//AI伤害处理
	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理

	void handle_center_tem_die(TemItem* tem_item, SMBattlerInfo* battler);
	void handle_camp_tem_die(TemItem* tem_item, SMBattlerInfo* battler);

	virtual void reset(void);
	void run_scene();
	void stop_battle();

	void init_sm_scene(int real_scene, int space_id);
	void handle_sm_kill(Int64 attackor, Int64 defener);
	void notify_all_sm_info();
	void shout_contine_kill(SMBattlerInfo* battler);
	void generate_center_tem(bool shout = false);
	void generate_camp_tem();
	void recycle_camp_tem();

	void register_player(SMBattlerInfo* battler);
	void unregister_player(Int64 role_id);

	void enter_player(SMBattlerInfo* battler);
	void exit_player(SMBattlerInfo* battler);

	TemItem* find_tem_by_sort(int sort);
	TemItem* find_tem_by_conf_index(int index);

	int is_finish();
	int is_player_full();
	int fetch_camp_index();
	int fetch_player_rank(Int64 role_id);

	void make_up_rank_info(Int64 role_id, Proto50400803* respond);
	void make_up_self_info(Int64 role_id, Proto80100501* respond);
	void make_up_other_info(Proto80100501* respond);

private:
	void add_online_score(Int64 now_tick);
	void add_battler_score(Int64 role_id, int score);
	void check_reward_battler(SMBattlerInfo* battler);

	void check_and_generage_scene_monster();
	void sort_rank();

private:
	int real_scene_;

	//在线积分配置
	int score_value_;
	int score_span_time_;
	Int64 score_tick_;

	int hurt_score_;		//伤害积分
	int killed_score_;		//被杀积分
	int kill_score_;		//杀人积分
	int con_kill_score_;	//连续杀人积分
	int max_con_kill_score_;//最大连杀积分
	PairObjVec point_reward_;	//积分奖励

	int actor_limit_;	//配置总人数
	BattleTimer battle_timer_;

	IntPair center_time_;
	IntPair camp_time_;

	IntMap con_kill_shout_;	//连杀播报
	TemItemMap tem_item_map_;

	int rank_flag_;
	ThreeObjVec rank_vec_;		//排行榜
	LongMap rank_map_;			//key:role id, value: rank
	LongMap attender_map_;

	SMCampInfo camp_info_[TOTAL_CAMP];
};

#endif /* SMBATTLESCENE_H_ */
