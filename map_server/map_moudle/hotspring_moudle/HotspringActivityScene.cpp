/*
 * HotspringActivityScene.cpp
 *
 *  Created on: 2016年9月18日
 *      Author: lzy
 */

#include "HotspringActivityScene.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "SerialRecord.h"

void HotspringActivityScene::CheckStartTimer::set_act_type(int type)
{
	this->act_type_ = type;
}

int HotspringActivityScene::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int HotspringActivityScene::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return HOTSPRING_INSTANCE->handle_hotspring_timeout(this->act_type_);
}

int HotspringActivityScene::WaitTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int HotspringActivityScene::WaitTimer::handle_timeout(const Time_Value &tv)
{
	HOTSPRING_INSTANCE->wait_timeout();
	return 0;
}

int HotspringActivityScene::ActivityStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int HotspringActivityScene::ActivityStartTimer::handle_timeout(const Time_Value &tv)
{
	HOTSPRING_INSTANCE->start_timeout();
	return 0;
}


int HotspringActivityScene::handle_hotspring_timeout(int type)
{
	this->hotspring_detail_.cycle_id_ = type;
	ActivityTimeInfo &time_info_ = this->get_time_info();
	int last_state = time_info_.cur_state_;
	time_info_.set_next_stage();
	return this->handle_hotspring_i(last_state);
}


int HotspringActivityScene::handle_hotspring_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_hotspring_event();
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

	this->check_start_timer_map[this->hotspring_detail_.cycle_id_].cancel_timer();
	this->check_start_timer_map[this->hotspring_detail_.cycle_id_].schedule_timer
		(this->time_info_map_[this->hotspring_detail_.cycle_id_].refresh_time_);

	return 0;
}

int HotspringActivityScene::ahead_hotspring_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->hotspring_detail_.cycle_id_,
			this->time_info_map_[this->hotspring_detail_.cycle_id_].refresh_time_);

	return 0;
}

HotspringActivityScene::HotspringActivityScene()
{

}

HotspringActivityScene::~HotspringActivityScene()
{

}

HotspringActivityScene* HotspringActivityScene::instance()
{
	static HotspringActivityScene* instance = NULL;

	if (instance == NULL)
	{
		instance = new HotspringActivityScene;
	}

	return instance;
}

void HotspringActivityScene::new_start()
{
	this->hotspring_detail_.cur_answer = 0;
	this->hotspring_detail_.cur_second = 0;
	this->hotspring_detail_.cur_third = 0;
	this->hotspring_detail_.cur_state_answer_list_.clear();
	this->hotspring_detail_.double_major_list_.clear();
	this->hotspring_detail_.player_answer_list_.clear();
	this->hotspring_detail_.right_player_.clear();
	this->hotspring_detail_.exp_map_.clear();
	this->hotspring_detail_.cur_stage_ = 1;
	this->hotspring_detail_.cur_status = 1;
	this->hotspring_detail_.act_player_vip_info.clear();

	int shout_id = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_)["shout_start"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_start(this->hotspring_detail_.cycle_id_,
			this->time_info_map_[this->hotspring_detail_.cycle_id_].refresh_time_);

	this->wait_timer_.cancel_timer();
	this->wait_timer_.schedule_timer(this->hotspring_detail_.wait_time_);
}

