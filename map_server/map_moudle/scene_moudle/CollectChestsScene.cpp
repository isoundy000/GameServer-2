/*
 * CollectChestsScene.cpp
 *
 *  Created on: 2016年8月9日
 *      Author: lzy0927
 */

#include "CollectChestsScene.h"
#include "AIManager.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "SerialRecord.h"

void CollectChestsScene::CheckStartTimer::set_act_type(int type)
{
	this->act_type_ = type;
}

int CollectChestsScene::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int CollectChestsScene::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return COLLECTCHESTS_INSTANCE->handle_chests_timeout(this->act_type_);
}

int CollectChestsScene::ActivityStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int CollectChestsScene::ActivityStartTimer::handle_timeout(const Time_Value &tv)
{
	COLLECTCHESTS_INSTANCE->new_start();
	return 0;
}

int CollectChestsScene::RefreshTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int CollectChestsScene::RefreshTimer::handle_timeout(const Time_Value &tv)
{

	time_t now_tick = ::time(NULL);
	COLLECTCHESTS_INSTANCE->handle_refresh_chests(now_tick);

	return 0;
}

int CollectChestsScene::CountDownTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int CollectChestsScene::CountDownTimer::handle_timeout(const Time_Value &tv)
{
	return COLLECTCHESTS_INSTANCE->notify_refresh_chests();
}

int CollectChestsScene::BroadcastTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int CollectChestsScene::BroadcastTimer::handle_timeout(const Time_Value &tv)
{
	return COLLECTCHESTS_INSTANCE->handle_broadcast_timer();
}

CollectChestsScene::CollectChestsScene()
{

}

CollectChestsScene::~CollectChestsScene()
{

}

int CollectChestsScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	this->handle_activity_ai_die(game_ai, benefited_attackor);

	return 0;
}

int CollectChestsScene::handle_ai_alive_recycle(GameAI* game_ai)
{
	return this->erase_notify_alive_info(game_ai);
}


IntPair CollectChestsScene::fetch_addition_exp()
{
	return IntPair(0, 0);
}

CollectChestsScene* CollectChestsScene::instance()
{
	static CollectChestsScene* instance = NULL;

	if (instance == NULL)
	{
		instance = new CollectChestsScene;
	}

	return instance;
}

int CollectChestsScene::erase_notify_alive_info(GameAI* game_ai)
{
	if (game_ai != NULL)
	{
		this->chests_detail_.alive_collect_.erase(game_ai->ai_id());
	}

	Proto80400400 alive_info;
	alive_info.set_chest_id(game_ai->ai_id());

	this->notify_all_player_msg(ACTIVE_ERASE_CHESTS, &alive_info);

	return 0;
}


int CollectChestsScene::request_enter_collect_chests(int gate_id, Proto30400051* request)
{

//	this->new_start(Scene* scene);//test

	int scene_id = CONFIG_INSTANCE->collect_chests_json(this->chests_detail_.cycle_id_)["scene_id"].asInt();
	Proto30400052 enter_info;
	enter_info.set_space_id(0);
	enter_info.set_scene_mode(SCENE_MODE_NORMAL);//need change
	enter_info.set_scene_id(scene_id);
	enter_info.set_enter_type(request->enter_type());

	const Json::Value& enter_pos = CONFIG_INSTANCE->collect_chests_json(this->chests_detail_.cycle_id_)["enter_pos"];
	int rand_index = std::rand() % enter_pos.size();

	MoverCoord enter_coord;
	enter_coord.set_pixel(enter_pos[rand_index][0u].asInt(),
			enter_pos[rand_index][1u].asInt());

	const Json::Value& collect_chests_json = CONFIG_INSTANCE->chests_table_json();
	int chest_index = std::rand() % collect_chests_json.size() + 1;
	const Json::Value& chest_json = CONFIG_INSTANCE->chests_table_json(chest_index);

	int x = chest_json["coordinate"][0u].asInt();
	int y = chest_json["coordinate"][1u].asInt();

	if (x > 0 && y > 0)
	{
		enter_coord.set_pixel(x, y);
	}

	enter_info.set_pos_x(enter_coord.pixel_x());
	enter_info.set_pos_y(enter_coord.pixel_y());

	return MAP_MONITOR->respond_enter_scene_begin(gate_id, request, &enter_info);
}


int CollectChestsScene::handle_broadcast_timer()//need change
{
	this->broad_timer_.cancel_timer();

	this->chests_detail_.broad_times_ += 1;
	JUDGE_RETURN(this->chests_detail_.broad_times_ < 5, -1);

	this->broad_timer_.schedule_timer(Time_Value(Time_Value::HOUR));
//	MAP_MONITOR->announce_world(SHOUT_ALL_START_XIANYE_CRAZY);

	return 0;
}


