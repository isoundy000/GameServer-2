/*
 * AnswerActivityScene.cpp
 *
 *  Created on: 2016年8月13日
 *      Author: lzy
 */

#include "AnswerActivityScene.h"
#include "AIManager.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "SerialRecord.h"

int AnswerActivityScene::CheckStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::CheckStartTimer::handle_timeout(const Time_Value &tv)
{
	return ANSWERACTIVITY_INSTANCE->handle_answer_timeout();
}

int AnswerActivityScene::TopPlayerCoordTimer::type(void)
{
	return GTT_MAP_MONSTER;
}

int AnswerActivityScene::TopPlayerCoordTimer::handle_timeout(const Time_Value &tv)
{
	ANSWERACTIVITY_INSTANCE->notify_top_player_coord();
	return 0;
}

int AnswerActivityScene::SuperviseTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::SuperviseTimer::handle_timeout(const Time_Value &tv)
{
	ANSWERACTIVITY_INSTANCE->refresh_player_info();
	return 0;
}

int AnswerActivityScene::WaitTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::WaitTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();
	ANSWERACTIVITY_INSTANCE->wait_timeout();
	return 0;
}

int AnswerActivityScene::ActivityStartTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::ActivityStartTimer::handle_timeout(const Time_Value &tv)
{
	ANSWERACTIVITY_INSTANCE->new_start();
	return 0;
}

int AnswerActivityScene::RefreshTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::RefreshTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();
	ANSWERACTIVITY_INSTANCE->refresh_timeout();
	return 0;
}

int AnswerActivityScene::BroadcastTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int AnswerActivityScene::BroadcastTimer::handle_timeout(const Time_Value &tv)
{
	return ANSWERACTIVITY_INSTANCE->handle_broadcast_timer();
}

AnswerActivityScene::AnswerActivityScene()
{

}

AnswerActivityScene::~AnswerActivityScene()
{
	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();

	this->start_timer_.cancel_timer();
}


IntPair AnswerActivityScene::fetch_addition_exp()
{
	return IntPair(0, 0);
}

AnswerActivityScene* AnswerActivityScene::instance()
{
	static AnswerActivityScene* instance = NULL;

	if (instance == NULL)
	{
		instance = new AnswerActivityScene;
	}

	return instance;
}

int AnswerActivityScene::player_wait_time(Int64 player_id)
{
	//计算等待时间
	Int64 wait_time = 0;
	if (this->wait_timer_.is_registered())
	{
		wait_time = this->wait_timer_.left_second();
	}
	else
	{
		wait_time = this->refresh_timer_.left_second();
	}


	MapPlayerEx *player = NULL;
	if (MAP_MONITOR->find_player(player_id, player) != 0) return 0;

	AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);
	this->answer_detail_.player_info_list[player_id].name = player->role_detail().__name;
	//返回消息
	Proto50405021 respond;
	respond.set_wait_time(wait_time);
	respond.set_next_num(this->answer_detail_.cur_stage_ + 1);

	int i = 0;
	for (std::vector<std::pair<Int64, int> >::iterator it = this->answer_detail_.player_rank_list.begin();
			it != this->answer_detail_.player_rank_list.end(); ++i, ++it)
	{
		Int64 top_player_id = it->first;
		i++;

		MapPlayerEx *top_player  = NULL;
		if (MAP_MONITOR->find_player(top_player_id, top_player) != 0) continue;
		string player_name = top_player->role_name();

		respond.add_player_list(player_name);
		respond.add_player_score_list(it->second);
	}

	respond.set_right_num(it->second.right_num);
	respond.set_score_num(it->second.score_num);
	respond.set_rank_num(it->second.rank_num);

	player->respond_to_client(RETURN_GET_PLAYER_WAIT_TIME, &respond);
	return 0;
}

AnswerActivityDetail &AnswerActivityScene::get_AnswerActivityDetail()
{
	return this->answer_detail_;
}

