/*
 * AIBasicPre.cpp
 *
 *  Created on: Jun 22, 2013
 *      Author: peizhibi
 */

#include "AIBasicPre.h"
#include "Scene.h"
#include "GameAI.h"
#include "AIBTCName.h"
#include "ScriptStruct.h"
#include "MapPlayerEx.h"

bool BaseAIPrecond::ExternalCondition(NodeInputParam &input)
{
	GameAI* game_ai = input.game_ai_;
	JUDGE_RETURN(game_ai != NULL, false);

	return this->AIExternalCond(game_ai);
}

/*
 * die
 * */
bool AIDiePre::AIExternalCond(GameAI* game_ai)
{
	return game_ai->is_death() == true;
}

/*
 * area recycle
 * */
bool AreaRecyclePre::AIExternalCond(GameAI* game_ai)
{
	Scene *scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);
	return scene->is_ai_area_recycle(game_ai);
}

/*
 * sleep
 * */
bool AISleepPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_death() == false, false);

    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    return scene->player_amount() == 0;
}

/*
 * pause
 */
bool AIPausePre::AIExternalCond(GameAI* game_ai)
{
	// 普通场景走一下，停三秒
	if (game_ai->ai_detail().__pause_flag > 0)
	{
		game_ai->ai_detail().__pause_flag = 0;
		return true;
	}
	else
	{
		game_ai->ai_detail().__pause_flag = 1;
		return false;
	}
}

/*
 * auto
 * */
bool AIAutoPre::AIExternalCond(GameAI* game_ai)
{
	return true;
}

/*
 * move
 * */
bool AIMovePre::AIExternalCond(GameAI* game_ai)
{
	return game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false;
}

/*
 * random move
 * */

AIRandMovePre::AIRandMovePre(void)
{
	this->move_interval_ = 0;
	this->move_max_last_ = 0;
}

bool AIRandMovePre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, false);

	// 普通场景的随机巡逻，呼吸调到最少3秒
	if (this->move_interval_ <= this->move_max_last_)
	{
		game_ai->push_schedule_time(1);
		++this->move_interval_;
		return false;
	}
	else
	{
		this->move_max_last_ = std::rand() % 7 + 4;
		this->move_interval_ = 0;
		return true;
	}
}

/*
 * no aim
 * */
bool AINoAimPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_death() == false, false);
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_AIM) == false, false);
	JUDGE_RETURN(game_ai->fetch_aim_object() == NULL, false);

	return true;
}

/*
 * have aim
 * */
bool AIHaveAimPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_death() == false, false);

	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, false);

	if (game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false)
	{
		//可以移动
		return true;
	}

	if (game_ai->is_arrive_attack_distance(aim_fighter) == true)
	{
		//不可以移动，若达到可攻击的距离，则返回true
		return true;
	}
	else
	{
		game_ai->set_aim_object(0);
		return false;
	}
}

/*
 * back
 * */
bool AIBackPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_death() == false, false);
	JUDGE_RETURN(game_ai->ai_is_back() == false, true);

	int between_distance = ::coord_offset_grid(game_ai->location(),
			game_ai->birth_coord());

	if (game_ai->fetch_aim_object() != NULL)
	{
		JUDGE_RETURN(between_distance > GameEnum::AI_CHASE_BACK_DISTANCE, false);
	}
	else
	{
		JUDGE_RETURN(between_distance > GameEnum::AI_BACK_DISTANCE, false);
	}

	return true;
}

/*
 * idle back
 * */
AIIdleBackPre::AIIdleBackPre(int back_distance)
{
	this->idle_back_distance_ = back_distance;
}

//目标位置, 离怪物出生点太远, 则清除
void AIIdleBackPre::check_birth_distance(GameAI* game_ai)
{
	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, ;);

	int distance = ::coord_offset_grid(game_ai->birth_coord(), aim_fighter->location());
	JUDGE_RETURN(distance > this->idle_back_distance_, ;);

	game_ai->clean_aim_object();
}

