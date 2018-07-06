/*
 * AutoMapFighter.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: peizhibi
 */

#include "AutoMapFighter.h"

#include "MapMonitor.h"
#include "Scene.h"
#include "MapBeast.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "GameAI.h"
#include "MapTeamer.h"

int AutoMapFighter::AutoActionTimer::type(void)
{
	return GTT_MAP_PLAYER;
}

int AutoMapFighter::AutoActionTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->run_timer_ == true, -1);

	this->auto_fighter_->auto_action_timeout(tv);
	JUDGE_RETURN(this->run_timer_ == true, -1);

	Time_Value interval;
	JUDGE_RETURN(this->auto_fighter_->pop_schedule_time(interval) == 0, -1);

	return this->refresh_tick(tv, interval);
}

AutoMapFighter::AutoMapFighter()
{
	// TODO Auto-generated constructor stub
	this->action_timer_.auto_fighter_ = this;
}

AutoMapFighter::~AutoMapFighter()
{
	// TODO Auto-generated destructor stub
}

int AutoMapFighter::auto_action_timeout(const Time_Value& now_time)
{
    return this->auto_launch_guide_skill(now_time);
}

int AutoMapFighter::fetch_default_step()
{
	return 1;
}

int AutoMapFighter::fetch_attack_distance()
{
	if (this->auto_detail_.attack_distance_ > 0)
	{
		return this->auto_detail_.attack_distance_;
	}

	return GameFighter::fetch_attack_distance();
}

int AutoMapFighter::fetch_select_distance()
{
	if (this->auto_detail_.select_distance_ > 0)
	{
		return this->auto_detail_.select_distance_;
	}

	return GameEnum::AI_SELECT_RADIUS;
}

int AutoMapFighter::fetch_max_chase_distance()
{
	return GameEnum::DEFAULT_CHASE_MAX_DISTANCE;
}

void AutoMapFighter::push_attack_interval()
{
	this->push_schedule_time(1.7);
}

int AutoMapFighter::insert_skill_i(int skill_id, int level)
{
	GameFighter::insert_skill_i(skill_id, level);

	FighterSkill* skill = NULL;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);
	JUDGE_RETURN(skill->is_active_skill() == true, -1);

	this->auto_detail_.skill_queue_.push(skill);
	return 0;
}

void AutoMapFighter::reset_auto_fighter()
{
	this->reset_fighter();
	this->auto_detail_.reset();
	this->action_timer_.run_timer_ = false;
	this->action_timer_.cancel_timer();
}

void AutoMapFighter::clear_skill_queue()
{
	this->auto_detail_.skill_queue_.clear();
}

void AutoMapFighter::clear_last_skill(FighterSkill* skill)
{
	JUDGE_RETURN(skill->__lend_skill > 0, ;);

	GameFighter *owner_fighter = this->fetch_self_owner();
	JUDGE_RETURN(owner_fighter != NULL, ;);

	owner_fighter->fight_detail().__last_skill = 0;
}

void AutoMapFighter::insert_sequence_skill(const int skill_id, const int skill_level/*=1*/)
{
	GameFighter::insert_skill(skill_id, skill_level);

	FighterSkill *skill = NULL;
    JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, ;);

	this->auto_detail_.skill_sequence_.push(skill);
}

void AutoMapFighter::start_auto_action(double schedule_time)
{
	this->push_schedule_time(schedule_time);

	this->fighter_timer_.cancel_timer();
	this->fighter_timer_.schedule_timer(0.3);

    this->action_timer_.schedule_timer(schedule_time);
	this->action_timer_.run_timer_ = true;
}

void AutoMapFighter::stop_auto_action()
{
	this->action_timer_.run_timer_ = false;
	this->action_timer_.cancel_timer();
	this->fighter_timer_.cancel_timer();
}

void AutoMapFighter::continue_run_timer()
{
	this->action_timer_.run_timer_ = true;
}

void AutoMapFighter::stop_run_timer()
{
	this->action_timer_.run_timer_ = false;
}

const MoverCoord &AutoMapFighter::aim_coord(void)
{
    return this->auto_detail_.aim_coord_;
}

Int64 AutoMapFighter::aim_object_id(void)
{
    return this->auto_detail_.aim_id_;
}

Int64 AutoMapFighter::last_attacker_id(void)
{
    return this->auto_detail_.last_attacker_id_;
}

Int64 AutoMapFighter::self_owner_id(void)
{
    return this->auto_detail_.owner_id_;
}

int AutoMapFighter::group_id(void)
{
    return this->auto_detail_.group_id_;
}

int AutoMapFighter::is_attack(void)
{
	return this->auto_detail_.is_attack_;
}

int AutoMapFighter::is_arrive_attack_distance(GameFighter* fighter)
{
	int ret = ::check_coord_distance(this->location(), fighter->location(), this->fetch_attack_distance());
//	MSG_USER("[%d, %d] <---> [%d, %d] [%d, %d]", this->location().pixel_x(), this->location().pixel_y(),
//			fighter->location().pixel_x(), fighter->location().pixel_y(), this->fetch_attack_distance(), ret);
	return ret;
}

void AutoMapFighter::set_aim_coord(const MoverCoord& aim_coord)
{
	this->auto_detail_.aim_coord_ = aim_coord;
}