int CollectChestsScene::check_and_start_broadcast(int start_tick)//need change
{
	JUDGE_RETURN(start_tick > this->chests_detail_.start_tick_, -1);
/*
	this->chests_detail_.start_tick_ = start_tick;
	JUDGE_RETURN(this->broad_timer_.is_registered() == false, -1);

	this->chests_detail_.broad_times_ = 0;
	this->broad_timer_.schedule_timer(Time_Value(Time_Value::MINUTE));

	MAP_MONITOR->announce_world(SHOUT_ALL_START_XIANYE_CRAZY);
	MAP_MONITOR->notify_all_player_info(ACTIVE_XIANYE_CRAZY_TIPS);

	MSG_USER("start tianye broadcast timer....");
	*/
	return 0;
}


int CollectChestsScene::handle_refresh_chests(time_t now_tick)
{
//	JUDGE_RETURN(now_tick > this->lvye_detail_.next_hour_tick_, -1)
//	JUDGE_RETURN(now_tick >= this->lvye_detail_.next_min_tick_, -1);

	if (this->chests_detail_.cur_stage_>= this->chests_detail_.total_stage_)
	{
//		this->new_stop();
		return 0;
	}

	int cycle_id = this->chests_detail_.cycle_id_;
	int stage = this->chests_detail_.cur_stage_;

 	int chests_num = CONFIG_INSTANCE->collect_chests_json(cycle_id)["chest_sum"][stage].asInt();

	int chests_total = (int)CONFIG_INSTANCE->chests_table_json().size();
	int result[chests_num];
	int data[chests_total];
	int i = 0, index = 0;

	for (i = 0; i < chests_total; i++)  //初始化
	{
		data[i] = i;
	}

	for (i = 0; i < chests_num; i++)
	{
	     index = rand() % (chests_total - i);
	     result[i] = data[index];
	     data[index] = data[chests_total - i - 1];
	}

	//当前宝箱清除
	this->chests_detail_.alive_collect_.clear();
	this->chests_detail_.count_ = 0;
	this->chests_detail_.left_ = 0;

	CollectChestsDetail::ChestsMap::iterator iter;
	for (i = 0 ; i < chests_num; ++i)
	{
		iter = this->chests_detail_.chest_map_.find(result[i]+1);
		handle_refresh_chests(iter->second);
	}
	this->chests_detail_.cur_stage_ ++;
	this->count_down_timer_.cancel_timer();
	this->count_down_timer_.schedule_timer(this->chests_detail_.refresh_ - 10);

	this->refresh_timer_.cancel_timer();
	this->refresh_timer_.schedule_timer(this->chests_detail_.refresh_);
	return 0;
}

int CollectChestsScene::notify_refresh_chests()
{
	this->count_down_timer_.cancel_timer();
	Proto80405002 alive_info;
	alive_info.set_chests_sum(CONFIG_INSTANCE->collect_chests_json(this->chests_detail_.cycle_id_)
			["chests_sum"][this->chests_detail_.cur_stage_].asInt());
	LongMap::iterator it = this->chests_detail_.alive_collect_.begin();

	for (;it != this->chests_detail_.alive_collect_.end(); ++it)
	{
		alive_info.add_chests_list(it->second);
	}

	this->scene_->notify_all_player_msg(ACTIVE_GET_NEW_CHESTS, &alive_info);
	return 0;
}

int CollectChestsScene::handle_refresh_chests(CollectChestsDetail::ChestsItem& item)
{
//	this->chests_detail_.cur_stage_++;


	MoverCoord born_coord;
//	GameCommon::fetch_rand_coord(born_coord, this->scene_id());
	born_coord.set_pixel(item.coord_x_ , item.coord_y_);

	Int64 monster_index = AIMANAGER->generate_monster_by_sort(item.monster_id_, born_coord, this->scene_);

	GameAI* game_ai = this->scene_->find_ai(monster_index);
	JUDGE_RETURN(game_ai != NULL, -1);

	this->chests_detail_.alive_collect_[this->chests_detail_.count_] = game_ai->ai_id();
	this->chests_detail_.count_ ++;
	this->chests_detail_.left_ ++;

	return 0;
}

int CollectChestsScene::get_cycle_id()
{
	return this->chests_detail_.cycle_id_;
}

ActivityTimeInfo &CollectChestsScene::get_time_info()
{
	return this->time_info_map[this->chests_detail_.cycle_id_];
}

