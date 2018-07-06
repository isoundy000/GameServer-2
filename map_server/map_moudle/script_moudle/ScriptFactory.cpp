/*
 * ScriptFactory.cpp
 *
 * Created on: 2013-12-26 20:46
 *     Author: lyz
 */

#include "ScriptFactory.h"
#include "ExpScript.h"
#include "LeagueFbScript.h"
#include "ClimbTowerScript.h"
#include "UpclassScript.h"
#include "TowerDefenseScript.h"
#include "FourAltarScript.h"
#include "SealBossScript.h"
#include "CouplesScript.h"
#include "GameConfig.h"
#include "MonsterTowerScript.h"
#include "LegendTopScript.h"
#include "SwordTopScript.h"
#include "AdvanceScript.h"
#include "VipScript.h"
#include "StoryScript.h"
#include "RamaScript.h"
#include "NewAdvanceScript.h"
#include "NewStoryScript.h"
#include "TravelScript.h"

ScriptFactory::ScriptFactory(void)
{
    this->exp_script_pool_ = new ExpScriptPool;
    this->league_fb_script_pool_ = new LeagueFbPool;
    this->climb_script_pool_ = new ClimbTowerScriptPool;
    this->upclass_script_pool_ = new UpclassScriptPool;
    this->tower_defense_script_pool_ = new TowerDefenseScriptPool;
    this->four_altar_script_pool_ = new FourAltarScriptPool;
    this->seal_boss_script_pool_ = new SealBossScriptPool;
    this->couples_script_pool_ = new CouplesScriptPool;
    this->monster_tower_script_pool_ = new MonsterTowerScriptPool;
    this->legend_top_script_pool_ = new LegendTopScriptPool;
    this->sword_top_script_pool_ = new SwordTopScriptPool;
    this->advance_script_pool_ = new AdvanceScriptPool;
    this->vip_script_pool_ = new VipScriptPool;
    this->story_script_pool_ = new StoryScriptPool;
    this->rama_script_pool_ = new RamaScriptPool;
    this->new_advance_script_pool_ = new NewAdvanceScriptPool;
    this->new_story_script_pool_ = new NewStoryScriptPool;
    this->travel_script_pool_ = new TravelScriptPool;
}

ScriptFactory::~ScriptFactory(void)
{
    SAFE_DELETE(this->exp_script_pool_);
    SAFE_DELETE(this->league_fb_script_pool_);
    SAFE_DELETE(this->climb_script_pool_);
    SAFE_DELETE(this->upclass_script_pool_);
    SAFE_DELETE(this->tower_defense_script_pool_);
    SAFE_DELETE(this->four_altar_script_pool_);
    SAFE_DELETE(this->seal_boss_script_pool_);
    SAFE_DELETE(this->couples_script_pool_);
    SAFE_DELETE(this->monster_tower_script_pool_);
    SAFE_DELETE(this->legend_top_script_pool_);
    SAFE_DELETE(this->sword_top_script_pool_);
    SAFE_DELETE(this->advance_script_pool_);
    SAFE_DELETE(this->vip_script_pool_);
    SAFE_DELETE(this->story_script_pool_);
    SAFE_DELETE(this->rama_script_pool_);
    SAFE_DELETE(this->new_advance_script_pool_);
    SAFE_DELETE(this->new_story_script_pool_);
    SAFE_DELETE(this->travel_script_pool_);
}

BaseScript *ScriptFactory::pop_script(const int script_sort)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    switch (script_json["type"].asInt())
    {
    case GameEnum::SCRIPT_T_EXP:
        return this->exp_script_pool_->pop();
    case GameEnum::SCRIPT_T_LEAGUE_FB:
    	return this->league_fb_script_pool_->pop();
    case GameEnum::SCRIPT_T_CLIMB_TOWER:
        return this->climb_script_pool_->pop();
    case GameEnum::SCRIPT_T_UPCLASS:
        return this->upclass_script_pool_->pop();
    case GameEnum::SCRIPT_T_TOWER_DEFENSE:
        return this->tower_defense_script_pool_->pop();
    case GameEnum::SCRIPT_T_FOUR_ALTAR:
        return this->four_altar_script_pool_->pop();
    case GameEnum::SCRIPT_T_SEAL:
        return this->seal_boss_script_pool_->pop();
    case GameEnum::SCRIPT_T_COUPLES:
        return this->couples_script_pool_->pop();
    case GameEnum::SCRIPT_T_MONSTER_TOWER:
    	return this->monster_tower_script_pool_->pop();
    case GameEnum::SCRIPT_T_LEGEND_TOP:
    	return this->legend_top_script_pool_->pop();
    case GameEnum::SCRIPT_T_SWORD_TOP:
    	return this->sword_top_script_pool_->pop();
    case GameEnum::SCRIPT_T_TRVL:
    	return this->travel_script_pool_->pop();
    case GameEnum::SCRIPT_T_ADVANCE:
    	return this->advance_script_pool_->pop();
    case GameEnum::SCRIPT_T_VIP:
    	return this->vip_script_pool_->pop();
    case GameEnum::SCRIPT_T_STORY:
    	return this->story_script_pool_->pop();
    case GameEnum::SCRIPT_T_RAMA:
    	return this->rama_script_pool_->pop();
    case GameEnum::SCRIPT_T_NEW_ADVANCE:
    	return this->new_advance_script_pool_->pop();
    case GameEnum::SCRIPT_T_NEW_STORY:
    	return this->new_story_script_pool_->pop();

    default:
        return 0;
    }
}