void HotspringActivityScene::new_stop()
{
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);
		Int64 role_id = map_player->role_id();
		MapPlayerEx *actor  = NULL;
		if (MAP_MONITOR->find_player(role_id, actor) != 0) return ;
		actor->get_hotspring_detail().reset();
	}

	this->notify_player_get_award();
	//返回出生地
	LongVec player_set;
	this->fetch_player_set(player_set);

	for (LongVec::iterator iter = player_set.begin(); iter != player_set.end(); ++iter)
	{
		MapPlayerEx* player = NULL;
		JUDGE_CONTINUE(MAP_MONITOR->find_player_with_offline(*iter, player) == 0);

		if (GameCommon::is_normal_scene(player->prev_scene_id()) == false || player->prev_scene_mode() != SCENE_MODE_NORMAL)
		{
			player->transfer_to_born();
		}
		else
		{
			player->transfer_to_save_scene();
		}
	}

	int player_num = 0, player_vip_num = 0;
	LongMap::iterator it = this->hotspring_detail_.act_player_vip_info.begin();
	for (; it != this->hotspring_detail_.act_player_vip_info.end(); ++it)
	{
		player_num ++;
		if (it->second >= VIP_2) player_vip_num++;
	}
	SERIAL_RECORD->record_activity(SERIAL_ACT_HOTSPRING, player_num, player_vip_num);

 	this->hotspring_detail_.player_answer_list_.clear();
	this->hotspring_detail_.double_major_list_.clear();
	this->hotspring_detail_.right_player_.clear();
	this->hotspring_detail_.exp_map_.clear();
	this->hotspring_detail_.cur_state_answer_list_.clear();

	this->start_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();

	int shout_id = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_)["shout_end"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_stop(this->hotspring_detail_.cycle_id_);

	MSG_USER("HOTSPRING_SYS %d %d %d %d %d %d", this->hotspring_detail_.cur_state_answer_list_.size(),
			this->hotspring_detail_.act_player_vip_info.size(), this->hotspring_detail_.double_major_list_.size(),
			this->hotspring_detail_.exp_map_.size(), this->hotspring_detail_.player_answer_list_.size(),
			this->hotspring_detail_.right_player_.size());
}

int HotspringActivityScene::is_player_double_major(Int64 id)
{
	return this->hotspring_detail_.double_major_list_[id] != 0;
}

Int64 HotspringActivityScene::find_double_player(Int64 role_id)
{
	return this->hotspring_detail_.double_major_list_[role_id];
}

int HotspringActivityScene::is_player_has_choose(Int64 id)
{
	return this->hotspring_detail_.player_answer_list_[id] != 0;
}

int HotspringActivityScene::player_choose_answer(Int64 id, int answer)
{
	if (this->is_player_has_choose(id)) return -1;

	this->hotspring_detail_.player_answer_list_[id] = answer;
	this->hotspring_detail_.cur_state_answer_list_[id] = answer;
	return 1;
}

int HotspringActivityScene::unbind_double_major(Int64 player_a, Int64 player_b)
{
	if (!(this->is_player_double_major(player_a)))
	{
		return ERROR_FIRST_HAS_UNBIND;
	}

	if (!(this->is_player_double_major(player_b)))
	{
		return ERROR_SECOND_HAS_UNBIND;
	}

	LongMap::iterator it1 = this->hotspring_detail_.double_major_list_.find(player_a);
	this->hotspring_detail_.double_major_list_.erase(it1);

	LongMap::iterator it2 = this->hotspring_detail_.double_major_list_.find(player_b);
	this->hotspring_detail_.double_major_list_.erase(it2);

	return 1;
}

int HotspringActivityScene::bind_double_major(Int64 player_a, Int64 player_b)
{
	if (this->is_player_double_major(player_a))
	{
		return ERROR_FIRST_HAS_BIND;
	}

	if (this->is_player_double_major(player_b))
	{
		return ERROR_SECOND_HAS_BIND;
	}

	this->hotspring_detail_.double_major_list_[player_a] = player_b;
	this->hotspring_detail_.double_major_list_[player_b] = player_a;

	return 1;
}

int HotspringActivityScene::get_cycle_id()
{
	return this->hotspring_detail_.cycle_id_;
}

int HotspringActivityScene::get_cur_status()
{
	return this->hotspring_detail_.cur_status;
}

HotspringActivityDetail &HotspringActivityScene::get_hotspring_detail()
{
	return this->hotspring_detail_;
}