int AnswerActivityScene::request_enter_answer_activity(int gate_id, Proto30400051* request)
{
	Int64 player_id = request->role_id();

	if (this->answer_detail_.player_info_list.count(player_id) == 0)
	{
		AnswerActivityDetail::player_info temp;

		this->answer_detail_.player_info_list.insert(AnswerMap::value_type (player_id, temp));

		if (this->answer_detail_.player_info_list[player_id].rank_num <= 0)
		{
			this->answer_detail_.player_info_list[player_id].rank_num = this->answer_detail_.player_info_list.size();
		}
		int score_num = this->answer_detail_.player_info_list[player_id].score_num;

		this->answer_detail_.player_rank_list.push_back(std::make_pair(player_id, score_num));
	}
	Proto30400052 enter_info;
	enter_info.set_space_id(this->space_id());
	enter_info.set_scene_mode(SCENE_MODE_LEAGUE); //need change

	int x = CONFIG_INSTANCE->answer_activity_json(this->answer_detail_.cycle_id_)["scence_pos"][0u].asInt();
	int y = CONFIG_INSTANCE->answer_activity_json(this->answer_detail_.cycle_id_)["scence_pos"][1u].asInt();

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



int AnswerActivityScene::handle_broadcast_timer()//need change
{
	this->broad_timer_.cancel_timer();

	this->answer_detail_.broad_times_ += 1;
	JUDGE_RETURN(this->answer_detail_.broad_times_ < 5, -1);

	this->broad_timer_.schedule_timer(Time_Value(Time_Value::HOUR));
//	MAP_MONITOR->announce_world(SHOUT_ALL_START_XIANYE_CRAZY);

	return 0;
}


int AnswerActivityScene::check_and_start_broadcast(int start_tick)//need change
{
	JUDGE_RETURN(start_tick > this->answer_detail_.start_tick_, -1);
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

void AnswerActivityScene::refresh_player_info()
{
	//work
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);
		Int64 player_id = map_player->role_id();

		AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);
		if (GameCommon::mover_in_area(player->location(),
				this->answer_detail_.a_area[0][0],this->answer_detail_.a_area[0][1],
				this->answer_detail_.a_area[1][0],this->answer_detail_.a_area[1][1]))
		{
			if (it->second.answer_num == 1) continue;

			it->second.answer_num = 1;
			it->second.time_num = GameCommon::fetch_cur_day_sec();
		}
		else if (GameCommon::mover_in_area(player->location(),
				this->answer_detail_.b_area[0][0],this->answer_detail_.b_area[0][1],
				this->answer_detail_.b_area[1][0],this->answer_detail_.b_area[1][1]))
		{
			if (it->second.answer_num == 2) continue;

			it->second.answer_num = 2;
			it->second.time_num = GameCommon::fetch_cur_day_sec();
		}
		else
		{
			it->second.answer_num = 0;
		}

	}
}

void AnswerActivityScene::refresh_timeout()
{
	time_t now_tick = ::time(NULL);
	this->supervise_timer_.cancel_timer();
	this->top_player_coord_timer_.cancel_timer();

	this->wait_timer_.cancel_timer();
	if (this->answer_detail_.cur_stage_ > this->answer_detail_.total_stage_)
	{
//		this->new_stop();
		return ;
	}
	this->wait_timer_.schedule_timer(this->answer_detail_.wait_time_);

	this->handle_wait_time(now_tick);
}

void AnswerActivityScene::wait_timeout()
{
	time_t now_tick = ::time(NULL);
	this->refresh_timer_.cancel_timer();
	if (this->answer_detail_.cur_stage_ > this->answer_detail_.total_stage_)
	{
//		this->new_stop();
		return ;
	}
	this->refresh_timer_.schedule_timer(this->answer_detail_.answer_time_);

	this->top_player_coord_timer_.cancel_timer();
	this->top_player_coord_timer_.schedule_timer(0.3);

	this->supervise_timer_.cancel_timer();
	this->supervise_timer_.schedule_timer(1);

	this->handle_refresh_topic(now_tick);

}

int AnswerActivityScene::handle_wait_time(time_t now_tick)
{
	this->answer_done_work();

	this->notify_right_answer();
	//刷新排行榜
	this->refresh_rank_list();

	this->notify_answer_activity_info();
	return 0;
}

