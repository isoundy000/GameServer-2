/*
 * AIBasicAction.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: peizhibi
 */

#include "AIBasicAction.h"

#include "Scene.h"
#include "GameAI.h"

#include "AIManager.h"
#include "AIBTCName.h"
#include "ProtoDefine.h"

int BaseAIAction::DoExecute(NodeOutputParam &output, NodeInputParam &input)
{
	GameAI* game_ai = input.game_ai_;
	JUDGE_RETURN(game_ai != NULL, BT::BEV_RS_FINISH);

	int ret = this->DoAIExecute(game_ai);
	this->dump_info(game_ai);

	return ret;
}

void BaseAIAction::dump_info(GameAI* game_ai)
{
	static int debug_flag = CONFIG_INSTANCE->bt_factory()["debug_ai"].asInt();
	JUDGE_RETURN(debug_flag == true, ;);

    Scene* scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, ;);

	MSG_USER("AI Action Name %s, Scene: %d, Monster Sort: %d, Tree Name: %s",
			this->node_name_.c_str(), scene->scene_id(), game_ai->ai_sort(),
			game_ai->tree_name().c_str());
}

/*
 * die action
 * */
AIDieAction::AIDieAction(double die_time)
{
	this->die_time_ = die_time;
}

int AIDieAction::DoAIExecute(GameAI* game_ai)
{
	MSG_USER("die monster %d, %ld %d %d(%d,%d)", game_ai->layout_index(),
			game_ai->mover_id(), game_ai->ai_sort(), game_ai->scene_id(),
			game_ai->location().pixel_x(), game_ai->location().pixel_y());

	if (game_ai->is_boss())
	{
		game_ai->push_schedule_time(0.2);
	}
	else
	{
		game_ai->push_schedule_time(this->die_time_);
	}

	return BT::BEV_RS_FINISH;
}

/*
 * recycle action
 * */
int AIRecycleAction::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->fight_detail().left_relive_time(1) == 0, BT::BEV_RS_FINISH);

	MSG_USER("recycle monster %d %ld %d %d(%d,%d)", game_ai->space_id(),
			game_ai->mover_id(), game_ai->ai_sort(), game_ai->scene_id(),
			game_ai->location().pixel_x(), game_ai->location().pixel_y());

	if (game_ai->is_death() == false)
	{
		game_ai->modify_blood_by_fight(game_ai->fight_detail().__blood_total_i(game_ai));
	}

	Scene* scene = game_ai->fetch_scene();
	if (scene != NULL)
	{
		scene->handle_boss_die_action(game_ai);
	}

	game_ai->exit_scene();
	game_ai->sign_out();

	return BT::BEV_RS_FINISH;
}

int AIAliveRecycleAction::DoAIExecute(GameAI* game_ai)
{
	Scene *scene = game_ai->fetch_scene();
	if (scene != NULL)
	{
		scene->handle_ai_alive_recycle(game_ai);
	}

	game_ai->exit_scene();
	game_ai->sign_out();

	return BT::BEV_RS_FINISH;
}

/*
 * select action
 * */
int AISelectAction::DoAIExecute(GameAI* game_ai)
{
	return BT::BEV_RS_FINISH;
}

/*
 * sleep action
 * */
AISleepAction::AISleepAction(int sleep_time)
{
	this->sleep_time_ = sleep_time;
}

int AISleepAction::DoAIExecute(GameAI* game_ai)
{
	game_ai->clean_aim_object();
	game_ai->push_schedule_time(this->sleep_time_);
	return BT::BEV_RS_FINISH;
}

int AIPauseAction::DoAIExecute(GameAI* game_ai)
{
	game_ai->push_schedule_time(1);
	return BT::BEV_RS_FINISH;
}

/*
 * auto action
 * */
int AIAutoAction::DoAIExecute(GameAI* game_ai)
{
	this->ai_speak_action(game_ai);
	return BT::BEV_RS_FINISH;
}

int AIAutoAction::ai_speak_action(GameAI* game_ai)
{
	return 0;
}

/*
 * move action
 * */
int AIMoveAction::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, BT::BEV_RS_FINISH);

	game_ai->schedule_move_fighter();
	return BT::BEV_RS_FINISH;
}

/*
 * ai chase
 * */