bool AIIdleBackPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_death() == false, false);

	this->check_birth_distance(game_ai);

	int idle_back_distance = this->idle_back_distance_;
    if (game_ai->aim_object_id() <= 0)
    {
        idle_back_distance = GameEnum::AI_BACK_DISTANCE;
    }

    int between_distance = ::coord_offset_grid(game_ai->location(), game_ai->birth_coord());
    JUDGE_RETURN(between_distance > idle_back_distance, false);

    game_ai->clean_aim_object();
    game_ai->hurt_map().clear();
	game_ai->set_aim_coord(game_ai->ai_detail().__birth_coord);

	return true;
}

/*
 * select the last attacker in current time.
 * if had selected, don't select again.
 * */
bool AISelectModeAPre::AIExternalCond(GameAI* game_ai)
{
	GameFighter* last_attacker = game_ai->fetch_last_attacker();
	JUDGE_RETURN(last_attacker != NULL, false);

	Int64 adjust_id = game_ai->fetch_adjust_fighter_id(last_attacker->mover_id());
	if (adjust_id == 0)
	{
		game_ai->set_last_attacker(0);
		return false;
	}
	else
	{
		game_ai->set_aim_object(adjust_id);
		return true;
	}
}

/*
 * if aim object's level is too lower, attack it.
 * */
bool AISelectModeBPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    //player level
    Scene::MoverMap& player_map = scene->player_map();
    for (Scene::MoverMap::iterator iter = player_map.begin();
    		iter != player_map.end(); ++iter)
    {
    	GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
    	JUDGE_CONTINUE(fighter != NULL && fighter->is_death() == false);

		game_ai->set_aim_object(iter->first);
    	return true;
    }

    return false;
}

/*
 * random select aim object from around
 * */
AISelectModeCPre::AISelectModeCPre(int)
{
}

bool AISelectModeCPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    Scene::MoverMap player_map;
    JUDGE_RETURN(scene->fetch_all_around_player(game_ai, player_map, game_ai->location(),
    		game_ai->fetch_select_distance(), 10) == 0, false);

	return game_ai->rand_set_aim_object(player_map);

}

/*
 * random select aim object from full scene
 * */
bool AISelectModeDPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    // full scene
    Scene::MoverMap& player_map = scene->player_map();
    return game_ai->rand_set_aim_object(player_map);
}

bool AISelectModeGPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    Scene::MoverMap player_map;
    JUDGE_RETURN(scene->fetch_all_around_player(game_ai, player_map, game_ai->location(),
    		game_ai->fetch_select_distance(), 10) == 0, false);

	return game_ai->rand_set_aim_object(player_map);
}

bool AISelectModeOutsidePre::AIExternalCond(GameAI* game_ai)
{
	Int64 aim_id = game_ai->aim_select();
	if(aim_id > 0)
	{
		game_ai->set_aim_object(aim_id);
		return true;
	}
	else
	{
		game_ai->set_aim_object(0);
		return false;
	}
}

/*
 * select protected npc as fighter(only for script)
 * "ext_precond" : "select_mode_npc"
 * */
bool AISelectModeNpcPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);
    JUDGE_RETURN(scene->protect_npc_id() > 0, false);

    GameFighter *target = NULL;
    JUDGE_RETURN(scene->find_fighter(scene->protect_npc_id(), target) == 0 &&
    		coord_offset_grid(target->location(), game_ai->location()) <= 10, false);

    game_ai->set_aim_object(scene->protect_npc_id());
	return true;
}

bool AISelectModeMonsterPre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    Scene::MoverMap monster_map;
    scene->fetch_all_around_monster(game_ai, monster_map, game_ai->location(), 20, 5);
    for (Scene::MoverMap::iterator iter = monster_map.begin();
    		iter != monster_map.end(); ++iter)
    {
    	GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
    	if (fighter == 0 || fighter->is_death())
    		continue;
        if (fighter->camp_id() == GameEnum::MONSTER_CAMP_NPC)
        	continue;
		game_ai->set_aim_object(iter->first);
    	return true;
    }

    return false;
}

