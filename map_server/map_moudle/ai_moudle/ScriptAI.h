/*
 * ScriptAI.h
 *
 * Created on: 2014-01-03 10:02
 *     Author: lyz
 */

#ifndef _SCRIPTAI_H_
#define _SCRIPTAI_H_

#include "GameAI.h"

class BaseScript;

class ScriptAI : public GameAI
{
public:
    ScriptAI(void);
    virtual ~ScriptAI(void);

    virtual int auto_action_timeout(const Time_Value &nowtime);

    virtual void reset(void);

    virtual int sign_out(void);
    virtual int enter_scene(const int type = ENTER_SCENE_LOGIN);

    ScriptAIDetail &script_ai_detail(void);
    void set_script_sort(const int sort);
    int script_sort(void);

    void set_scene_config_index(const int index);
    int scene_config_index(void);

    int record_index(void);
    void set_record_index(const int index);

    void set_level_index(const int index);
    int level_index(void);

    void set_wave_id(const int wave);
    int wave_id(void);

    virtual const Json::Value &fetch_layout_item(void);

    virtual int die_process(const int64_t fighter_id = 0);
    BaseScript *fetch_script(void);

    virtual bool is_movable_coord(const MoverCoord &coord);
    virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
    		const int64_t attackor = 0, const int skill_id = 0);
    virtual int schedule_move_fighter(void);
    virtual void produce_drop_items(const Json::Value &monster_drop, LongMap& player_map,
    		LongSet &coord_set, int *drop_size);


protected:
    virtual void init_base_property(void);
    virtual int recycle_self(void);
    virtual void produce_drop_items(const Json::Value &monster_drop,
    		LongMap& player_map, const ItemObj& item_obj, LongSet &coord_set, int *drop_size);

    virtual int validate_movable(const MoverCoord &step);
    virtual int modify_player_experience(int64_t fighter_id, int inc_exp);
    virtual int process_push_away(const int range);

    virtual void fetch_drop_money_amount(int *money_type, int *money_amount, const Json::Value &monster_drop);
protected:
    ScriptAIDetail script_ai_detail_;

};

#endif //_SCRIPTAI_H_