int AnswerActivityScene::calc_player_score(int time_num)
{
	const Json::Value &score_list_json = CONFIG_INSTANCE->answer_activity_json(this->answer_detail_.cycle_id_);

	int size = score_list_json["score_list"].size();
	for (int i = 0; i < size; ++i)
	{
		int time = score_list_json["score_list"][i][0u].asInt();
		int score = score_list_json["score_list"][i][1u].asInt();

		if (time_num <= time) return score;
	}
	return 0;
}

int AnswerActivityScene::answer_done_work()
{
	for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);
		Int64 player_id = map_player->role_id();
		AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);

		int right_num = CONFIG_INSTANCE->topic_bank_json(this->answer_detail_.cur_topic_id)["answer"].asInt();

		int exp_num = 0;
		if (it->second.answer_num == right_num)
		{
			it->second.score_num += this->calc_player_score(this->answer_detail_.answer_time_ -
					(GameCommon::fetch_cur_day_sec() - it->second.time_num));
			it->second.right_num ++;
			exp_num = CONFIG_INSTANCE->role_level(0,map_player->level())["question_right_exp"].asInt();
		}
		else
		{
			exp_num = CONFIG_INSTANCE->role_level(0,map_player->level())["question_wrong_exp"].asInt();
		}

		it->second.answer_num = 0;
		map_player->modify_element_experience(exp_num, SerialObj(EXP_FROM_ANSWER_ACTIVITY));
	}

	return 0;
}

int AnswerActivityScene::notify_right_answer()
{

	int answer_num = CONFIG_INSTANCE->topic_bank_json(this->answer_detail_.cur_topic_id)["answer"].asInt();
	for (MoverMap::iterator iter = this->player_map_.begin();
					iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);
		Int64 player_id = map_player->role_id();
		AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);

		Proto80405013 notify_info;
		notify_info.set_answer_id(answer_num);
		notify_info.set_score_num(it->second.score_num);
		notify_info.set_right_num(it->second.right_num);

		player->respond_to_client(ACTIVE_GET_ANSWER_NUM, &notify_info);
	}

	return 0;
}


int AnswerActivityScene::handle_refresh_topic(time_t now_tick)
{
	this->answer_detail_.cur_stage_++;
	if (this->answer_detail_.cur_stage_ > this->answer_detail_.total_stage_)
	{
//		this->new_stop();
		return 0;
	}
	this->notify_refresh_topic();
	return 0;
}


int AnswerActivityScene::notify_refresh_topic()
{
	int len = (int)this->answer_detail_.total_topic.size();
	int rand_index = std::rand() % len;
	int topic_id = this->answer_detail_.total_topic[rand_index];
	this->answer_detail_.cur_topic_id = topic_id;
	Proto80405011 notify_info;

	const Json::Value &topic_json = CONFIG_INSTANCE->topic_bank_json(topic_id);
	notify_info.set_topic_id(this->answer_detail_.cur_stage_);
	notify_info.set_topic_desc(topic_json["topic_str"].asCString());
	notify_info.set_a_answer(topic_json["a_str"].asCString());
	notify_info.set_b_answer(topic_json["b_str"].asCString());
	notify_info.set_cur_stage(this->answer_detail_.cur_stage_);

	IntVec::iterator it = find(this->answer_detail_.total_topic.begin(),
			this->answer_detail_.total_topic.end(), topic_id);

	this->answer_detail_.total_topic.erase(it);

	this->notify_all_player_msg(ACTIVE_REFRESH_TOPIC, &notify_info);
	return 0;
}

int AnswerActivityScene::handle_answer_timeout()
{
	int last_state = this->time_info_.cur_state_;
	this->time_info_.set_next_stage();
	return this->handle_answer_i(last_state);
}

int AnswerActivityScene::handle_answer_i(int state)
{
	switch (state)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->ahead_answer_event();
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

	this->check_start_timer_.cancel_timer();
	this->check_start_timer_.schedule_timer(this->time_info_.refresh_time_);

	return 0;
}