int CollectChestsScene::handle_chests_timeout(int type)
{
	this->chests_detail_.cycle_id_ = type;
	int last_state = this->time_info_map[this->chests_detail_.cycle_id_].cur_state_;
	this->time_info_map[this->chests_detail_.cycle_id_].set_next_stage();
	return this->handle_chests_i(last_state);
}

int CollectChestsScene::handle_chests_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_chests_event();
		break;
	}

	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->new_start();
		break;
	}

	case GameEnum::ACTIVITY_STATE_START:
	{
		this->new_stop();
		break;
	}
	}

	this->check_start_timer_map[this->chests_detail_.cycle_id_].cancel_timer();
	this->check_start_timer_map[this->chests_detail_.cycle_id_].schedule_timer(
			this->time_info_map[this->chests_detail_.cycle_id_].refresh_time_);

	return 0;
}

int CollectChestsScene::ahead_chests_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->chests_detail_.cycle_id_,
			this->time_info_map[this->chests_detail_.cycle_id_].refresh_time_);

	return 0;
}

void CollectChestsScene::init_collect_chests(Scene* scene)
{
	this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].set_act_type(
			GameEnum::FIRST_COLLECT_CHESTS_ID);
	this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].set_act_type(
			GameEnum::SECOND_COLLECT_CHESTS_ID);

	MSG_USER("COLLECT_SYS %d %d %d", this->chests_detail_.alive_collect_.size(),
			this->chests_detail_.chest_map_.size(), this->chests_detail_.player_num.size());
	time_t lt;
	lt =time(0);
	struct tm *local = localtime(&lt);

	this->chests_detail_.cur_year = local->tm_year + 1900;
	this->chests_detail_.cur_month = local->tm_mon + 1;
	this->chests_detail_.cur_day = local->tm_mday;

	this->scene_ = scene;
	this->start_timer_.cancel_timer();
	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();

//	this->start_timer_.schedule_timer(this->next_start_times());

	this->chests_detail_.chest_map_.clear();

	const Json::Value& chest_table = CONFIG_INSTANCE->chests_table_json();
	Json::Value::iterator iter = chest_table.begin();
	for (int i = 1; iter != chest_table.end(); ++i, ++iter)
	{
		Json::Value &chest = *iter;
		CollectChestsDetail::ChestsItem temp;
		temp.coord_x_ = chest["coordinate"][0u].asInt();
		temp.coord_y_ = chest["coordinate"][1u].asInt();
		temp.award_id_ = chest["award_id"].asInt();
		temp.monster_id_ = chest["monster_id"].asInt();
		temp.chest_id_ = chest["chest_id"].asInt();

		this->chests_detail_.chest_map_.insert(CollectChestsDetail::ChestsMap::value_type(i, temp));
	}

	//第一场宝箱
	const Json::Value& first_activity_conf = CONFIG_INSTANCE->common_activity(GameEnum::FIRST_COLLECT_CHESTS_ID);
	JUDGE_RETURN(first_activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID], first_activity_conf);

	//第二场宝箱
	const Json::Value& second_activity_conf = CONFIG_INSTANCE->common_activity(GameEnum::SECOND_COLLECT_CHESTS_ID);
	JUDGE_RETURN(second_activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID], second_activity_conf);

    this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].cancel_timer();
    this->check_start_timer_map[GameEnum::FIRST_COLLECT_CHESTS_ID].schedule_timer(
    		this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID].refresh_time_);

    this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].cancel_timer();
    this->check_start_timer_map[GameEnum::SECOND_COLLECT_CHESTS_ID].schedule_timer(
    		this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID].refresh_time_);

	MSG_USER("COLLECT_SYS %d %d %d", this->chests_detail_.alive_collect_.size(),
			this->chests_detail_.chest_map_.size(), this->chests_detail_.player_num.size());

    if (this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
    	this->chests_detail_.cycle_id_ = GameEnum::FIRST_COLLECT_CHESTS_ID;
    	this->new_start();
    }

    if (this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
    	this->chests_detail_.cycle_id_ = GameEnum::SECOND_COLLECT_CHESTS_ID;
    	this->new_start();
    }

	this->chests_detail_.cycle_id_ = (this->time_info_map[GameEnum::FIRST_COLLECT_CHESTS_ID].fetch_left_time() <
						this->time_info_map[GameEnum::SECOND_COLLECT_CHESTS_ID].fetch_left_time()) ?
						GameEnum::FIRST_COLLECT_CHESTS_ID : GameEnum::SECOND_COLLECT_CHESTS_ID;
}