int AIChaseAction::DoAIExecute(GameAI* game_ai)
{
	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, BT::BEV_RS_FAILURE);

	const MoverCoord &aim_coord = game_ai->aim_coord();
	DynamicMoverCoord dynamic_aim(aim_coord.pixel_x(), aim_coord.pixel_y());
	if (coord_offset_grid(aim_coord, aim_fighter->location()) <= GameEnum::DEFAULT_USE_SKILL_DISTANCE
			&& game_ai->is_movable_path_coord(dynamic_aim) == true)
	{
		game_ai->schedule_move_fighter();
		return BT::BEV_RS_FINISH;
	}

    MoverCoord chase_coord = game_ai->chase_coord(aim_fighter->location());

//    int between_distance = ::coord_offset_grid(game_ai->location(), chase_coord);
//    int max_chase_distance = GameEnum::DEFAULT_CHASE_MAX_DISTANCE;
//
//    const Json::Value &prop_json = game_ai->fetch_prop_config();
//	if (prop_json.isMember(AIBTCName::MAX_CHASE_RADIUS_FIELD))
//	{
//		max_chase_distance = prop_json[AIBTCName::MAX_CHASE_RADIUS_FIELD].asInt();
//	}
//	else if (max_chase_distance < game_ai->fetch_select_distance())
//	{
//		max_chase_distance = game_ai->fetch_select_distance();
//	}
//
//    if (between_distance > max_chase_distance)
//    {
//    	game_ai->clean_aim_object();
//    	return BT::BEV_RS_FAILURE;
//    }

	game_ai->set_aim_coord(chase_coord);
	game_ai->schedule_move_fighter();


	return BT::BEV_RS_FINISH;
}

int AIDirectChaseAction::DoAIExecute(GameAI* game_ai)
{
	GameFighter* aim_fighter = game_ai->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != NULL, BT::BEV_RS_FAILURE);

    int between_distance = ::coord_offset_grid(game_ai->location(), aim_fighter->location());
    if (between_distance > GameEnum::DEFAULT_CHASE_MAX_DISTANCE)
    {
    	game_ai->set_aim_object(0);
    	return BT::BEV_RS_FAILURE;
    }

	game_ai->set_aim_coord(aim_fighter->location());
	game_ai->schedule_move_fighter();

    return BT::BEV_RS_FINISH;
}

/*
 * ai patrol
 * */
AIRandMoveAction::AIRandMoveAction(int rand_radius)
{
	this->rand_radius_ = rand_radius;
}

int AIRandMoveAction::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false,
			BT::BEV_RS_FINISH);

    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != NULL, BT::BEV_RS_FAILURE);

    if (game_ai->born_type() == GameName::AREA_BORN)
    {
    	MoverCoord aim_coord = AIMANAGER->fetch_ai_area_coord(game_ai, this->rand_radius_);
    	JUDGE_RETURN(aim_coord != game_ai->location(), BT::BEV_RS_FINISH);

    	game_ai->set_aim_coord(aim_coord);
    }
    else
    {
        MoverCoord aim_coord = scene->rand_ai_move_coord(game_ai);
        JUDGE_RETURN(aim_coord != game_ai->location(), BT::BEV_RS_FINISH);

    	game_ai->set_aim_coord(aim_coord);
    }

	return AIMoveAction::DoAIExecute(game_ai);
}

/*
 * ai back
 * */
AIBackAction::AIBackAction(void)
{
	this->set_action_type_flag(GameEnum::AAT_BACK);
}

int AIBackAction::DoAIExecute(GameAI* game_ai)
{
	if (game_ai->ai_is_back() == true)
	{
		return this->check_and_restore(game_ai);
	}
	else
	{
		game_ai->set_ai_back(true);
		game_ai->clean_aim_object();

		game_ai->set_aim_coord(game_ai->birth_coord());
		return AIMoveAction::DoAIExecute(game_ai);
	}
}

int AIBackAction::check_and_restore(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->ai_is_back() == true, BT::BEV_RS_FINISH);

	if (this->is_back_finish(game_ai) == true)
	{
		game_ai->set_ai_back(false);
		game_ai->fighter_check_and_restore_all();
		return BT::BEV_RS_FINISH;
	}
	else
	{
		return AIMoveAction::DoAIExecute(game_ai);
	}
}

