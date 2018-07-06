/*
 * HotspringActor.cpp
 *
 *  Created on: 2016年9月19日
 *      Author: lzy
 */

#include "HotspringActor.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "HotspringActivityScene.h"

HotspringActor::ActivityStartTimer::ActivityStartTimer():parent_(0)
{

}

HotspringActor::ActivityStartTimer::~ActivityStartTimer()
{

}

void HotspringActor::ActivityStartTimer::set_parent(HotspringActor *parent)
{
	this->parent_ = parent;
}

int HotspringActor::ActivityStartTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int HotspringActor::ActivityStartTimer::handle_timeout(const Time_Value &tv)
{
	this->parent_->get_hotspring_exp();
	return 0;
}

HotspringActor::HotspringActor()
{
	this->start_timer_.cancel_timer();
	this->start_timer_.set_parent(this);
}

HotspringActor::~HotspringActor()
{

}

HotspringDetail &HotspringActor::get_hotspring_detail()
{
	return this->hotspring_detail_;
}

void HotspringActor::reset()
{
	this->hotspring_detail_.reset();
	this->start_timer_.cancel_timer();
}

int HotspringActor::sign_out(const bool is_save_player)
{
	HOTSPRING_INSTANCE->refresh_player_info(this->role_id());
	Int64 first = this->role_id();
	Int64 second = HOTSPRING_INSTANCE->find_double_player(first);

	int ret = HOTSPRING_INSTANCE->unbind_double_major(first, second);

//	if (ret != 1) return -1;
	MapPlayerEx *first_player = NULL;
	MapPlayerEx *second_player = NULL;
	if (MAP_MONITOR->find_player(first, first_player) != 0) return -1;
	if (MAP_MONITOR->find_player(second, second_player) != 0) return -1;

	Proto50405031 respond;
	respond.set_first_player_id(first);
	respond.set_second_player_id(second);
	respond.set_first_player_name(first_player->role_name());
	respond.set_second_player_name(second_player->role_name());
	respond.set_type(1);
	respond.set_choose(ret);
	respond.set_status(2);
	return second_player->respond_to_client(RETURN_HOTSPRING_DOUBLE_MAJOR, &respond);
}


int HotspringActor::request_enter_hotspring_activity()
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_HOTSPRING_ACTIVITY_ENTER, ERROR_NORMAL_SCENE);

	static int need_lvl = 1;
//	static int need_lvl = CONFIG_INSTANCE->answer_activity_json(1)["open_level"].asInt();
	CONDITION_NOTIFY_RETURN(this->level() >= need_lvl, RETURN_HOTSPRING_ACTIVITY_ENTER,
			ERROR_NEED_LEVLE_39);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_HOTSPRING_ACTIVITY);

	int ret = this->send_request_enter_info(GameEnum::HOTSPRING_SCENE_ID, enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_HOTSPRING_ACTIVITY_ENTER, ret);

	return 0;
}

bool HotspringActor::is_on_hotspring_scene()
{
	return (this->scene_id() == GameEnum::HOTSPRING_SCENE_ID ? true : false);
}

int HotspringActor::request_Hotspring_info()
{
//	JUDGE_RETURN(this->scene_id() == GameEnum::HOTSPRING_ACTIVITY_SCENE_ID, -1);
	HOTSPRING_INSTANCE->refresh_player_info(this->role_id());
	Proto50405030 respond;

	respond.set_cur_stage(this->hotspring_detail_.cur_stage);
	respond.set_first_npc(this->hotspring_detail_.first_npc);
	respond.set_second_npc(this->hotspring_detail_.second_npc);
	respond.set_third_npc(this->hotspring_detail_.third_npc);
	respond.set_left_time(HOTSPRING_INSTANCE->get_refresh_time());
	respond.set_total_exp(HOTSPRING_INSTANCE->get_player_exp(this->role_id()));
	respond.set_player_npc(this->hotspring_detail_.player_npc);
	respond.set_vip_level(this->vip_type());
	respond.set_status(HOTSPRING_INSTANCE->get_cur_status());
	respond.set_is_right(this->hotspring_detail_.is_right);
	FINER_PROCESS_RETURN(RETURN_FETCH_HOTSPRING_INFO, &respond);
}

int HotspringActor::get_hotspring_exp()
{
	if (this->scene_id() != GameEnum::HOTSPRING_SCENE_ID)
	{
		this->start_timer_.cancel_timer();
		return 0;
	}

	int exp_num = CONFIG_INSTANCE->role_level(0, this->level())["hot_spring_in_exp"].asInt();
	//vip 经验加成
	int add_scale = CONFIG_INSTANCE->vip(this->vip_type())["hotspring_exp"].asInt();
	if (this->hotspring_detail_.double_major_player != 0)
	{
		add_scale += CONFIG_INSTANCE->hotspring_activity_json(HOTSPRING_INSTANCE->get_cycle_id())["double_major"].asInt();
	}
	if (add_scale > 0)
	{
		exp_num += int((double)exp_num * (double)add_scale / (double)GameEnum::DAMAGE_ATTR_PERCENT + 0.5);
	}


	exp_num = this->modify_element_experience(exp_num, SerialObj(EXP_FROM_HOTSPRING_ACTIVITY));
	HOTSPRING_INSTANCE->get_player_exp(this->role_id()) += exp_num;
	this->request_Hotspring_info();
	return 0;
}

