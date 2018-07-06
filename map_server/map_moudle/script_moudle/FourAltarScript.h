/*
 * FourAltarScript.h
 *
 * Created on: 2015-03-17 21:10
 *     Author: lyz
 */

#ifndef _FOURALTARSCRIPT_H_
#define _FOURALTARSCRIPT_H_

#include "BaseScript.h"

class FourAltarScript : public BaseScript
{
public:
    enum SCENE_TARGET
    {
        ST_PLAYER   = 1,
        ST_BOSS     = 2,
        ST_ALL_MONSTER = 3,
        ST_END
    };
    enum SCENE_EFFECT_TYPE
    {
        SET_ATTACK = 1,
        SET_DEFENCE = 2,
        SET_AVOID = 3,
        SET_SPEED = 4,
        SET_DIZZY = 5,
        SET_BLOOD = 6,
        SET_METEOR = 7,
        SET_PROP = 8,
        SET_RECOVERT_MAGIC = 9,
        SET_END
    };
public:
    virtual ~FourAltarScript(void);
    virtual void reset(void);
    virtual int sync_increase_monster(ScriptAI *script_ai);
    virtual int sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id);

protected:
    virtual int init_scene_exec_condition(const Json::Value &scene_json);
    virtual int other_script_timeout(const Time_Value &nowtime);
    int init_effect_detail(FourAltarDetail::EffectDetail &effect_detail, const Json::Value &json);
    int convert_effect_type(const std::string &effect_name);
    int active_four_altar(ScriptAI *script_ai);

    int process_oncebuff_active(FourAltarDetail::SceneSkillDetail &sceneskill_detail);
    int process_scene_skill_launch(const int boss_sort, FourAltarDetail::SceneSkillDetail &sceneskill_detail);
    int launch_scene_skill_effect(FourAltarDetail::EffectDetail &effect_detail);
    int process_effect_to_defender(FourAltarDetail::EffectDetail &effect_detail, GameFighter *defender);

protected:
    virtual void recycle_self_to_pool(void);

protected:
    FourAltarDetail four_altar_detail_;
};

#endif //_FOURALTARSCRIPT_H_