bool AISelectModeMonsterLowBlood::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    double monster_aim_blood = game_ai->cur_blood();
    game_ai->set_aim_object(game_ai->fighter_id());

    Scene::MoverMap monster_map;
    scene->fetch_all_around_monster(game_ai, monster_map, game_ai->location(), 20, 5);
    for (Scene::MoverMap::iterator iter = monster_map.begin();
    		iter != monster_map.end(); ++iter)
    {
    	GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
    	if (fighter == 0 || fighter->is_death())
    		continue;
        if (fighter->camp_id() == GameEnum::MONSTER_CAMP_NPC)
        	continue;

        JUDGE_CONTINUE(fighter->fight_detail().__blood !=
        		fighter->fight_detail().__blood_total(fighter));

        double cur_monster_blood = fighter->fight_detail().__blood;
        if(cur_monster_blood < monster_aim_blood)
        {
        	monster_aim_blood = cur_monster_blood;
        	game_ai->set_aim_object(iter->first);
        }
    }

    return true;
}

/*
 * chase aim object
 * */
AIChasePre::AIChasePre(int attack_distance)
{
	this->attack_distance_ = attack_distance;
}

bool AIChasePre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, false);

	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, false);

	if (game_ai->ai_detail().__full_screen_chase == true)
	{
		return true;
	}

	int between_distance = ::coord_offset_grid(game_ai->location(),
			aim_fighter->location());
	if (between_distance < game_ai->fetch_max_chase_distance())
	{
		return true;
	}
	else
	{
		game_ai->clean_aim_object();
		return false;
	}
}

/*
 * circle chase aim
 * */
AICircleChasePre::AICircleChasePre(int distance)
{
    this->attack_distance_ = distance;
}

bool AICircleChasePre::AIExternalCond(GameAI *game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, false);

	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, false);

	int between_distance = ::coord_offset_grid(game_ai->location(), aim_fighter->location()),
        birth_distance = ::coord_offset_grid(aim_fighter->location(), game_ai->birth_coord());

    int chase_distance = this->attack_distance_, cfg_birth_distance = this->attack_distance_;
    const Json::Value &prop_json = game_ai->fetch_prop_config();
    if (this->field_name_1().empty() == false && prop_json.isMember(this->field_name_1()))
    	chase_distance = prop_json[this->field_name_1()].asInt();
    else if (prop_json.isMember(AIBTCName::CIRCLE_CHASE_FIELD))
    	chase_distance = prop_json[AIBTCName::CIRCLE_CHASE_FIELD].asInt();

    if (prop_json.isMember("birth_chase_radius"))
    {
        cfg_birth_distance = prop_json["birth_chase_radius"].asInt();
        JUDGE_RETURN(birth_distance <= cfg_birth_distance, false);
    }
	JUDGE_RETURN(between_distance > chase_distance, false);

	return true;
}

/*
 * attack aim
 * */
AIAttackPre::AIAttackPre(int attack_distance)
{
	this->attack_distance_ = attack_distance;
}

bool AIAttackPre::AIExternalCond(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_ATTACK) == false, false);

	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, false);

	return ::check_coord_distance(game_ai->location(), aim_fighter->location(),
			game_ai->fetch_attack_distance());
}

AIRecycleTickPre::AIRecycleTickPre(int tick)
{
}

bool AIRecycleTickPre::AIExternalCond(GameAI *game_ai)
{
	return game_ai->ai_is_timeout();
}

AIAttackIntervalPre::AIAttackIntervalPre(int tick)
{
    this->default_interval_ = tick;
}

