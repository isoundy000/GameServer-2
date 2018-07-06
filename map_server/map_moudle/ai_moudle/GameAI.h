/*
 * GameAI.h
 *
 * Created on: 2013-04-02 11:17
 *     Author: lyz
 */

#ifndef _GAMEAI_H_
#define _GAMEAI_H_

#include "AIStruct.h"
#include "BehaviorNode.h"
#include "AutoMapFighter.h"

class GameAI : public AutoMapFighter
{
public:
	struct GatherLimitTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

	    GameAI* game_ai_;
	};

public:
	GameAI(void);
	virtual ~GameAI(void);

	const Json::Value& monster_conf();
	const Json::Value& prop_monster();

	virtual int auto_action_timeout(const Time_Value& now_time);
	virtual int fetch_default_step();
	virtual Int64 league_id(void);
	virtual int fighter_sort();
	virtual const char* name();
	virtual void push_attack_interval();
	virtual void trigger_left_blood_skill(Int64 attackor);
	virtual int fetch_mover_volume();
	virtual int fetch_max_chase_distance();
	virtual int fetch_auto_skill_id();
	// 自动扣除周围敌人的血
	virtual int auto_modify_around_blood(const Time_Value &now_time);

	int fetch_left_blood_skill();

	void trigger_ice_inside_skill(FighterSkill* skill);
	void trigger_jian_drop_skill(FighterSkill* skill, Int64 attackor);

	int ai_sort();
	Int64 ai_id();
    double hit_stiff_last(void);

	AIDetail &ai_detail(void);
    AIDetail::PlayerHurtMap &hurt_map(void);

    bool is_boss(void);
    bool is_gather_goods(void);
    bool has_back_action(void);

    void clean_when_recycle(void);

	virtual int gate_sid(void);
	virtual int client_sid(void);

	virtual void reset(void);
	virtual int team_id(void);
	virtual int64_t entity_id(void);
	virtual MoverCoord &birth_coord(void);

	bool is_config_scene(void);
	bool is_config_monster(void);
	const Json::Value &fetch_prop_config(void);

	virtual void set_chase_index(const int index);
	virtual int chase_index(void);
	virtual MoverCoord chase_coord(const MoverCoord src_coord);

	virtual void set_patrol_index(const int index);
	virtual int patrol_index(void);

    void set_moveto_action_coord(const MoverCoord coord);
    MoverCoord &moveto_action_coord(void);

    virtual void set_aim_object(Int64 aim_id, const bool is_from_group = false);
	virtual int enter_scene(const int type = ENTER_SCENE_LOGIN);
	virtual int exit_scene(const int type = EXIT_SCENE_LOGOUT);

	// AI data in scene layout
	int sign_in_with_scene(int monster_sort, const MoverCoord &coord, Scene* scene);
	// AI data in prop_monster.json
	int sign_in_with_sort(int monster_sort, const MoverCoord &coord, Scene* scene);
	virtual int sign_out(void);

	virtual int make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate);

	virtual int die_process(const int64_t fighter_id = 0);
	virtual int copy_fighter_property(GameFighter *fighter);

	virtual int modify_blood_by_fight(const double inc_val, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);

	int modify_blood_mode_a(const double inc_val, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);
	int modify_blood_mode_b(const double inc_val, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);

    virtual bool is_movable_path_coord(const DynamicMoverCoord &coord);
    virtual int process_skill_post_launch(int skill_id, int source);

    virtual int auto_fighter_attack(void);
    virtual int schedule_move_fighter(void);
    int schedule_move_fighter_no_dynamic_mpt(void);

    virtual int gather_limit_collect_begin(Int64 role_id, int time = 10);
    virtual int gather_limit_collect_done(Int64 role_id, int result, ItemObj &gather_item);
    int gather_limit_timeout();

