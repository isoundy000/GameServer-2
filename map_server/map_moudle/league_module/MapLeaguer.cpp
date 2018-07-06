/*
 * MapLeaguer.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: peizhibi
 */

#include "MapLeaguer.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "AIManager.h"
#include "ProtoDefine.h"
#include "LeagueMonitor.h"
#include "AreaField.h"

#include "GameAI.h"
#include "AreaMonitor.h"
#include "CollectChestsScene.h"
#include "AnswerActivityScene.h"
#include "HotspringActivityScene.h"
#include "SerialRecord.h"
#include "LeagueBoss.h"

MapLeaguerInfo::MapLeaguerInfo()
{
	this->skill_prop_set_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	this->skill_prop_set_.push_back(GameEnum::ATTACK);
	this->skill_prop_set_.push_back(GameEnum::DEFENSE);
	this->skill_prop_set_.push_back(GameEnum::HIT);
	this->skill_prop_set_.push_back(GameEnum::AVOID);
	this->skill_prop_set_.push_back(GameEnum::BLOOD_MAX);

	MapLeaguerInfo::reset();
}

void MapLeaguerInfo::reset()
{
	this->flag_lvl_ = 0;
	this->leader_id_ = 0;
	this->skill_map_.clear();
}

MapLeaguer::MapLeaguer()
{
	// TODO Auto-generated constructor stub
	this->escort_refresh_timer_.father_ = this;
	this->escort_refresh_timer_.cancel_timer();
}

MapLeaguer::~MapLeaguer()
{
	// TODO Auto-generated destructor stub
	this->escort_refresh_timer_.cancel_timer();
}

int MapLeaguer::RefreshTimer::type(void)
{
	return GTT_MAP_MONSTER;
}

int MapLeaguer::RefreshTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();
	this->father_->escort_refresh();
	return 0;
}

int MapLeaguer::enter_scene(const int type)
{
	switch (this->scene_id())
	{
	case GameEnum::ARENA_SCENE_ID:
	{
		return this->enter_area_field(type);
	}
	case GameEnum::LBOSS_SCENE_ID:
	{
		return this->enter_league_boss(type);
	}
	case GameEnum::ANSWER_ACTIVITY_SCENE_ID:
	{
		return this->enter_answer_activity(type);
	}
	case GameEnum::HOTSPRING_SCENE_ID:
	{
		return this->enter_hotspring_activity(type);
	}
	}

	return 0;
}

int MapLeaguer::exit_scene(const int type)
{
	this->leaguer_exit_scene(type);
	return MapPlayer::exit_scene(type);
}

int MapLeaguer::sign_out(const bool is_save_player)
{
	this->escort_stop_protect();
    return 0;
}


int MapLeaguer::obtain_area_info(int request_flag)
{
    return MapPlayer::obtain_area_info(request_flag);
}

int MapLeaguer::die_process(const int64_t fighter_id)
{
	MapPlayer::die_process(fighter_id);

	this->area_die_process(fighter_id);
	return 0;
}

int MapLeaguer::modify_blood_by_fight(const double value, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	int real_value = MapPlayer::modify_blood_by_fight(value, fight_tips, attackor, skill_id);
	this->notify_arena_fight_info();
	return real_value;
}

int MapLeaguer::modify_magic_by_notify(const double value, const int fight_tips)
{
	int real_value = MapPlayer::modify_magic_by_notify(value, fight_tips);
	this->notify_arena_fight_info();
	return real_value;
}

int MapLeaguer::request_relive(Message* msg)
{
	return MapPlayer::request_relive(msg);
}

int MapLeaguer::notify_client_enter_info()
{
	return MapPlayer::notify_client_enter_info();
}

int MapLeaguer::is_in_safe_area()
{
	return MapPlayer::is_in_safe_area();
}

int MapLeaguer::request_exit_league()
{
	this->transfer_to_save_scene();
	return 0;
}