int AIBackAction::is_back_finish(GameAI* game_ai)
{
	Scene* scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, true);

	DynamicMoverCoord cur_loc(game_ai->location().pixel_x(), game_ai->location().pixel_y());
	DynamicMoverCoord bir_loc(game_ai->birth_coord().pixel_x(), game_ai->birth_coord().pixel_y());

	// back finish
	if (cur_loc == bir_loc || game_ai->location() == game_ai->birth_coord())
	{
		return true;
	}

	if (scene->is_movable_dynamic_coord(bir_loc) == true)
	{
		return false;
	}

	// occupied
	int total_size = ARRAY_LENGTH(Scene::SELECT_POINT);

	int loc_x = bir_loc.dynamic_pos_x();
	int loc_y = bir_loc.dynamic_pos_y();

	IntPairVec pair_set;
	pair_set.reserve(total_size);

	for (int i = 0; i < total_size; ++i)
	{
		pair_set.push_back(IntPair(loc_x + Scene::SELECT_POINT[i][0] ,
				loc_y + Scene::SELECT_POINT[i][1]));
	}

	for (IntPairVec::iterator iter = pair_set.begin(); iter != pair_set.end(); ++iter)
	{
		DynamicMoverCoord new_bir;
		new_bir.set_dynamic_pos(iter->first, iter->second);
		JUDGE_CONTINUE(scene->is_movable_dynamic_coord(new_bir) == true);

		MoverCoord new_coord;
		new_coord.set_pixel(new_bir.pixel_x(), new_bir.pixel_y());
		JUDGE_CONTINUE(GameCommon::is_movable_coord(scene->scene_id(), new_coord) == true);

		// set new birth
		game_ai->birth_coord() = new_coord;
		game_ai->set_aim_coord(new_coord);

		return false;
	}

	return true;
}

/*
 * ai idle back
 * */
AIIdleBackAction::AIIdleBackAction(void)
{
	this->set_action_type_flag(GameEnum::AAT_IDLE_BACK);
}

int AIIdleBackAction::DoAIExecute(GameAI* game_ai)
{
	if (this->is_idle_back_finish(game_ai))
	{
		return BT::BEV_RS_FINISH;
	}
	else
	{
		return AIMoveAction::DoAIExecute(game_ai);
	}
}

bool AIIdleBackAction::is_idle_back_finish(GameAI* game_ai)
{
	Scene* scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, true);

	DynamicMoverCoord cur_loc(game_ai->location().pixel_x(), game_ai->location().pixel_y());
	DynamicMoverCoord bir_loc(game_ai->birth_coord().pixel_x(), game_ai->birth_coord().pixel_y());

	// back finish
	if (cur_loc == bir_loc || game_ai->location() == game_ai->birth_coord())
	{
		game_ai->ai_detail().__extend_back_distance = 0;
		return true;
	}

	if (scene->is_movable_dynamic_coord(bir_loc) == false)
	{
		game_ai->ai_detail().__extend_back_distance = 0;
		return true;	// 出生点不可走？ 那就待在这里吧
	}

	return false;
}

/*
 * ai attack aim fighter
 * */
int AIAttackAction::DoAIExecute(GameAI* game_ai)
{
	if (game_ai->auto_fighter_attack() != 0)
	{
		JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, BT::BEV_RS_FINISH);
		JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_ATTACK) == false, BT::BEV_RS_FINISH);
		game_ai->schedule_move_fighter();
	}
	return BT::BEV_RS_FINISH;
}

/*
 * ai patrol route
 * */