bool AIAttackIntervalPre::AIExternalCond(GameAI *game_ai)
{
    Time_Value nowtime = Time_Value::gettimeofday();

    int interval = this->default_interval_;
    const Json::Value &prop_json = game_ai->fetch_prop_config();
    const std::string *field_name = &(AIBTCName::ATTACK_INTERVAL_FIELD);
    if (this->field_name_1().empty() == false)
    	field_name = &(this->field_name_1());
    if (prop_json.isMember(*field_name))
        interval = prop_json[*field_name].asInt();

    Time_Value interval_tick = game_ai->auto_attack_tick(*field_name) + Time_Value(interval);
    if (interval_tick <= nowtime)
    {
    	game_ai->ai_detail().__last_is_move = false;
    	game_ai->ai_detail().__is_interval_attack = true;
    	game_ai->set_auto_attack_tick(*field_name, nowtime);
        return true;
    }
    return false;
}

bool AIFollowPre::AIExternalCond(GameAI* game_ai)
{
	return true;
}

bool AIAimDistancePre::AIExternalCond(GameAI* game_ai)
{
	GameFighter *aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != 0, false);

	JUDGE_RETURN(coord_offset_grid(game_ai->location(), aim_fighter->location()) <= 1, false);
	return true;
}

bool AINearInnerPre::AIExternalCond(GameAI* game_ai)
{
	GameFighter *aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != 0, false);

    static int diff[9][2] = {
        {0,0},  // NULL
        {0,-1}, // UP
        {1,-1}, // RIGHT_UP
        {1,0},  // RIGHT
        {1,1},  // RIGHT_DOWN
        {0,1},  // DOWN
        {-1,1}, // LEFT_DOWN
        {-1,0}, // LEFT
        {-1,-1} // LEFT_UP
    };

	MoverCoord move_coord = game_ai->location();
	int ai_distance = coord_offset_grid(aim_fighter->location(), move_coord);
	for (int i = 1; i < 9; ++i)
	{
		MoverCoord check_coord;
		check_coord.set_pos(move_coord.pos_x() + diff[i][0], move_coord.pos_y() + diff[i][1]);
		if (game_ai->is_movable_path_coord(DynamicMoverCoord(check_coord.pixel_x(), check_coord.pixel_y())) == true)
		{
			int tmp_distance = coord_offset_grid(aim_fighter->location(), check_coord);
			if (tmp_distance < ai_distance)
			{
				game_ai->set_aim_coord(check_coord);
				return true;
			}
		}
	}

	return false;
}

AINearLeaguePre::AINearLeaguePre(int distance)
{
	this->distance_ = distance;
}

bool AINearLeaguePre::AIExternalCond(GameAI* game_ai)
{
	Scene* scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);

	Scene::MoverMap player_map;
	JUDGE_RETURN(scene->fetch_all_around_player(game_ai, player_map, game_ai->location(),
			this->distance_) == 0, false);

	Int64 league_id = game_ai->league_id();
	for (Scene::MoverMap::iterator iter = player_map.begin(); iter != player_map.end(); ++iter)
	{
		MapPlayer* player = dynamic_cast<MapPlayer*>(iter->second);
		JUDGE_CONTINUE(player->league_id() == league_id);
		return true;
	}

	return false;
}


AINearPlayerPre::AINearPlayerPre(int distance)
{
	this->distance_ = distance;
}

bool AINearPlayerPre::AIExternalCond(GameAI* game_ai)
{
	Scene* scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);

	Scene::MoverMap player_map;
	JUDGE_RETURN(scene->fetch_all_around_player(game_ai, player_map, game_ai->location(),
			this->distance_) == 0, false);

	Int64 league_id = game_ai->league_id();
	for (Scene::MoverMap::iterator iter = player_map.begin(); iter != player_map.end(); ++iter)
	{
		MapPlayer* player = dynamic_cast<MapPlayer*>(iter->second);
		JUDGE_CONTINUE(player->role_id() == league_id);
		return true;
	}

	return false;
}

bool AINotSinglePre::AIExternalCond(GameAI* game_ai)
{
	Scene *scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);

	int monster_amount = 0;
	for(Scene::MoverMap::iterator iter = scene->mover_map().begin();
			iter != scene->mover_map().end(); ++iter)
	{
		GameMover* mover = iter->second;
		JUDGE_CONTINUE(mover->is_monster());

		++monster_amount;
        if (monster_amount >= 2)
            return true;
	}

	return false;
}