int MapLeaguer::enter_area_field(const int type)
{
	AreaField* area_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(area_field != NULL && area_field->validate_enter(
			this->role_id()) == true, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(area_field);
	this->set_camp_id(MAP_MONITOR->generate_camp_id());

	this->mover_detial_.__toward = 0;

	this->notify_enter_arena_info();
	this->fighter_check_and_restore_all();

	return MapPlayer::enter_scene(type);
}


int MapLeaguer::enter_league_boss(const int type)
{
	LeagueBoss* league_boss = LEAGUE_MONITOR->find_boss(this->league_id());
	JUDGE_RETURN(league_boss != NULL, ERROR_LEAGUE_NO_EXIST);

	this->init_mover_scene(league_boss);
//	this->set_camp_id(GameEnum::CAMP_ONE);
	MapPlayer::enter_scene(type);

	league_boss->enter_player(this->role_id());

	return 0;
}

int MapLeaguer::enter_collectchests(const int type)
{
	this->init_mover_scene(COLLECTCHESTS_INSTANCE);
	this->set_camp_id(GameEnum::CAMP_ONE);

	MapPlayer::enter_scene(type);

	return 0;
}

int MapLeaguer::enter_hotspring_activity(const int type)
{
	Scene* bonfire = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_scene(0,
		GameEnum::HOTSPRING_SCENE_ID, bonfire) == 0, ERROR_SCENE_NO_EXISTS);

	MapPlayerEx* player = dynamic_cast<MapPlayerEx *>(this);
	player->hotspring_activity_start();
	this->init_mover_scene(bonfire);
	this->set_camp_id(GameEnum::CAMP_ONE);

	MapPlayer::enter_scene(type);

	return 0;
}

int MapLeaguer::enter_answer_activity(const int type)
{
	Scene* bonfire = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_scene(0,
		GameEnum::ANSWER_ACTIVITY_SCENE_ID, bonfire) == 0, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(bonfire);
	this->set_camp_id(GameEnum::CAMP_ONE);

	MapPlayer::enter_scene(type);

	return 0;
}

int MapLeaguer::request_player_wait_time()
{
	return ANSWERACTIVITY_INSTANCE->player_wait_time(this->role_id());
}

void MapLeaguer::leaguer_reset()
{
	this->map_leaguer_info_.reset();
	this->escort_refresh_timer_.cancel_timer();
}

void MapLeaguer::leaguer_set_nextday()
{
}

void MapLeaguer::leaguer_exit_scene(int type)
{
}

int MapLeaguer::set_self_league_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400423*, request, -1);

	this->send_to_logic_thread(*request);

	this->role_detail_.__league_id = request->league_index();
	this->role_detail_.__league_name = request->league_name();

	this->notify_update_player_info(GameEnum::PLAYER_INFO_LEAGUE);
	this->check_cur_scene_validate();

	return 0;
}

int MapLeaguer::check_cur_scene_validate()
{
	JUDGE_RETURN(this->league_id() == 0, -1);

	const Json::Value& validate_scene = CONFIG_INSTANCE->tiny("no_quit_league");
	JUDGE_RETURN(GameCommon::is_value_in_config(validate_scene, this->scene_id()) == true, -1);

	return this->request_exit_league();
}

int MapLeaguer::notify_enter_arena_info()
{
	AreaField* arena_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(arena_field != NULL, -1);

	Proto80400351 enter_info;
	arena_field->fetch_arena_enter_info(&enter_info);

	FINER_PROCESS_RETURN(ACTIVE_ARENA_ENTER_INFO, &enter_info);
}

int MapLeaguer::notify_arena_fight_info()
{
	JUDGE_RETURN(this->scene_id() == GameEnum::ARENA_SCENE_ID, -1);

	AreaField* arena_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(arena_field != NULL, -1);

	return arena_field->notify_all_fight_info();
}


int MapLeaguer::request_enter_league_boss_begin()
{
	CONDITION_NOTIFY_RETURN(this->league_id() > 0,
			RETURN_ENTER_LEAGUE_BOSS, ERROR_LEAGUE_NO_EXIST);

	CONDITION_NOTIFY_RETURN(this->scene_id() != GameEnum::LBOSS_SCENE_ID,
			RETURN_ENTER_LEAGUE_BOSS, ERROR_IN_LEAGUE_BOSS);

	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_ENTER_LEAGUE_BOSS, ERROR_NORMAL_SCENE);

	Proto30400051 enter_info;
	enter_info.set_league_index(this->league_id());
	enter_info.set_enter_type(GameEnum::ET_LEAGUE_BOSS);

	int ret = this->send_request_enter_info(GameEnum::LBOSS_SCENE_ID, enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_ENTER_LEAGUE_BOSS, ret);

	return 0;
}

