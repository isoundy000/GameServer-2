/*
 * MapMaster.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: peizhibi
 */

#include "MapMaster.h"
#include "MapBeast.h"
#include "GameField.h"
#include "Scene.h"
#include "DBCommon.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"

#include <mongo/client/dbclient.h>

MapMaster::MapMaster(void)
{
	// TODO Auto-generated constructor stub
}

MapMaster::~MapMaster(void)
{
	// TODO Auto-generated destructor stub
}

int MapMaster::process_skill_post_effect(int skill_id)
{
	JUDGE_RETURN(GameCommon::fetch_skill_id_fun_type(skill_id) == GameEnum::SKILL_FUN_RAMA, -1);

	MapPlayerEx* player = this->self_player();
	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_BEAST_EQUIP);

	FighterSkill* skill = mount_detail.find_skill(MapMaster::BEAST_EQUIP_BUQU);
	JUDGE_RETURN(skill != NULL, -1);

	return this->direct_luanch_skill_effect(skill, this);
}

double MapMaster::fetch_sub_skill_use_rate(FighterSkill* skill)
{
	JUDGE_RETURN(skill->__sub_rate_skill > 0, 0);

	MapPlayerEx* player = this->self_player();
	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_BEAST_EQUIP);

	FighterSkill* sub_skill = mount_detail.find_skill(skill->__sub_rate_skill);
	JUDGE_RETURN(sub_skill != NULL, 0);

	return sub_skill->detail()["percent"].asInt();
}

DoublePair MapMaster::fetch_reduce_crit_hurt_pair()
{
	DoublePair pair;
	JUDGE_RETURN(this->is_blood_full() == false, pair);

	MountDetail& mount_detail = this->self_player()->mount_detail(GameEnum::FUN_BEAST_WING);
	JUDGE_RETURN(mount_detail.open_ == true, pair);

	static int SKILL_SET[] = {BEAST_SKILL_REDUCE_CRIT, BEAST_SKILL_REDUCE_CRIT_RATE};
	int left_percent = this->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);

	pair.first = mount_detail.left_blood_value(SKILL_SET[0], left_percent, 0);
	pair.second = mount_detail.left_blood_value(SKILL_SET[1], left_percent, 1);

	return pair;
}

double MapMaster::fetch_reduce_hurt_rate()
{
	MountDetail& mount_detail = this->self_player()->mount_detail(GameEnum::FUN_BEAST_WING);
	JUDGE_RETURN(mount_detail.open_ == true, 0);

	int left_percent = this->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);
	return mount_detail.left_blood_value(BEAST_SKILL_REUDCE_HURT_RATE, left_percent, 1);
}

double MapMaster::fetch_increase_crit_hurt_rate(GameFighter* fighter)
{
	MountDetail& mount_detail = this->self_player()->mount_detail(GameEnum::FUN_BEAST_MAO);
	JUDGE_RETURN(mount_detail.open_ == true, 0);

	int left_percent = fighter->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);
	return mount_detail.left_blood_value(BEAST_SKILL_ADD_CRIT_RATE, left_percent, 1);
}

double MapMaster::fetch_increase_hurt_rate(GameFighter* fighter)
{
	MountDetail& mount_detail = this->self_player()->mount_detail(GameEnum::FUN_BEAST_MAO);
	JUDGE_RETURN(mount_detail.open_ == true, 0);

	int left_percent = fighter->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);
	return mount_detail.left_blood_value(BEAST_SKILL_ADD_HURT_RATE, left_percent, 1);
}

void MapMaster::reset_master()
{
	MapMaster::master_sign_out();
	this->beast_unrecycle_ = false;
	this->beast_skill_blood_.stop_time();
    this->beast_skill_blood_.sub1_ = MapMaster::BEAST_SKILL_ADD_BLOOD;
}