void HotspringActivityScene::start_timeout()
{
	this->start_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();
	this->wait_timer_.schedule_timer(this->hotspring_detail_.wait_time_);
	this->hotspring_detail_.cur_status = 1;
	this->hotspring_detail_.cur_stage_ ++;

	if (this->hotspring_detail_.cur_stage_ > this->hotspring_detail_.total_stage_)
	{
		this->hotspring_detail_.cur_stage_ = this->hotspring_detail_.total_stage_;
		this->wait_timer_.cancel_timer();
	}
	else
	{
		this->notify_player_start_timeout();
	}
}
void HotspringActivityScene::wait_timeout()
{
	this->wait_timer_.cancel_timer();
	this->start_timer_.cancel_timer();
	this->start_timer_.schedule_timer(this->hotspring_detail_.play_time_);
	this->hotspring_detail_.cur_status = 2;

	this->notify_player_wait_timeout();
}

void HotspringActivityScene::notify_player_get_award()
{
	for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayerEx *map_player = dynamic_cast<MapPlayerEx *>(player);
		map_player->notify_player_get_hotspring_award();
	}
}

void HotspringActivityScene::notify_player_wait_timeout()
{
	int first = std::rand() % 3;
	int second = std::rand() % 3;
	while(second == first) second = std::rand() % 3;
	int third = 3 - first - second;
	first++; second++; third++;

	this->hotspring_detail_.cur_answer = first;
	this->hotspring_detail_.cur_second = second;
	this->hotspring_detail_.cur_third = third;
	Proto80405017 respond;
	respond.set_first_npc(first);
	respond.set_second_npc(second);
	respond.set_third_npc(third);
	respond.set_status(this->hotspring_detail_.cur_status);

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayerEx *map_player = dynamic_cast<MapPlayerEx *>(player);
		Int64 role_id = map_player->role_id();
		respond.set_is_right(this->get_player_is_right(role_id));
		this->refresh_player_info(role_id);
		map_player->request_Hotspring_info();
		map_player->respond_to_client(ACTIVE_HOTSPRING_INFO, &respond);
	}
}

void HotspringActivityScene::notify_player_start_timeout()
{
	const Json::Value& hotspring_json = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_);
	Proto80405017 respond;
	respond.set_status(this->hotspring_detail_.cur_status);
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		MapPlayerEx *map_player = dynamic_cast<MapPlayerEx *>(player);
		Int64 role_id = map_player->role_id();
		int award_id = 0;

		if (this->hotspring_detail_.cur_state_answer_list_.count(role_id) != 0)
		{
			if (this->hotspring_detail_.cur_state_answer_list_[role_id] == this->hotspring_detail_.cur_answer)
			{
				award_id = this->hotspring_detail_.win_award_id; //need amend
				this->hotspring_detail_.right_player_[role_id] = 2;
			}
			else
			{
				award_id = this->hotspring_detail_.comfort_award_id; //need amend
				this->hotspring_detail_.right_player_[role_id] = 1;
			}
		}

		respond.set_is_right(this->get_player_is_right(role_id));
		this->refresh_player_info(role_id);

		map_player->request_Hotspring_info();
		map_player->respond_to_client(ACTIVE_HOTSPRING_INFO, &respond);


//		map_player->request_add_reward(award_id,  SerialObj(ITEM_HOTSPRING_ACTIVITY));

	}
	for (LongMap::iterator it = this->hotspring_detail_.cur_state_answer_list_.begin();
			it != this->hotspring_detail_.cur_state_answer_list_.end(); ++it)
	{
		Int64 role_id = it->first;
		int award_id = 0;
		if (it->second == this->hotspring_detail_.cur_answer)
		{
			award_id = this->hotspring_detail_.win_award_id; //need amend
			this->hotspring_detail_.right_player_[role_id] = 2;
		}
		else
		{
			award_id = this->hotspring_detail_.comfort_award_id; //need amend
			this->hotspring_detail_.right_player_[role_id] = 1;
		}

		int mail_id = hotspring_json["result_mail_id"].asInt();
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str());
		mail_info->add_goods(award_id);
		GameCommon::request_save_mail_content(role_id, mail_info);
	}
	const Json::Value &activity_json = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_);
	int shout_id = activity_json["shout_result"].asInt();
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, activity_json["npc"][this->hotspring_detail_.cur_answer - 1].asString());
	GameCommon::announce(shout_id, &para_vec);

	shout_id = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_)["shout_guess"].asInt();
	GameCommon::announce(shout_id);
	this->hotspring_detail_.cur_state_answer_list_.clear();
}