bool AIPlayerAlivePre::AIExternalCond(GameAI* game_ai)
{
	Scene *scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, false);

	int alive_player_amount = 0;
	for(Scene::MoverMap::iterator iter = scene->player_map().begin();
			iter != scene->player_map().end(); ++iter)
	{
		MapPlayerEx* player = dynamic_cast<MapPlayerEx*>(iter->second);
		JUDGE_CONTINUE(0 != player);
		JUDGE_CONTINUE(!player->is_death());

		++alive_player_amount;
	}

	return alive_player_amount > 0;
}

bool AIDistancePre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, false);

    // 此处会导致ＡＩ不会移动
    MoverCoord &moveto_coord = game_ai->moveto_action_coord();
    if (moveto_coord.pixel_x() > 0 && moveto_coord.pixel_y() > 0 && moveto_coord != game_ai->location())
        return true;

    int distance = 6;
    const Json::Value &prop_json = game_ai->fetch_prop_config();
    if (this->field_name_1().empty() == false && prop_json.isMember(this->field_name_1()))
        distance = prop_json[this->field_name_1()].asInt();
    else if (prop_json.isMember(AIBTCName::AI_DISTANCE_FIELD))
    	distance = prop_json[AIBTCName::AI_DISTANCE_FIELD].asInt();
    
    // BUG 会来回走
    Scene::MoverMap monster_map;
    scene->fetch_all_around_monster(game_ai, monster_map, game_ai->location(), distance, 50);
    for (Scene::MoverMap::iterator iter = monster_map.begin(); iter != monster_map.end(); ++iter)
    {
        GameAI *monster = dynamic_cast<GameAI *>(iter->second);
        if (monster == 0)
            continue;
        if (monster->ai_sort() == game_ai->ai_sort())
            continue;
        
        MoverCoord target_coord = game_ai->fetch_direct_point(game_ai->location(), monster->location(), distance, true);
        DynamicMoverCoord d_target_coord(target_coord.pixel_x(), target_coord.pixel_y());
        if (game_ai->is_movable_path_coord(d_target_coord) == false)
            target_coord = game_ai->chase_coord(target_coord);
        if (target_coord == game_ai->location())
            continue;

        game_ai->set_moveto_action_coord(target_coord);
        return true;
    }

    return false;
}

bool AIIntervalPre::AIExternalCond(GameAI* game_ai)
{
	return (game_ai->ai_detail().__next_interval <= Time_Value::gettimeofday());
}

bool AIWithinDistancePre::AIExternalCond(GameAI* game_ai)
{
	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
	JUDGE_RETURN(prop_monster_json.isMember("within_dist"), false);

	const Json::Value &under_dist_json = prop_monster_json["within_dist"];
	uint under_dist_n = under_dist_json.size();

	bool monster_exists = false;
	for(uint i=1; i<under_dist_n; i+=2)
	{
		int monster_sort = under_dist_json[i-1].asInt();
		int distance = under_dist_json[i].asInt();
		Scene *scene = game_ai->fetch_scene();
		JUDGE_CONTINUE(scene != 0);

		Scene::MoverMap &mover_map = scene->mover_map();
		for(Scene::MoverMap::iterator iter = mover_map.begin(); iter !=mover_map.end(); ++iter)
		{
			GameMover* game_mover = iter->second;
			JUDGE_CONTINUE(game_mover != 0);

			GameAI* other_ai = dynamic_cast<GameAI*>(game_mover);
			JUDGE_CONTINUE(other_ai != 0);
			JUDGE_CONTINUE(other_ai->ai_detail().__sort == monster_sort);
			monster_exists = true;
			JUDGE_RETURN(scene->is_validate_around_mover(other_ai, game_ai->location(), distance), false);
		}
	}

//	if(!monster_exists)
//		return false;
	return true;
}