void MapMaster::start_beast_skill_blood()
{
	MapPlayerEx* player = this->self_player();
	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_LING_BEAST);

	FighterSkill* skill = mount_detail.find_skill(this->beast_skill_blood_.sub1_);
	JUDGE_RETURN(skill != NULL, ;);

	const Json::Value& detail = skill->detail();
	this->beast_skill_blood_.sub2_ = detail["percent"].asInt();

	JUDGE_RETURN(this->beast_skill_blood_.start_ == false, ;);
	this->beast_skill_blood_.start_interval(detail["interval"].asInt());
}

void MapMaster::check_beast_skill_timeout()
{
	JUDGE_RETURN(this->is_death() == false, ;);
	JUDGE_RETURN(this->is_blood_full() == false, ;);

	int ret = this->beast_skill_blood_.update_time();
	JUDGE_RETURN(ret == true, ;);

	MapBeast* a_beast = this->fetch_cur_beast(MapMaster::TYPE_BEAST);
	JUDGE_RETURN(a_beast != NULL, ;);

	MountDetail& mount_detail = this->self_player()->mount_detail(GameEnum::FUN_LING_BEAST);
	FighterSkill* skill = mount_detail.find_skill(this->beast_skill_blood_.sub1_);
	JUDGE_RETURN(skill != NULL, ;);

	int add_blood = this->fetch_max_blood(this->beast_skill_blood_.sub2_);
	this->modify_blood_by_fight(-1 * add_blood, FIGHT_TIPS_SKILL, 0,
			this->beast_skill_blood_.sub1_);

	a_beast->notify_launch_skill(skill, this);
	this->beast_skill_blood_.reset_time();
}

void MapMaster::trigger_left_blood_skill(Int64 attackor)
{
	JUDGE_RETURN(this->is_death() == false, ;);
	JUDGE_RETURN(this->is_blood_full() == false, ;);

	int left_percent = this->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);
	this->trigger_beast_magic_skill(left_percent);
	this->trigger_role_shield_appear(left_percent);
}

void MapMaster::trigger_beast_magic_skill(int left_percent)
{
	static int MAGIC_SKILL[] = {MAGIC_SKILL_HUDU, MAGIC_SKILL_HUTI,
			MAGIC_SKILL_LINGQIAO, MAGIC_SKILL_JULI};
	static int TOTAL_SIZE = ARRAY_LENGTH(MAGIC_SKILL);

	MapPlayerEx* player = this->self_player();
	JUDGE_RETURN(player != NULL, ;);

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_MAGIC_EQUIP);
	JUDGE_RETURN(mount_detail.open_ == true, ;);

	MapBeast* b_beast = player->fetch_cur_beast(MapMaster::TYPE_MAGIC);
	JUDGE_RETURN(b_beast != NULL, ;);

	for (int i = 0; i < TOTAL_SIZE; ++i)
	{
		int skill_id = MAGIC_SKILL[i];

		FighterSkill* skill = mount_detail.find_skill(skill_id);
		JUDGE_CONTINUE(skill != NULL);

		const Json::Value& detail_conf = skill->detail();
		int buff_id	= detail_conf["id"].asInt();
		int blood_per = detail_conf["blood_per"].asInt();

		if (left_percent > blood_per)
		{
			this->remove_status(buff_id);
		}
		else
		{
			JUDGE_CONTINUE(this->is_have_status(buff_id) == false);
			b_beast->direct_luanch_skill_effect(skill, this);
		}
	}
}

void MapMaster::trigger_role_shield_appear(int left_percent)
{
	MapPlayerEx* player = this->self_player();
	JUDGE_RETURN(player != NULL, ;);
	JUDGE_RETURN(player->is_lrf_change_mode() == false, ;);

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_TIAN_GANG);
	JUDGE_RETURN(mount_detail.open_ == true, ;);

	FighterSkill* skill = mount_detail.find_skill(TIAN_GANG_SKILL_SHIELD);
	JUDGE_RETURN(skill != NULL && skill->is_cool_finish() == true, ;);

	int blood_per = skill->detail()["blood_per"].asInt();
	JUDGE_RETURN(left_percent < blood_per, ;);

	int buff_id	= skill->detail()["id"].asInt();
	JUDGE_RETURN(this->is_have_status(buff_id) == false, ;);

	skill->add_use_tick();
	this->direct_luanch_skill_effect(skill);
}