void AutoMapFighter::set_attackor_aim_coord(const MoverCoord& aim_coord)
{
	double delta_x = aim_coord.pixel_x() - this->location().pixel_x();
	double delta_y = aim_coord.pixel_y() - this->location().pixel_y();

	double distance = ::sqrt(delta_x * delta_x + delta_y * delta_y);
	if (distance <= 1)
	{
		this->set_aim_coord(aim_coord);
	}
	else
	{
		double l = (this->fetch_attack_distance() - 1) * 30;

		double x = aim_coord.pixel_x() - l * delta_x / distance;
		double y = aim_coord.pixel_y() - l * delta_y / distance;

		MoverCoord coord;
		coord.set_pixel(x, y);
		this->set_aim_coord(coord);
	}
}

void AutoMapFighter::set_aim_object(Int64 aim_id, const bool is_from_group)
{
	this->auto_detail_.aim_id_ = aim_id;
}

void AutoMapFighter::set_last_attacker(Int64 obj_id)
{
	this->auto_detail_.last_attacker_id_ = obj_id;
}

void AutoMapFighter::set_self_owner(Int64 owner_id)
{
	this->auto_detail_.owner_id_ = owner_id;
}

void AutoMapFighter::set_group_id(const int id)
{
    this->auto_detail_.group_id_ = id;
}

void AutoMapFighter::set_is_attack(const int flag)
{
	this->auto_detail_.is_attack_ = flag;
}

void AutoMapFighter::push_schedule_time(double schedule_time)
{
	this->push_schedule_time(Time_Value::gettime(schedule_time));
}

void AutoMapFighter::push_schedule_time(const Time_Value& schedule_time)
{
	this->auto_detail_.schedule_list_.push_back(schedule_time);
}

// 如果是宠物，则获取主人的ID，其他的不变
Int64 AutoMapFighter::fetch_adjust_fighter_id(Int64 obj_id)
{
	JUDGE_RETURN(GameCommon::fetch_mover_type(obj_id) == MOVER_TYPE_BEAST, obj_id);

	GameFighter* fighter = NULL;
	JUDGE_RETURN(this->find_fighter(obj_id, fighter) == 0, 0);
	JUDGE_RETURN(fighter->is_death() == false, 0);

	MapPlayerEx* player = dynamic_cast<MapBeast*>(fighter)->fetch_master();
	JUDGE_RETURN(player != NULL, 0);

	return player->mover_id();
}

//如果当前没有目标，设置为obj_id. 如果当前已有目标，则不变
void AutoMapFighter::keep_and_set_aim_object(Int64 obj_id)
{
	JUDGE_RETURN(this->auto_detail_.aim_id_ <= 0, ;);

	Int64 adjust_id = this->fetch_adjust_fighter_id(obj_id);
	this->set_aim_object(adjust_id);
}

//不管当前有没有目标，以当前为准
void AutoMapFighter::change_and_set_aim_object(Int64 obj_id)
{
	JUDGE_RETURN(this->auto_detail_.aim_id_ != obj_id, ;);

	Int64 adjust_id = this->fetch_adjust_fighter_id(obj_id);
	this->set_aim_object(adjust_id);
}

void AutoMapFighter::clean_aim_object()
{
	this->set_aim_object(0);
	this->set_last_attacker(0);
}

void AutoMapFighter::chase_or_attack_fighter()
{
	JUDGE_RETURN(this->validate_fighter_movable() == 0, ;);

	GameFighter* fighter = this->fetch_aim_object();
	JUDGE_RETURN(fighter != NULL, ;);

	if (this->is_arrive_attack_distance(fighter) == false)
	{
		this->set_attackor_aim_coord(fighter->location());
		this->schedule_move_fighter();
	}
	else
	{
		this->auto_fighter_attack();
	}
}

void AutoMapFighter::check_and_self_return()
{
//    if (this->is_in_safe_area() == true)
//    {
//	    this->set_aim_object(0);
//	    this->set_last_attacker(0);
//    }

	GameFighter* fighter = this->fetch_self_owner();
	JUDGE_RETURN(fighter != NULL, ;);

	int between_distance = ::coord_offset_grid(this->location(), fighter->location());
	JUDGE_RETURN(between_distance >= GameEnum::DEFAUL_OWNER_DISTANCE, ;);

	this->set_aim_object(0);
	this->set_last_attacker(0);

	this->set_aim_coord(fighter->location());
	this->schedule_move_fighter();
}

int AutoMapFighter::schedule_move_fighter()
{
	JUDGE_RETURN(this->is_beast() == false, -1);
    JUDGE_RETURN(this->guide_skill() == 0, -1);
	JUDGE_RETURN(this->validate_fighter_movable() == 0, -1);

    if (coord_offset_grid(this->aim_coord(), this->location()) > 50)
    {
//        MSG_USER("ERROR aim coord too far %ld %d(%d,%d)->(%d,%d)",
//                this->fighter_id(), this->scene_id(),
//                this->location().pixel_x(), this->location().pixel_y(),
//                this->aim_coord().pixel_x(), this->aim_coord().pixel_y());
        return -1;
    }

    {
    	// 防止跨区域的寻路死循环
    	int fighter_area_id = GameCommon::fetch_area_id_by_coord(this->scene_id(), this->location());
    	int target_area_id = GameCommon::fetch_area_id_by_coord(this->scene_id(), this->aim_coord());
    	JUDGE_RETURN(fighter_area_id == target_area_id, -1);
    }

	Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->generate_move_path(this->auto_detail_.aim_coord_) == 0, -1);

    this->monitor()->move_total_use_ += (Time_Value::gettimeofday() - nowtime);
    JUDGE_RETURN(this->mover_detial_.__step_list.empty() == false, -1);

    MoverCoord &step = *(this->mover_detail().__step_list.rbegin());
    JUDGE_RETURN(step != this->location(), -1);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    this->push_schedule_time(this->mover_detial_.__step_time);
    return scene->refresh_mover_location(this, step);
}