int AIPatrolRouteAction::DoAIExecute(GameAI* game_ai)
{
    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
    const Json::Value &patrol_route_json = prop_monster_json[AIBTCName::PATROL_PATH_FIELD];
    if (patrol_route_json.size() <= 0)
        return BT::BEV_RS_FAILURE;

    int patrol_index = game_ai->patrol_index() % patrol_route_json.size();
    MoverCoord aim_coord;
    aim_coord.set_pixel(patrol_route_json[patrol_index][0u].asInt(),
            patrol_route_json[patrol_index][1u].asInt());
    int offset = coord_offset_grid(aim_coord, game_ai->aim_coord());
    DynamicMoverCoord ai_aim_coord(game_ai->aim_coord().pixel_x(), game_ai->aim_coord().pixel_y()),
    		ai_location(game_ai->location().pixel_x(), game_ai->location().pixel_y());
    if (offset > (GameEnum::DEFAULT_USE_SKILL_DISTANCE + 2) || (game_ai->is_movable_path_coord(ai_aim_coord) == false && ai_aim_coord != ai_location))
    	game_ai->set_aim_coord(game_ai->chase_coord(aim_coord));

    ai_aim_coord.set_dynamic_pixel(game_ai->aim_coord().pixel_x(), game_ai->aim_coord().pixel_y());
    if (ai_location != ai_aim_coord)
    {
	    return AIMoveAction::DoAIExecute(game_ai);
    }

    patrol_index = (patrol_index + 1) % patrol_route_json.size();
    const Json::Value &prop_json = game_ai->fetch_prop_config();
    if (prop_json["patrol_no_loop"].asInt() == 1)
    {
    	if (patrol_index > 0)
    	{
    		game_ai->set_patrol_index(patrol_index);
    	}
    	else
    	{
    		if (prop_json["patrol_finish_recycle"].asInt() == 1)
    		{
    		      game_ai->exit_scene();
    		      game_ai->sign_out();
    		}
    	}
    }
    else
    {
    	game_ai->set_patrol_index(patrol_index);
    }

    return BT::BEV_RS_FINISH;
}

/*
 * ai patrol route
 * */
int AIPatrolRouteBAction::DoAIExecute(GameAI* game_ai)
{
    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
    const Json::Value &patrol_route_json = prop_monster_json[AIBTCName::PATROL_PATH_FIELD];
    JUDGE_RETURN(patrol_route_json.size() > 0, BT::BEV_RS_FAILURE);

    int patrol_index = game_ai->patrol_index();
    JUDGE_RETURN((uint)patrol_index < patrol_route_json.size(), BT::BEV_RS_FAILURE);

    DynamicMoverCoord ai_aim_coord(patrol_route_json[patrol_index][0u].asInt(),
            patrol_route_json[patrol_index][1u].asInt());
    DynamicMoverCoord ai_location(game_ai->location().pixel_x(),
            game_ai->location().pixel_y());

    if (ai_location != ai_aim_coord)
    {
        MoverCoord aim_coord;
        aim_coord.set_pixel(ai_aim_coord.pixel_x(), ai_aim_coord.pixel_y());

        game_ai->set_aim_coord(aim_coord);
        return AIMoveAction::DoAIExecute(game_ai);
    }

    if (prop_monster_json["patrol_no_loop"].asInt() == 0)
    {
    	patrol_index = (patrol_index + 1) % patrol_route_json.size();
		game_ai->set_patrol_index(patrol_index);
    }
    else
    {
    	patrol_index += 1;
    	game_ai->set_patrol_index(patrol_index);
    }

    return BT::BEV_RS_FINISH;
}

int AIFollowAction::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, BT::BEV_RS_FINISH);

	Int64 caller_id = game_ai->caller();
	GameFighter *caller=0;
	game_ai->find_fighter(caller_id, caller);
	if(caller != 0)
		game_ai->set_aim_coord(caller->location());
    return AIMoveAction::DoAIExecute(game_ai);
}

int AIKeepAwayAction::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, BT::BEV_RS_FINISH);

	game_ai->schedule_move_fighter_no_dynamic_mpt();
	return BT::BEV_RS_FINISH;
}

int AIMovetoAction::DoAIExecute(GameAI* game_ai)
{
    game_ai->set_aim_coord(game_ai->moveto_action_coord());
    return AIMoveAction::DoAIExecute(game_ai);
}

int AIComboAttackAction::DoAIExecute(GameAI* game_ai)
{
	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
	const Json::Value &skill_list = prop_monster_json[AIBTCName::AI_COMBO_SKILL_LIST];
	uint skill_n = skill_list.size();

	if(game_ai->ai_detail().__combo_tag >= (int) skill_n)
	{
		game_ai->ai_detail().__combo_tag = 0;
		return BT::BEV_RS_FINISH;
	}

	int skill_id = skill_list[game_ai->ai_detail().__combo_tag].asInt();
	FighterSkill *skill = 0;
	JUDGE_RETURN(game_ai->find_skill(skill_id, skill) == 0, BT::BEV_RS_EXECUTING);// 如果不存在技能则跳过
	Time_Value now_time = Time_Value::gettimeofday();
	//JUDGE_RETURN(now_time >= skill->__use_tick, BT::BEV_RS_EXECUTING);// 技能已经冷却
	JUDGE_RETURN(game_ai->auto_fighter_launch_skill(skill_id) == 0, BT::BEV_RS_EXECUTING);// 使用技能

	game_ai->ai_detail().__combo_tag += 1;
	return BT::BEV_RS_EXECUTING;
}

