/*
 * ScriptFactory.h
 *
 * Created on: 2013-12-18 22:29
 *     Author: lyz
 */

#ifndef _SCRIPTFACTORY_H_
#define _SCRIPTFACTORY_H_

#include "ObjectPoolEx.h"

class BaseScript;
class ExpScript;
class LeagueFbScript;
class ClimbTowerScript;
class UpclassScript;
class LegendTopScript;
class SwordTopScript;
class TowerDefenseScript;
class FourAltarScript;
class SealBossScript;
class CouplesScript;
class MonsterTowerScript;
class AdvanceScript;
class VipScript;
class StoryScript;
class RamaScript;
class NewAdvanceScript;
class NewStoryScript;
class TravelScript;

class ScriptFactory
{
public:
    typedef ObjectPoolEx<ExpScript> ExpScriptPool;
    typedef ObjectPoolEx<LeagueFbScript> LeagueFbPool;
    typedef ObjectPoolEx<ClimbTowerScript> ClimbTowerScriptPool;
    typedef ObjectPoolEx<UpclassScript> UpclassScriptPool;
    typedef ObjectPoolEx<TowerDefenseScript> TowerDefenseScriptPool;
    typedef ObjectPoolEx<FourAltarScript> FourAltarScriptPool;
    typedef ObjectPoolEx<SealBossScript> SealBossScriptPool;
    typedef ObjectPoolEx<CouplesScript> CouplesScriptPool;
    typedef ObjectPoolEx<MonsterTowerScript> MonsterTowerScriptPool;
    typedef ObjectPoolEx<LegendTopScript> LegendTopScriptPool;
    typedef ObjectPoolEx<SwordTopScript> SwordTopScriptPool;
    typedef ObjectPoolEx<AdvanceScript> AdvanceScriptPool;
    typedef ObjectPoolEx<VipScript> VipScriptPool;
    typedef ObjectPoolEx<StoryScript> StoryScriptPool;
    typedef ObjectPoolEx<RamaScript> RamaScriptPool;
    typedef ObjectPoolEx<NewAdvanceScript> NewAdvanceScriptPool;
    typedef ObjectPoolEx<NewStoryScript> NewStoryScriptPool;
    typedef ObjectPoolEx<TravelScript> TravelScriptPool;

public:
    ScriptFactory(void);
    ~ScriptFactory(void);

    BaseScript *pop_script(const int script_sort);
    int push_script(ExpScript *script);
    int push_script(LeagueFbScript *script);
    int push_script(ClimbTowerScript *script);
    int push_script(UpclassScript *script);
    int push_script(TowerDefenseScript *script);
    int push_script(FourAltarScript *script);
    int push_script(SealBossScript *script);
    int push_script(CouplesScript *script);
    int push_script(MonsterTowerScript *script);
    int push_script(LegendTopScript *script);
    int push_script(SwordTopScript *script);
    int push_script(AdvanceScript *script);
    int push_script(VipScript *script);
    int push_script(StoryScript *script);
    int push_script(RamaScript *script);
    int push_script(NewAdvanceScript *script);
    int push_script(NewStoryScript *script);
    int push_script(TravelScript* script);

    void report_pool_info(std::ostringstream &msg_stream);

protected:
    ExpScriptPool *exp_script_pool_;
    LeagueFbPool *league_fb_script_pool_;
    ClimbTowerScriptPool *climb_script_pool_;
    UpclassScriptPool *upclass_script_pool_;
    TowerDefenseScriptPool *tower_defense_script_pool_;
    FourAltarScriptPool *four_altar_script_pool_;
    SealBossScriptPool *seal_boss_script_pool_;
    CouplesScriptPool *couples_script_pool_;
    MonsterTowerScriptPool *monster_tower_script_pool_;
    LegendTopScriptPool *legend_top_script_pool_;
    SwordTopScriptPool *sword_top_script_pool_;
    AdvanceScriptPool *advance_script_pool_;
    VipScriptPool *vip_script_pool_;
    StoryScriptPool *story_script_pool_;
    RamaScriptPool *rama_script_pool_;
    NewAdvanceScriptPool *new_advance_script_pool_;
    NewStoryScriptPool *new_story_script_pool_;
    TravelScriptPool* travel_script_pool_;
};

#endif //_SCRIPTFACTORY_H_