void MapMaster::trigger_role_shield_disappear(int value)
{
	MapPlayerEx* player = this->self_player();
	JUDGE_RETURN(player->is_death() == false, ;);

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_TIAN_GANG);
	JUDGE_RETURN(mount_detail.open_ == true, ;);

	for (int i = 0; i < 2; ++i)
	{
		FighterSkill* skill = mount_detail.find_skill(TIAN_GANG_SKILL_2 + i);
		JUDGE_CONTINUE(skill != NULL);
		this->direct_luanch_skill_effect(skill);
	}

	FighterSkill* last_skill = mount_detail.find_skill(TIAN_GANG_SKILL_4);
	if (last_skill != NULL)
	{
		int percent = last_skill->detail()["percent"].asInt();
		this->modify_blood_by_fight(GameCommon::div_percent(percent) * value * -1);
		this->notify_launch_skill(last_skill, this);
	}
}

int MapMaster::master_exit_scene(int type)
{
	JUDGE_RETURN(this->beast_unrecycle_ == false, -1);
	this->all_beast_exit_scene();
	return 0;
}

int MapMaster::master_sign_out(int caculate_flag)
{
	JUDGE_RETURN(this->beast_unrecycle_ == false, -1);

	for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
	{
		this->callback_cur_beast(i);
	}

	return 0;
}

void MapMaster::update_cur_beast_speed()
{
	MapBeast* beast = this->fetch_cur_beast();
	JUDGE_RETURN(beast != NULL, ;);

	beast->update_beast_speed();
}

void MapMaster::update_cur_beast_pk_state()
{
	for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
	{
		MapBeast* beast = this->fetch_cur_beast(i);
		JUDGE_CONTINUE(beast != NULL);
		beast->set_pk_state(this->fight_detail_.__pk_state);
	}
}

void MapMaster::set_cur_beast_offline()
{
//	MapBeast* beast = this->fetch_cur_beast();
//	JUDGE_RETURN(beast != NULL, ;);
//
//	beast->GameFighter::exit_scene(EXIT_SCENE_JUMP);
//	MAP_BEAST_PACKAGE->unbind_object(beast->beast_id());
//
//	beast->set_beast_offline(this->self_player());
//	beast->update_beast_info(GameEnum::UPDATE_TYPE_BEAST_ID);
//
//	MAP_BEAST_PACKAGE->bind_object(beast->beast_id(), beast);
//	beast->GameFighter::enter_scene(ENTER_SCENE_JUMP);
//
//	this->cur_beast_id_ = beast->beast_id();
}

void MapMaster::set_cur_beast_aim(Int64 aim_id)
{
	if (this->is_in_league_region() == true && aim_id > 0)
	{
		JUDGE_RETURN(this->is_lrf_change_mode() == false, ;);
	}

	for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
	{
		MapBeast* beast = this->fetch_cur_beast(i);
		JUDGE_CONTINUE(beast != NULL);
		JUDGE_CONTINUE(beast->is_attack_beast() == true);
		beast->change_and_set_aim_object(aim_id);
	}
}

void MapMaster::start_beast_offline(const BSONObj& res)
{
//	JUDGE_RETURN(res.isEmpty() == false, ;);
//	JUDGE_RETURN(res[DBOfflineBeast::FLAG].numberInt() == true, ;);
//
//	Int64 beast_id = MAP_MONITOR->generate_beast_copy_id();
//	MapBeast* beast = this->find_and_pop_beast(beast_id);
//	JUDGE_RETURN(beast != NULL, ;);
//
//	this->set_beast_offline_db(res, beast);
//	this->beast_enter_scene(beast);
}