int MapLeaguer::upgrade_escort_level(Message* msg)
{
	JUDGE_RETURN(this->add_validate_operate_tick(0.4, GameEnum::ESCORT_OPERATE) == true, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401524*, request, RETURN_ESCORT_UPGRADE);

	Escort_detail &item = this->get_escort_detail();
    item.target_level = std::min<int>(request->target_level(), 4);
    item.till = request->till();

	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
    int item_id = convoy["upgrade_item_id"].asInt();
    int item_num = 0;

    if (item.till)
    {
    	for (int i = item.escort_type(); i < item.target_level; ++i)
    	{
    		item_num += convoy["item_num_list"][i-1].asInt();
    	}
    }
    else
    {
    	if (item.escort_type() <= (int)convoy["item_num_list"].size())
    	{
    		item_num = convoy["item_num_list"][item.escort_type() - 1].asInt();
    	}
    }

    if (item_num == 0)
    {
    	Proto30400202 temp;
    	temp.set_error(0);
    	this->upgrade_escort_level_done(&temp);
    }
    else
    {
    	this->request_use_item(INNER_MAP_USE_GOODS, ITEM_UPGRADE_ESCORT,
    			item_id, item_num, request->auto_buy(), 1);
    }

	return 0;
}

int MapLeaguer::upgrade_escort_level_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400202*, request, RETURN_ESCORT_UPGRADE);
	CONDITION_NOTIFY_RETURN(request->error() == 0, RETURN_ESCORT_UPGRADE, request->error());

    Escort_detail &item = this->get_escort_detail();
	int escort_type = item.till == 0 ? item.escort_type() + 1 : item.target_level;
	item.set_escort_type(escort_type);

	return this->get_activity_status(GameEnum::ESCORT_INFO_UPGRADE);
}

int MapLeaguer::respond_escort_upgrade_info()
{
//    Escort_detail &item = this->get_escort_detail();
////    const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
//
//	Proto50401524 respond;
//	respond.set_cur_level(item.escort_type_);
//	respond.set_status(1);
//	FINER_PROCESS_RETURN(RETURN_ESCORT_UPGRADE, &respond);
	return this->respond_escort_open_info();
}

int MapLeaguer::open_pescort_car_info(int type)
{
	int need_lvl = CONFIG_INSTANCE->convoy_json()["need_level"].asInt();
	JUDGE_RETURN(this->role_detail().__level >= need_lvl, 0);

	this->generate_escort_info();
	this->get_activity_status(GameEnum::ESCORT_INFO_OPEN);

    return 0;
}

int MapLeaguer::respond_escort_open_info()
{
    Escort_detail &item = this->get_escort_detail();
    const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();

	Proto50401523 open_info;
	open_info.set_escort_times(convoy["escort_times"].asInt() - this->role_detail().escort_times);
	open_info.set_protect_times(convoy["protect_times"].asInt() - this->role_detail().protect_times);
	open_info.set_rob_times(convoy["rob_times"].asInt() - this->role_detail().rob_times);
	open_info.set_convoy_exp(item.total_exp_);
	open_info.set_cur_level(item.escort_type());
	open_info.set_start_tick(item.start_tick_);

	FINER_PROCESS_RETURN(RETURN_ESCORT_OPEN_INFO, &open_info);
}

int MapLeaguer::select_pescort_car_begin(Message* msg)
{
	int need_lvl = CONFIG_INSTANCE->convoy_json()["need_level"].asInt();
	CONDITION_NOTIFY_RETURN(this->level() >= need_lvl, RETURN_ESCORT_SELECT_CAR,
			ERROR_PLAYER_LEVEL);

	MSG_DYNAMIC_CAST_NOTIFY(Proto10401522*, request, RETURN_ESCORT_SELECT_CAR);

    Escort_detail &item = this->get_escort_detail();
	CONDITION_NOTIFY_RETURN(item.car_index_ == 0, RETURN_ESCORT_SELECT_CAR,
			ERROR_HAVE_LEAGUE_ESCORT);

	int max_times = CONFIG_INSTANCE->convoy_json()["escort_times"].asInt();
	CONDITION_NOTIFY_RETURN(this->role_detail().escort_times < max_times,
			RETURN_ESCORT_SELECT_CAR, ERROR_LESCORT_NOTIMES);

	item.start_tick_ = ::time(NULL);
	item.car_index_ = this->role_id();

	MapPlayerEx* player = this->self_player();
	player->notify_quit_trvl_team();
	player->update_beast_state(1, MapMaster::TYPE_ESCORT);

	return this->get_activity_status(GameEnum::ESCORT_INFO_SELECT);
}

