/*
 * AutoMapFighter.h
 *
 *  Created on: Mar 11, 2014
 *      Author: peizhibi
 */

#ifndef AUTOMAPFIGHTER_H_
#define AUTOMAPFIGHTER_H_

#include "GameFighter.h"

class AutoMapFighter : virtual public GameFighter
{
public:
	class AutoActionTimer : public GameTimer
	{
	public:
	    virtual int type(void);
	    virtual int handle_timeout(const Time_Value &tv);

	    bool run_timer_;
	    AutoMapFighter* auto_fighter_;
	};

public:
	AutoMapFighter();
	virtual ~AutoMapFighter();

	virtual int auto_action_timeout(const Time_Value& now_time);
    virtual int fetch_default_step();
    virtual int fetch_attack_distance();
    virtual int fetch_select_distance();
    virtual int fetch_max_chase_distance();

    virtual void push_attack_interval();
    virtual int insert_skill_i(int skill_id, int level = 1);

public:
	void reset_auto_fighter();
	void clear_skill_queue();

	void clear_last_skill(FighterSkill* skill);
	void insert_sequence_skill(const int skill_id, const int skill_level = 1);

	void start_auto_action(double schedule_time = MAP_MONSTER_INTERVAL);
	void stop_auto_action();

	void continue_run_timer();
	void stop_run_timer();

    const MoverCoord &aim_coord(void);
    Int64 aim_object_id(void);
    Int64 last_attacker_id(void);
    Int64 self_owner_id(void);

    int group_id(void);
    int is_attack(void);
    int is_arrive_attack_distance(GameFighter* fighter);

	void set_aim_coord(const MoverCoord& aim_coord);
	void set_attackor_aim_coord(const MoverCoord& aim_coord);

	virtual void set_aim_object(Int64 aim_id, const bool is_from_group = false);
    void set_last_attacker(Int64 obj_id);
    void set_self_owner(Int64 owner_id);
    void set_group_id(const int id);
    void set_is_attack(const int flag);

	void push_schedule_time(double schedule_time);
    void push_schedule_time(const Time_Value& schedule_time);

    Int64 fetch_adjust_fighter_id(Int64 obj_id);

    //如果当前没有目标，设置为obj_id. 如果当前已有目标，则不变
    void keep_and_set_aim_object(Int64 obj_id);
    //不管当前有没有目标，已当前为准
    void change_and_set_aim_object(Int64 obj_id);
    //清除目标
    void clean_aim_object();

    void chase_or_attack_fighter();
    void check_and_self_return();

    virtual int schedule_move_fighter();
    int pop_schedule_time(Time_Value& interval);

    int fetch_launch_skill_distance(FighterSkill* fighter_skill);
    int fetch_max_launch_skill_distance();

    int fetch_map_skill_id();
    int fetch_copy_skill_id();
    int fetch_queue_skill_id();
    int fetch_beast_skill_id();
    int fetch_sequence_skill_id();

    int rand_set_aim_object(const MoverMap& fighter_map);

    virtual int auto_fighter_attack();
    virtual int fetch_auto_skill_id();
    virtual int validate_auto_skill_usage(FighterSkill* fighter_skill);
    virtual int auto_fighter_launch_skill(int skill_id);
    int make_up_skill_request(Proto10400201* request, FighterSkill* fighter_skill);

    GameFighter* fetch_aim_object();
    GameFighter* fetch_self_owner();
    GameFighter* fetch_last_attacker(void);

    virtual const MoverCoord &rect_skill_coord(void);
    virtual void set_rect_skill_coord(const MoverCoord &coord);

    const Time_Value &guide_tick(void);
    int guide_skill(void);
    int auto_launch_guide_skill(const Time_Value &nowtime);
    bool is_guide_skill(FighterSkill *skill);
    int check_launch_pre_skill(FighterSkill *skill);
    void update_guide_skill(FighterSkill *skill);
    virtual void stop_guide_skill(void);
    virtual void stop_guide_skill_by_skill(const int skill_id);
    void notify_guid_range_effect_appear(void);
    void notify_guid_range_effect_disappear(void);
    int make_up_appear_range_effect(Block_Buffer *buff);
    int make_up_disappear_range_effect(Block_Buffer *buff);

    virtual int make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate = false);
    virtual int make_up_disappear_other_info(Block_Buffer *buff, const bool send_by_gate = false);

    /* 处理技能提示播放 */
    virtual int process_skill_note(int skill_id, const Json::Value& detail_json);
    virtual int process_monster_talk(int skill_id, int mover_id);
protected:
    virtual int die_process(const int64_t fighter_id = 0);

protected:
    AutoActionTimer action_timer_;
    AutoFighterDetail auto_detail_;
};

#endif /* AUTOMAPFIGHTER_H_ */