int AnswerActivityScene::ahead_answer_event()
{
	GameCommon::map_sync_activity_tips_ahead(this->answer_detail_.cycle_id_,
			this->time_info_.refresh_time_);

	return 0;
}

void AnswerActivityScene::init_answer_scene()
{
	time_t lt;
	lt =time(0);
	struct tm *local = localtime(&lt);

	this->answer_detail_.cur_year = local->tm_year + 1900;
	this->answer_detail_.cur_month = local->tm_mon + 1;
	this->answer_detail_.cur_day = local->tm_mday;

	this->init_scene(0, GameEnum::ANSWER_ACTIVITY_SCENE_ID);
	MAP_MONITOR->bind_scene(0, GameEnum::ANSWER_ACTIVITY_SCENE_ID, this);
	this->start_scene();

	MSG_USER("ANSWER_SYS %d %d %d %d", this->answer_detail_.player_info_list.size(),
			this->answer_detail_.player_rank_list.size(), this->answer_detail_.reward_list.size(),
			this->answer_detail_.total_topic.size());

	if (this->answer_detail_.cycle_id_ != ANSWER_ACTIVITY_ID)
		this->answer_detail_.cycle_id_ = ANSWER_ACTIVITY_ID;

	const Json::Value& activity = CONFIG_INSTANCE->answer_activity_json(
			this->answer_detail_.cycle_id_);

	this->answer_detail_.reward_list.clear();
	int reward_size = activity["award_list"].size();
	for (int i = 0; i < reward_size; ++i)
	{
		int answer_time = activity["award_list"][i][0u].asInt();
		int reward_num = activity["award_list"][i][1u].asInt();

		this->answer_detail_.reward_list.insert(IntMap::value_type (answer_time, reward_num));
	}

	this->answer_detail_.total_stage_ = activity["refresh_num"].asInt();
	this->answer_detail_.answer_time_ = activity["answer_time"].asInt();
	this->answer_detail_.wait_time_ = activity["wait_time"].asInt();

	this->answer_detail_.a_area[0][0] = activity["a_area"][0u][0u].asInt();
	this->answer_detail_.a_area[0][1] = activity["a_area"][0u][1u].asInt();
	this->answer_detail_.a_area[1][0] = activity["a_area"][1u][0u].asInt();
	this->answer_detail_.a_area[1][1] = activity["a_area"][1u][1u].asInt();

	this->answer_detail_.b_area[0][0] = activity["b_area"][0u][0u].asInt();
	this->answer_detail_.b_area[0][1] = activity["b_area"][0u][1u].asInt();
	this->answer_detail_.b_area[1][0] = activity["b_area"][1u][0u].asInt();
	this->answer_detail_.b_area[1][1] = activity["b_area"][1u][1u].asInt();

	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();

	this->start_timer_.cancel_timer();

	const Json::Value& activity_conf = CONFIG_INSTANCE->common_activity(this->answer_detail_.cycle_id_);
	JUDGE_RETURN(activity_conf.empty() == false, ;);

    GameCommon::cal_activity_info(this->time_info_, activity_conf);
    this->check_start_timer_.cancel_timer();
    this->check_start_timer_.schedule_timer(this->time_info_.refresh_time_);
    JUDGE_RETURN(this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START, ;);

    this->new_start();

	MSG_USER("ANSWER_SYS %d %d %d %d", this->answer_detail_.player_info_list.size(),
			this->answer_detail_.player_rank_list.size(), this->answer_detail_.reward_list.size(),
			this->answer_detail_.total_topic.size());

//	this->start_timer_.schedule_timer(this->next_start_times());
}