int AutoMapFighter::pop_schedule_time(Time_Value& schedule_time)
{
	JUDGE_RETURN(this->auto_detail_.schedule_list_.empty() == false, -1);

	schedule_time = this->auto_detail_.schedule_list_.back();
	JUDGE_RETURN(this->auto_detail_.schedule_list_.size() > 1, 0);

	this->auto_detail_.schedule_list_.pop_back();
	return 0;
}

int AutoMapFighter::fetch_launch_skill_distance(FighterSkill* fighter_skill)
{
	return 0;
}

int AutoMapFighter::fetch_max_launch_skill_distance()
{
	return 0;
}

int AutoMapFighter::fetch_map_skill_id()
{
	IntVec unrate_set;
	IntPairVec rate_set;

	unrate_set.reserve(this->fight_detail_.__skill_map.size());
	rate_set.reserve(this->fight_detail_.__skill_map.size());

	int total_rate = 0;
	Time_Value now_time = Time_Value::gettimeofday();

	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		FighterSkill* fighter_skill = iter->second;
		JUDGE_CONTINUE(fighter_skill->is_cool_finish() == true);
		JUDGE_CONTINUE(GameCommon::validate_skill_usage(this, fighter_skill) == 0);

        const Json::Value &skill_json = CONFIG_INSTANCE->skill(fighter_skill->__skill_id);
		if (skill_json.isMember("use_rate") == true)
		{
			int cur_rate = GameCommon::json_by_level(skill_json["use_rate"], fighter_skill->__level).asInt();

			total_rate += cur_rate;
			rate_set.push_back(IntPair(iter->first, total_rate));
		}
		else
		{
			unrate_set.push_back(iter->first);
		}
	}

	if (rate_set.empty() == false)
	{
		int rand_rate = std::rand() % total_rate;
		for (IntPairVec::iterator iter = rate_set.begin(); iter != rate_set.end(); ++iter)
		{
			JUDGE_CONTINUE(rand_rate < iter->second);
			return iter->first;
		}
	}
	else if (unrate_set.empty() == false)
	{
		return unrate_set[std::rand() % unrate_set.size()];
	}

    return 0;
}

int AutoMapFighter::fetch_copy_skill_id()
{
	IntMap skill_map;
	IntMap multi_skill_map;

	ThreeObjVec skill_set;
	skill_set.reserve(this->fight_detail_.__skill_map.size());

	Time_Value now_time = Time_Value::gettimeofday();
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		FighterSkill* fighter_skill = iter->second;
		JUDGE_CONTINUE(fighter_skill->is_cool_finish() == true);
		JUDGE_CONTINUE(GameCommon::is_copy_player_skill(fighter_skill) == 1);
		JUDGE_CONTINUE(GameCommon::validate_skill_usage(this, fighter_skill) == 0);

		{
			const Json::Value &skill_json = CONFIG_INSTANCE->skill(fighter_skill->__skill_id);
			JUDGE_CONTINUE(skill_json["passive_trigger"].asInt() <= 0 && skill_json["trigger_when_insert"].asInt() <= 0);
		}

		ThreeObj obj;
		obj.id_ = fighter_skill->__skill_id;
		obj.tick_ = fighter_skill->__use_tick.sec();

		skill_set.push_back(obj);
		skill_map[fighter_skill->__skill_id] = true;

		if (GameCommon::is_multi_target_skill(fighter_skill) == true)
		{
			multi_skill_map[fighter_skill->__skill_id] = true;
		}
	}

	JUDGE_RETURN(skill_set.empty() == false, -1);

//	// left blood
//	static int need_left_blood = CONFIG_INSTANCE->copy_player("left_blood").asInt();
//	if (this->validate_left_blood(need_left_blood) == true)
//	{
//		for (IntVec::iterator iter = MAP_MONITOR->blood_skill_set_.begin();
//				iter != MAP_MONITOR->blood_skill_set_.end(); ++iter)
//		{
//			JUDGE_CONTINUE(skill_map.count(*iter) > 0);
//			FighterSkill* fighter_skill = NULL;
//			JUDGE_CONTINUE(this->find_skill(*iter, fighter_skill) == 0);
//			return *iter;
//		}
//	}

	// around
	if (multi_skill_map.empty() == false)
	{
		Scene* scene = this->fetch_scene();
		JUDGE_RETURN(scene != NULL, -1);

		static uint around_count = CONFIG_INSTANCE->copy_player("around_count").asInt();
		static uint around_distance = CONFIG_INSTANCE->copy_player("around_distance").asInt();

		Scene::MoverMap fighter_map;
		scene->fetch_all_around_fighter(this, fighter_map, this->location(),
				around_distance, around_count);

		// multi target
		if (fighter_map.size() >= around_count)
		{
			return multi_skill_map.begin()->first;
		}
	}

	// normal
	int unrate_skill = 0;
	std::sort(skill_set.begin(), skill_set.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = skill_set.begin(); iter != skill_set.end(); ++iter)
	{
		FighterSkill* fighter_skill = NULL;
		JUDGE_CONTINUE(this->find_skill(iter->id_, fighter_skill) == 0);

        const Json::Value &skill_json = CONFIG_INSTANCE->skill(fighter_skill->__skill_id);
		if (skill_json.isMember("use_rate") == true)
		{
            int use_rate = GameCommon::json_by_level(skill_json["use_rate"], fighter_skill->__level).asInt();
			JUDGE_CONTINUE(std::rand() % 100 < use_rate);
			return iter->id_;
		}
		else if (unrate_skill == 0)
		{
			unrate_skill = iter->id_;
		}
	}

	return unrate_skill;
}