int AIRamdonComboAttackAction::DoAIExecute(GameAI* game_ai)
{
	if(game_ai->ai_detail().__random_combo_skill.size() <= 0)
	{
		const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
		const Json::Value &skill_list_json = prop_monster_json[AIBTCName::AI_RANDOM_COMBO_SKILL_LIST];
		IntVec skill_set = GameCommon::random_int_vector_from_config(skill_list_json);

		int skill_set_n = skill_set.size();
		if(skill_set_n <=0)
			return BT::BEV_RS_EXECUTING;

		for(int i=0; i<skill_set_n; ++i)
		{
			game_ai->ai_detail().__random_combo_skill.push(skill_set[i]) ;
		}

		return BT::BEV_RS_FINISH;
	}

	int skill_id = game_ai->ai_detail().__random_combo_skill.front();
	FighterSkill *skill = 0;
	JUDGE_RETURN(game_ai->find_skill(skill_id, skill) == 0, BT::BEV_RS_EXECUTING);// 如果不存在技能则跳过
	Time_Value now_time = Time_Value::gettimeofday();
	//JUDGE_RETURN(now_time >= skill->__use_tick, BT::BEV_RS_EXECUTING);// 技能已经冷却
	JUDGE_RETURN(game_ai->auto_fighter_launch_skill(skill_id) == 0, BT::BEV_RS_EXECUTING);// 使用技能

	game_ai->ai_detail().__random_combo_skill.pop();

	return BT::BEV_RS_EXECUTING;
}

int AINormalAttackAction::DoAIExecute(GameAI* game_ai)
{
//	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
//
//	int skill_id = 0;
//	if(prop_monster_json.isMember(AIBTCName::AI_NORMAL_SKILL))
//	{
//		skill_id = prop_monster_json[AIBTCName::AI_NORMAL_SKILL].asInt();
//	}
//	else
//	{
//		for(int i=0; i<100; ++i)
//		{
//			skill_id = game_ai->fetch_auto_skill_id();
//
//			if(GameCommon::json_int_array_exists(prop_monster_json[AIBTCName::AI_COMBO_SKILL_LIST],skill_id) ||
//					GameCommon::json_int_array_exists(prop_monster_json[AIBTCName::AI_RANDOM_COMBO_SKILL_LIST], skill_id))
//			{
//				skill_id = 0;
//				continue;
//			}
//
//			break;
//		}
//	}
//
//	JUDGE_RETURN(skill_id > 0, BT::BEV_RS_EXECUTING);
//
//	FighterSkill *skill = 0;
//	JUDGE_RETURN(game_ai->find_skill(skill_id, skill) == 0, BT::BEV_RS_EXECUTING);
//	game_ai->insert_auto_skill(skill);
//
//	game_ai->auto_fighter_launch_skill(skill_id);
	return BT::BEV_RS_FINISH;
}

int AIIntervalFinAction::DoAIExecute(GameAI* game_ai)
{
	game_ai->ai_detail().__next_interval =
			Time_Value::gettimeofday() + game_ai->ai_detail().__interval;

	return BT::BEV_RS_FINISH;
}

int AIAddBuffAction::DoAIExecute(GameAI* game_ai)
{
	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
    const Json::Value *json = NULL;
    if (this->field_name_1().empty())
    {
        if (prop_monster_json.isMember(AIBTCName::AI_ADD_BUFF))
            json = &(prop_monster_json[AIBTCName::AI_ADD_BUFF]);
    }
    else
    {
        if (prop_monster_json.isMember(this->field_name_1()))
            json = &(prop_monster_json[this->field_name_1()]);
    }
	if(json != NULL)
	{
		const Json::Value &buff_json = *json;
		uint buff_n = buff_json.size();
		for(uint i= 0; i<buff_n; ++i)
		{
		    BasicStatus *clean_status = 0;
		    int status_id = buff_json[i]["buff"].asInt();
//		    int view_status_id = buff_json[i]["view_buff"].asInt();
		    int last_time = buff_json[i]["last"].asInt();
		    int percent = buff_json[i]["percent"].asInt();
		    int value = buff_json[i]["value"].asInt();

		    if (game_ai->find_status(status_id, clean_status) != 0)
		    {
				game_ai->insert_defender_status(game_ai, status_id, 0, last_time, 0, percent, value);
		    }
		}
	}

	return BT::BEV_RS_FINISH;
}