void CollectChestsScene::new_start()
{
	for (MoverMap::iterator iter = this->scene_->player_map().begin();
			iter != this->scene_->player_map().end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);

		Proto31400190 info;
		info.set_player_id(map_player->role_id());
		map_player->send_to_logic_thread(info);
	}
	Int64 now_tick = ::time(NULL);

	const Json::Value& activity = CONFIG_INSTANCE->collect_chests_json(
			this->chests_detail_.cycle_id_);

	this->chests_detail_.cur_stage_ = 0;
	this->chests_detail_.total_stage_ = activity["refresh_num"].asInt();
	this->chests_detail_.count_ = 0;
	this->chests_detail_.broad_times_ = 0;
	this->chests_detail_.left_ = 0;
	this->chests_detail_.refresh_ = activity["interval"].asInt();

	this->chests_detail_.player_num.clear();
/*
	const Json::Value& start_time = CONFIG_INSTANCE->collect_chests("start_time");
	int temp = start_time[0u].asInt() * 100 + start_time[1u].asInt();
	this->chests_detail_.start_tick_ = GameCommon::fetch_day_sec(temp);

	const Json::Value& end_time = CONFIG_INSTANCE->collect_chests("end_time");
	temp = end_time[0u].asInt() * 100 + end_time[1u].asInt();
	this->chests_detail_.stop_tick_ = GameCommon::fetch_day_sec(temp);
*/
	time_t lt;
	lt =time(0);
	struct tm *local = localtime(&lt);

	this->chests_detail_.cur_year = local->tm_year + 1900;
	this->chests_detail_.cur_month = local->tm_mon + 1;
	this->chests_detail_.cur_day = local->tm_mday;

	handle_refresh_chests(now_tick);

	int shout_id = CONFIG_INSTANCE->collect_chests_json(this->chests_detail_.cycle_id_)["shout_start"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_start(this->chests_detail_.cycle_id_,
			this->time_info_map[this->chests_detail_.cycle_id_].refresh_time_);
}

void CollectChestsScene::new_stop()
{
	LongMap::iterator it = this->chests_detail_.alive_collect_.begin();

	for (;it != this->chests_detail_.alive_collect_.end(); ++it)
	{
		this->scene_->recycle_one_mover(it->second);
	}

	int player_num = 0, player_finish_num = 0;
	LongMap::iterator iter = this->chests_detail_.player_num.begin();
	for (; iter != this->chests_detail_.player_num.end(); ++iter)
	{
		player_num++;
		if (iter->second >= MAX_NUM) player_finish_num++;
	}
	SERIAL_RECORD->record_activity(SERIAL_ACT_CHESTS, player_num, player_finish_num);

	this->chests_detail_.player_num.clear();
	this->start_timer_.cancel_timer();
	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();

	int shout_id = CONFIG_INSTANCE->collect_chests_json(this->chests_detail_.cycle_id_)["shout_end"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_stop(this->chests_detail_.cycle_id_);
	MSG_USER("COLLECT_SYS %d %d %d", this->chests_detail_.alive_collect_.size(),
			this->chests_detail_.chest_map_.size(), this->chests_detail_.player_num.size());
}

int CollectChestsScene::is_player_have_join(Int64 role_id)
{
	return this->chests_detail_.player_num.count(role_id);
}

void CollectChestsScene::handle_activity_ai_die(Int64 killer_id)
{
	if (this->chests_detail_.player_num.count(killer_id) != 0)
	{
		LongMap::iterator it = this->chests_detail_.player_num.find(killer_id);
		it->second++;
	}
	else
	{
		this->chests_detail_.player_num.insert(IntMap::value_type (killer_id, 1));
	}
}

void CollectChestsScene::handle_activity_ai_die(GameAI* game_ai, Int64 killer_id)
{

	if (this->chests_detail_.player_num.count(killer_id) != 0)
	{
		LongMap::iterator it = this->chests_detail_.player_num.find(killer_id);
		it->second++;
	}
	else
	{
		this->chests_detail_.player_num.insert(IntMap::value_type (killer_id, 1));
	}


//	MapPlayerEx* player = this->find_player(game_ai->ai_detail().__killed_id);
//	JUDGE_RETURN(player != NULL, ;);
//
//	player->add_task_item_ai(game_ai);
}

void CollectChestsScene::test_chests(int id, int set_time)
{
	this->chests_detail_.cycle_id_ = GameEnum::FIRST_COLLECT_CHESTS_ID;
	this->time_info_map[this->get_cycle_id()].cur_state_ = (id + 1) % this->time_info_map[this->get_cycle_id()].time_span_;
	this->time_info_map[this->get_cycle_id()].refresh_time_ = set_time;
	this->handle_chests_i(id);
}

