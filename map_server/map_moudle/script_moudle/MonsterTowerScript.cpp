#include "MonsterTowerScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ProtoDefine.h"
#include "GameCommon.h"
#include "MapPlayerEx.h"
#include "ScriptAI.h"
#include "ScriptScene.h"
#include "AIManager.h"

MonsterTowerScript::MonsterTowerScript()
{

}

MonsterTowerScript::~MonsterTowerScript()
{

}

void MonsterTowerScript::recycle_self_to_pool()
{
	this->monitor()->script_factory()->push_script(this);
}

int MonsterTowerScript::broad_script_pass()
{
	ScriptDetail& script_detail = this->script_detail();
	JUDGE_RETURN(script_detail.__is_single == false, -1);
	//广播
//	BrocastParaVec para_vec;
//	GameCommon::push_brocast_para_role_detail(para_vec,role_id, role_name, false);
//	GameCommon::push_brocast_para_item(para_vec, item);
//    MAP_MONITOR->announce_world(brocast_type, para_vec);

	return 0;
}

int MonsterTowerScript::make_up_special_script_finish_detail(MapPlayerScript *player, Message *msg)
{
	JUDGE_RETURN(player != NULL, -1);
	if(player->fight_detail().__blood <= 0 )
	{
//		this->script_detail().__relive.__left_relive = 1;
//		Proto10400203 respond;
//		respond.set_relive(1);
//		player->request_relive(&respond);
		player->fight_detail().__blood = player->fight_detail().__blood_max.__total_i();
		Proto80400202 respond;
		player->make_up_fight_update_info(&respond, FIGHT_UPDATE_BLOOD, player->fight_detail().__blood, 0, 0,
				player->fight_detail().__blood, FIGHT_TIPS_RELIVE);
		player->respond_to_client(ACTIVE_UPDATE_FIGHT, &respond);

		player->notify_mover_cur_location();
	}

	DYNAMIC_CAST_RETURN(Proto80400929 *, respond, msg, -1);
	int present_sort = this->script_sort();
	int next_sort = 0;
	int preset_level = present_sort / 100;
	if(preset_level >= 281 && preset_level < 287)
	{
		next_sort = (preset_level + 1) * 100 + present_sort % 100;
	}
	respond->set_present_sort(present_sort);
	respond->set_next_sort(next_sort);
	ScriptPlayerDetail::ScriptRecord *script_rec = player->refresh_script_record(this->script_sort());
	JUDGE_RETURN(script_rec != NULL, -1);
//	int pass_times = player->get_monster_tower_pass_time_by_floor(this->script_sort(), player->script_detail());
	int pass_times = script_rec->__day_pass_times;
	if(preset_level == 287 && pass_times > 1)
	{
		respond->set_no_card(1);
	}

    const Json::Value &script_json = CONFIG_INSTANCE->script(present_sort);

    int star_lvl = script_rec->__award_star;
    int award_score = 0;
	if(script_json != Json::Value::null && pass_times == 1 &&
			script_json["finish_condition"].isMember("award_score_lvl") &&
			script_json["finish_condition"]["award_score_lvl"].isArray() &&
			star_lvl <= (int)script_json["finish_condition"]["award_score_lvl"].size())
	{
		award_score = script_json["finish_condition"]["award_score_lvl"][star_lvl - 1].asInt();
	}
	respond->set_ai_score(award_score);
	MSG_DEBUG("MonsterTowerScript role_name:%s finish script_id:%d", player->role_name().c_str(), this->script_sort());
	return 0;
}

int MonsterTowerScript::get_tower_floor()
{
	int sort = this->script_sort();
	return (sort - 28000) / 100;
}
int MonsterTowerScript::get_task_difficulty()
{
	int sort = this->script_sort();
	return (sort % 100) / 10;
}

int MonsterTowerScript::make_up_special_script_progress_detail(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto50400903 *, respond, msg, -1);
	int floor = this->get_tower_floor();
	respond->set_current_floor(floor);
	respond->set_difficulty(this->get_task_difficulty());
	if(floor == 6)
	{
		const ScriptDetail& sd = this->script_detail();
		if(sd.__monster.__magic_monster._flag == 1)
		{
			Proto80400235 respond;
			respond.set_flag(sd.__monster.__magic_monster._flag);
			respond.set_show_ai_id(sd.__monster.__magic_monster._monster_ai_id);
			respond.set_hide_ai_id(sd.__monster.__magic_monster._boss_ai_id);
			respond.set_show_ai_name(sd.__monster.__magic_monster._name);
			Scene* scene = this->fetch_current_scene();
			JUDGE_RETURN(scene != NULL, -1);
			GameAI *boss = scene->find_ai(sd.__monster.__magic_monster._boss_ai_id);
			if(boss != NULL)
			{
				boss->respond_to_broad_area(&respond, 1);
			}
		}
	}
	return 0;
}