int AIRemoveBuffAction::DoAIExecute(GameAI* game_ai)
{
	return 0;
}

int AIRelAddSkillAction::DoAIExecute(GameAI* game_ai)
{
    Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != 0, BT::BEV_RS_FINISH);

//    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
//    if (prop_monster_json.isMember("rel_sort"))
//    {
//        for (uint i = 0; i < prop_monster_json["rel_sort"].size(); ++i)
//        {
//            int ai_sort = prop_monster_json["rel_sort"][i].asInt();
//            BLongSet &ai_set = scene->scene_detail().ai_sort_map_[ai_sort];
//            for (BLongSet::iterator ai_iter = ai_set.begin();
//                    ai_iter != ai_set.end(); ++ai_iter)
//            {
//                GameAI *rel_ai = scene->find_ai(*ai_iter);
//                if (rel_ai == 0 || rel_ai->is_death())
//                    continue;
//
//                for (uint j = 0; j < prop_monster_json["rel_add_skill"].size(); ++j)
//                {
//                    int skill_id = prop_monster_json["rel_add_skill"][i].asInt();
//                    rel_ai->insert_auto_skill(skill_id);
//                }
//            }
//        }
//    }
    return BT::BEV_RS_FINISH;
}

int AIAddSkillAction::DoAIExecute(GameAI* game_ai)
{
//    JUDGE_RETURN(game_ai != NULL, BT::BEV_RS_FINISH);
//
//    const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
//    JUDGE_RETURN(prop_monster_json.isMember("add_skill"), BT::BEV_RS_FINISH);
//
//    for (uint i = 0; i < prop_monster_json["add_skill"].size(); ++i)
//    {
//        game_ai->insert_auto_skill(prop_monster_json["add_skill"][i].asInt());
//    }

    return BT::BEV_RS_FINISH;
}

int AIGenGiftboxAction::DoAIExecute(GameAI* game_ai)
{
    game_ai->generate_gift_box();

    return BT::BEV_RS_FINISH;
}

LimitDisChaseAction::LimitDisChaseAction(int back_distance)
{
	this->range_ = back_distance;
}

int LimitDisChaseAction::DoAIExecute(GameAI* game_ai)
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
		return BT::BEV_RS_FINISH;;
	}

    return AIChaseAction::DoAIExecute(game_ai);
}

int ReturnPatrolAction::DoAIExecute(GameAI* game_ai)
{
	const Json::Value &prop_monster_json = game_ai->fetch_prop_config();
	const Json::Value &patrol_route_json = prop_monster_json[AIBTCName::PATROL_PATH_FIELD];
	JUDGE_RETURN(patrol_route_json.size() > 0, BT::BEV_RS_FAILURE);

	int patrol_index = game_ai->patrol_index();
	JUDGE_RETURN((uint)patrol_index < patrol_route_json.size(), BT::BEV_RS_FAILURE);
	MoverCoord pre_coord;
	pre_coord.set_pixel(patrol_route_json[patrol_index][0u].asInt(),patrol_route_json[patrol_index][1u].asInt());

	game_ai->set_aim_coord(pre_coord);
	if(game_ai->last_attacker_id() == game_ai->aim_object_id())
	{
		game_ai->set_last_attacker(0);
	}
	game_ai->set_aim_object(0);
	return AIMoveAction::DoAIExecute(game_ai);
}

int NotifyBossRemoveAttr::DoAIExecute(GameAI* game_ai)
{
	Int64 call_id = game_ai->caller();
    GameFighter *caller = 0;
    if (call_id > 0 && game_ai->find_fighter(call_id, caller) == 0)
    {
    	BasicStatus *status;
        if(caller->find_status(BasicStatus::ATTACK, status) == 0 && status != NULL)
        {
//        	caller->reduce_status_effect(status->__status);
        	int& accumulate_tims = status->__accumulate_tims;
        	if(accumulate_tims > 0)
        	{
        		status->__value2 -=  status->__value2 / accumulate_tims;
        		if(std::abs(status->__value2) < 0.000001)
        		{
        			status->__value2 = 0;
        		}
        		accumulate_tims--;
        	}
        	caller->refresh_single_status(status);
        	caller->refresh_all_status_property();
        }
    }
    return BT::BEV_RS_FINISH;
}