void MapMaster::set_beast_offline_db(const BSONObj &res, MapBeast *beast)
{
//	JUDGE_RETURN(res.isEmpty() == false, ;);
//	JUDGE_RETURN(res[DBOfflineBeast::FLAG].numberInt() == true, ;);
//
//	JUDGE_RETURN(beast != NULL, ;);
//
//	BeastDetail* beast_detail = beast->fetch_beast_detail();
//	beast_detail->beast_name_ = res[DBOfflineBeast::NAME].str();
//	beast_detail->beast_sort_ = res[DBOfflineBeast::SORT].numberInt();
//
//	beast_detail->offline_flag_ = true;
//	beast_detail->src_beast_id_ = res[DBOfflineBeast::BID].numberLong();
//
//	IntMap prop_map;
//	GameCommon::bson_to_map(prop_map, res.getObjectField(
//			DBOfflineBeast::PROP_SET.c_str()));
//
//	IntVec skill_set;
//	DBCommon::bson_to_int_vec(skill_set, res.getObjectField(
//			DBOfflineBeast::SKILL_SET.c_str()));
//
//	for (IntMap::iterator iter = prop_map.begin(); iter != prop_map.end(); ++iter)
//	{
//		ProtoPairObj pair_obj;
//		pair_obj.set_obj_id(iter->first);
//		pair_obj.set_obj_value(iter->second);
//		beast->refresh_fight_property_item(BasicElement::TOTAL_PROP, pair_obj);
//	}
//
//	beast->insert_auto_skill(GameEnum::BASE_BEAST_SKILL_ID);
//	for (IntVec::iterator iter = skill_set.begin(); iter != skill_set.end(); ++iter)
//	{
//		beast->insert_auto_skill(*iter);
//	}
//	this->cur_beast_id_ = beast->beast_id();
}

void MapMaster::set_beast_offline_info(ProtoOfflineBeast* beast_info)
{
//	JUDGE_RETURN(beast_info->beast_sort() > 0, ;);
//
//	Int64 beast_id = MAP_MONITOR->generate_beast_copy_id();
//	MapBeast* beast = this->find_and_pop_beast(beast_id);
//	JUDGE_RETURN(beast != NULL, ;);
//
//	BeastDetail* beast_detail = beast->fetch_beast_detail();
//	beast_detail->beast_sort_ = beast_info->beast_sort();
//	beast_detail->beast_name_ = beast_info->beast_name();
//
//	beast_detail->src_beast_id_ = beast_info->beast_id();
//	beast_detail->offline_flag_ = true;
//
//	for (int i = 0; i < beast_info->skill_set_size(); ++i)
//	{
//		const ProtoPairObj& tmp = beast_info->skill_set(i);
//		beast->insert_auto_skill(tmp.obj_id(), 1);
//	}
//
//	for (int i = 0; i < beast_info->prop_set_size(); ++i)
//	{
//		const ProtoPairObj& pair_obj = beast_info->prop_set(i);
//		beast->refresh_fight_property_item(BasicElement::TOTAL_PROP, pair_obj);
//	}
//
//	this->cur_beast_id_ = beast->beast_id();
//	beast->set_room_scene_index(this->room_scene_index());
}