int MapLeaguer::escort_seek_help()
{
    Escort_detail &item = this->get_escort_detail();
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, this->name());

	string npc_name = convoy["npc_name"][item.escort_type() - 1].asString();
	GameCommon::push_brocast_para_string(para_vec, npc_name);

	int shout_id = convoy["shout_help"].asInt();
	GameCommon::announce(shout_id, &para_vec);

	Proto50401529 respond;
	respond.set_role_id(this->role_id());
	respond.set_role_name(this->role_detail().__name);
	respond.set_escort_level(item.escort_type());

	MoverCoord& temp = this->location();
	temp.serialize(respond.mutable_coord());
	respond.set_scene_id(this->scene_id());
	respond.set_space_id(this->space_id());
	respond.set_start_tick(item.start_tick_);
	FINER_PROCESS_RETURN(RETURN_ESCORT_SEEK_HELP, &respond);
}

int MapLeaguer::respond_escort_start_info()
{
    Escort_detail &item = this->get_escort_detail();

	Proto50401522 car_info;
	car_info.set_ai_id(item.car_index_);
	car_info.set_start_tick(item.start_tick_);

	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	if (item.escort_type()  == convoy["convoy_level"].asInt())
	{
		int reward_id = convoy["ext_award_id"].asInt();
		this->request_add_reward(reward_id, ADD_FROM_ESCORT_MOST_LEVEL);

		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, this->name());

		string npc_name = convoy["npc_name"][item.escort_type() - 1].asString();
		GameCommon::push_brocast_para_string(para_vec, npc_name);

		int shout_id = convoy["shout_most"].asInt();
		GameCommon::announce(shout_id, &para_vec);
	}

	this->act_serial_info_update(SERIAL_ACT_ESCORT, this->role_id(), 1);
	this->insert_escort_speed_buff();

//	this->insert_escort_speed_buff();
	FINER_PROCESS_RETURN(RETURN_ESCORT_SELECT_CAR, &car_info);
}