int ScriptFactory::push_script(ExpScript *script)
{
    return this->exp_script_pool_->push(script);
}

int ScriptFactory::push_script(LeagueFbScript *script)
{
    return this->league_fb_script_pool_->push(script);
}

int ScriptFactory::push_script(ClimbTowerScript *script)
{
    return this->climb_script_pool_->push(script);
}

int ScriptFactory::push_script(UpclassScript *script)
{
    return this->upclass_script_pool_->push(script);
}

int ScriptFactory::push_script(TowerDefenseScript *script)
{
    return this->tower_defense_script_pool_->push(script);
}

int ScriptFactory::push_script(FourAltarScript *script)
{
    return this->four_altar_script_pool_->push(script);
}

int ScriptFactory::push_script(SealBossScript *script)
{
    return this->seal_boss_script_pool_->push(script);
}

int ScriptFactory::push_script(CouplesScript *script)
{
    return this->couples_script_pool_->push(script);
}

int ScriptFactory::push_script(MonsterTowerScript *script)
{
    return this->monster_tower_script_pool_->push(script);
}

int ScriptFactory::push_script(LegendTopScript *script)
{
	return this->legend_top_script_pool_->push(script);
}

int ScriptFactory::push_script(SwordTopScript *script)
{
	return this->sword_top_script_pool_->push(script);
}

int ScriptFactory::push_script(AdvanceScript* script)
{
	return this->advance_script_pool_->push(script);
}

int ScriptFactory::push_script(VipScript* script)
{
	return this->vip_script_pool_->push(script);
}

int ScriptFactory::push_script(StoryScript* script)
{
	return this->story_script_pool_->push(script);
}

int ScriptFactory::push_script(RamaScript* script)
{
	return this->rama_script_pool_->push(script);
}

int ScriptFactory::push_script(NewAdvanceScript* script)
{
	return this->new_advance_script_pool_->push(script);
}

int ScriptFactory::push_script(NewStoryScript* script)
{
	return this->new_story_script_pool_->push(script);
}

int ScriptFactory::push_script(TravelScript* script)
{
	return this->travel_script_pool_->push(script);
}

void ScriptFactory::report_pool_info(std::ostringstream &msg_stream)
{
    if (this->exp_script_pool_ != 0)
    {
        msg_stream << "ExpScript Pool" << std::endl;
        this->exp_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->league_fb_script_pool_ != 0)
    {
        msg_stream << "LeagueFbScript Pool" << std::endl;
        this->league_fb_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->climb_script_pool_ != 0)
    {
        msg_stream << "ClimbScript Pool" << std::endl;
        this->climb_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->upclass_script_pool_ != 0)
    {
        msg_stream << "UpclassScript Pool" << std::endl;
        this->upclass_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->tower_defense_script_pool_ != 0)
    {
        msg_stream << "TowerDefenseScript Pool" << std::endl;
        this->tower_defense_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->four_altar_script_pool_ != 0)
    {
        msg_stream << "FourAltarScript Pool" << std::endl;
        this->four_altar_script_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->seal_boss_script_pool_ != 0)
    {
        msg_stream << "SealBossScript Pool" << std::endl;
        this->seal_boss_script_pool_->dump_info_to_stream(msg_stream);
    }
    if(this->monster_tower_script_pool_ != 0)
    {
    	msg_stream << "MonsterTowerScript Pool" << std::endl;
    	this->monster_tower_script_pool_->dump_info_to_stream(msg_stream);
    }
    if(this->legend_top_script_pool_ != 0)
    {
    	msg_stream << "LegendTopScript Pool" << std::endl;
    	this->legend_top_script_pool_->dump_info_to_stream(msg_stream);
    }
    if(this->sword_top_script_pool_ != 0)
    {
        msg_stream << "SwordTopScript Pool" << std::endl;
        this->sword_top_script_pool_->dump_info_to_stream(msg_stream);
    }
    if(this->advance_script_pool_ != 0)
    {
        msg_stream << "AdvanceScript Pool" << std::endl;
        this->advance_script_pool_->dump_info_to_stream(msg_stream);
    }
    if(this->vip_script_pool_ != 0)
    {
		msg_stream << "VipScript Pool" << std::endl;
		this->vip_script_pool_->dump_info_to_stream(msg_stream);
	}
    if(this->story_script_pool_ != 0)
    {
		msg_stream << "StoryScript Pool" << std::endl;
		this->story_script_pool_->dump_info_to_stream(msg_stream);
	}
    if(this->rama_script_pool_ != 0)
    {
		msg_stream << "RamaScript Pool" << std::endl;
		this->rama_script_pool_->dump_info_to_stream(msg_stream);
	}
    if(this->new_advance_script_pool_ != 0)
    {
		msg_stream << "NewAdvanceScript Pool" << std::endl;
		this->new_advance_script_pool_->dump_info_to_stream(msg_stream);
	}
    if(this->new_story_script_pool_ != 0)
    {
    	msg_stream << "NewStoryScript Pool" << std::endl;
    	this->new_story_script_pool_->dump_info_to_stream(msg_stream);
    }
}