int AutoMapFighter::fetch_queue_skill_id()
{
    int skill_id = 0;
    SkillVec skill_vc;

	Time_Value now_time = Time_Value::gettimeofday();
    while (this->auto_detail_.skill_queue_.size() > 0)
    {
    	FighterSkill* skill = this->auto_detail_.skill_queue_.top();
        JUDGE_BREAK(now_time >= skill->__use_tick);

        this->auto_detail_.skill_queue_.pop();
        skill_vc.push_back(skill);

        int ret = this->validate_auto_skill_usage(skill);
        JUDGE_CONTINUE(ret == 0);

		skill_id = skill->__skill_id;
		skill->add_use_tick();
		break;
    }

    for (SkillVec::iterator iter = skill_vc.begin(); iter != skill_vc.end(); ++iter)
    {
        this->auto_detail_.skill_queue_.push(*iter);
    }

    return skill_id;
}

int AutoMapFighter::fetch_beast_skill_id()
{
	IntVec unrate_set;
	IntPairVec rate_set;

	unrate_set.reserve(this->fight_detail_.__skill_map.size());
	rate_set.reserve(this->fight_detail_.__skill_map.size());

	int total_rate = 0;
	for (SkillMap::iterator iter = this->fight_detail_.__skill_map.begin();
			iter != this->fight_detail_.__skill_map.end(); ++iter)
	{
		FighterSkill* fighter_skill = iter->second;
		JUDGE_CONTINUE(fighter_skill->is_cool_finish() == true);
		JUDGE_CONTINUE(GameCommon::validate_skill_usage(this, fighter_skill) == 0);

		if (fighter_skill->__use_rate > 0)
		{
			total_rate += fighter_skill->__use_rate;
			rate_set.push_back(IntPair(iter->first, total_rate));
		}
		else
		{
			unrate_set.push_back(iter->first);
		}
	}

	total_rate = std::max<int>(GameEnum::DAMAGE_ATTR_PERCENT, total_rate);
	int rand_rate = std::rand() % total_rate;

	for (IntPairVec::iterator iter = rate_set.begin(); iter != rate_set.end(); ++iter)
	{
		JUDGE_CONTINUE(rand_rate < iter->second);

		FighterSkill* fighter_skill = NULL;
		JUDGE_CONTINUE(this->find_skill(iter->first, fighter_skill) == 0);

		return iter->first;
	}

	if (unrate_set.empty() == false)
	{
		return unrate_set[std::rand() % unrate_set.size()];
	}

    return 0;
}

int AutoMapFighter::fetch_sequence_skill_id()
{
	JUDGE_RETURN(this->auto_detail_.skill_sequence_.empty() == false, 0);

	FighterSkill *skill = this->auto_detail_.skill_sequence_.front();
	this->auto_detail_.skill_sequence_.pop();

	this->auto_detail_.skill_sequence_.push(skill);
	return skill->__skill_id;
}

//随机设定目标
int AutoMapFighter::rand_set_aim_object(const MoverMap& fighter_map)
{
	JUDGE_RETURN(fighter_map.empty() == false, false);

	int rand_index = ::rand() % fighter_map.size(), i = 0;
	for (MoverMap::const_iterator iter = fighter_map.begin();
			iter != fighter_map.end(); ++iter, ++i)
	{
		JUDGE_CONTINUE(i == rand_index);
		this->set_aim_object(iter->first);
		return true;
	}

	return false;
}

int AutoMapFighter::auto_fighter_attack()
{
	JUDGE_RETURN(this->validate_fighter_movable() == 0, -1);

	int skill_id = this->fetch_auto_skill_id();
	JUDGE_RETURN(skill_id > 0, -1);

	return this->auto_fighter_launch_skill(skill_id);
}

int AutoMapFighter::auto_fighter_launch_skill(int skill_id)
{
    JUDGE_RETURN(this->guide_skill() <= 0, -1);
    JUDGE_RETURN(this->mutual_skill() <= 0, -1);

	FighterSkill *skill = 0;
	JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);

	Proto10400201 req;
	int ret = this->make_up_skill_request(&req, skill);
	if( ret != 0)
	{
		MSG_DEBUG("make_up_skill_request() return %d", ret);
		return -1;
	}

	ret = this->prepare_fight_skill(&req);
	this->clear_last_skill(skill);

	if (ret == 0)
	{
		this->push_attack_interval();
	}

	if (skill->__is_launch_once)
	{
		this->auto_detail_.skill_queue_.remove(skill);
		this->remove_skill(skill->__skill_id);
	}

	return ret;
}