int NotifyShowBoss::DoAIExecute(GameAI* game_ai)
{
	Int64 call_id = game_ai->caller();
    GameFighter *caller = 0;
    if (call_id > 0 && game_ai->find_fighter(call_id, caller) == 0)
    {
        GameFighter *caller = 0;
        GameAI* caller_ai = 0;
        Int64 hide_ai_id = 0, show_ai_id = 0;
        Proto80400235 respond;
        if (call_id > 0 && game_ai->find_fighter(call_id, caller) == 0 && caller != NULL)
        {
			caller_ai = dynamic_cast<GameAI *>(caller);
			if(caller_ai != NULL)
			{
				show_ai_id = caller_ai->ai_id();
				respond.set_show_ai_id(show_ai_id);
				respond.set_hide_ai_id(hide_ai_id);
				respond.set_flag(2);
				caller_ai->respond_to_broad_area(&respond, 1);
				caller_ai->reset_ai_flag();
			}
        }
    }
    return BT::BEV_RS_FINISH;
}

int MonsterRunAwayInQiXi::DoAIExecute(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->test_ai_flag(GameEnum::AI_CF_NO_MOVE) == false, BT::BEV_RS_FINISH);

	Scene *scene = game_ai->fetch_scene();
    JUDGE_RETURN(scene != 0, BT::BEV_RS_FINISH);
    DynamicMoverCoord dynamic_coord;
    dynamic_coord.set_dynamic_pixel(game_ai->ai_detail().__qixi_aim_coord.pixel_x(),game_ai->ai_detail().__qixi_aim_coord.pixel_y());
	if(game_ai->is_movable_path_coord(dynamic_coord) == true)
	{
    	game_ai->set_aim_coord(game_ai->ai_detail().__qixi_aim_coord);
    	int length = CONFIG_INSTANCE->tiny("length").asInt();
		if(length < 0)
		   length = 1140;
		int max_path = length / 30 /2;
    	if (coord_offset_grid(game_ai->aim_coord(), game_ai->location()) < max_path)
    	{
    		game_ai->schedule_move_fighter();
    		return BT::BEV_RS_FINISH;
    	}
	}
	return BT::BEV_RS_FAILURE;
}

int MonsterPatrolInQiXi::DoAIExecute(GameAI* game_ai)
{
	//GameCommon::fetch_rand_qixi_coord(target_coord,game_ai->location(),game_ai->birth_coord(),game_ai->scene_id());
	Scene *scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL,false);
	int length = CONFIG_INSTANCE->tiny("length").asInt();
	if(length < 0)
	   length = 1140;
	int max_path = length / 30 /2;

	int diff_distance = 0;
	int limit_rang = 0;
	int loops_times = 5000;
	MoverCoord target_coord;
	while (loops_times--)
	{
		target_coord = scene->rand_coord(game_ai->location(),max_path);
		limit_rang = coord_offset_grid(target_coord,game_ai->birth_coord());
		diff_distance = coord_offset_grid(target_coord,game_ai->location());

		if (GameCommon::is_movable_coord_no_border(game_ai->scene_id(), target_coord) == true && limit_rang < max_path &&
				diff_distance <= 8 && diff_distance >= 6)
		{
			break;
		}
	}
	if(loops_times <= 0)
	{
		target_coord.set_pixel(game_ai->location().pixel_x(),game_ai->location().pixel_y());
	}

	JUDGE_RETURN(target_coord != game_ai->location(), BT::BEV_RS_FAILURE);
	DynamicMoverCoord dynamic_coord;
	dynamic_coord.set_dynamic_pixel(target_coord.pixel_x(),target_coord.pixel_y());
	if(game_ai->is_movable_path_coord(dynamic_coord) == true)
	{
		game_ai->set_aim_coord(target_coord);

		if (coord_offset_grid(game_ai->aim_coord(), game_ai->location()) < max_path)
		{
			game_ai->schedule_move_fighter();
			return BT::BEV_RS_FINISH;
		}
	}
	return BT::BEV_RS_FAILURE;
	//return AIMoveAction::DoAIExecute(game_ai);
}