int HotspringActivityScene::refresh_player_info(Int64 id)
{
	MapPlayerEx *player  = this->find_player(id);
	JUDGE_RETURN(player != NULL, -1);

	HotspringDetail &temp_info = player->get_hotspring_detail();
	temp_info.cur_stage = this->hotspring_detail_.cur_stage_;
	temp_info.first_npc = this->hotspring_detail_.cur_answer;
	temp_info.is_right = this->get_player_is_right(id);
	temp_info.second_npc = this->hotspring_detail_.cur_second;
	temp_info.third_npc = this->hotspring_detail_.cur_third;
	temp_info.player_npc = this->hotspring_detail_.player_answer_list_[id];
	temp_info.double_major_player = this->hotspring_detail_.double_major_list_[id];

	return 0;
}

ActivityTimeInfo &HotspringActivityScene::get_time_info()
{
	return this->time_info_map_[this->hotspring_detail_.cycle_id_];
}

int HotspringActivityScene::request_enter_hotspring_activity(int gate_id, Proto30400051* request)
{

	Proto30400052 enter_info;
	enter_info.set_space_id(this->space_id());
	enter_info.set_scene_mode(SCENE_MODE_LEAGUE); //need amend

	JUDGE_RETURN(this->get_time_info().cur_state_ == GameEnum::ACTIVITY_STATE_START, -1);

	if (this->hotspring_detail_.right_player_.count(request->role_id()) == 0)
		this->hotspring_detail_.right_player_[request->role_id()] = 0;

	if (this->hotspring_detail_.exp_map_.count(request->role_id()) == 0)
		this->hotspring_detail_.exp_map_[request->role_id()] = 0;

	const Json::Value &act_json = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_);
	//need amend
	int x = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_)["scene_pos"][0u].asInt();
	int y = CONFIG_INSTANCE->hotspring_activity_json(this->hotspring_detail_.cycle_id_)["scene_pos"][1u].asInt();

	string str = "scene_area_list";
	int area_x_1 = act_json[str][0u].asInt(), area_x_2 = act_json[str][2u].asInt(),
		area_y_1 = act_json[str][1u].asInt(), area_y_2 = act_json[str][3u].asInt();

	int sub_1 = area_x_2 - area_x_1 > 0 ? area_x_2 - area_x_1 : 1,
		sub_2 = area_y_2 - area_y_1 > 0 ? area_y_2 - area_y_1 : 1;

	int f_x = area_x_1 + std::rand() % sub_1;
	int f_y = area_y_1 + std::rand() % sub_2;

	if (f_y > 0 && f_x > 0)
	{
		x = f_x; y = f_y;
	}

//	x = 1665;
//	y = 1411;
	/*
	const Json::Value& enter_pos = CONFIG_INSTANCE->answer_activity("enter_pos");
	int rand_index = std::rand() % enter_pos.size();

	MoverCoord enter_coord;
	enter_coord.set_pixel(enter_pos[rand_index][0u].asInt(),
			enter_pos[rand_index][1u].asInt());

	enter_info.set_pos_x(enter_coord.pos_x());
	enter_info.set_pos_y(enter_coord.pos_y());
	*/
	enter_info.set_pos_x(x);
	enter_info.set_pos_y(y);

	return MAP_MONITOR->respond_enter_scene_begin(gate_id, request, &enter_info);
}