int MapMaster::logic_upsert_beast(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400401*, request, -1);
//	this->check_and_callback_beast(request->enter_flag());
//
//	MapBeast* beast = this->find_and_pop_beast(request->beast_id());
//	JUDGE_RETURN(beast != NULL, -1);
//
//	beast->recycle_all_skill_map();
//	beast->insert_auto_skill(GameEnum::BASE_BEAST_SKILL_ID);
//
//	BeastDetail* beast_detail = beast->fetch_beast_detail();
//	beast_detail->beast_sort_ = request->beast_sort();
//	beast_detail->beast_name_ = request->beast_name();
//	beast_detail->beast_force_ = request->beast_force();
//
//	beast_detail->ability_ = request->ability();
//	beast_detail->growth_lvl_ = request->growth_lvl();
//	beast_detail->action_state_ = request->action_state();
//	beast->mover_detail().__speed.set_single(this->speed_total_i(), BasicElement::BASIC);
//
//	for (int i = 0; i < request->skill_set_size(); ++i)
//	{
//		const ProtoPairObj& proto_pair = request->skill_set(i);
//		beast->insert_auto_skill(proto_pair.obj_id(), proto_pair.obj_value());
//	}
//
//	if (request->enter_flag() == true)
//	{
//		// enter scene
//		this->beast_enter_scene(beast);
//		this->update_fight_property();
//	}
//	else
//	{
//		// update property
//		this->update_fight_property();
//		JUDGE_RETURN(request->notify_flag() == true, 0);
//
//		Scene* scene = this->fetch_scene();
//		JUDGE_RETURN(scene != NULL, -1);
//
//		scene->notify_appear(beast);
//	}

	return 0;
}

int MapMaster::update_beast_state(int flag, int type)
{
	if (flag == true)
	{
		this->beast_enter_scene_b(type);
	}
	else
	{
		this->callback_cur_beast(type);
	}

	return 0;
}

int MapMaster::callback_cur_beast(int type)
{
	MapBeast* beast = this->fetch_cur_beast(type, false);
	JUDGE_RETURN(beast != NULL, -1);

	beast->exit_scene();
	this->cur_beast_id_[type] = 0;

	return MAP_BEAST_PACKAGE->unbind_and_push(beast->beast_id(), beast);
}

int MapMaster::callback_cur_beast(Message* msg)
{
	return 0;
}

int MapMaster::check_and_callback_beast(int enter_flag)
{
	return 0;
}

int MapMaster::beast_enter_scene_b(int type)
{
	switch (type)
	{
	case MapMaster::TYPE_BEAST:
	{
		return this->a_beast_enter_scene();
	}
	case MapMaster::TYPE_MAGIC:
	{
		return this->b_beast_enter_scene();
	}
	case MapMaster::TYPE_ESCORT:
	{
		return this->escort_beast_enter_scene();
	}
	case MapMaster::TYPE_MAO:
	{
		return this->d_beast_enter_scene();
	}
	}

	return 0;
}

int MapMaster::a_beast_enter_scene()
{
	this->start_beast_skill_blood();

	MapPlayerEx* player = this->self_player();
	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_LING_BEAST);
	JUDGE_RETURN(mount_detail.on_mount_ == true, -1);
	JUDGE_RETURN(mount_detail.mount_shape_ > 0, -1);

	MapBeast* beast = this->pop_beast(MapMaster::TYPE_BEAST);
	JUDGE_RETURN(beast != NULL, -1);

	BeastDetail* beast_detail = beast->fetch_beast_detail();
	beast_detail->type_ 		= MapMaster::TYPE_BEAST;
	beast_detail->master_id_	= this->mover_id();
	beast_detail->beast_sort_ 	= mount_detail.mount_shape_;

	int base_skill = GameCommon::fetch_base_skill(MOVER_TYPE_BEAST, beast_detail->type_);
	beast->insert_skill(base_skill, 1);

	static int BEAST_SKILL[] = {BEAST_SKILL_CRAZY_ANGRY, BEAST_SKILL_MULTI_HURT};
	static int TOTAL_SIZE = ARRAY_LENGTH(BEAST_SKILL);
	for (int i = 0; i < TOTAL_SIZE; ++i)
	{
		FighterSkill* skill = mount_detail.find_skill(BEAST_SKILL[i]);
		JUDGE_CONTINUE(skill != NULL);
		beast->insert_skill(skill->__skill_id, skill->__level);
	}

	if (beast->is_active() == true)
	{
		//update info
		return beast->update_beast_info(GameEnum::UPDATE_BEAST_SORT,
				beast_detail->beast_sort_);
	}
	else
	{
		//enter scene
		return this->beast_enter_scene(beast);
	}
}

