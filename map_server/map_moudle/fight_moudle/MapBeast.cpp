/*
 * MapBeast.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: peizhibi
 */

#include "MapBeast.h"
#include "MapPlayerEx.h"
#include "Scene.h"
#include "MapMonitor.h"
#include "FightField.h"
#include "ProtoDefine.h"

MapBeast::MapBeast(void) : master_(0)
{
	this->beast_detial_.beast_id_ = MAP_MONITOR->generate_beast_id();
}

MapBeast::~MapBeast(void)
{ /*NULL*/ }

int MapBeast::team_id(void)
{
	return 0;
}

int MapBeast::client_sid(void)
{
	return 0;
}

int64_t MapBeast::entity_id(void)
{
	return this->beast_detial_.beast_id_;
}

Int64 MapBeast::league_id(void)
{
	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, 0);
	return player->league_id();
}

int MapBeast::auto_action_timeout(const Time_Value& now_time)
{
//    if (this->is_in_script_mode() == true)
//    {
//    	Scene *scene = this->fetch_scene();
//    	if (scene == 0 || scene->is_started_scene() == false)
//    		return -1;
//    }

	JUDGE_RETURN(this->is_active() == true, -1);
	AutoMapFighter::auto_action_timeout(now_time);

	this->check_and_self_return();
	this->chase_or_attack_fighter();

	return 0;
}

int MapBeast::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
	Proto80400103 appear_info;
	appear_info.set_beast_id(this->beast_id());
	appear_info.set_type(this->beast_detial_.type_);
	appear_info.set_beast_sort(this->beast_sort());
	appear_info.set_beast_speed(this->speed_total_i());
	appear_info.set_master_id(this->beast_detial_.master_id_);

	appear_info.set_mount_shape(1);
	appear_info.set_space_id(this->space_id());
	appear_info.set_toward(this->mover_detial_.__toward);
	this->location().serialize(appear_info.mutable_location());
	this->fetch_other_shape_info(&appear_info);

	ProtoClientHead head;
	head.__recogn = ACTIVE_BEAST_APPEAR;

	return this->make_up_client_block(buff, &head, &appear_info);
}

const char* MapBeast::name()
{
	return this->beast_detial_.beast_name_.c_str();
}

GameFighter* MapBeast::fetch_hurt_figher()
{
	return this->fetch_master();
}

BlockIndexType MapBeast::cur_block_index(void)
{
	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, 0);

	return player->cur_block_index();
}

void MapBeast::push_attack_interval()
{
	static double b_attack_interval = std::max<double>(0.5,
			CONFIG_INSTANCE->const_set("beast_attack_time") / 1000.0);
	static double m_attack_interval = std::max<double>(0.5,
			CONFIG_INSTANCE->const_set("magic_attack_time") / 1000.0);

	if (this->is_a_beast() == true)
	{
		this->push_schedule_time(b_attack_interval);
	}
	else
	{
		this->push_schedule_time(m_attack_interval);
	}
}

void MapBeast::set_cur_location(const MoverCoord& coord)
{
	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, ;);

//	MoverCoord aim_loc = coord;
//	if (this->is_a_beast() == true)
//	{
//		aim_loc = scene->fetch_beast_coord(coord);
//	}

	AutoMapFighter::set_cur_location(coord);
}

double MapBeast::fetch_beast_hurt_rate()
{
//	JUDGE_RETURN(this->is_a_beast() == true, 1);

	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, 1);

	int mount_type = 0;
	if (this->is_a_beast() == true)
	{
		mount_type = GameEnum::FUN_LING_BEAST;
	}
	else
	{
		mount_type = GameEnum::FUN_MAGIC_EQUIP;
	}

	MountDetail& mount_detail = player->mount_detail(mount_type);
	FighterSkill* skill = mount_detail.find_skill(MapMaster::BEAST_SKILL_ADD_HURT);

	double total = 0.08 + 0.02 * mount_detail.mount_grade_;
	if (skill != NULL)
	{
		double percent = GameCommon::div_percent(skill->detail()["percent"].asInt());
		total *= (1 + percent);
	}

	return total;
}

double MapBeast::fetch_beast_hurt_value()
{
	JUDGE_RETURN(this->is_a_beast() == true, 0);

	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, 0);

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_BEAST_MOUNT);
	FighterSkill* skill = mount_detail.find_skill(MapMaster::BEAST_SKILL_ADD_HURT_V);
	JUDGE_RETURN(skill != NULL, 0);

	return skill->detail()["value"].asInt();
}