void HotspringActivityScene::init_hotspring_scene()
{
	this->wait_timer_.cancel_timer();
	this->start_timer_.cancel_timer();
	this->check_start_timer_map[HOTSPRING_ACTIVITY_FIRST_ID].set_act_type(
			HOTSPRING_ACTIVITY_FIRST_ID);
	this->check_start_timer_map[HOTSPRING_ACTIVITY_SECOND_ID].set_act_type(
			HOTSPRING_ACTIVITY_SECOND_ID);


	MSG_USER("HOTSPRING_SYS %d %d %d %d %d %d", this->hotspring_detail_.cur_state_answer_list_.size(),
			this->hotspring_detail_.act_player_vip_info.size(), this->hotspring_detail_.double_major_list_.size(),
			this->hotspring_detail_.exp_map_.size(), this->hotspring_detail_.player_answer_list_.size(),
			this->hotspring_detail_.right_player_.size());

	this->init_scene(0, GameEnum::HOTSPRING_SCENE_ID);
	MAP_MONITOR->bind_scene(0, GameEnum::HOTSPRING_SCENE_ID, this);

	this->start_scene();
	const Json::Value& hotspring_json = CONFIG_INSTANCE->hotspring_activity_json(HOTSPRING_ACTIVITY_FIRST_ID);
	this->hotspring_detail_.play_time_ = hotspring_json["game_time"].asInt();
	this->hotspring_detail_.wait_time_ = hotspring_json["choose_time"].asInt();
	this->hotspring_detail_.win_award_id = hotspring_json["win_award_id"].asInt();
	this->hotspring_detail_.comfort_award_id = hotspring_json["comfort_award_id"].asInt();
	this->hotspring_detail_.total_stage_ = hotspring_json["total_stage"].asInt();

	this->hotspring_detail_.wait_time_ = this->hotspring_detail_.wait_time_ == 0 ? 60 : this->hotspring_detail_.wait_time_;
	this->hotspring_detail_.play_time_ = this->hotspring_detail_.play_time_ == 0 ? 240 : this->hotspring_detail_.play_time_;

	const Json::Value& activity_conf_1 = CONFIG_INSTANCE->common_activity(HOTSPRING_ACTIVITY_FIRST_ID);
	JUDGE_RETURN(activity_conf_1.empty() == false, ;);
    GameCommon::cal_activity_info(this->time_info_map_[HOTSPRING_ACTIVITY_FIRST_ID], activity_conf_1);

	const Json::Value& activity_conf_2 = CONFIG_INSTANCE->common_activity(HOTSPRING_ACTIVITY_SECOND_ID);
	JUDGE_RETURN(activity_conf_2.empty() == false, ;);
    GameCommon::cal_activity_info(this->time_info_map_[HOTSPRING_ACTIVITY_SECOND_ID], activity_conf_2);

    this->check_start_timer_map[HOTSPRING_ACTIVITY_FIRST_ID].cancel_timer();
    this->check_start_timer_map[HOTSPRING_ACTIVITY_FIRST_ID].schedule_timer(
    		this->time_info_map_[HOTSPRING_ACTIVITY_FIRST_ID].refresh_time_);

    this->check_start_timer_map[HOTSPRING_ACTIVITY_SECOND_ID].cancel_timer();
    this->check_start_timer_map[HOTSPRING_ACTIVITY_SECOND_ID].schedule_timer(
    		this->time_info_map_[HOTSPRING_ACTIVITY_SECOND_ID].refresh_time_);

	MSG_USER("HOTSPRING_SYS %d %d %d %d %d %d", this->hotspring_detail_.cur_state_answer_list_.size(),
			this->hotspring_detail_.act_player_vip_info.size(), this->hotspring_detail_.double_major_list_.size(),
			this->hotspring_detail_.exp_map_.size(), this->hotspring_detail_.player_answer_list_.size(),
			this->hotspring_detail_.right_player_.size());

    if (this->time_info_map_[HOTSPRING_ACTIVITY_FIRST_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
    	this->hotspring_detail_.cycle_id_ = HOTSPRING_ACTIVITY_FIRST_ID;
    	this->new_start();
    	return ;
    }

    if (this->time_info_map_[HOTSPRING_ACTIVITY_SECOND_ID].cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
    	this->hotspring_detail_.cycle_id_ = HOTSPRING_ACTIVITY_SECOND_ID;
    	this->new_start();
    	return ;
    }

	this->hotspring_detail_.cycle_id_ = (this->time_info_map_[HOTSPRING_ACTIVITY_FIRST_ID].fetch_left_time() <
						this->time_info_map_[HOTSPRING_ACTIVITY_SECOND_ID].fetch_left_time()) ?
						HOTSPRING_ACTIVITY_FIRST_ID : HOTSPRING_ACTIVITY_SECOND_ID;
}

int HotspringActivityScene::announce_work(int type, int ext_int, string ext_str)
{

	BrocastParaVec para_vec;
	int shout_id = 0;
	switch (type)
	{
	case 0 :
		break;
	case 1 :
		break;
	case 2 :
		break;
	case 3 :
		break;
	}

	GameCommon::push_brocast_para_string(para_vec, ext_str);
	GameCommon::announce(shout_id, &para_vec);
	return 0;
}

Int64 &HotspringActivityScene::get_player_exp(Int64 player_id)
{
	return this->hotspring_detail_.exp_map_[player_id];
}

int HotspringActivityScene::get_player_is_right(Int64 player_id)
{
	return this->hotspring_detail_.right_player_[player_id];
}

void HotspringActivityScene::test_hotspring(int id, int set_time)
{
//	this->hotspring_detail_.cycle_id_ = HOTSPRING_ACTIVITY_FIRST_ID;
	this->time_info_map_[this->get_cycle_id()].cur_state_ = (id + 1) % this->time_info_map_[this->get_cycle_id()].time_span_;
	this->time_info_map_[this->get_cycle_id()].refresh_time_ = set_time;
	this->handle_hotspring_i(id);
}

int HotspringActivityScene::get_refresh_time()
{
	return this->check_start_timer_map[this->hotspring_detail_.cycle_id_].left_second();
}

int HotspringActivityScene::makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info)
{
	Int64 target_role = this->find_double_player(player->role_id());
	if (target_role > 0)
	{
		appear_info->set_hotspring_status(1);
		appear_info->set_wedding_id(target_role);
	}
	else
	{
		appear_info->set_hotspring_status(0);
		appear_info->set_wedding_id(0);
	}

	return 0;
}

int HotspringActivityScene::find_near_player(Int64 role_id, MoverCoord& coord, string& name, Int64& aim_id)
{
	MapPlayerEx* own = this->find_player(role_id);
	JUDGE_RETURN(own != NULL, -1);
	MoverCoord& coord_1 = own->location();

	int min = INT_MAX;
	for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		MapPlayerEx *map_player = dynamic_cast<MapPlayerEx *>(player);
		Int64 player_id = map_player->role_id();
		JUDGE_CONTINUE(role_id != player_id);
		JUDGE_CONTINUE(!this->is_player_double_major(player_id));

		MoverCoord& coord_2 = map_player->location();
		int distance = (int)std::sqrt((int)(std::pow(double(coord_1.pixel_x() - coord_2.pixel_x()), 2) +
				std::pow(double(coord_1.pixel_y() - coord_2.pixel_y()), 2)));

		if (distance < min)
		{
			coord = coord_2;
			name = map_player->role_detail().__name;
			aim_id = player_id;
			min = distance;
		}
	}
	if (min == INT_MAX) return ERROR_NO_PLAYER_HERE;
	return 1;
}