bool AIBeyondDistancePre::AIExternalCond(GameAI* game_ai)
{
	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
	JUDGE_RETURN(prop_monster_json.isMember("beyond_dist"), false);

	bool monster_exists = false;
	const Json::Value &under_dist_json = prop_monster_json["beyond_dist"];
	uint under_dist_n = under_dist_json.size();
	for(uint i=1; i<under_dist_n; i+=2)
	{
		int monster_sort = under_dist_json[i-1].asInt();
		int distance = under_dist_json[i].asInt();
		Scene *scene = game_ai->fetch_scene();

		Scene::MoverMap &mover_map = scene->mover_map();
		for(Scene::MoverMap::iterator iter = mover_map.begin(); iter !=mover_map.end(); ++iter)
		{
			GameMover* game_mover = iter->second;
			JUDGE_CONTINUE(game_mover != 0);

			GameAI* other_ai = dynamic_cast<GameAI*>(game_mover);
			JUDGE_CONTINUE(other_ai != 0);
			JUDGE_CONTINUE(other_ai->ai_detail().__sort == monster_sort);
			monster_exists = true;
			JUDGE_RETURN(scene->is_validate_around_mover(other_ai, game_ai->location(), distance) == false, false);
		}
	}

	return monster_exists;
}

bool AIAllRoleDistancePre::AIExternalCond(GameAI* game_ai)
{
    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
    int distance = 5;
    if (prop_monster_json.isMember("role_distance"))
        distance = prop_monster_json["role_distance"].asInt();
    if (distance <= 0)
        distance = 5;

    std::vector<MoverCoord> coord_list;
    Scene *scene = game_ai->fetch_scene();
    Scene::MoverMap &player_map = scene->player_map();
    for (Scene::MoverMap::iterator iter = player_map.begin();
            iter != player_map.end(); ++iter)
    {
        GameMover *player = iter->second;
        if (player->is_enter_scene() == false || player->is_active() == false)
            continue;
        coord_list.push_back(player->location());
    }
    if (coord_list.size() <= 0)
        return false;
    game_ai->set_moveto_action_coord(this->generate_distance_point(game_ai, coord_list, distance));
    return true;
}

MoverCoord AIAllRoleDistancePre::generate_distance_point(GameAI *game_ai, std::vector<MoverCoord> &coord_list, const int distance)
{
    MoverCoord target, n_target;
    MoverCoord &ai_point = game_ai->location();
    int t_dist = distance * 30;
    if (coord_list.size() == 1)
    {
        MoverCoord &start_point = coord_list[0];
        double dx = ai_point.pixel_x() - start_point.pixel_x(),
               dy = ai_point.pixel_y() - start_point.pixel_y();
        double len = std::sqrt(dx * dx + dy * dy);
        target.set_pixel(t_dist * dx / len + start_point.pixel_x(),
                t_dist * dy / len + start_point.pixel_y());
        n_target.set_pixel(-(t_dist + len) * dx / len + ai_point.pixel_x(),
                -(t_dist + len) * dy / len + ai_point.pixel_y());
        int i = 0;
        while (i++ < 10)
        {
            if (game_ai->is_movable_coord(target) == true)
                return target;
            if (game_ai->is_movable_coord(n_target) == true)
                return n_target;
            t_dist += 30;
            target.set_pixel(t_dist * dx / len + start_point.pixel_x(),
                    t_dist * dy / len + start_point.pixel_y());
            n_target.set_pixel(-(t_dist + len) * dx / len + ai_point.pixel_x(),
                    -(t_dist + len) * dy / len + ai_point.pixel_y());
        }
        i = 0;
        while (i++ < 10)
        {
            target.set_pixel(start_point.pixel_x() + rand() % (t_dist * 3) - t_dist * 1.5,
                    start_point.pixel_y() + rand() % (t_dist * 3) - t_dist * 1.5);
            if (game_ai->is_movable_coord(target) == true)
                return target;
        }
        return target;
    }
    else
    {
        MoverCoord &p1 = coord_list[0], &p2 = coord_list[1];
        MoverCoord mid_point, check_point;
        mid_point.set_pixel((p1.pixel_x() + p2.pixel_x()) / 2, (p1.pixel_y() + p2.pixel_y()) / 2);
        check_point = mid_point;
        int i = 0, dx = p2.pixel_x() - p1.pixel_x(), dy = p2.pixel_y() - p1.pixel_y(),
            inc_x = p2.pixel_x() + p1.pixel_x(), inc_y = p2.pixel_y() + p1.pixel_y();
        while (coord_offset_grid(check_point, p1) < t_dist && //
                coord_offset_grid(check_point, p2) < t_dist && //
                game_ai->is_movable_coord(check_point) == false)
        {
            if (i++ > 50)
                break;
            int pixel_x = mid_point.pixel_x() + 30 * i;
            int pixel_y = double(inc_x * dx + inc_y * dy) / dy - dx * pixel_x / double(dy);
            check_point.set_pixel(pixel_x, pixel_y);
            if (game_ai->is_movable_coord(check_point) == false)
            {
                pixel_x = mid_point.pixel_x() - 30 * i;
                pixel_y = double(inc_x * dx + inc_y * dy) / dy - dx * pixel_x / double(dy);
                check_point.set_pixel(pixel_x, pixel_y);
            }
        }
        if (game_ai->is_movable_coord(check_point) == false)
        {
            i = 0;
            int index = rand() % coord_list.size();
            MoverCoord &refer_coord = coord_list[index];
            while (i++ < 10)
            {
                check_point.set_pixel(refer_coord.pixel_x() + rand() % (t_dist * 3) - t_dist * 1.5,
                        refer_coord.pixel_y() + rand() % (t_dist * 3) - t_dist * 1.5);
                if (game_ai->is_movable_coord(check_point) == true)
                    return check_point;
            }
        }
        return check_point;
    }
}

