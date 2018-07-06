/*
 * GameFighter.h
 *
 * Created on: 2013-03-26 15:41
 *     Author: lyz
 */

#ifndef _GAMEFIGHTER_H_
#define _GAMEFIGHTER_H_

#include "GameMover.h"
#include "GameStatus.h"

class GameFighter : virtual public GameMover,
    public GameStatus
{
public:
	enum
	{
		TOTAL_LOOP = 2,
		END
	};

    typedef HashMap<int, Time_Value, NULL_MUTEX> PrepareTickMap;
    typedef HashMap<int, HistoryStatus *, NULL_MUTEX> HistoryStatusMap;

    typedef Heap<DelaySkillInfo, DelaySkillCmp> SkillLaunchQueue;
    typedef std::map<int, SkillFreezeDetail> SkillFreezeMap;	// key: 触发冻结的技能

    class FighterTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		GameFighter* fighter_;
	};

public:
    GameFighter(void);
    virtual ~GameFighter(void);
    
    virtual int time_up(const Time_Value &nowtime);
    virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
    virtual int fetch_attack_distance();
    virtual void set_cur_toward(const MoverCoord& coord);

    void reset_fighter(void);

    bool is_blood_full();
    bool is_magic_full();

    int cur_blood();
	int total_recover_blood();
    int fetch_check_distance();
    int fetch_blood_differ();

    double cur_blood_percent(int percent = 1);
    double fetch_max_blood(double percent);
    double fetch_cur_blood(double percent);

    void fighter_restore_all(int fight_tips = FIGHT_TIPS_SYSTEM_AUTO,
    		int restore_type = 0, double percent = 1.0);
    void fighter_check_and_restore_all();
    void shrink_cur_blood();
    void init_klv_property();

    FightDetail &fight_detail(void);
    CurrentSkill &fight_cur_skill(void);
    virtual int64_t fighter_id(void);
    virtual int fighter_id_low(void);
    virtual int fighter_id_high(void);
    virtual int team_id(void) = 0;
    virtual int fight_career(void);
    virtual int fight_sex(void);
    virtual int fetch_name_color();
    virtual bool is_attack_by_id(Int64 role_id);
    virtual int fighter_sort();
    virtual int is_lrf_change_mode();

    virtual void set_camp_id(const int camp);
    virtual int camp_id(void);
    virtual int level(void);
    virtual Int64 league_id(void);

    int skill_id_for_step(void);
    int skill_step(void);
    Time_Value &skill_step_tick(void);

    void refresh_skill_step(int skill_step);

    virtual bool is_vip();
    virtual int vip_type();

    virtual int send_to_logic_thread(int recogn);
    virtual int send_to_logic_thread(Message& msg);
    virtual int send_to_other_scene(const int scene_id, Message& msg);

    int find_fighter(Int64 fighter_id, GameFighter *&fighter);
    MapPlayerEx* find_player(Int64 fighter_id);	//查找在线玩家
    MapPlayerEx* find_player_with_offline(Int64 fighter_id);	//查找包括镜像的玩家
    MapPlayerEx* fetch_benefited_player(Int64 attackor_id);
    Int64 fetch_benefited_attackor_id(Int64 attackor_id);

    GameAI* self_ai();
    MapPlayerEx* self_player();
    MapPlayerEx* dynamic_player();

    void update_pk_tick(void);
    virtual int set_pk_state(const int state);

    bool check_pk_state(const int state);
    bool is_pk_state(void);
    bool is_plenary_pk(void);
    bool is_team_pk(void);
    bool is_league_pk(void);
    bool is_same_pk_state(int state);

    bool is_fight_state(void);
    bool is_fight_active(void);		// 主动攻击
    bool is_fight_passive(void);	// 被动攻击
    bool is_gather_state(void);		// 是否采集状态

    bool is_death(void);
    bool is_jumping(void);
    bool is_skill_frozen(int skill_id);

    int fetch_fight_state();

    virtual int gather_state_end(); // 终止采集
    virtual void update_fight_state(const Int64 fighter_id, const int state, const Time_Value &last_tick = Time_Value(5));
    virtual void update_active_fight_state(const Int64 target_id);

    virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK, const int64_t attackor = 0, const int skill_id = 0);
    virtual int modify_blood_by_relive(const double value);
    virtual int modify_blood_by_levelup(const double value, const int fight_tips = FIGHT_TIPS_BLANK);
    virtual int modify_magic_by_notify(const double value, const int fight_tips = FIGHT_TIPS_BLANK);
    virtual void trigger_left_blood_skill(Int64 attackor);

    int insert_passive_skill_queue(FighterSkill *skill);
    virtual int insert_skill(int skill_id, int level = 1, int notify = true);
    virtual int insert_skill_i(int skill_id, int level = 1);
    virtual int update_skill_level(FighterSkill *skill, const int target_level);
    int remove_skill(int skill_id, int record = false);
    int remove_passive_skill(FighterSkill* skill);
    int remove_skill_buff(FighterSkill* skill);
    int is_have_skill(int skill_id);
    int find_skill(int skill_id, FighterSkill *&skill);

    virtual int prepare_fight_skill(Message *msg);
    virtual int launch_fight_skill(Message *msg, const bool is_check_prepare = true);
    virtual int direct_luanch_skill_effect(FighterSkill* skill, GameFighter* defender = NULL);

    void set_client_target_id(Int64 target_id);
    void add_history_target_id(Int64 target_id);
    void set_current_skill_radian(double radian);
    void use_skill_magic(const Json::Value &skill_json);
    void set_start_jump(const Json::Value &effect);
    void set_finish_jump(BasicStatus *status, int is_timeout);
    void set_current_skill_display(const Json::Value &skill_json);
    void adjust_player_coord(Proto10400201* request);	//校正服务端和客户端位置不一致
    void adjust_current_skill_coord(int aoeType, Proto10400201* request);

    virtual GameFighter *fetch_defender(void);
    virtual GameFighter *fetch_self_owner(void);
    virtual CurrentSkill &current_skill(void);

    virtual int modify_element_experience(const int value, const SerialObj &serial_obj);
    virtual int force_total_i(void);

    virtual double fetch_beast_hurt_rate();
    virtual double fetch_beast_hurt_value();
    virtual double fetch_reduce_hurt_rate();
    virtual double fetch_reduce_hurt_value();
    virtual DoublePair fetch_reduce_crit_hurt_pair();
    virtual double fetch_increase_hurt_rate(GameFighter* fighter);
    virtual double fetch_increase_hurt_value();
    virtual double fetch_increase_crit_hurt_rate(GameFighter* fighter);
    virtual double fetch_increase_crit_hurt_value();
    virtual double fetch_sub_skill_use_rate(FighterSkill* skill);	//扣除使用概率

    double fetch_passive_skill_use_rate(FighterSkill* skill);	//被动技能使用概率

    virtual int make_up_fight_update_info(Message *msg, const int type, const int value = 0, const int64_t attackor = 0, const int skill_id = 0,
    		const int tips1 = 0, const int tips2 = 0, const int tips3 = 0, const int tips4 = 0, const int64_t experience = 0.0);
    virtual int notify_fight_update(const int type, int value = 0, const int64_t attackor = 0, const int skill_id = 0,
    		const int tips1 = 0, const int tips2 = 0, const int tips3 = 0, const int tips4 = 0, const int64_t experience = 0.0);

    int update_fighter_jump_value(int value, int skill_id = 0);
    int update_fighter_speed(int type, double value, int offset = BasicElement::BASIC);
    virtual int notify_update_speed(void);
    virtual int notify_fighter_stay(void);
    virtual int notify_fighter_exit_stay(void);

    virtual int update_fight_property(int type = -1);
    virtual int notify_update_player_info(int update_type, Int64 value = 0);

    virtual int validate_fighter_movable(void);
    virtual int validate_movable(const MoverCoord &step);

    bool validate_online_player();
    bool validate_left_blood(int need_left_blood);
    bool validate_enough_magic(int need_magic);

    virtual int mutual_skill(void);
    virtual void stop_guide_skill(void);
    virtual void stop_guide_skill_by_skill(const int skill_id);
    void stop_last_skill(const int status);

    int alive_summon_ai_size(void);
    int insert_summon_ai(const Int64 ai_id);
    int remove_summon_ai(const Int64 ai_id);

    void recycle_all_skill_map(void);

    virtual int find_avoid_buff(const int status);
    virtual GameFighter* fetch_hurt_figher();

    double avoid_rate_in_hurt(GameFighter *defender);	// 计算闪避率
    double hit_rate_in_hurt(GameFighter *defender);		// 计算命中率

    bool is_jump_avoid_hurt();
    bool is_hit_in_hurt(GameFighter *defender);

    double attack_in_hurt(void);		// 计算攻击力
    double defence_in_hurt(void);		// 计算防御力
    double crit_power_in_hurt(GameFighter *defender);	// 计算暴击倍率
    bool is_crit_in_hurt(GameFighter *defender);
    int hurt(double value, double percent = 0.0);
    int cure(double value, double percent = 0.0);

    const Json::Value &cur_skill_detail_conf();
    const Json::Value &detail_conf(const int skill_id, const int skill_level = 1);

    virtual int team_notify_teamer_blood(int type = 0);
	virtual MoverCoord fetch_direct_point(const MoverCoord &src_coord,
			const MoverCoord &reference_point, const int range,
			const bool is_ignore_middle = false);

	virtual int clean_status(int status_id);

	int blood_max_set(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int blood_max_add(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int blood_max_reduce(const double value, const int offset = BasicElement::BASIC);
	int blood_max_multi_set(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int blood_max_multi_add(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int blood_max_multi_reduce(const double value, const int offset = BasicElement::BASIC);

	int magic_max_set(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int magic_max_add(const double value, const int offset = BasicElement::BASIC);
	int magic_max_reduce(const double value, const int offset = BasicElement::BASIC);
	int magic_max_multi_set(const double value, const int offset = BasicElement::BASIC, const int enter_type = 0);
	int magic_max_multi_add(const double value, const int offset = BasicElement::BASIC);
	int magic_max_multi_reduce(const double value, const int offset = BasicElement::BASIC);
    
    int base_skill_id();
    int first_skill_id();
    int launch_shield_hurt(BasicStatus *status);
    int add_and_notify_angry_value(int value = 1);

    void remove_die_protect(void);
    void notify_launch_skill(FighterSkill* fighter_skill,
    		const SkillExtInfo& ext_info = SkillExtInfo());
    void call_protected_process_push_away(int range);

    int direct_hurt_by_defender_max_blood(GameFighter* defender, double percent);
    int direct_hurt_by_defender_max_blood(GameFighter* defender, const DoublePair& pair);

protected:
    virtual int modify_blood(const int value, const int64_t attackor = 0);
    virtual int modify_magic(const int value);

    virtual int validate_prepare_attack_target(void);
    virtual int validate_launch_attack_target(void);
    virtual int validate_attack_target(void);
    virtual int is_in_safe_area();
    virtual int validate_safe_area(GameFighter *defender);
    int validate_attack_to_defender(GameFighter *defender, int skill_id = 0);
    int validate_transfer_skill(FighterSkill* skill);

    virtual int validate_skill_usage(void);
    virtual int validate_trig_skill_effect(const Json::Value &effect);
    int validate_skill_attack_type(int skill_id, GameEnum::SkillTargetFlagSet &target_flag_set);

    int refresh_skill_info(int skill_id = 0);
    int notify_launch_skill(int flag = false);
    virtual int notify_launch_effect_ai_skill(const Json::Value &skill_json); // effect_ai 的技能效果，只播放特效

    // 对defender产生效果的effect
    int process_skill_effect_to_defender(void);
    int trigger_skill_effect_to_defender(const Json::Value &effect);
    // 对自己或对空地生火产生效果的effect
    int process_skill_effect_no_defender(void);
    int trigger_skill_effect_no_defender(const Json::Value &effect, int aoe_type = 0);

    // 技能效果结束调用
    virtual int process_skill_post_effect(int skill_id);
    // 技能启动结束调用
    virtual int process_skill_post_launch(int skill_id, int source = 0);

    virtual int notify_fight_state(const Int64 fighter_id);
    // 刷新战斗状态
    virtual int refresh_fight_state(const Time_Value &nowtime);
    // 刷新回血
    virtual int refresh_blood_recover(const Time_Value &now_time);
    // 自动扣除周围敌人的血
    virtual int auto_modify_around_blood(const Time_Value &now_time);

    virtual int level_upgrade(void);
    virtual int die_process(const int64_t fighter_id = 0);

    virtual int process_launch_skill(const Time_Value &nowtime);
    // 处理被攻击者的反弹效果
    virtual int process_defender_rebound(const double cur_hurt, const double total_hurt);

    // 自动恢复法
    virtual int recovert_magic(const Time_Value &nowtime);
    virtual int recovert_blood(const Time_Value &nowtime);

    virtual int make_up_skill_target(MoverMap &fighter_map, const int skill_id, const MoverCoord &skill_coord, const double angle = 90.0);
    virtual int make_up_skill_target(FighterSkill* skill, const MoverCoord &skill_coord, Proto10400201* request);

    virtual int generate_area_hurt_monster(const int sort, MoverCoord &location, const double angle);
    virtual int summon_monster(const Json::Value &effect);

    virtual int process_pull_near(void);
    virtual int process_push_away(const int range);
    virtual int process_self_forward(const int range);
    virtual int process_assault(const int range, const Json::Value &effect, GameFighter *calc_fighter=0);
    virtual int process_self_backward(const int range);
    virtual int notify_fighter_teleport(const Int64 attackor_id, const int skill_id, const MoverCoord &target_coord);

    int process_launch_mutual_skill(const Time_Value &nowtime);
    int notify_cancel_loop_skill(const int skill_id, const int stop_by_buff = 0);
    int process_launch_loop_skill(const Time_Value &nowtime);
    int process_launch_loop_skill(LoopSkillDetail& loop_skill, const Time_Value &nowtime);

    virtual int process_area_hurt(const Json::Value& effect, int aoe_type);

    /* 技能冻结(使用一个技能时冻结指定技能) */
    virtual int process_skill_freeze(const Json::Value& skill_json);
    virtual int check_skill_freeze_timeout(const Time_Value &nowtime);

    /* 处理技能提示播放 */
    virtual int process_skill_note(int skill_id, const Json::Value& detail_json);
    virtual int process_monster_talk(int skill_id , int mover_id);
    virtual int process_player_force_up(int force);
    virtual void insert_attack_me(Int64 role_id);

    int correct_add_blood_by_status(const int add_value);

    virtual int notify_exit_scene_cancel_info(int type, int scene_id = 0);

    int process_skill_launch_way(FighterSkill *skill, std::auto_ptr<Proto10400202> launch_req);

    int process_rate_use_auto_skill_in_fight(FighterSkill* src_skill);
    int process_launch_rate_use_skill(FighterSkill *skill);
    int notify_fight_passive_skill(void);
    int notify_fight_passive_skill_talisman();
    int process_rate_use_skill_by_hurt(GameFighter *attacker);
    int process_launch_use_skill_by_hurt(GameFighter *attacker, FighterSkill *skill);

    int monster_hurt_reduce(GameFighter *defender, double &real_hurt);
    int check_and_die_away_safe_range(GameFighter *defender);

    int check_enter_stiff(const int skill_id, const int skill_level);
    int process_magic_reduce_hurt(GameFighter *attackor, const double value, const double percent);

    int process_insert_shield_effect(GameFighter *defender, const Json::Value &effect);
    int process_insert_life_shield_effect(GameFighter *defender, const Json::Value &effect);
    double calc_shield_reduce_value(double inc_value, BasicStatus* status);
    double calc_shield_reduce_value_b(double inc_value, BasicStatus* status);
    double calc_shield_reduce_value_c(double inc_value, BasicStatus* status, Int64 attackor);
    int hurt_no_defence(GameFighter *defender, const double hurt);

    int process_insert_percy_effect(GameFighter *defender, const Json::Value &effect);
    int process_insert_flash_avoid_effect(GameFighter *defender, const Json::Value &effect);
    int calc_rebound_hurt_when_avoid(GameFighter *attacker);

//    int notify_
    virtual int insert_master_status(const int status,
            const double interval, const double last, const int accumulate_times = 0,
    		const double val1 = 0.0, const double val2 = 0.0, const double val3 = 0.0,
            const double val4 = 0.0, const double val5 = 0.0);

    int process_generate_stone_player(GameFighter *effect_defender, const Json::Value &effect);

protected:
    FightDetail fight_detail_;
    FighterTimer fighter_timer_;

    CurrentSkill fight_skill_;			//当前技能
    PrepareTickMap prepare_tick_map_;
    Time_Value fight_frequency_tick_;	// 攻击限速
    int fight_times_;

    SkillLaunchQueue skill_delay_launch_queue_;

    Int64 magic_recovert_tick_;
    Int64 blood_recovert_tick_;

    LoopSkillDetail mutual_skill_;
    int loop_index_;
    LoopSkillDetail loop_skill_[TOTAL_LOOP];

    IntSet	frozen_skills_;	// 所有当前被冻结的技能
    SkillFreezeMap skill_freeze_map_;
};

#endif //_GAMEFIGHTER_H_