int MapMaster::b_beast_enter_scene()
{
	MapPlayerEx* player = this->self_player();

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_MAGIC_EQUIP);
	JUDGE_RETURN(mount_detail.on_mount_ == true, -1);
	JUDGE_RETURN(mount_detail.mount_shape_ > 0, -1);

	MapBeast* beast = this->pop_beast(MapMaster::TYPE_MAGIC);
	JUDGE_RETURN(beast != NULL, -1);

	BeastDetail* beast_detail = beast->fetch_beast_detail();
	beast_detail->type_			= MapMaster::TYPE_MAGIC;
	beast_detail->master_id_ 	= this->mover_id();
	beast_detail->beast_sort_ 	= mount_detail.mount_shape_;

	int base_skill = GameCommon::fetch_base_skill(MOVER_TYPE_BEAST, beast_detail->type_);
	beast->insert_skill(base_skill, 1);

	if (beast->is_active() == true)
	{
		//update info
		return beast->update_beast_info(GameEnum::UPDATE_BEAST_SORT,
				beast_detail->beast_sort_);
	}
	else
	{
		//enter scene
		return this->beast_enter_scene(beast);
	}
}

int MapMaster::escort_beast_enter_scene()
{
	MapPlayerEx* player = this->self_player();
	Escort_detail &item = player->get_escort_detail();
	JUDGE_RETURN(item.escort_type() > 0 && item.car_index_ > 0, -1);

	MapBeast* beast = this->pop_beast(MapMaster::TYPE_ESCORT);
	JUDGE_RETURN(beast != NULL, -1);

	BeastDetail* beast_detail = beast->fetch_beast_detail();
	beast_detail->type_ 		= MapMaster::TYPE_ESCORT;
	beast_detail->master_id_	= this->mover_id();
	beast_detail->beast_sort_ 	= 951000000 + item.escort_type();

	//enter scene
	return this->beast_enter_scene(beast);
}

int MapMaster::d_beast_enter_scene()
{
	MapPlayerEx* player = this->self_player();

	MountDetail& mount_detail = player->mount_detail(GameEnum::FUN_BEAST_MAO);
	JUDGE_RETURN(mount_detail.on_mount_ == true, -1);
	JUDGE_RETURN(mount_detail.shape_id() > 0, -1);

	MapBeast* beast = this->pop_beast(MapMaster::TYPE_MAO);
	JUDGE_RETURN(beast != NULL, -1);

	BeastDetail* beast_detail = beast->fetch_beast_detail();
	beast_detail->type_			= MapMaster::TYPE_MAO;
	beast_detail->master_id_ 	= this->mover_id();
	beast_detail->beast_sort_ 	= mount_detail.shape_id();

	if (beast->is_active() == true)
	{
		//update info
		return beast->update_beast_info(GameEnum::UPDATE_BEAST_SORT,
				beast_detail->beast_sort_);
	}
	else
	{
		//enter scene
		return this->beast_enter_scene(beast);
	}
}

int MapMaster::beast_enter_scene(MapBeast* beast)
{
	JUDGE_RETURN(beast != NULL, -1);

	const Json::Value& conf = this->scene_set_conf();
	JUDGE_RETURN(conf["beast_disappear"].asInt() == 0, -1);

	beast->set_room_scene_index(this->room_scene_index());
	beast->enter_scene();

	return 0;
}

int MapMaster::beast_exit_scene(int type)
{
	MapBeast* beast = this->fetch_cur_beast(type);
	JUDGE_RETURN(beast != NULL, -1);
	return beast->exit_scene();
}