int MapBeast::enter_scene()
{
	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, -1);

	Scene *scene = player->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	this->fight_detail_.__level = player->level();
	this->fight_detail_.__blood_max.set_single(1000, BasicElement::BASIC);
	this->fight_detail_.__magic_max.set_single(1000, BasicElement::BASIC);
	this->fight_detail_.__blood = this->fight_detail_.__blood_total_i(this);
	this->fight_detail_.__magic = this->fight_detail_.__magic_total_i(this);

	this->init_mover_scene(scene);
	this->mover_detial_.__scene_mode = player->scene_mode();
	this->mover_detial_.__toward = player->mover_detail().__toward;
	this->mover_detial_.__speed.set_single(player->speed_total(), BasicElement::BASIC);
	this->auto_detail_.fetch_skill_mode_ = AutoFighterDetail::SKILL_MODE_QUEUE;
	this->auto_detail_.attack_distance_ = 2 * CONFIG_INSTANCE->const_set("remote_attack_dis");
	this->set_cur_location(player->location());

	this->set_camp_id(player->camp_id());
	this->set_self_owner(player->role_id());

	this->set_pk_state(player->fight_detail().__pk_state);
	this->start_auto_action();

	GameFighter::sign_in();
	this->notify_enter_info();
	return 0;
}

int MapBeast::exit_scene()
{
	this->set_aim_object(0);
	this->stop_auto_action();

	this->notify_exit_info();
	GameFighter::sign_out();

	return 0;
}

int MapBeast::notify_enter_info()
{
	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	scene->register_fighter(this);

    Block_Buffer buff;
    this->make_up_appear_info(&buff);
    return scene->notify_area_info(this, buff, this->room_scene_index());
}

int MapBeast::notify_exit_info()
{
	JUDGE_RETURN(this->is_active() == true, -1);

	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	scene->unregister_fighter(this);

	Block_Buffer buff;
	this->make_up_disappear_info(&buff);

	return scene->notify_area_info(this, buff, this->room_scene_index());
}

int MapBeast::refresh_fight_property(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30400405*, request, -1);
//
//	int offset = request->offset();
//	for (int i = 0; i < request->prop_set_size(); ++i)
//	{
//		const ProtoPairObj& pair_obj = request->prop_set(i);
//		this->refresh_fight_property_item(offset, pair_obj);
//	}

	return 0;
}

int MapBeast::refresh_fight_property_item(int offset, const ProtoPairObj& pair_obj)
{
//	FightDetail& fight_detail = this->fight_detail_;
//
//	switch (pair_obj.obj_id())
//	{
//	case GameEnum::ATTACK:
//	{
//		fight_detail.__attack_lower.set_single(pair_obj.obj_value(), offset);
//		fight_detail.__attack_upper.set_single(pair_obj.obj_value(), offset);
//		break;
//	}
//
//	case GameEnum::HIT:
//	{
//		fight_detail.__hit.set_single(pair_obj.obj_value(), offset);
//		break;
//	}
//
//	case GameEnum::CRIT:
//	{
//		fight_detail.__crit.set_single(pair_obj.obj_value(), offset);
//		break;
//	}
//
//	case GameEnum::BEAST_FORCE:
//	{
//		this->beast_detial_.beast_force_ = pair_obj.obj_value();
//		break;
//	}
//	}

	return 0;
}

int MapBeast::is_a_beast()
{
	return this->beast_detial_.type_ == MapMaster::TYPE_BEAST;
}

int MapBeast::is_attack_beast()
{
	switch(this->beast_detial_.type_)
	{
	case MapMaster::TYPE_BEAST:
	case MapMaster::TYPE_MAGIC:
	{
		return true;
	}
	default:
	{
		return false;
	}
	}
}

int MapBeast::set_beast_offline(MapPlayerEx* player)
{
//	this->beast_detial_.src_beast_id_ = this->beast_id();
//	this->beast_detial_.beast_id_ = MAP_MONITOR->generate_beast_copy_id();
//
//	this->beast_detial_.offline_flag_ = true;
//	this->beast_detial_.master_id_ = player->role_id();
//	this->master_ = player;

	return 0;
}

void MapBeast::reset()
{
	this->master_ = NULL;
	this->reset_auto_fighter();
	this->beast_detial_.reset();
}

void MapBeast::recycle_all_skill_map()
{
	GameFighter::recycle_all_skill_map();
	this->clear_skill_queue();
}

int MapBeast::update_beast_speed()
{
	JUDGE_RETURN(this->is_a_beast() == true, -1);

	MapPlayer* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, -1);

	return this->update_fighter_speed(GameEnum::SPEED, player->speed_total_i());
}