int HotspringActor::hotspring_activity_start()
{
	this->start_timer_.schedule_timer(HOTSPRING_ACTIVITY_EXP_INTERVAL);

	LongMap &player_list = HOTSPRING_INSTANCE->get_hotspring_detail().act_player_vip_info;
	if (player_list.count(this->role_id()) == 0)
		player_list[this->role_id()] = this->vip_type();

	HOTSPRING_INSTANCE->refresh_player_info(this->role_id());

	Proto80405017 respond;
	respond.set_status(HOTSPRING_INSTANCE->get_cur_status());
	respond.set_is_right(HOTSPRING_INSTANCE->get_player_is_right(this->role_id()));
	FINER_PROCESS_RETURN(ACTIVE_HOTSPRING_INFO, &respond);
}

int HotspringActor::request_double_major(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10405031*, request, RETURN_HOTSPRING_DOUBLE_MAJOR);

	HOTSPRING_INSTANCE->refresh_player_info(this->role_id());
	Int64 first = request->first_player_id();
	Int64 second = request->second_player_id();

//	if (this->role_id() == second)
//	{
//		int temp = first;
//		first = second;
//		second = temp;
//	}

	if (second == 0)
	{
		second = HOTSPRING_INSTANCE->find_double_player(first);
	}

	MapPlayerEx *first_player = NULL;
	MapPlayerEx *second_player = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_player(first, first_player) == 0, -1);
	JUDGE_RETURN(MAP_MONITOR->find_player(second, second_player) == 0, -1);

	Proto50405031 respond;
	respond.set_type(request->type());
	respond.set_first_player_id(first);
	respond.set_second_player_id(second);
	respond.set_first_player_name(first_player->role_name());
	respond.set_second_player_name(second_player->role_name());

	MoverCoord first_coord = first_player->location();
	ProtoCoord *first_proto_coord = respond.mutable_first_player_coord();
	first_coord.serialize(first_proto_coord);

	MoverCoord second_coord = second_player->location();
	ProtoCoord *second_proto_coord = respond.mutable_second_player_coord();
	second_coord.serialize(second_proto_coord);

	//work
//	JUDGE_RETURN(request->status() == 1, 0); //玩家请求类型

	if (request->type() == 0)//请求绑定
	{
		int ret = HOTSPRING_INSTANCE->bind_double_major(first, second);
		respond.set_choose(ret);
		respond.set_status(2);
		CONDITION_NOTIFY_RETURN(ret == 1, RETURN_HOTSPRING_DOUBLE_MAJOR, ret);
		second_player->respond_to_client(RETURN_HOTSPRING_DOUBLE_MAJOR, &respond);

		this->notify_update_player_info(GameEnum::PLAYER_INFO_HOTSRING, second);
		second_player->notify_update_player_info(GameEnum::PLAYER_INFO_HOTSRING, first);

		FINER_PROCESS_RETURN(RETURN_HOTSPRING_DOUBLE_MAJOR, &respond);
	}
	else //解除绑定
	{
		int ret = HOTSPRING_INSTANCE->unbind_double_major(first, second);
		respond.set_choose(ret);
		respond.set_status(2);
		CONDITION_NOTIFY_RETURN(ret == 1, RETURN_HOTSPRING_DOUBLE_MAJOR, ret);
		second_player->respond_to_client(RETURN_HOTSPRING_DOUBLE_MAJOR, &respond);

		this->notify_update_player_info(GameEnum::PLAYER_INFO_HOTSRING, 0);
		second_player->notify_update_player_info(GameEnum::PLAYER_INFO_HOTSRING, 0);

		FINER_PROCESS_RETURN(RETURN_HOTSPRING_DOUBLE_MAJOR, &respond);
	}
}

int HotspringActor::request_hotspring_near_player(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10405035*, request, RETURN_HOTSPRING_NEAR_PLAYER);
	Int64 role_id = request->role_id();
	string name = "";
	MoverCoord aim_coord;
	Int64 aim_id;

	int ret = HOTSPRING_INSTANCE->find_near_player(role_id, aim_coord, name, aim_id);
	CONDITION_NOTIFY_RETURN(ret == 1, RETURN_HOTSPRING_NEAR_PLAYER, ret);

	Proto50405035 respond;
	respond.set_player_name(name);
	respond.set_player_id(aim_id);
	aim_coord.serialize(respond.mutable_coord());

	FINER_PROCESS_RETURN(RETURN_HOTSPRING_NEAR_PLAYER, &respond);
}

int HotspringActor::request_player_guess(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10405032*, request, RETURN_HOTSPRING_GUESS);
	HOTSPRING_INSTANCE->refresh_player_info(this->role_id());
	Proto50405032 respond;
	respond.set_choose(request->npc_num());

	CONDITION_NOTIFY_RETURN(HOTSPRING_INSTANCE->get_cur_status() == 1, RETURN_HOTSPRING_GUESS, ERROR_HOTSPRING_ON_START);
	CONDITION_NOTIFY_RETURN(HOTSPRING_INSTANCE->is_player_has_choose(this->role_id()) == 0, RETURN_HOTSPRING_GUESS, ERROR_PLAYER_HAS_CHOOSE);

	respond.set_status(true);
	HOTSPRING_INSTANCE->player_choose_answer(this->role_id(), request->npc_num());
	this->hotspring_detail_.player_npc = request->npc_num();

	FINER_PROCESS_RETURN(RETURN_HOTSPRING_GUESS, &respond);
}

int HotspringActor::notify_player_info(void)
{
//	Proto80400101 respond;

//	FINER_PROCESS_RETURN(RETURN_HOTSPRING_GUESS, &respond);
	return 0;
}

int HotspringActor::notify_player_get_hotspring_award()
{

	return 0;
}