bool AIRelAiDiePre::AIExternalCond(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != 0, false);

    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
    JUDGE_RETURN(prop_monster_json.isMember("rel_sort"), false);

    for (uint i = 0; i < prop_monster_json["rel_sort"].size(); ++i)
    {
        int ai_sort = prop_monster_json["rel_sort"][i].asInt();
        BLongSet &ai_set = scene->scene_detail().ai_sort_map_[ai_sort];
        for (BLongSet::iterator ai_iter = ai_set.begin();
                ai_iter != ai_set.end(); ++ai_iter)
        {
            GameAI *rel_ai = scene->find_ai(*ai_iter);
            if (rel_ai == NULL || rel_ai->is_death())
                continue;

            return false;
        }
    }
    return true;
}

ChaseBeyondDistancePre::ChaseBeyondDistancePre(int back_distance)
{
	this->range_ = back_distance;
}

bool ChaseBeyondDistancePre::AIExternalCond(GameAI* game_ai)
{
	if(AIHaveAimPre::AIExternalCond(game_ai))
	{
		int range = this->range_;
		const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
	    if (prop_monster_json.isMember("idle_back_distance"))
	    	range = prop_monster_json["idle_back_distance"].asInt();
	    if (range <= 0)
	    	range = this->range_;
		const Json::Value &patrol_route_json = prop_monster_json[AIBTCName::PATROL_PATH_FIELD];
		JUDGE_RETURN(patrol_route_json.size() > 0, BT::BEV_RS_FAILURE);

		int patrol_index = game_ai->patrol_index();
		JUDGE_RETURN((uint)patrol_index < patrol_route_json.size(), BT::BEV_RS_FAILURE);
		MoverCoord pre_coord;
		pre_coord.set_pixel(patrol_route_json[patrol_index][0u].asInt(),patrol_route_json[patrol_index][1u].asInt());

		int cur_dis = ::coord_offset_grid(game_ai->location(),pre_coord);
		if(cur_dis >= range)
		{
			return true;
		}

	}
    return false;
}

bool QIXIRUNAWAYPre::AIExternalCond(GameAI* game_ai)
{
	return false;
}