void AnswerActivityScene::new_start()
{
	if (this->answer_detail_.cycle_id_ != ANSWER_ACTIVITY_ID)
		this->answer_detail_.cycle_id_ = ANSWER_ACTIVITY_ID;
//	Int64 now_tick = ::time(NULL);
	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();
	this->supervise_timer_.cancel_timer();
	this->start_timer_.cancel_timer();

	this->answer_detail_.cur_topic_id = 0;
	this->answer_detail_.cur_stage_ = 0;

	this->answer_detail_.player_info_list.clear();
	this->answer_detail_.total_topic.clear();
//read question bank
	const Json::Value& topic_bank_json = CONFIG_INSTANCE->topic_bank_json();
	for (int i = 0 ; i < (int)topic_bank_json.size(); ++i)
	{
		this->answer_detail_.total_topic.push_back(i + 1);
	}

	time_t lt;
	lt =time(0);
	struct tm *local = localtime(&lt);

	this->answer_detail_.cur_year = local->tm_year + 1900;
	this->answer_detail_.cur_month = local->tm_mon + 1;
	this->answer_detail_.cur_day = local->tm_mday;
/*
	const Json::Value& start_time = CONFIG_INSTANCE->answer_activity("start_time");
	int temp = start_time[0u].asInt() * 100 + start_time[1u].asInt();
	this->answer_detail_.start_tick_ = GameCommon::fetch_day_sec(temp);

	const Json::Value& end_time = CONFIG_INSTANCE->answer_activity("end_time");
	temp = end_time[0u].asInt() * 100 + end_time[1u].asInt();
	this->answer_detail_.stop_tick_ = GameCommon::fetch_day_sec(temp);
*/
	this->answer_detail_.open_time = Time_Value::gettimeofday().get_tv().tv_sec;
	this->wait_timer_.cancel_timer();
	this->wait_timer_.schedule_timer(2 * this->answer_detail_.wait_time_);


	int shout_id = CONFIG_INSTANCE->answer_activity_json(this->answer_detail_.cycle_id_)["shout_start"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_start(this->answer_detail_.cycle_id_,
			this->time_info_.refresh_time_);
}

void AnswerActivityScene::new_stop()
{
	//退出前通知玩家获得奖励
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
	SERIAL_RECORD->record_activity(SERIAL_ACT_ANSWER, this->answer_detail_.player_info_list.size());

	this->top_player_coord_timer_.cancel_timer();
//	this->start_timer_.cancel_timer();
	this->broad_timer_.cancel_timer();
	this->refresh_timer_.cancel_timer();
	this->wait_timer_.cancel_timer();
//	this->start_timer_.schedule_timer(this->next_start_times());

	this->answer_detail_.player_rank_list.clear();
	this->answer_detail_.player_info_list.clear();
	int shout_id = CONFIG_INSTANCE->answer_activity_json(
			this->answer_detail_.cycle_id_)["shout_end"].asInt();
	GameCommon::announce(shout_id);
	GameCommon::map_sync_activity_tips_stop(this->answer_detail_.cycle_id_);

	MSG_USER("ANSWER_SYS %d %d %d %d", this->answer_detail_.player_info_list.size(),
			this->answer_detail_.player_rank_list.size(), this->answer_detail_.reward_list.size(),
			this->answer_detail_.total_topic.size());
}

void AnswerActivityScene::notify_top_player_coord()
{
	for (MoverMap::iterator it = this->player_map_.begin();
					it != this->player_map_.end(); ++it)
	{
		GameMover* player = it->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayerEx* map_player = dynamic_cast<MapPlayerEx *>(player);
		if (this->answer_detail_.player_rank_list.size() == 0) break;

		std::vector<std::pair<Int64, int> >::iterator iter = this->answer_detail_.player_rank_list.begin();

		MoverCoord mover_coord;
		Int64 player_id = 0;
		while(iter != this->answer_detail_.player_rank_list.end())
		{
			MapPlayerEx *top_player = NULL;
			int ret = MAP_MONITOR->find_player(iter->first, top_player);
			iter++;
			JUDGE_CONTINUE(ret == 0);
			if (top_player->scene_id() == GameEnum::ANSWER_ACTIVITY_SCENE_ID)
			{
				mover_coord = top_player->location();
				player_id = top_player->role_id();
				break;
			}
		}

		JUDGE_CONTINUE(player_id != 0);
		Proto80405016 respond;

		if (this->answer_detail_.player_info_list[player_id].rank_num >=
				this->answer_detail_.player_info_list[map_player->role_id()].rank_num)
		{
			respond.set_player_rank(-1);
		}
		else
		{
			respond.set_player_rank(this->answer_detail_.player_info_list[player_id].rank_num);
		}
//		if (player_id == 0)
//		{
//			player->respond_to_client_error(ACTIVE_TOP_PLAYER_COORD, ERROR_PLAYER_NO_HERE, &respond);
//		}
		ProtoCoord *proto_coord = respond.mutable_top_player();
        proto_coord->set_pixel_x(mover_coord.pixel_x());
        proto_coord->set_pixel_y(mover_coord.pixel_y());

        respond.set_player_id(player_id);
		player->respond_to_client(ACTIVE_TOP_PLAYER_COORD, &respond);
	}
}

int int_int_cmp(const std::pair<Int64, int>& x, const std::pair<Int64, int>& y)
{
	return x.second > y.second;
}

void AnswerActivityScene::notify_answer_activity_info()
{
//	sort(this->answer_detail_.player_rank_list.begin(), this->answer_detail_.player_rank_list.end(), int_int_cmp);

	for (MoverMap::iterator iter = this->player_map_.begin();
					iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		MapPlayer *map_player = dynamic_cast<MapPlayer *>(player);
		Int64 player_id = map_player->role_id();
		AnswerMap::iterator it_1 = this->answer_detail_.player_info_list.find(player_id);

		Proto80405014 respond;

		int i = 0;
		for (std::vector<std::pair<Int64, int> >::iterator it = this->answer_detail_.player_rank_list.begin();
				it != this->answer_detail_.player_rank_list.end(); ++i, ++it)
		{
			AnswerMap::iterator it_2 = this->answer_detail_.player_info_list.find(it->first);

			respond.add_player_list(it_2->second.name);
			respond.add_player_score_list(it_2->second.score_num);
		}

		respond.set_player_right_num(it_1->second.right_num);
		respond.set_player_score_num(it_1->second.score_num);
		respond.set_player_rank(it_1->second.rank_num);

		player->respond_to_client(ACTIVE_ANSWER_ACTIVITY_INFO, &respond);
	}
}

void AnswerActivityScene::refresh_rank_list(void)
{
	this->answer_detail_.player_rank_list.clear();
	//更新玩家排名
	for (AnswerActivityDetail::PlayerInfoMap::iterator iter = this->answer_detail_.player_info_list.begin();
					iter != this->answer_detail_.player_info_list.end(); ++iter)
	{
		Int64 player_id = iter->first;
		int score_num = iter->second.score_num;
		this->answer_detail_.player_rank_list.push_back(std::make_pair(player_id, score_num));
	}
	//排序
	sort(this->answer_detail_.player_rank_list.begin(), this->answer_detail_.player_rank_list.end(), int_int_cmp);

	//更新玩家排名
	int i = 1;
	for (std::vector<std::pair<Int64, int> >::iterator it = this->answer_detail_.player_rank_list.begin();
			it != this->answer_detail_.player_rank_list.end(); ++i, ++it)
	{
		Int64 player_id = it->first;
		AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);
		it->second.rank_num = i;
	}
}


