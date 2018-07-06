/*
 * MapMounter.cpp
 *
 *  Created on: Nov 26, 2013
 *      Author: peizhibi
 */

#include "MapMounter.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "MapBeast.h"

MapMounter::MapMounter()
{
	// TODO Auto-generated constructor stub
}

MapMounter::~MapMounter()
{
	// TODO Auto-generated destructor stub
}

void MapMounter::reset_mount(void)
{
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		mount_detail.reset(i);
	}
}

MountDetail &MapMounter::mount_detail(int type)
{
	return this->mount_detail_[type - 1];
}

int MapMounter::update_mount_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400406*, request, -1);

	int mount_type = request->type();

	MountDetail& mount_detail = this->mount_detail(mount_type);
	mount_detail.open_ = request->open();
	mount_detail.on_mount_ = request->on_mount();
	mount_detail.mount_grade_ = request->mount_grade();
	mount_detail.mount_shape_ = request->mount_shape();
	mount_detail.act_shape_ = request->act_shape();

	this->refresh_mount_skill(mount_type, request);
	this->refresh_mount_prop(mount_type);

	int notify_flag = request->notify_flag();
	JUDGE_RETURN(notify_flag > 0, 0);

	return this->refresh_mount_shape(mount_detail);
}

int MapMounter::refresh_mount_skill(int type, Proto30400406* request)
{
	MountDetail& mount_detail = this->mount_detail(type);
	for (int i = 0; i < request->skill_set_size(); ++i)
	{
		const ProtoPairObj& obj = request->skill_set(i);
		mount_detail.add_new_skill(obj.obj_id(), obj.obj_value());
	}

	switch (type)
	{
	case GameEnum::FUN_GOD_SOLIDER:
	case GameEnum::FUN_XIAN_WING:
//	case GameEnum::FUN_TIAN_GANG:
	{
		for (int i = 0; i < request->skill_set_size(); ++i)
		{
			const ProtoPairObj& obj = request->skill_set(i);
			this->insert_skill(obj.obj_id(), obj.obj_value(), false);
		}
		break;
	}
	}
	return 0;
}

int MapMounter::refresh_mount_prop(int type)
{
	switch (type)
	{
	case GameEnum::FUN_MOUNT:
	{
		this->refresh_fun_mount_prop();
		break;
	}
//	case GameEnum::FUN_LING_BEAST:
//	{
//		this->refresh_fun_beast_prop();
//		break;
//	}
	}

	return 0;
}

int MapMounter::refresh_mount_shape(MountDetail& mount_detail)
{
	int shape_update = mount_detail.set_conf()["shape_update"].asInt();
	switch (mount_detail.type_)
	{
	case GameEnum::FUN_MOUNT:
	case GameEnum::FUN_GOD_SOLIDER:
	case GameEnum::FUN_XIAN_WING:
	case GameEnum::FUN_TIAN_GANG:
	{
		this->notify_update_player_info(shape_update, mount_detail.shape_id());
		break;
	}
	case GameEnum::FUN_MAGIC_EQUIP:
	case GameEnum::FUN_LING_BEAST:
	case GameEnum::FUN_BEAST_MAO:
	{
		this->self_player()->update_beast_state(mount_detail.on_mount_, shape_update);
		break;
	}
	case GameEnum::FUN_BEAST_EQUIP:
	case GameEnum::FUN_BEAST_MOUNT:
	case GameEnum::FUN_BEAST_WING:
	{
		MapBeast* beast = this->self_player()->fetch_cur_beast(MapMaster::TYPE_BEAST);
		JUDGE_RETURN(beast != NULL, -1);
		beast->update_beast_info(shape_update, mount_detail.shape_id());
		break;
	}
	}
	return 0;
}

int MapMounter::mount_grade(int type)
{
	MountDetail& mount_detail = this->mount_detail(type);
	return mount_detail.mount_grade_;
}

int MapMounter::is_on_mount(int type)
{
	MountDetail& mount_detail = this->mount_detail(type);
	return mount_detail.on_mount_;
}

int MapMounter::fetch_mount_id(int type)
{
	MountDetail& mount_detail = this->mount_detail(type);
	return mount_detail.shape_id();
}

int MapMounter::mount_speed()
{
	JUDGE_RETURN(this->is_on_mount() == true, 0);

	MountDetail& mount_detail = this->mount_detail(GameEnum::FUN_MOUNT);
	return mount_detail.conf()["speed"].asInt();
}

int MapMounter::fetch_mount_special_force()
{
	int mount_force = 0;
	for (int i = GameEnum::FUN_MOUNT; i <= GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		MountDetail& mount_detail = this->mount_detail(i);
		JUDGE_CONTINUE(mount_detail.open_ == true);
		mount_force += mount_detail.skill_force();
	}

	return mount_force;
}

int MapMounter::refresh_fun_mount_prop()
{
	int speed_multi = this->mount_speed();
	return this->update_fighter_speed(GameEnum::SPEED_MULTI,
			speed_multi, BasicElement::MOUNT);
}

int MapMounter::refresh_fun_beast_prop()
{
	return 0;
}