public:
	virtual int produce_drop_package();
	int produce_drop_package_a(const Json::Value& monster_drop_item, LongSet &coord_set, int *drop_size);
	int produce_drop_package_b();
	int fetch_drop_player_map(int drop_type, LongMap& player_map, const Json::Value &monster_drop_item);
	virtual Int64 fetch_killed_id(void);

	void set_caller(const Int64 id);
	void set_leader(const Int64 id);
	Int64 caller(void);
	Int64 leader(void);

	void set_fighter_sort();
	void set_ai_back(bool back_flag);
	void set_ai_patrol(bool patrol_flag);
	void ai_die_reward(Int64 benefited_attackor);

    int update_task_monster(MapPlayerEx* player);
    int update_player_exp(MapPlayerEx* player);	//经验
    int update_player_angry(MapPlayerEx* player); // 增加怒气
    int reward_exp_mode_a(MapPlayerEx* player);	//最后一刀
    int reward_exp_mode_b(MapPlayerEx* player);	//共享

	bool test_ai_flag(int condition_flag);
	bool validate_ai_flag(int condition_flag);

	bool ai_is_back();
	bool ai_is_timeout();
	bool ai_is_area_recycle();

	std::string tree_name();
	std::string born_type();

	virtual const Json::Value& fetch_layout_item();
	virtual void update_fight_state(const Int64 fighter_id, const int state,
			const Time_Value &last_tick = Time_Value(5));
    virtual int refresh_fight_state(const Time_Value &nowtime);
    void ai_fight_state_to_spd(const bool new_is_fight);

	virtual Time_Value &auto_attack_tick(const std::string &field);

	void set_layout_index(int layout_index);
    int layout_index(void);

	void set_auto_attack_tick(const std::string &field, const Time_Value &tick);

	int fetch_left_alive_time();

	virtual int find_avoid_buff(const int status);

	virtual Int64 find_max_hurt_fighter(void);
	virtual Int64 aim_select(void);
    int fetch_hurt_range_role(LongVec &role_set, const int max_begin, const int max_end);

    virtual void shout_drop_goods(const PackageItem *drop_item);

    virtual void generate_gift_box(void);
	void set_ai_falg(int condition_flag);
	void reset_ai_flag(int condition_flag);
	void reset_ai_flag();

	int update_labour_task_info(Int64 benefited_attackor);

protected:
	virtual int recycle_self(void);

	int execute_ai_tree();
	int add_boss_avoid_buff();
	int init_left_blood_skill();

	virtual void base_sign_in(int monster_sort, const MoverCoord &coord, Scene* scene);
	virtual void init_base_property(void);

	virtual void init_prop_monster(void);
	virtual void init_scene_monster(void);

	virtual void init_ai_tree(const Json::Value& prop_monster);
	virtual void init_flag_property(const Json::Value& prop_monster);
	virtual void init_skill_property(const Json::Value& prop_monster);
	virtual void init_safe_area(const Json::Value &prop_monster);
    virtual void init_group_id(const Json::Value &prop_monster);

    virtual void fetch_drop_money_amount(int *money_type, int *money_amount, const Json::Value &monster_drop);
	virtual void produce_drop_money(const Json::Value &monster_drop, LongMap& player_map, LongSet &coord_set, int *drop_size);
    virtual void produce_drop_money(const Json::Value &monster_drop, LongMap& player_map, 
            const int money_type, const int amount, LongSet &coord_set, int *drop_size);
	virtual void produce_drop_items(const Json::Value &monster_drop, LongMap& player_map, LongSet &coord_set, int *drop_size);
	virtual void produce_drop_items(const Json::Value &monster_drop, LongMap& player_map,
			const ItemObj& item_obj, LongSet &coord_set, int *drop_size);

	void produce_drop_items_b(Scene* scene, Int64 role, const ItemObj& item_obj);
	void update_hurt_map(const Int64 fighter_id, const int value);
	void send_festival_hurt_info();
	void send_festival_kill_info();

protected:
	AIDetail ai_detail_;
	GatherLimitTimer gather_timer_;

	Int64 ai_id_;
	std::string tree_name_;
	BehaviorNode* behavior_tree_;

	bool is_has_drop_;
};

#endif //_GAMEAI_H_