int MapMaster::beast_schedule_move(Message *msg)
{
//    MSG_DYNAMIC_CAST_NOTIFY(Proto10400501 *, request, RETURN_BEAST_MOVE);
//
//    MapBeast* beast = this->fetch_cur_beast();
//    CONDITION_NOTIFY_RETURN(beast != 0 && beast->is_enter_scene(), RETURN_BEAST_MOVE, ERROR_CLIENT_OPERATE);
//
//    MoverCoord step;
//    beast->mover_detail().__step_list.clear();
//    for (int i = 0; i < request->step_list_size(); ++i)
//    {
//        const ProtoCoord &coord = request->step_list(i);
//        step.set_pixel(coord.pixel_x(), coord.pixel_y());
//        beast->mover_detail().__step_list.push_back(step);
//
//        int ret = beast->validate_movable(step);
//        if (ret != 0)
//        {
//            Proto50400501 respond;
//            respond.set_scene_id(beast->scene_id());
//            respond.set_pixel_x(beast->location().pixel_x());
//            respond.set_pixel_y(beast->location().pixel_y());
//        	return this->respond_to_client_error(RETURN_BEAST_MOVE, ret, &respond);
//        }
//    }
//
//    int ret = beast->schedule_move(step, request->toward());
//    if (ret != 0)
//    {
//        Proto50400501 respond;
//        respond.set_scene_id(beast->scene_id());
//        respond.set_pixel_x(beast->location().pixel_x());
//        respond.set_pixel_y(beast->location().pixel_y());
//    	return this->respond_to_client_error(RETURN_BEAST_MOVE, ret, &respond);
//    }

    return 0;
}


void MapMaster::all_beast_enter_scene(int type)
{
	if (type == 1)
	{
		//跳跃
		this->a_beast_enter_scene();
		this->d_beast_enter_scene();
	}
	else
	{
		this->a_beast_enter_scene();
		this->b_beast_enter_scene();
		this->escort_beast_enter_scene();
		this->d_beast_enter_scene();
	}
}

void MapMaster::all_beast_exit_scene(int type)
{
	if (type == 1)
	{
		//跳跃
		this->beast_exit_scene(MapMaster::TYPE_MAO);
		this->beast_exit_scene(MapMaster::TYPE_BEAST);
	}
	else
	{
		for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
		{
			this->beast_exit_scene(i);
		}
	}
}

int MapMaster::refresh_beast_fight_prop(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30400405*, request, -1);
//
//	MapBeast* beast = this->find_beast(request->beast_id());
//	JUDGE_RETURN(beast != NULL, -1);
//
//	beast->refresh_fight_property(msg);
//	this->update_fight_property();

	return 0;
}

Int64 MapMaster::fetch_cur_beast_id(int type)
{
	MapBeast* beast = this->fetch_cur_beast(type);
	JUDGE_RETURN(beast != NULL, 0);
	return this->cur_beast_id_[type];
}

MapBeast* MapMaster::fetch_cur_beast(int type, int active)
{
	MapBeast* beast = this->find_beast(this->cur_beast_id_[type]);
	JUDGE_RETURN(beast != NULL, NULL);

	if (active == false)
	{
		return beast;
	}

	if (beast->is_active() == false)
	{
		return NULL;
	}

	return beast;
}

MapBeast* MapMaster::find_beast(Int64 beast_id)
{
	return MAP_BEAST_PACKAGE->find_object(beast_id);
}

MapBeast* MapMaster::pop_beast(int type)
{
	MapBeast* beast = this->fetch_cur_beast(type, false);
	JUDGE_RETURN(beast == NULL, beast);

	beast = MAP_BEAST_PACKAGE->pop_object();
	JUDGE_RETURN(beast != NULL, NULL);

	this->cur_beast_id_[type] = beast->entity_id();
	MAP_BEAST_PACKAGE->bind_object(beast->entity_id(), beast);

	return beast;
}

int MapMaster::notify_beast_call_back(int type)
{
	return 0;
}

int MapMaster::update_cur_beast_location()
{
	for (int i = MapMaster::TYPE_BEAST; i < MapMaster::BEAST_TOTAL; ++i)
	{
		 MapBeast* beast = this->fetch_cur_beast(i);
		 JUDGE_CONTINUE(beast != NULL);
		 beast->set_cur_location(this->location());
	}

    return 0;
}

