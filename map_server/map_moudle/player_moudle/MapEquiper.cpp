/*
 * MapEquiper.cpp
 *
 *  Created on: 2013-12-13
 *      Author: louis
 */

#include "MapEquiper.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

MapEquiper::MapEquiper() {
	// TODO Auto-generated constructor stub
	  this->magic_weapon_id_ = 0;
	  this->magic_weapon_rank_lvl_ = 0;
	  this->talisman_pre_skill_id_ = 0;
	  this->talisman_pre_skill_lvl_ = 0;
	  this->equip_refine_lvl_ = 0;
	  this->fairy_act_info_ = IntPair(0,0);
}

MapEquiper::~MapEquiper() {
	// TODO Auto-generated destructor stub
}

int MapEquiper::refresh_player_equip_shape(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400601*, request, -1);

	for(int i = 0; i < request->shape_list_size(); ++i)
	{
		const ProtoPairObj& proto_pair = request->shape_list(i);
		int id 		= proto_pair.obj_id();
		int value 	= proto_pair.obj_value();
		this->__shape_map[id] = value;
	}

//	JUDGE_RETURN(no_notify == false, 0);
	JUDGE_RETURN(this->is_enter_scene() == true, 0);
	return this->notify_update_player_info(GameEnum::PLAYER_INFO_EQUIP_SHAPE);
}

ShapeDetail& MapEquiper::shape_detail(void)
{
	return this->__shape_map;
}

MapEquipDetail& MapEquiper::map_equip_detail()
{
	return this->equip_detail_;
}

int MapEquiper::get_shape_item_id(const int part)
{
	JUDGE_RETURN(this->__shape_map.count(part) > 0, 0);
	return this->__shape_map[part];
}

int MapEquiper::reset_shape(void)
{
	this->__shape_map.clear();
	this->__label_info.clear();
    this->equip_detail_.reset();
    this->magic_weapon_id_ = 0;
    this->magic_weapon_rank_lvl_ = 0;
    this->talisman_pre_skill_id_ = 0;
    this->talisman_pre_skill_lvl_ = 0;
    this->equip_refine_lvl_ = 0;
    this->fairy_act_info_ = IntPair(0,0);
	return 0;
}


int MapEquiper::refresh_player_cur_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400602*, request, -1);
	this->__label_info.clear();
	this->__label_info.insert(std::pair<int,Int64>(request->cur_label(), request->expire_tick()));
	this->notify_update_player_info(GameEnum::PLAYER_INFO_LABEL);
	return 0;
}

std::map<int, Int64>& MapEquiper::label_info(void)
{
	return this->__label_info;
}

int MapEquiper::get_cur_label(void)
{
	JUDGE_RETURN(this->__label_info.size() > 0, 0);
	std::map<int, Int64>::iterator it = this->__label_info.begin();
	int label_id = it->first;
	if(label_id == 0)
		return 0;

	int label_type = GameCommon::fetch_label_type(label_id);
	if(label_type == GameEnum::LABEL_TYPE_LIMIT_TIME)
	{
		Int64 expire_tick = it->second;
		label_id = GameCommon::left_time(expire_tick) > 0 ? label_id : 0;
	}
	return label_id;
}

int MapEquiper::request_map_logic_add_label(int label_id, int type)
{
	Proto31401701 request;
	request.set_label_id(label_id);
	request.set_type(type);
	return this->send_to_logic_thread(request);
}

int MapEquiper::sync_arena_shape_info()
{
	Proto30100228 shape_info;
	shape_info.set_weapon(this->get_shape_item_id(GameEnum::EQUIP_WEAPON));
	shape_info.set_clothes(this->get_shape_item_id(GameEnum::EQUIP_YIFU));

	shape_info.set_fashion_weapon(this->get_shape_item_id(GameEnum::FASHION_WEAPON));
	shape_info.set_fashion_clothes(this->get_shape_item_id(GameEnum::FASHION_YIFU));

	return MAP_MONITOR->dispatch_to_logic(this, &shape_info);
}

void MapEquiper::check_and_fix_label()
{
}

int MapEquiper::sync_transfer_shape_and_label()
{
	Proto30400112 request;
	//shape
	ShapeDetail::iterator it = this->__shape_map.begin();
	for(; it != this->__shape_map.end(); ++it)
	{
		ProtoPairObj* proto_obj = request.add_equip_shape();
		proto_obj->set_obj_id(it->first);
		proto_obj->set_obj_value(it->second);
	}

	//label
	std::map<int, Int64>::iterator iter = this->__label_info.begin();
	for(; iter != this->__label_info.end(); ++iter)
	{
		ProtoSyncLabel* proto_label = request.add_label_list();
		proto_label->set_label_id(iter->first);
		proto_label->set_expire_tick(iter->second);
	}

	return this->send_to_other_scene(this->scene_id(), request);
}