void AnswerActivityScene::notify_player_get_award(void)
{
	const Json::Value& answer = CONFIG_INSTANCE->answer_activity_json(this->answer_detail_.cycle_id_);
	for (std::vector<std::pair<Int64, int> >::iterator it = this->answer_detail_.player_rank_list.begin();
			it != this->answer_detail_.player_rank_list.end(); ++it)
	{
		Int64 player_id = it->first;
		AnswerMap::iterator it = this->answer_detail_.player_info_list.find(player_id);

		int reward_num = -1;

		IntMap::iterator it1 = this->answer_detail_.reward_list.begin();
		for (;it1 != this->answer_detail_.reward_list.end() ; ++it1)
		{
			reward_num = it1->second;
			if (it->second.rank_num <= it1->first)
			{
				break;
			}
		}

		int mail_id = answer["rank_mail_id"].asInt();
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), it->second.rank_num);
		mail_info->add_goods(reward_num);
		GameCommon::request_save_mail_content(player_id, mail_info);

		if (it->second.rank_num == 1)
		{
			BrocastParaVec para_vec;
			GameCommon::push_brocast_para_string(para_vec, this->answer_detail_.player_info_list[player_id].name);
			int shout_id = CONFIG_INSTANCE->answer_activity_json(
					this->answer_detail_.cycle_id_)["shout_win"].asInt();
			GameCommon::announce(shout_id, &para_vec);
		}