int MapLeaguer::select_pescort_car_done(const Int64 rob_id, const string& rob_name)
{
	JUDGE_RETURN(this->add_validate_operate_tick(3, GameEnum::ESCORT_OPERATE) == true, 0);

	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();

	int buff_id = convoy["buff_id"].asInt();
	this->remove_status(buff_id);

	Escort_detail &item = this->get_escort_detail();
	int max_times = CONFIG_INSTANCE->convoy_json()["escort_times"].asInt();

	if (this->role_detail().escort_times >= max_times)
	{
		//清除错误状态
		item.car_index_ = 0;
		return this->respond_to_client_error(RETURN_ESCORT_SELECT_CAR_DONE, ERROR_LESCORT_NOTIMES);
	}

	JUDGE_RETURN(item.car_index_ > 0 && item.escort_type() > 0, 0);

	this->role_detail().escort_times++;
	this->record_other_serial(ESCORT_SERIAL, 0,	item.escort_type(),
			item.escort_times_, item.total_exp_);

	MapPlayerEx* player = this->self_player();
	player->update_beast_state(0, MapMaster::TYPE_ESCORT);

	int status = 1;
	int scale = 0;

	if (item.escort_times_ != 2)
	{
		item.escort_times_ = 1;
	}

	if (rob_id > 0)
	{
		status = -1;
		scale = convoy["dead_scale"].asInt();
	}
	else if (::time(NULL) - item.start_tick_ >= convoy["escort_time"].asInt())
	{
		status = 0;
		scale = convoy["overtime_scale"].asInt();
	}
	else
	{
		status = 1;
		scale = 10000;
	}

	item.total_exp_ *= GameCommon::div_percent(scale);
	this->modify_element_experience(item.total_exp_, EXP_FROM_ESCORT);

	Proto50401525 respond;
	respond.set_is_double(item.escort_times_);
	respond.set_status(status);
	respond.set_convoy_exp(item.total_exp_);
	respond.set_convoy_scale(scale / 100);
	respond.set_left_time(max_times - this->role_detail().escort_times);

	if (status == 1)
	{
		for (LongSet::iterator it = item.protect_map.begin(); it != item.protect_map.end(); ++it)
		{
			MapPlayerEx* pro_player = NULL;
			JUDGE_CONTINUE(MAP_MONITOR->find_player(*it, pro_player) == 0);

			Escort_detail &player_item = pro_player->get_escort_detail();
			if (pro_player->role_detail().protect_times < convoy["protect_times"].asInt())
			{
				pro_player->request_add_reward(convoy["protect_award_id"].asInt(), ADD_FROM_ESCORT_PROTECT);
				pro_player->role_detail().protect_times ++;
			}

			player_item.protect_id = 0;

        	Proto80400409 respond;
        	respond.set_status(1);
        	respond.set_award_id(convoy["protect_award_id"].asInt());
        	respond.set_escort_times(convoy["escort_times"].asInt() - pro_player->role_detail().escort_times);
        	respond.set_protect_times(convoy["protect_times"].asInt() - pro_player->role_detail().protect_times);
        	respond.set_rob_times(convoy["rob_times"].asInt() - pro_player->role_detail().rob_times);
        	pro_player->respond_to_client(ACTIVE_ESCORT_REWARD, &respond);
		}

		//更新护送成功成就
		player->notify_ML_to_update_achievement(GameEnum::ESCORT_ACH, 1, 0, GameEnum::ACHIEVE_TYPE_1);
	}

	item.escort_type_ = 0;
	item.car_index_ = 0;
	item.start_tick_ = 0;
	item.protect_map.clear();

	this->update_sword_pool_info();
	this->update_cornucopia_task_info();
	this->update_labour_task_info();
	this->open_pescort_car_info();
	respond.set_rob_id(rob_id);
	respond.set_rob_name(rob_name);

	FINER_PROCESS_RETURN(RETURN_ESCORT_SELECT_CAR_DONE, &respond);
}

int MapLeaguer::transfer_to_escort_npc(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10400118 *, request, RETURN_TRANSFER_ESCORT);
//	JUDGE_RETURN( this->scene_id() == GameEnum::ESCORT_SCENE_ID, ERROR_CLIENT_OPERATE );
	MoverCoord point;
	point.set_pixel(request->pixel_x(), request->pixel_y());
	return this->transfer_dispatcher( GameEnum::ESCORT_SCENE_ID, point);
}

int MapLeaguer::escort_refresh(void)
{
	Escort_detail &my_item = this->get_escort_detail();

	MapPlayerEx* player = NULL;
	CONDITION_NOTIFY_RETURN(MAP_MONITOR->find_player(my_item.protect_id, player) == 0,
			ACTIVE_ESCORT_PROTECT_INFO, ERROR_ESCORT_PROTECT_ERROR);

    Escort_detail &item = player->get_escort_detail();
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();

	Proto80400408 respond;
	MoverCoord temp = player->location();
	temp.serialize(respond.mutable_coord());
	respond.set_scene_id(player->scene_id());
	respond.set_space_id(player->space_id());
	respond.set_start_tick(item.start_tick_);
	respond.set_car_id(item.car_index_);
	respond.set_protect_id(my_item.protect_id);
	respond.set_escort_times(convoy["escort_times"].asInt() - this->role_detail().escort_times);
	respond.set_protect_times(convoy["protect_times"].asInt() - this->role_detail().protect_times);
	respond.set_rob_times(convoy["rob_times"].asInt() - this->role_detail().rob_times);
	respond.set_protect_name(player->name());

	for (LongSet::iterator it = item.protect_map.begin(); it != item.protect_map.end(); ++it)
	{
		JUDGE_CONTINUE((*it) != this->role_id());
		MapPlayerEx* other_player = NULL;
		if (MAP_MONITOR->find_player(*it, other_player) == 0)
		{
			respond.add_other_protect(*it);
			respond.add_other_protect_name(other_player->name());
		}
	}

	FINER_PROCESS_RETURN(ACTIVE_ESCORT_PROTECT_INFO, &respond);
}