int MapEquiper::read_transfer_shape_and_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400112*, request, -1);

	//shape
	for(int i = 0; i < request->equip_shape_size(); ++i)
	{
		const ProtoPairObj& proto_obj = request->equip_shape(i);
		int shape_part = proto_obj.obj_id();
		int shape_id   = proto_obj.obj_value();

		this->__shape_map[shape_part] = shape_id;
	}

	//label
	for(int i = 0; i < request->label_list_size(); ++i)
	{
		const ProtoSyncLabel& proto_label = request->label_list(i);
		int label_id = proto_label.label_id();
		Int64 expire_tick = proto_label.expire_tick();

		this->__label_info[label_id] =  expire_tick;
	}

//    this->__wing_info.__wing_id = request->wing_id();
//    this->__wing_info.__wing_level = request->wing_level();
//    this->equip_detail_.weapon_lvl_ = request->weapon_lvl();

	return 0;
}

int MapEquiper::refresh_player_magic_weapon_skill(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto31401002 *, request, msg, -1);
//
//	if ((this->talisman_pre_skill_id_ != request->cur_wm_skill_id()) && this->talisman_pre_skill_id_ > 0)
//	{
//		this->remove_skill(this->talisman_pre_skill_id_);
//	}
//
//	if ((request->cur_wm_skill_id() > 0) && (request->cur_wm_skill_level() > 0))
//	{
//		FighterSkill *skill = 0;
//		if (this->find_skill(request->cur_wm_skill_id(), skill) == 0)
//		{
//			skill->__level = request->cur_wm_skill_level();
//		}
//		else
//		{
//			this->insert_skill(request->cur_wm_skill_id(), request->cur_wm_skill_level());
//		}
//		this->talisman_pre_skill_id_ = request->cur_wm_skill_id();
//		this->talisman_pre_skill_lvl_ = request->cur_wm_skill_level();
//	}
	return 0;
}

int MapEquiper::refresh_player_magic_weapon_shape(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401003 *, request, -1);

	this->magic_weapon_id_ = request->mw_id();
	this->magic_weapon_rank_lvl_ = request->mw_rank_lvl();
	this->init_magic_weapon_skill(request->mw_skill_id(),request->mw_skill_lvl());
	return this->notify_update_player_info(GameEnum::PLAYER_INFO_MAGICWEAPON);
}

int MapEquiper::init_magic_weapon_skill(const int magic_skill_id,const int magic_skill_lvl)
{
//	JUDGE_RETURN(magic_skill_id > 0 && magic_skill_lvl > 0, 0);
//
//	FighterSkill *skill = 0;
//	if (this->find_skill(magic_skill_id, skill) == 0)
//	{
//		skill->__level = magic_skill_lvl;
//	}
//	else
//	{
//		this->insert_skill(magic_skill_id, magic_skill_lvl);
//	}
//	this->talisman_pre_skill_id_ = magic_skill_id;
//	this->talisman_pre_skill_lvl_ = magic_skill_lvl;

	return 0;
}


int MapEquiper::magic_weapon_id()
{
	JUDGE_RETURN(this->is_lrf_change_mode() == false, 0);
	return this->magic_weapon_id_;
}

int MapEquiper::magic_weapon_lvl()
{
	return this->magic_weapon_rank_lvl_;
}

void MapEquiper::set_magic_weapon_id(int id)
{
	 this->magic_weapon_id_ = id;
}

void MapEquiper::set_magic_weapon_lvl(int lvl)
{
    this->magic_weapon_rank_lvl_ = lvl;
}

int MapEquiper::magic_weapon_curr_skill_id()
{
	return this->talisman_pre_skill_id_;
}

int MapEquiper::refresh_equip_refine_lvl(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto31400600 *, request, msg, -1);
	this->equip_refine_lvl_ = request->lvl();

	return 0;
}

void MapEquiper::set_equip_refine_lvl(int lvl)
{
	this->equip_refine_lvl_ = lvl;
}

int MapEquiper::refresh_fairy_act(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400603 *, request, -1);
	return 0;
}

void MapEquiper::make_up_fairy_act(ProtoPairObj* pair)
{
	pair->set_obj_id(this->fairy_act_info_.first);
	pair->set_obj_value(this->fairy_act_info_.second);
}


