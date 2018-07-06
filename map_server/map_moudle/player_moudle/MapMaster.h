/*
 * MapMaster.h
 *
 *  Created on: Nov 22, 2013
 *      Author: peizhibi
 */

#ifndef MAPMASTER_H_
#define MAPMASTER_H_

#include "GameFighter.h"

class MapMaster : virtual public GameFighter
{
public:
	enum
	{
		TYPE_BEAST 	= 0,		//宠物
		TYPE_MAGIC 	= 1,		//法器
		TYPE_ESCORT = 2,		//护送美女
		TYPE_MAO	= 3,		//灵猫
		BEAST_TOTAL = 4,

		BEAST_SKILL_ADD_BLOOD			= 400070002,	//治愈
		BEAST_SKILL_ADD_HURT 			= 400070003,	//赐福
		BEAST_SKILL_CRAZY_ANGRY			= 400070004,	//狂怒
		BEAST_SKILL_MULTI_HURT			= 400070005,	//群爆
		MAGIC_SKILL_HUDU				= 400090002,	//护盾
		MAGIC_SKILL_HUTI				= 400090003,	//护体
		MAGIC_SKILL_LINGQIAO			= 400090004,	//灵巧
		MAGIC_SKILL_JULI				= 400090005,	//聚力
		BEAST_EQUIP_BUQU				= 400100004,	//不屈
		BEAST_SKILL_ADD_HURT_V			= 400110004,	//破体
		BEAST_SKILL_REDUCE_CRIT			= 400120001,	//减爆
		BEAST_SKILL_REDUCE_CRIT_RATE	= 400120002,	//降爆
		BEAST_SKILL_REUDCE_HURT_RATE	= 400120003,	//减伤
		BEAST_SKILL_ADD_CRIT_RATE		= 400130002,	//爆杀
		BEAST_SKILL_ADD_HURT_RATE		= 400130003,	//狂击
		TIAN_GANG_SKILL_SHIELD			= 400140001,	//霸体
		TIAN_GANG_SKILL_2				= 400140002,	//刚毅
		TIAN_GANG_SKILL_3				= 400140003,	//灵动
		TIAN_GANG_SKILL_4				= 400140004,	//复苏

		END
	};

public:
	MapMaster(void);
	virtual ~MapMaster(void);

	virtual int process_skill_post_effect(int skill_id);
	virtual double fetch_sub_skill_use_rate(FighterSkill* skill);
	virtual DoublePair fetch_reduce_crit_hurt_pair();
	virtual double fetch_reduce_hurt_rate();
	virtual double fetch_increase_crit_hurt_rate(GameFighter* fighter);
	virtual double fetch_increase_hurt_rate(GameFighter* fighter);
	virtual void trigger_left_blood_skill(Int64 attackor);

	void reset_master();
	void start_beast_skill_blood();
	void check_beast_skill_timeout();
	void trigger_beast_magic_skill(int left_percent);
	void trigger_role_shield_appear(int left_percent);
	void trigger_role_shield_disappear(int value);

	int master_exit_scene(int type = EXIT_SCENE_TRANSFER);
	int master_sign_out(int caculate_flag = true);

	void update_cur_beast_speed();
	void update_cur_beast_pk_state();

	void set_cur_beast_offline();
	void set_cur_beast_aim(Int64 aim_id);

	void start_beast_offline(const BSONObj& res);
	void set_beast_offline_db(const BSONObj &res, MapBeast *beast);
	void set_beast_offline_info(ProtoOfflineBeast* beast_info);

	int logic_upsert_beast(Message* msg);
	int update_beast_state(int flag, int type);

	int callback_cur_beast(int type);
	int callback_cur_beast(Message* msg);
	int check_and_callback_beast(int enter_flag);

	int beast_enter_scene_b(int type);
	int a_beast_enter_scene();
	int b_beast_enter_scene();
	int escort_beast_enter_scene();
	int d_beast_enter_scene();

	int beast_enter_scene(MapBeast* beast);
	int beast_exit_scene(int type);
    int beast_schedule_move(Message *msg);

	void all_beast_enter_scene(int type = -1);
	void all_beast_exit_scene(int type = -1);
	int refresh_beast_fight_prop(Message* msg);

	Int64 fetch_cur_beast_id(int type = MapMaster::TYPE_BEAST);
	MapBeast* fetch_cur_beast(int type = MapMaster::TYPE_BEAST, int active = true);
	MapBeast* find_beast(Int64 beast_id);
	MapBeast* pop_beast(int type);

	int notify_beast_call_back(int type);
	int update_cur_beast_location();

protected:
	int beast_unrecycle_;
	Int64 cur_beast_id_[BEAST_TOTAL];
	SecondTimeout beast_skill_blood_;
};

#endif /* MAPMASTER_H_ */