int MapLeaguer::escort_stop_protect()
{
	Escort_detail &my_item = this->get_escort_detail();
	if (my_item.protect_id == 0) return -1;

	MapPlayerEx* player = NULL;
//	CONDITION_NOTIFY_RETURN(MAP_MONITOR->find_player(my_item.protect_id, player) == 0,
//			RETURN_ESCORT_STOP_PROTECT, ERROR_ESCORT_PROTECT_ERROR);
	int ret = MAP_MONITOR->find_player(my_item.protect_id, player);
	JUDGE_RETURN(ret == 0, ret);

	player->insert_protect_buff();
	Proto50401528 respond;
	respond.set_player_id(player->role_id());
	respond.set_player_name(player->role_detail().__name);
	respond.set_protect_id(this->role_id());
	respond.set_protect_name(this->role_detail().__name);
	respond.set_player_type(2);
	player->respond_to_client(RETURN_ESCORT_STOP_PROTECT, &respond);

	respond.set_player_type(1);
	FINER_PROCESS_RETURN(RETURN_ESCORT_STOP_PROTECT, &respond);
}

int MapLeaguer::insert_escort_speed_buff()
{
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	return this->insert_defender_status(this, convoy["buff_id"].asInt(), 0,
			convoy["escort_time"].asInt(), 5, 0, convoy["buff_percent"].asDouble());
}

int MapLeaguer::insert_protect_buff()
{
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	int buff_id = convoy["protect_buff"].asInt();
	this->remove_status(buff_id);

	Escort_detail &item = this->get_escort_detail();
	int num = item.protect_map.size();

	if (num <= 0) return -1;

	int percent = convoy["protect_reduce"].asInt();
	int time = convoy["escort_time"].asInt();
	this->insert_defender_status(this, buff_id,
			double(time), 0, 5, 0, double(num) * double(percent));
	return 0;
}

int MapLeaguer::protect_escort(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10401526 *, request, RETURN_ESCORT_PROTECT);

//	EscortScene* escort_scene = NULL;
//    Scene* scene = this->fetch_scene();
//    if( dynamic_cast<EscortScene *>(scene) != 0 )
 //   {
 //       escort_scene = dynamic_cast<EscortScene *>(scene);
 //   }

//    if (scene == NULL) return -1;

    Int64 id = request->player_id();
    MapPlayerEx* player = NULL;
	CONDITION_NOTIFY_RETURN(MAP_MONITOR->find_player(id, player) == 0, RETURN_ESCORT_PROTECT, ERROR_ESCORT_PROTECT_ERROR);
    Escort_detail &item = player->get_escort_detail();
	Escort_detail &my_item = this->get_escort_detail();

	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
	CONDITION_NOTIFY_RETURN((int)item.protect_map.size() <= convoy["protect_limit"].asInt(),
			RETURN_ESCORT_PROTECT, ERROR_ESCORT_PROTECT_LIMIT);
	CONDITION_NOTIFY_RETURN(my_item.protect_id == 0, RETURN_ESCORT_PROTECT, ERROR_ESCORT_IS_PROTECT);

	item.protect_map.insert(this->role_id());
	my_item.protect_id = id;

	Proto50401526 respond;
	MoverCoord temp = player->location();
	temp.serialize(respond.mutable_coord());
	respond.set_scene_id(player->scene_id());
	respond.set_status(1);
	player->insert_protect_buff();
	this->escort_refresh_timer_.schedule_timer(0.3);

	FINER_PROCESS_RETURN(RETURN_ESCORT_PROTECT, &respond);
}

int MapLeaguer::wish_escort()
{
	const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();

	int percent = convoy["wish_reduce"].asInt();
	int buff_id = convoy["wish_buff"].asInt();
	int time = convoy["wish_time"].asInt();
	this->insert_defender_status(this, buff_id, 0, double(time), 5, 0, double(percent));

	FINER_PROCESS_NOTIFY(RETURN_ESCORT_WISH);
}