//		Proto80405012 respond;
//		respond.set_award_id(reward_num);

//		player->respond_to_client(ACTIVE_GET_ANSWER_AWARD, &respond);
	}

	int total_num = this->answer_detail_.player_rank_list.size();

	JUDGE_RETURN(total_num != 0, ;);
	int luck_num = (std::rand() % total_num) + 1;
	Int64 luck_id = -1;
	for (std::vector<std::pair<Int64, int> >::iterator it = this->answer_detail_.player_rank_list.begin();
				it != this->answer_detail_.player_rank_list.end(); ++it)
	{
		if (it->second == luck_num)
		{
			luck_id = it->first;
		}
	}
	JUDGE_RETURN(luck_id != -1, ;);

	int mail_id = answer["luck_mail_id"].asInt();
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str());
	mail_info->add_goods(answer["luck_award_id"].asInt());
	GameCommon::request_save_mail_content(luck_id, mail_info);
}

int AnswerActivityScene::next_start_times()
{
	int id = ANSWER_ACTIVITY_ID; //test
	int	interval = this->is_on_activity(id, this->answer_detail_.cur_year,
			this->answer_detail_.cur_month, this->answer_detail_.cur_day);
	if (interval >= 0)
	{
		MSG_USER("活动开启 From the COLLECT CHESTS activity start %d second", interval);
		this->answer_detail_.cycle_id_ = id;
		return interval;
	}

	//下次开启时间为第二天活动开启时间
	id = ANSWER_ACTIVITY_ID;
	Time_Value start_time;
	int start_h = CONFIG_INSTANCE->answer_activity_json(id)["start_time"][0u].asInt();
	int start_m = CONFIG_INSTANCE->answer_activity_json(id)["start_time"][1u].asInt();
	GameCommon::make_time_value(start_time, this->answer_detail_.cur_year,
			this->answer_detail_.cur_month, this->answer_detail_.cur_day, start_h, start_m);

	this->answer_detail_.cycle_id_ = id;

	return 24*60*60 - (Time_Value::gettimeofday().get_tv().tv_sec - start_time.get_tv().tv_sec);
}

int AnswerActivityScene::is_on_activity(int cycle_id, int year, int mouth, int day)
{
	int start_h = CONFIG_INSTANCE->answer_activity_json(cycle_id)["start_time"][0u].asInt();
	int start_m = CONFIG_INSTANCE->answer_activity_json(cycle_id)["start_time"][1u].asInt();

//	int end_h = CONFIG_INSTANCE->answer_activity_json(cycle_id)["end_time"][0u].asInt();
//	int end_m = CONFIG_INSTANCE->answer_activity_json(cycle_id)["end_time"][1u].asInt();

	Time_Value start_time;
	Time_Value end_time;

	GameCommon::make_time_value(start_time, year, mouth, day, start_h, start_m);
//	GameCommon::make_time_value(end_time, year, mouth, day, end_h, end_m);

	if (start_time > Time_Value::gettimeofday()) return start_time.get_tv().tv_sec - Time_Value::gettimeofday().get_tv().tv_sec;

	return -1;
}

ActivityTimeInfo &AnswerActivityScene::get_time_info()
{
	return this->time_info_;
}

void AnswerActivityScene::test_answer(int id, int set_time)
{
	if (this->answer_detail_.cycle_id_ != ANSWER_ACTIVITY_ID)
		this->answer_detail_.cycle_id_ = ANSWER_ACTIVITY_ID;
	this->time_info_.cur_state_ = (id + 1) % this->time_info_.time_span_;
	this->time_info_.refresh_time_ = set_time;
	this->handle_answer_i(id);
}