int AutoMapFighter::make_up_skill_request(Proto10400201* request, FighterSkill* fighter_skill)
{
	const Json::Value &skill_json = fighter_skill->conf();

	int skill_id = fighter_skill->__skill_id;
	int skill_level = fighter_skill->__level;

    //技能步数
    request->set_skill_id(skill_id);
	request->set_skill_level(skill_level);

    switch (fighter_skill->__aoe_type)
    {
	case GameEnum::SKILL_AOE_CUR_AIM_TARGET:
	case GameEnum::SKILL_AOE_TARGET_RECT:
	{
		GameFighter *aim_fighter = this->fetch_aim_object();
		JUDGE_RETURN(aim_fighter != NULL, -1);

		request->set_skill_pixel_x(aim_fighter->location().pixel_x());
		request->set_skill_pixel_y(aim_fighter->location().pixel_y());
		request->add_target_list()->set_target_id(aim_fighter->fighter_id());
		break;
	}
	case GameEnum::SKILL_AOE_SELF_TARGET:
	{
		request->set_skill_pixel_x(this->location().pixel_x());
		request->set_skill_pixel_y(this->location().pixel_y());
		request->add_target_list()->set_target_id(this->fighter_id());
		break;
	}
	case GameEnum::SKILL_AOE_SELF_OWNER_TARGET:
	{
		GameFighter* owner_fighter = this->fetch_self_owner();
		JUDGE_RETURN(GameCommon::validate_fighter(owner_fighter) == true, -1);

		request->set_skill_pixel_x(owner_fighter->location().pixel_x());
		request->set_skill_pixel_y(owner_fighter->location().pixel_y());
		request->add_target_list()->set_target_id(owner_fighter->fighter_id());
		break;
	}
	case GameEnum::SKILL_AOE_TARGET_CIRCLE:
	{
		GameFighter* aim_fighter = this->fetch_aim_object();
		JUDGE_RETURN(aim_fighter != NULL, -1);

		Scene *scene = this->fetch_scene();
		JUDGE_RETURN(scene != NULL, -1);

		request->set_skill_pixel_x(aim_fighter->location().pixel_x());
		request->set_skill_pixel_y(aim_fighter->location().pixel_y());

//		bool is_mapping = GameCommon::json_by_level(skill_json["mapping_coord"], skill_level).asBool();

		Scene::MoverMap fighter_map;
		scene->fetch_all_around_fighter(this, fighter_map, aim_fighter->location(),
									fighter_skill->__radius, fighter_skill->__object);

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			request->add_target_list()->set_target_id(iter->first);
		}

		break;
	}
	case GameEnum::SKILL_AOE_SELF_CIRCLE:
	{
		request->set_skill_pixel_x(this->location().pixel_x());
		request->set_skill_pixel_y(this->location().pixel_y());

		int radius = std::max(GameCommon::json_by_level(skill_json["radius"], skill_level).asInt(), 4);
		int object = GameCommon::json_by_level(skill_json["object"], skill_level).asInt();
		bool is_mapping = GameCommon::json_by_level(skill_json["mapping_coord"], skill_level).asBool();

		Scene::MoverMap fighter_map;
		Scene *scene = this->fetch_scene();
		if (scene != 0)
		{
			if(is_mapping)
			{
				if (radius > 30)    // 全屏技能只攻击玩家
					scene->fetch_all_around_player_mapping(this, fighter_map, this->location(),
							radius, GameEnum::CLIENT_MAPPING_FACTOR, object);
				else
					scene->fetch_all_around_fighter_mapping(this, fighter_map, this->location(),
							radius, GameEnum::CLIENT_MAPPING_FACTOR, object);
			}
			else
			{
				if (radius > 30)    // 全屏技能只攻击玩家
					scene->fetch_all_around_player(this, fighter_map, this->location(), radius, object);
				else
					scene->fetch_all_around_fighter(this, fighter_map, this->location(), radius, object);
			}
		}

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			request->add_target_list()->set_target_id(iter->first);
		}
		break;
	}
	case GameEnum::SKILL_AOE_SELF_RING:
	{
		request->set_skill_pixel_x(this->location().pixel_x());
		request->set_skill_pixel_y(this->location().pixel_y());

		int outer_radius = std::max(GameCommon::json_by_level(skill_json["outer_radius"], skill_level).asInt(), 4);
		int inner_radius = std::max(GameCommon::json_by_level(skill_json["inner_radius"], skill_level).asInt(), 4);
		if (outer_radius <= inner_radius)
			outer_radius = inner_radius + 4;

		int object = GameCommon::json_by_level(skill_json["object"], skill_level).asInt();

		Scene::MoverMap fighter_map;
		Scene *scene = this->fetch_scene();
		if (scene != 0)
		{
			scene->fetch_all_around_player_in_ring(this, fighter_map, this->location(), inner_radius, outer_radius, object);
		}

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			request->add_target_list()->set_target_id(iter->first);
		}
		break;
	}
	case GameEnum::SKILL_AOE_SELF_SECTOR:
	{
		request->set_skill_pixel_x(this->location().pixel_x());
		request->set_skill_pixel_y(this->location().pixel_y());

		int radius = std::max(GameCommon::json_by_level(skill_json["radius"], skill_level).asInt(), 4);
		int object = GameCommon::json_by_level(skill_json["object"], skill_level).asInt();

		Scene::MoverMap fighter_map;
		Scene *scene = this->fetch_scene();
		if (scene != 0)
			scene->fetch_all_sector_fighter(this, fighter_map, this->location(), this->location(), radius, 60, object);

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			request->add_target_list()->set_target_id(iter->first);
		}
		break;
	}

	case GameEnum::SKILL_AOE_SELF_RECT:
	{
		MoverCoord skill_coord = this->rect_skill_coord();
		if (skill_coord.pixel_x() == 0 && skill_coord.pixel_y() == 0)
		{
			GameFighter *aim_fighter = this->fetch_aim_object();
			if (aim_fighter != NULL)
				skill_coord = aim_fighter->location();
		}
		request->set_skill_pixel_x(skill_coord.pixel_x());
		request->set_skill_pixel_y(skill_coord.pixel_y());

		int  object = GameCommon::json_by_level(skill_json["object"], skill_level).asInt(),
			width = GameCommon::json_by_level(skill_json["width"], skill_level).asInt() * 30,
			height = GameCommon::json_by_level(skill_json["height"], skill_level).asInt() * 30;
		double angle = this->mover_detial_.__toward * PI / 180;
		if (skill_coord.pixel_x() != this->location().pixel_x() || skill_coord.pixel_y() != this->location().pixel_y())
		{
			this->fight_skill_.set_radian(vector_to_radian(this->location(), skill_coord));
		}

		Scene::MoverMap fighter_map;
		Scene *scene = this->fetch_scene();
		if (scene != 0)
			scene->fetch_all_rect_fighter(this, fighter_map, skill_coord, angle, width, height, object);

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			request->add_target_list()->set_target_id(iter->first);
		}

		break;
	}
	case GameEnum::SKILL_AOE_SCENE_LEFT_SIDE:
	case GameEnum::SKILL_AOE_SCENE_RIGHT_SIDE:
	{
		request->set_skill_pixel_x(this->location().pixel_x());
		request->set_skill_pixel_y(this->location().pixel_y());

		int radius = std::max(GameCommon::json_by_level(skill_json["radius"], skill_level).asInt(), 4);
		int object = GameCommon::json_by_level(skill_json["object"], skill_level).asInt();

		Scene::MoverMap fighter_map;
		Scene *scene = this->fetch_scene();

		if (scene != 0)
		{   // 技能只攻击玩家
			scene->fetch_all_around_player(this, fighter_map, this->location(), radius, object);
		}

		for (Scene::MoverMap::iterator iter = fighter_map.begin();
				iter != fighter_map.end(); ++iter)
		{
			GameMover* mover = iter->second;
			int mover_side = mover->location().pixel_x() - this->location().pixel_x();
			JUDGE_CONTINUE((fighter_skill->__aoe_type == GameEnum::SKILL_AOE_SCENE_LEFT_SIDE) == (mover_side < 0));

			request->add_target_list()->set_target_id(iter->first);
		}
		break;
	}
	case GameEnum::SKILL_AOE_BOSS:
	{
		Scene *scene = this->fetch_scene();
		JUDGE_RETURN(scene != 0, -1);

		int distance = std::max(GameCommon::json_by_level(skill_json["distance"], skill_level).asInt(), 4);

		Scene::MoverMap boss_map;
		JUDGE_RETURN(scene->fetch_all_around_boss(this, boss_map, this->location(), distance) == 0, -1);

		int i = 0;
		for (Scene::MoverMap::iterator iter = boss_map.begin();
				iter != boss_map.end(); ++iter)
		{
			GameMover *mover = iter->second;
			if (i++ == 0)
			{
				request->set_skill_pixel_x(mover->location().pixel_x());
				request->set_skill_pixel_y(mover->location().pixel_y());
			}
			request->add_target_list()->set_target_id(iter->first);
		}
		break;
	}
	case GameEnum::SKILL_AOE_RANK_HURT_LIST:
	{
		GameAI *ai = dynamic_cast<GameAI *>(this);
		JUDGE_RETURN(ai != NULL, -1);

		Int64 role_id = 0;
		AIDetail::PlayerHurtMap &hurt_map = ai->hurt_map();
		int loop = 0;
		while (role_id == 0 && (loop++) < 5)
		{
			int rand_value = rand() % hurt_map.size(), i = 0;
			for (AIDetail::PlayerHurtMap::iterator iter = hurt_map.begin(); iter != hurt_map.end(); ++iter)
			{
				if ((i++) < rand_value)
					continue;

				MapPlayerEx *player = this->find_player(iter->first);
				JUDGE_CONTINUE(player != NULL && player->is_death() == false && player->is_enter_scene());

				role_id = iter->first;

				request->set_skill_pixel_x(player->location().pixel_x());
				request->set_skill_pixel_y(player->location().pixel_y());
				request->add_target_list()->set_target_id(role_id);
				break;
			}
		}
		break;
	}
	default:
	{
		break;
	}
    }

    if (request->target_list_size() <= 0 && skill_json["voidHurt"].asInt() == 0 && skill_json["no_object_limit"].asInt() == 0)
    {
    	return -1;
    }

    return 0;
}