int MapLeaguer::rob_escort(const int64_t fighter_id)
{
    Escort_detail &item = this->get_escort_detail();
    JUDGE_RETURN(item.car_index_ > 0, -1);

	MapPlayerEx* player = NULL;
	if (MAP_MONITOR->find_player(fighter_id, player) == 0)
	{
		const Json::Value &convoy = CONFIG_INSTANCE->convoy_json();
		if (player->role_detail().rob_times < convoy["rob_times"].asInt())
		{
			int exp = item.total_exp_ * GameCommon::div_percent(convoy["rob_scale"].asInt());
			player->modify_element_experience(exp, EXP_FROM_ESCORT);
			player->role_detail().rob_times ++;

			Proto80400409 respond;
			respond.set_status(2);
			respond.set_exp_num(exp);
			respond.set_escort_times(convoy["escort_times"].asInt() - player->role_detail().escort_times);
			respond.set_protect_times(convoy["protect_times"].asInt() - player->role_detail().protect_times);
			respond.set_rob_times(convoy["rob_times"].asInt() - player->role_detail().rob_times);
			player->respond_to_client(ACTIVE_ESCORT_REWARD, &respond);
		}

		//更新打劫成功成就
		this->select_pescort_car_done(player->role_id(), player->name());
		player->notify_ML_to_update_achievement(GameEnum::ESCORT_ACH, 1, 0, GameEnum::ACHIEVE_TYPE_2);
	}
	else
	{
		this->select_pescort_car_done(fighter_id);
	}

	return 0;
}

int MapLeaguer::sync_transfer_leaguer()
{
	MapLeaguerInfo &leaguer_info = this->leaguer_info();

	Proto30400117 request;
	request.set_flag_lvl(leaguer_info.flag_lvl_);
	request.set_leader_id(leaguer_info.leader_id_);

	for (IntMap::iterator iter = leaguer_info.skill_map_.begin();
			iter != leaguer_info.skill_map_.end(); ++iter)
	{
		ProtoPairObj *obj = request.add_skill_map();
		obj->set_obj_id(iter->first);
		obj->set_obj_value(iter->second);
	}

	return this->send_to_other_scene(this->scene_id(), request);
}

int MapLeaguer::read_transfer_leaguer(Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400117*, request, -1);

    MapLeaguerInfo &leaguer_info = this->leaguer_info();
    leaguer_info.flag_lvl_ = request->flag_lvl();
    leaguer_info.leader_id_ = request->leader_id();

    for (int i = 0; i < request->skill_map_size(); ++i)
    {
    	ProtoPairObj obj = request->skill_map(i);
    	leaguer_info.skill_map_[obj.obj_id()] = obj.obj_value();
    }

    this->init_league_flag_property();
    this->init_league_skill_property();

	return 0;
}

void MapLeaguer::area_die_process(Int64 fighter_id)
{
	JUDGE_RETURN(this->scene_id() == GameEnum::ARENA_SCENE_ID, ;);

	AreaField* area_field = AREA_MONITOR->find_area_field(this->space_id());
	JUDGE_RETURN(area_field != NULL, ;);

	area_field->area_field_finish(this->mover_id());
}

int MapLeaguer::sync_league_fb_flag(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400448 *, request, -1);
	this->role_detail().fb_flag_ = request->fb_flag();
	return 0;
}

int MapLeaguer::update_sword_pool_info()
{
	Proto31402901 inner_res;
	inner_res.set_left_add_flag(1);
	inner_res.set_left_add_num(1);

	int task_id = GameEnum::SPOOL_TASK_ESCORT;
	inner_res.set_task_id(task_id);

	MSG_USER("MapLeaguer, update_sword_pool_info, Proto31402901: %s", inner_res.Utf8DebugString().c_str());

	return this->send_to_logic_thread(inner_res);
}

int MapLeaguer::update_leaguer_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400803 *, request, -1);

	MapLeaguerInfo &leaguer_info = this->leaguer_info();

	if (request->flag_lvl() > 0)
	{
		leaguer_info.flag_lvl_ = request->flag_lvl();
	}
	else
	{
		for (int i = 0; i < request->skill_map_size(); ++i)
		{
			ProtoPairObj obj = request->skill_map(i);
			if (obj.obj_id() != 0)
			{
				leaguer_info.skill_map_[obj.obj_id()] = obj.obj_value();
			}
		}
	}

	return 0;
}