int MonsterTowerScript::summon_ai_inherit_player_attr(ScriptAI *script_ai, const Json::Value &json)
{
	int size = this->script_detail().__player_set.size();
	JUDGE_RETURN(size > 0 && script_ai != NULL && json != Json::Value::null, -1);
	JUDGE_RETURN(json.isMember("inherit_player_attr_percent"), -1);
	int rand = std::rand() % size, i = 0;
    MapPlayerEx *player = 0;

    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end() && i <= rand; ++iter, ++i)
    {
        if (this->monitor()->find_player(*iter, player) != 0)
        {
        	continue;
        }
    }
    JUDGE_RETURN(player != NULL, -1);

    Int64 call_id = script_ai->caller();
    GameFighter *caller = 0;
    GameAI* caller_ai = 0;
    Int64 hide_ai_id = 0, show_ai_id = script_ai->ai_id();
    Proto80400235 respond;
    if (call_id > 0 && script_ai->find_fighter(call_id, caller) == 0)
    {
    	caller_ai = dynamic_cast<GameAI *>(caller);
    	JUDGE_RETURN(caller_ai != NULL, -1);
    	//本体隐藏之后不能攻击，也不能被攻击
		caller_ai->set_ai_falg(GameEnum::AI_CF_NO_ATTACK);
		caller_ai->set_ai_falg(GameEnum::AI_CF_NO_BE_ATTACKED);
	    //caller_ai->set_ai_falg(GameEnum::AI_CF_NO_MOVE);
		hide_ai_id = caller_ai->ai_id();
    }
    const Json::Value &script_json = CONFIG_INSTANCE->script_monster_tower();
    std::string show_ai_name = std::string(player->role_detail().__name) + script_json["magical_name"].asString();
    respond.set_show_ai_id(show_ai_id);
    respond.set_hide_ai_id(hide_ai_id);
    respond.set_show_ai_name(show_ai_name);
    respond.set_flag(1);
    caller_ai->respond_to_broad_area(&respond, 1);

    ScriptDetail& sd = this->script_detail();
    sd.__monster.__magic_monster._boss_ai_id = hide_ai_id;
    sd.__monster.__magic_monster._monster_ai_id = show_ai_id;
    sd.__monster.__magic_monster._role_id = player->role_id();
    sd.__monster.__magic_monster._name = show_ai_name;
    sd.__monster.__magic_monster._flag = 1;

    FightDetail& ai = script_ai->fight_detail();
    FightDetail& play = player->fight_detail();

	int percent = json["inherit_player_attr_percent"].asInt();
    if(percent > 0)
    {
        double num = percent / 100;
        ai.__blood += play.__blood * num;

        ai.__attack_lower.add_single(play.__attack_lower.basic() * num, BasicElement::BASIC);
        ai.__attack_upper.add_single(play.__attack_upper.basic() * num, BasicElement::BASIC);

        ai.__defence_lower.add_single(play.__defence_lower.basic() * num, BasicElement::BASIC);
        ai.__defence_upper.add_single(play.__defence_upper.basic() * num, BasicElement::BASIC);

        ai.__hit.add_single(play.__hit.basic() * num, BasicElement::BASIC);
        ai.__crit.add_single(play.__crit.basic() * num);
        ai.__toughness.add_single(play.__toughness.basic() * num, BasicElement::BASIC);
        ai.__lucky.add_single(play.__lucky.basic() * num, BasicElement::BASIC);
        ai.__damage.add_single(play.__damage.basic() * num, BasicElement::BASIC);
        ai.__reduction.add_single(play.__reduction.basic() * num, BasicElement::BASIC);
        ai.__crit.add_single(play.__crit.basic() * num, BasicElement::BASIC);
    }

  	return 0;
}

int MonsterTowerScript::sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id)
{
	if(this->get_tower_floor() == 6)	//镇魔塔第六层幻体信息
	{
		//通知显示boss本体是在幻体死亡之前的行为树里做的
	    ScriptDetail& sd = this->script_detail();
		if(script_ai->ai_id() == sd.__monster.__magic_monster._monster_ai_id)
		{
			sd.__monster.__magic_monster._monster_ai_id = 0;
		    sd.__monster.__magic_monster._role_id = 0;
		    sd.__monster.__magic_monster._name.clear();
			sd.__monster.__magic_monster._flag = 2;
		}else if(script_ai->ai_id() == sd.__monster.__magic_monster._boss_ai_id)
		{
			sd.__monster.__magic_monster.reset();
		}
	}
	return BaseScript::sync_kill_monster(script_ai, fighter_id);
}