int AutoMapFighter::fetch_auto_skill_id()
{
	switch (this->auto_detail_.fetch_skill_mode_)
	{
	default:
	case AutoFighterDetail::SKILL_MODE_QUEUE:
	{
		return this->fetch_queue_skill_id();
	}
	case AutoFighterDetail::SKILL_MODE_MAP:
	{
		return this->fetch_map_skill_id();
	}
	case AutoFighterDetail::SKILL_MODE_BEAST:
	{
		return this->fetch_beast_skill_id();
	}
	case AutoFighterDetail::SKILL_MODE_COPY:
	{
		return this->fetch_copy_skill_id();
	}
	case AutoFighterDetail::SKILL_MOCE_SEQUENCE:
	{
		return this->fetch_sequence_skill_id();
	}
	}
}

int AutoMapFighter::validate_auto_skill_usage(FighterSkill* fighter_skill)
{
    JUDGE_RETURN(fighter_skill != NULL, -1);
//    JUDGE_RETURN(this->guide_skill() <= 0, -1);
    JUDGE_RETURN(this->mutual_skill() <= 0, -1);
    JUDGE_RETURN(this->is_active() == true, -1);

	if (fighter_skill->__use_rate > 0)
	{
		JUDGE_RETURN(std::rand() % GameEnum::DAMAGE_ATTR_PERCENT < fighter_skill->__use_rate, -1);
	}

	return GameCommon::validate_skill_usage(this, fighter_skill);
}