void MapLeaguer::init_league_flag_property()
{
	JUDGE_RETURN(this->league_id() > 0, ;);

	MapLeaguerInfo &leaguer_info = this->leaguer_info();
	const Json::Value &flag_info = CONFIG_INSTANCE->league_flag(leaguer_info.flag_lvl_);

	Proto30400011 prop_info;
	prop_info.set_offset(BasicElement::LEAGUE_FLAG);
	prop_info.set_enter_type(ENTER_SCENE_TRANSFER);
	this->add_flag_attr(&prop_info, GameEnum::ATTACK, flag_info["attack"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::DEFENSE, flag_info["defence"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::CRIT, flag_info["crit"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::TOUGHNESS, flag_info["toughness"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::HIT, flag_info["hit"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::AVOID, flag_info["dodge"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::BLOOD_MAX, flag_info["health"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::DAMAGE_MULTI, flag_info["damage_multi"].asInt());
	this->add_flag_attr(&prop_info, GameEnum::REDUCTION_MULTI, flag_info["decrease_hurt"].asInt());

	Int64 role_id = this->role_id();
	if (role_id == leaguer_info.leader_id_)
	{
		int leader_attack = flag_info["leader_attack"].asInt();
		if (leader_attack > 0)
		{
			leader_attack += flag_info["attack"].asInt();
			this->add_flag_attr(&prop_info, GameEnum::ATTACK, leader_attack);
		}
	}

	this->logic_refresh_fight_property(&prop_info);
}

void MapLeaguer::init_league_skill_property()
{
	JUDGE_RETURN(this->league_id() > 0, ;);

	MapLeaguerInfo &leaguer_info = this->leaguer_info();

	Proto30400011 prop_info;
	prop_info.set_offset(BasicElement::LEAGUE_SKILL);
	prop_info.set_enter_type(ENTER_SCENE_TRANSFER);

	int refresh_flag = false;

	for (uint i = 0; i < leaguer_info.skill_prop_set_.size(); ++i) {
	int prop_index = leaguer_info.skill_prop_set_[i];

		int skill_lvl = leaguer_info.skill_map_[prop_index];
		JUDGE_CONTINUE(skill_lvl > 0);

		ProtoPairObj* pair_obj = prop_info.add_prop_set();
		JUDGE_CONTINUE(pair_obj != NULL);

		pair_obj->set_obj_id(prop_index);
		const Json::Value &skill_info = CONFIG_INSTANCE->league_skill_info(skill_lvl+1);
		switch (prop_index)
		{
		case GameEnum::ATTACK:
		{
			pair_obj->set_obj_value(skill_info["attack"].asInt());
			break;
		}
		case GameEnum::DEFENSE:
		{
			pair_obj->set_obj_value(skill_info["defence"].asInt());
			break;
		}
		case GameEnum::HIT:
		{
			pair_obj->set_obj_value(skill_info["hit"].asInt());
			break;
		}
		case GameEnum::AVOID:
		{
			pair_obj->set_obj_value(skill_info["avoid"].asInt());
			break;
		}
		case GameEnum::BLOOD_MAX:
		{
			pair_obj->set_obj_value(skill_info["blood_max"].asInt());
			break;
		}
		default:
			break;
		};

		refresh_flag = true;
	}

	JUDGE_RETURN(refresh_flag == true, ;);
	this->logic_refresh_fight_property(&prop_info);
}

void MapLeaguer::add_flag_attr(Proto30400011* request, int add_index, int attr)
{
	JUDGE_RETURN(request != NULL, ;);

	ProtoPairObj* pair_obj = request->add_prop_set();
	pair_obj->set_obj_id(add_index);
	pair_obj->set_obj_value(attr);
}

MapLeaguerInfo &MapLeaguer::leaguer_info()
{
	return this->map_leaguer_info_;
}

int MapLeaguer::update_cornucopia_task_info()
{
	Proto31403200 task_info;
	task_info.set_task_id(GameEnum::CORNUCOPIA_TASK_ESCORT);
	task_info.set_task_finish_count(1);
	MSG_USER("MapLeaguer, update_cornucopia_task_info, Proto31403200: %s", task_info.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}

int MapLeaguer::update_labour_task_info()
{
	Proto31403201 task_info;
	task_info.set_task_id(GameEnum::LABOUR_TASK_ESCORT);
	task_info.set_task_finish_count(1);
	MSG_USER("MapLeaguer, update_labour_task_info, Proto31403201: %s", task_info.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}