int MapBeast::update_beast_info(int type, int value)
{
	Proto80100103 update_info;
	update_info.set_beast_id(this->beast_id());

	update_info.set_value(value);
	update_info.set_update_type(type);
	return this->respond_to_broad_area(&update_info);
}

Int64 MapBeast::beast_id()
{
	return this->beast_detial_.beast_id_;
}

Int64 MapBeast::master_id()
{
	return this->beast_detial_.master_id_;
}

int MapBeast::beast_sort()
{
	return this->beast_detial_.beast_sort_;
}

MapPlayerEx* MapBeast::fetch_master()
{
	JUDGE_RETURN(this->master_ == NULL, this->master_);

	MapPlayerEx* player = NULL;
	JUDGE_RETURN(this->monitor()->find_player_with_offline(
			this->master_id(), player) == 0, NULL);

	this->master_ = player;
	return this->master_;
}

BeastDetail* MapBeast::fetch_beast_detail()
{
	return &this->beast_detial_;
}

void MapBeast::fetch_prop_map(IntMap& prop_map)
{
//	FightDetail& fight_detail = this->fight_detail();
//
//	prop_map[GameEnum::ATTACK_LOWER] = fight_detail.__attack_lower_total(this);
//	prop_map[GameEnum::ATTACK_UPPER] = fight_detail.__attack_upper_total(this);
//	prop_map[GameEnum::HIT] = fight_detail.__hit_total(this);
//	prop_map[GameEnum::CRIT] = fight_detail.__crit_total(this);
//	prop_map[GameEnum::BEAST_FORCE] = this->beast_detial_.beast_force_;
}

void MapBeast::fetch_prop_map(IDMap& prop_map,int offset)
{
//	FightDetail& fight_detail = this->fight_detail();
//	prop_map[GameEnum::ATTACK_UPPER] = fight_detail.__attack_upper.single(offset);
//	prop_map[GameEnum::HIT] = fight_detail.__hit.single(offset);
//	prop_map[GameEnum::CRIT] = fight_detail.__crit.single(offset);
//	prop_map[GameEnum::ATTACK_MULTI] = fight_detail.__attack_upper_multi.single(offset);
//	prop_map[GameEnum::HIT_MULTI] = fight_detail.__hit_multi.single(offset);
//	prop_map[GameEnum::CRIT_HURT_MULTI] = fight_detail.__crit_hurt_multi.single(offset);
}

void MapBeast::fetch_skill_map(IntVec& skill_set)
{
	FightDetail& fight_detail = this->fight_detail();

	for (SkillMap::iterator iter = fight_detail.__skill_map.begin();
			iter != fight_detail.__skill_map.end(); ++iter)
	{
		skill_set.push_back(iter->first);
	}
}

void MapBeast::fetch_other_shape_info(Proto80400103* respond)
{
	JUDGE_RETURN(this->is_a_beast() == true, ;);

	MapPlayerEx* player = this->fetch_master();
	JUDGE_RETURN(player != NULL, ;);

	int beast_weapon = player->fetch_mount_id(GameEnum::FUN_BEAST_EQUIP);
	int beast_mount = player->fetch_mount_id(GameEnum::FUN_BEAST_MOUNT);
	int beast_wing = player->fetch_mount_id(GameEnum::FUN_BEAST_WING);

    if (GameCommon::is_hiddien_beast_model_scene(this->scene_id()) == false)
    {
	    respond->set_weapon_shape(std::max<int>(beast_weapon, 1));	//武器和宠物一起
	    respond->set_mount_shape(beast_mount);
	    respond->set_beast_wing(beast_wing);
    }

}

int MapBeast::validate_movable(const MoverCoord &step)
{
    int ret = this->validate_fighter_movable();
    if(ret != 0)
    {
    	MSG_DEBUG("ret %d", ret);
    	return ret;
    }

    if (this->is_movable_coord(step) == false &&
    		CONFIG_INSTANCE->is_border_coord(this->scene_id(), step.pos_x(), step.pos_y()) == false)
    {
        return ERROR_COORD_ILLEGAL;
    }
    return 0;
}

int MapBeast::insert_master_status(const int status,
            const double interval, const double last, const int accumulate_times,
    		const double val1, const double val2, const double val3,
            const double val4, const double val5)
{
	MapPlayerEx *master = this->fetch_master();
	JUDGE_RETURN(master != NULL, 0);

	return this->insert_defender_status(master, status, interval, last, accumulate_times,
			val1, val2, val3, val4, val5);
}