GameFighter* AutoMapFighter::fetch_aim_object()
{
	JUDGE_RETURN(this->auto_detail_.aim_id_ > 0, NULL);

	GameFighter* aim_fighter = NULL;
	if (this->find_fighter(this->auto_detail_.aim_id_, aim_fighter) != 0)
	{
		this->set_aim_object(0);
		return NULL;
	}

	if (aim_fighter->is_death() == true)
	{
		this->set_aim_object(0);
		return NULL;
	}

	return aim_fighter;
}

GameFighter* AutoMapFighter::fetch_self_owner()
{
	JUDGE_RETURN(this->auto_detail_.owner_id_ > 0, NULL);

	GameFighter* self_owner = NULL;
	if (this->find_fighter(this->auto_detail_.owner_id_, self_owner) != 0)
	{
		this->set_self_owner(0);
		return NULL;
	}

	return self_owner;
}

GameFighter *AutoMapFighter::fetch_last_attacker(void)
{
	Int64 id = this->auto_detail_.last_attacker_id_;
	JUDGE_RETURN(id > 0, NULL);

    Scene* scene = this->fetch_scene();
    if (scene == NULL)
    {
    	this->set_last_attacker(0);
    	return NULL;
    }

    GameFighter* last_attacker = scene->find_fighter(id);
    if (last_attacker == NULL || last_attacker->is_death() == true)
    {
    	this->set_last_attacker(0);
    	return NULL;
    }

    return last_attacker;
}

const MoverCoord &AutoMapFighter::rect_skill_coord(void)
{
	return this->auto_detail_.rect_skill_coord_;
}

void AutoMapFighter::set_rect_skill_coord(const MoverCoord &coord)
{
	this->auto_detail_.rect_skill_coord_ = coord;
}

const Time_Value &AutoMapFighter::guide_tick(void)
{
    return this->auto_detail_.guide_tick_;
}

int AutoMapFighter::guide_skill(void)
{
    return this->auto_detail_.guide_skill_;
}

int AutoMapFighter::auto_launch_guide_skill(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->guide_skill() > 0, -1);
    JUDGE_RETURN(this->guide_tick() <= nowtime, 0);

    this->notify_guid_range_effect_disappear();

    int skill_id = this->guide_skill();
    
    FighterSkill *skill = 0;
    JUDGE_RETURN(this->find_skill(skill_id, skill) == 0, -1);

    if (this->auto_detail_.fetch_skill_mode_ == AutoFighterDetail::SKILL_MODE_QUEUE)
    {
        this->auto_detail_.skill_queue_.remove(skill);
    }

    this->auto_detail_.guide_skill_ = 0;
    this->auto_detail_.guide_tick_ = Time_Value::zero;

    Proto10400201 req;
    this->make_up_skill_request(&req, skill);
    this->prepare_fight_skill(&req);
    this->clear_last_skill(skill);

    if(skill->__is_launch_once)
    	this->remove_skill(skill->__skill_id);
    else if (this->auto_detail_.fetch_skill_mode_ == AutoFighterDetail::SKILL_MODE_QUEUE)
        this->auto_detail_.skill_queue_.push(skill);

    return 0;
}

bool AutoMapFighter::is_guide_skill(FighterSkill *skill)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill->__skill_id);

    int guide_tick = GameCommon::json_by_level(skill_json["guide_tick"], skill->__level).asInt();
    JUDGE_RETURN(guide_tick > 0, false);

    return true;
}

int AutoMapFighter::check_launch_pre_skill(FighterSkill *skill)
{
    const Json::Value &detail_json = this->detail_conf(skill->__skill_id, skill->__level);
    const Json::Value &pre_launch_json = detail_json["pre_launch_skill"];

    if(pre_launch_json.isInt())
    {
    	return this->auto_fighter_launch_skill(pre_launch_json.asInt());
    }
    else if(pre_launch_json.isArray())
    {
    	uint pre_skill_n = pre_launch_json.size();
    	for(uint i=0; i<pre_skill_n; ++i)
    	{
    		this->auto_fighter_launch_skill(pre_launch_json[i].asInt());
    	}
    }

    return 0;
}
void AutoMapFighter::update_guide_skill(FighterSkill *skill)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill->__skill_id);
    int guide_tick = GameCommon::json_by_level(skill_json["guide_tick"], skill->__level).asInt();
    
    this->auto_detail_.guide_skill_ = skill->__skill_id;
    this->auto_detail_.guide_tick_ = Time_Value::gettimeofday() + Time_Value(guide_tick);

    if (skill_json.isMember("notify_range_effect") &&
    		GameCommon::json_by_level(skill_json["notify_range_effect"], skill->__level).asInt() > 0)
    {
    	this->auto_detail_.guide_range_effect_sort_ = GameCommon::json_by_level(skill_json["notify_range_effect"], skill->__level).asInt();
    	this->notify_guid_range_effect_appear();
    }

    if (GameCommon::json_by_level(skill_json["notify_guide"], skill->__level).asInt() >= 1)
    {
		Proto80400213 respond;
		respond.set_skill_id(this->guide_skill());
		respond.set_guide_tick_sec(guide_tick);
		respond.set_fighter_id(this->mover_id());
		if (GameCommon::json_by_level(skill_json["notify_guide"], skill->__level).asInt() == 2)
			respond.set_no_view_text(1);
		this->respond_to_broad_area(&respond);
    }
}

void AutoMapFighter::stop_guide_skill(void)
{
    JUDGE_RETURN(this->guide_skill() > 0, ;);

//    const Json::Value &skill_json = CONFIG_INSTANCE->skill(this->guide_skill());
//    JUDGE_RETURN(GameCommon::json_by_level(skill_json["can_stop_guide"], 1).asInt() == 1, ;);
//
//    this->auto_detail_.guide_skill_ = 0;
//    this->auto_detail_.guide_tick_ = Time_Value::zero;
//    this->notify_guid_range_effect_disappear();
//
//    Proto80400218 respond;
//    respond.set_fighter_id(this->mover_id());
//    this->respond_to_broad_area(&respond);
}

