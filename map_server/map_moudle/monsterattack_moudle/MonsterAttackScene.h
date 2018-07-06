/*
 * MonsterAttackScene.h
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#ifndef MONSTERATTACKSCENE_H_
#define MONSTERATTACKSCENE_H_

#include "Scene.h"

class MonsterAttackScene : public Scene
{
public:
	enum
	{
		CAMP_PLAYER = 0,
		CAMP_MONSTER,

		END
	};

	struct BirthRecord
	{
		MoverCoord __birth_coord;
		void reset(void);
	};
	typedef std::list<BirthRecord> BirthRecordList;
	typedef std::map<uint, BirthRecordList> BirthSideMap;

	struct WaveMonster {
		Int64 index_;
		int wave_id_;
		int sort_;
		int is_boss_;
		uint side_;
		MoverCoord loaction_;

		int kill_award_;
		int kill_award_rate_;
		int drop_chest_;
		int drop_num_;
		int drop_chest_rate_;

		WaveMonster();
	};
	typedef std::map<Int64, WaveMonster> WaveMonsterMap;

	class MAttackTimer: public GameTimer
	{
	public:
		MAttackTimer(void);
		~MAttackTimer(void);

		int type(void);
		int handle_timeout(const Time_Value &tv);

		void set_script_scene(MonsterAttackScene *scene);

		int state_;
		MonsterAttackScene* scene_;
	};

public:
	MonsterAttackScene();
	virtual ~MonsterAttackScene();

	virtual void reset(void);
	void run_scene();
	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理

	void stop_mattack();
	void init_mattack_scene(int real_scene, int space_id);
	void clean_monster();

	void notify_mattack_finish();

public:
	void init_monster_layout(int wave_id);
	void generate_boss(bool shout = false);
	void process_wave();
	void award_all_player();
	void generate_monster(uint side, int& is_shout);
	void generate_chest(GameAI* game_ai, int sort, int num);
	void notify_generate_buff();
	void notify_generate_boss_skill();

public:
	void register_player(MAttackInfo* mattack);
	void unregister_player(Int64 role_id);
	void enter_player(MAttackInfo* mattack);
	void exit_player(MAttackInfo* mattack);

	int fetch_player_camp();
	int fetch_monster_camp();
	int cal_award_rate(int rate);
	int fetch_is_boss_wave(int wave_id);

	WaveMonster *find_monster_by_index(Int64 index);

	void notify_mattack_info();

private:
	int real_scene_;
	int cur_wave_;
	int total_wave_;

	Int64 boss_id_;
	MoverCoord boss_point_;

	// 波数刷新时间
	int interval_tick_;  //间隔时间
	Int64 refresh_tick_; //刷新时间
	int wave_reward_;	 //波数奖励
	int is_boss_wave_;	 //是否boss关卡

	// 怪物清除时间
	int clean_flag_;
	int clean_interval_;
	Int64 clean_tick_;

	IntMap notify_label_; //通知buff记录
	MAttackTimer mattack_timer_;
	BirthRecordList birth_list_;
	BirthSideMap birth_side_map_;
	WaveMonsterMap wave_monster_map_;
};

#endif /* MONSTERATTACKSCENE_H_ */