void AutoMapFighter::stop_guide_skill_by_skill(const int skill_id)
{
	JUDGE_RETURN(this->guide_skill() > 0, ;);

//	const Json::Value &skill_json = CONFIG_INSTANCE->skill(this->guide_skill());
//	JUDGE_RETURN(skill_json.isMember("stop_guide_by_skill"), ;);
//
//	JUDGE_RETURN(GameCommon::is_value_in_config(skill_json["stop_guide_by_skill"], skill_id), ;);
//
//    this->auto_detail_.guide_skill_ = 0;
//    this->auto_detail_.guide_tick_ = Time_Value::zero;
//    this->notify_guid_range_effect_disappear();
//
//    Proto80400218 respond;
//    respond.set_fighter_id(this->mover_id());
//    this->respond_to_broad_area(&respond);
}

void AutoMapFighter::notify_guid_range_effect_appear(void)
{
	Proto80400106 req;
	req.set_effect_id(this->fighter_id() + 200000000);
	req.set_effect_sort(this->auto_detail_.guide_range_effect_sort_);
	req.set_pixel_x(this->location().pixel_x());
	req.set_pixel_y(this->location().pixel_y());
	this->respond_to_broad_area(&req);
}

void AutoMapFighter::notify_guid_range_effect_disappear(void)
{
	if (this->auto_detail_.guide_range_effect_sort_ > 0)
	{
    	Proto80400107 respond;
    	respond.set_effect_id(this->fighter_id() + 200000000);
    	respond.set_effect_sort(this->auto_detail_.guide_range_effect_sort_);
    	this->respond_to_broad_area(&respond);
    	this->auto_detail_.guide_range_effect_sort_ = 0;
	}
}

int AutoMapFighter::make_up_appear_range_effect(Block_Buffer *buff)
{
	JUDGE_RETURN(this->auto_detail_.guide_range_effect_sort_ > 0, 0);

    ProtoClientHead head;
    head.__recogn = ACTIVE_EFFECT_APPEAR;

	Proto80400106 respond;
	respond.set_effect_id(this->fighter_id() + 200000000);
	respond.set_effect_sort(this->auto_detail_.guide_range_effect_sort_);
	respond.set_pixel_x(this->location().pixel_x());
	respond.set_pixel_y(this->location().pixel_y());
	return this->make_up_client_block(buff, &head, &respond);
}

int AutoMapFighter::make_up_disappear_range_effect(Block_Buffer *buff)
{
	JUDGE_RETURN(this->auto_detail_.guide_range_effect_sort_ > 0, 0);

    ProtoClientHead head;
    head.__recogn = ACTIVE_EFFECT_DISAPPEAR;

	Proto80400107 respond;
	respond.set_effect_id(this->fighter_id() + 200000000);
	respond.set_effect_sort(this->auto_detail_.guide_range_effect_sort_);
	return this->make_up_client_block(buff, &head, &respond);
}

int AutoMapFighter::make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate)
{
	this->make_up_appear_range_effect(buff);
	return 0;
}
int AutoMapFighter::make_up_disappear_other_info(Block_Buffer *buff, const bool send_by_gate)
{
	this->make_up_disappear_range_effect(buff);
	return 0;
}

int AutoMapFighter::process_skill_note(int skill_id, const Json::Value& detail_json)
{
	JUDGE_RETURN(detail_json.isMember("skill_note"), 0);
	const Json::Value &note_json = detail_json["skill_note"];
	JUDGE_RETURN( note_json["times_range"].size() >= 2
					&& note_json["times_range"][1u].asInt() > 0
					&& note_json["text_id"].size() > 0, 0);

	AutoFighterDetail &detail = this->auto_detail_;
	if(detail.skill_note_countdown_.count(skill_id) == 0 ||
		detail.skill_note_countdown_.find(skill_id)->second <= 0)
	{
		int range_l = note_json["times_range"][0u].asInt();
		int range_r = note_json["times_range"][1u].asInt();
		// 闭区间: [range_l, range_r], 每x次播放一次
		int times = ::rand()%(range_r-range_l+1)+range_l;
		detail.skill_note_countdown_[skill_id] = times;
		int size = note_json["text_id"].size();
		int text_id = note_json["text_id"][::rand()%size].asInt();

		Proto80400219 res;
		res.set_text_id(text_id);
//		MSG_DEBUG("Proto80400219: %s", res.Utf8DebugString().c_str());
		this->respond_to_broad_area(&res);
	}
	detail.skill_note_countdown_[skill_id]--;
	return 0;
}

int AutoMapFighter::die_process(const int64_t fighter_id)
{
//	if (this->guide_skill() > 0)
//	{
//	    this->notify_guid_range_effect_disappear();
//	    this->auto_detail_.guide_skill_ = 0;
//		this->auto_detail_.guide_tick_ = Time_Value::zero;
//	}

	int ret = GameFighter::die_process(fighter_id);
	return ret;
}

int AutoMapFighter::process_monster_talk(int skill_id, int mover_id)
{
	const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
	JUDGE_RETURN(skill_json.isMember("monster_talk") == true, -1);

	Proto80400234 respond;
	respond.set_mover_id(mover_id);
	respond.set_note(skill_json["monster_talk"].asString());
	return this->respond_to_broad_area(&respond, 1);
}
