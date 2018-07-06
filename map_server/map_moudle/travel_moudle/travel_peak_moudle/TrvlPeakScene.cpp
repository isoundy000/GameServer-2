/*
 * TrvlPeakScene.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#include "TrvlPeakScene.h"
#include "MapMonitor.h"
#include "TrvlPeakMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"

int TrvlPeakScene::PeakTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlPeakScene::PeakTimer::handle_timeout(const Time_Value &tv)
{
	switch (this->state_)
	{
	case FIGHT_PREPARE:
	{
		this->scene_->handle_prep_timeout();
		this->scene_->notify_peak_fight();
		break;
	}
	case FIGHT_BEING:
	{
		this->scene_->handle_fighting_timeout();
		this->scene_->check_and_handle_offline();
		this->scene_->notify_peak_fight();
		break;
	}
	case FIGHT_FINISH:
	{
		this->scene_->handle_finish_timeout();
		break;
	}
	}

	return 0;
}

TrvlPeakScene::TrvlPeakScene() {
	// TODO Auto-generated constructor stub
	this->peak_timer_.scene_ = this;
}

TrvlPeakScene::~TrvlPeakScene() {
	// TODO Auto-generated destructor stub
}

void TrvlPeakScene::reset()
{
	Scene::reset();

	this->peak_timer_.state_ = FIGHT_PREPARE;
	this->peak_timer_.cancel_timer();

	this->left_team_ = NULL;
	this->right_team_ = NULL;

	this->prep_time_ = 0;
	this->left_time_ = 0;
	this->win_team_id_ = 0;

	this->die_rold_set_.clear();
	this->offline_set_.clear();
}

int TrvlPeakScene::init_peak_scene(int space_id, FourObj& obj)
{
	int scene_id = GameEnum::TRVL_PEAK_SCENE_ID;

	this->init_scene(space_id, scene_id);
	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, scene_id, this);

	TravPeakTeam* first = TRVL_PEAK_MONITOR->find_travel_team(obj.first_id_);
	TravPeakTeam* second = TRVL_PEAK_MONITOR->find_travel_team(obj.second_id_);
	if (first == NULL || second == NULL)
	{
		MAP_MONITOR->unbind_scene(this->space_id(), this->scene_id());
		TRVL_PEAK_MONITOR->push_scene(this);
		return -1;
	}

	this->set_team(first, SIDE_LEFT);
	this->set_team(second, SIDE_RIGHT);

	const Json::Value& conf = this->conf();
	this->prep_time_ = conf["prep_time"].asInt();
	this->left_time_ = conf["fighting_time"].asInt();

	this->peak_timer_.state_ = FIGHT_PREPARE;
	this->peak_timer_.schedule_timer(1);

	return 0;
}

void TrvlPeakScene::notify_peak_fight()
{
	Proto80101502 respond;

	TravPeakTeam *left_team = this->fetch_team(SIDE_LEFT);
	TravPeakTeam *right_team = this->fetch_team(SIDE_RIGHT);

	if (left_team != NULL)
	{
	    respond.set_left_team_id(left_team->__team_id);
	    for (TravPeakTeam::TeamerMap::iterator iter = left_team->__teamer_map.begin();
	    		iter != left_team->__teamer_map.end(); ++iter)
	    {
	        TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;

	        ProtoTravelTeamer *proto_travel_teamer = respond.add_left_teamers();
	        proto_travel_teamer->set_role_id(teamer_info.__teamer_id);
	        proto_travel_teamer->set_role_name(teamer_info.__teamer_name);
	        proto_travel_teamer->set_role_sex(teamer_info.__teamer_sex);
	        proto_travel_teamer->set_role_career(teamer_info.__teamer_career);
	        proto_travel_teamer->set_level(teamer_info.__teamer_level);
	        proto_travel_teamer->set_fight_force(teamer_info.__teamer_force);

	        MapPlayerEx *player = this->find_player(teamer_info.__teamer_id);
	        if (player == NULL)
	        {
	            player = teamer_info.__offline_player;
	            this->offline_set_.insert(teamer_info.__teamer_id);
	        }

	        if (player != NULL)
	        {
	            proto_travel_teamer->set_role_id(player->role_id());
	            proto_travel_teamer->set_left_blood(player->fight_detail().__blood);
	            proto_travel_teamer->set_total_blood(player->fight_detail().__blood_total_i(player));
	            proto_travel_teamer->set_left_magic(player->fight_detail().__magic);
	            proto_travel_teamer->set_total_magic(player->fight_detail().__magic_total_i(player));
	        }
	    }
	}

	if (right_team != NULL)
	{
	    respond.set_right_team_id(right_team->__team_id);

	    for (TravPeakTeam::TeamerMap::iterator iter = right_team->__teamer_map.begin();
	    		iter != right_team->__teamer_map.end(); ++iter)
	    {
			TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;

			ProtoTravelTeamer *proto_travel_teamer = respond.add_right_teamers();
			proto_travel_teamer->set_role_id(teamer_info.__teamer_id);
			proto_travel_teamer->set_role_name(teamer_info.__teamer_name);
			proto_travel_teamer->set_role_sex(teamer_info.__teamer_sex);
			proto_travel_teamer->set_role_career(teamer_info.__teamer_career);
			proto_travel_teamer->set_level(teamer_info.__teamer_level);
			proto_travel_teamer->set_fight_force(teamer_info.__teamer_force);

			MapPlayerEx *player = this->find_player(teamer_info.__teamer_id);
			if (player == NULL)
			{
				player = teamer_info.__offline_player;
				this->offline_set_.insert(teamer_info.__teamer_id);
			}

			if (player != NULL)
			{
				proto_travel_teamer->set_role_id(player->role_id()); // 修正头像ID，用于更新血量
				proto_travel_teamer->set_left_blood(player->fight_detail().__blood);
				proto_travel_teamer->set_total_blood(player->fight_detail().__blood_total_i(player));
				proto_travel_teamer->set_left_magic(player->fight_detail().__magic);
				proto_travel_teamer->set_total_magic(player->fight_detail().__magic_total_i(player));
			}
		}
	}

	if (this->is_prepare_fight())
	{
		if (this->prep_time_ < 0)
			this->prep_time_ = 0;

	    respond.set_fight_state(FIGHT_PREPARE);
	    respond.set_left_tick(this->prep_time_);
	}
	else if (this->is_fighting())
	{
		if (this->left_time_ < 0)
			this->left_time_ = 0;

	    respond.set_fight_state(FIGHT_BEING);
	    respond.set_left_tick(this->left_time_);
	}

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		Int64 team_id = TRVL_PEAK_MONITOR->find_team_id(iter->first);
		JUDGE_CONTINUE(team_id != 0);

		respond.set_my_team_id(team_id);
		player->respond_to_client(ACTIVE_TRAVPEAK_FIGHT_TEAMS, &respond);
	}
}

void TrvlPeakScene::notify_fight_result(TravPeakTeam *win_team, TravPeakTeam *loss_team)
{
	JUDGE_RETURN(win_team != NULL && loss_team != NULL, ;);

	Proto80101503 respond;
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		if (win_team->__teamer_map.count(iter->first) > 0)
		{
			respond.set_flag(1);
			respond.set_name(loss_team->__team_name);
			respond.set_score(win_team->__last_add_score);
			respond.set_reward_id(win_team->__last_reward_id);
		}
		else
		{
			respond.set_flag(0);
			respond.set_name(win_team->__team_name);
			respond.set_score(loss_team->__last_add_score);
			respond.set_reward_id(loss_team->__last_reward_id);
		}
		player->respond_to_client(ACTIVE_TRAVPEAK_FIGHT_RESULT, &respond);
	}
}

void TrvlPeakScene::check_and_handle_offline()
{
	JUDGE_RETURN(this->is_finish_fight() == false, ;);
	JUDGE_RETURN(this->left_time_ > 0, ;);

	LongSet signout_set;

	TravPeakTeam *left_team = this->fetch_team(SIDE_LEFT);
	if (left_team != NULL)
	{
		for (TravPeakTeam::TeamerMap::iterator iter = left_team->__teamer_map.begin();
				iter != left_team->__teamer_map.end(); ++iter)
		{
			JUDGE_CONTINUE(this->is_has_die_player(iter->first) == false);
			JUDGE_CONTINUE(this->offline_set_.count(iter->first) <= 0);

			MapPlayerEx* player = this->find_player(iter->first);
			JUDGE_CONTINUE(player == NULL);

			this->insert_die_player(iter->first);
			signout_set.insert(iter->first);
		}
	}

	TravPeakTeam *right_team = this->fetch_team(SIDE_RIGHT);
	if (right_team != NULL)
	{
		for (TravPeakTeam::TeamerMap::iterator iter = right_team->__teamer_map.begin();
				iter != right_team->__teamer_map.end(); ++iter)
		{
			JUDGE_CONTINUE(this->is_has_die_player(iter->first) == false);
			JUDGE_CONTINUE(this->offline_set_.count(iter->first) <= 0);

			MapPlayerEx* player = this->find_player(iter->first);
			JUDGE_CONTINUE(player == NULL);

			this->insert_die_player(iter->first);
			signout_set.insert(iter->first);
		}
	}

	JUDGE_RETURN(signout_set.size() > 0, ;);

	for (LongSet::iterator iter = signout_set.begin(); iter != signout_set.end(); ++iter)
	{
		TravPeakTeam *travel_team = TRVL_PEAK_MONITOR->find_travel_team_by_role_id(*iter);
		JUDGE_CONTINUE(travel_team != NULL);

		bool is_all_teamer_die = true;
		for (TravPeakTeam::TeamerMap::iterator iter = travel_team->__teamer_map.begin();
				iter != travel_team->__teamer_map.end(); ++iter)
		{
			TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;
			MapPlayerEx* player = this->find_player(iter->first);
			if (player == NULL)
			{
				player = teamer_info.__offline_player;
			}

			if (player != NULL && this->is_has_die_player(player->role_id()) == false)
			{
				is_all_teamer_die = false;
				break;
			}
		}

		if (is_all_teamer_die == true)
		{
			this->finish_fight_earlier();
			break;
		}
	}

}

void TrvlPeakScene::handle_prep_timeout()
{
	this->prep_time_ -= 1;
	JUDGE_RETURN(this->prep_time_ <= 0, ;);

	this->peak_timer_.cancel_timer();
	this->peak_timer_.state_ = FIGHT_BEING;
	this->peak_timer_.schedule_timer(1);
}

void TrvlPeakScene::handle_fighting_timeout()
{
	JUDGE_RETURN(this->is_finish_fight() == false, ;);

	this->left_time_ -= 1;
	JUDGE_RETURN(this->left_time_ <= 0, ;);

	this->finish_fight_earlier();
}

void TrvlPeakScene::handle_finish_timeout()
{
	this->peak_timer_.cancel_timer();

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->offline_set_.count(iter->first) > 0);

		MapPlayerEx *player = this->find_player(iter->first);
	    JUDGE_CONTINUE(player != NULL && player->is_enter_scene());

	    if (player->is_online_player() == false)
	    {
	    	// 机器人只离线不回收以待下次继续使用
	    	player->offline_exit_scene();
	    }
	}

	this->notify_all_player_exit(GameEnum::EXIT_TYPE_PREVE);

//	//回收机器人
//	if (this->left_team_ != NULL)
//	{
//		for (TravPeakTeam::TeamerMap::iterator iter = this->left_team_->__teamer_map.begin();
//				iter != this->left_team_->__teamer_map.end(); ++iter)
//		{
//			JUDGE_CONTINUE(this->offline_set_.count(iter->first) > 0);
//
//			MapPlayerEx *offline_player = iter->second.__offline_player;
//			JUDGE_CONTINUE(offline_player != NULL);
//
//			offline_player->offline_exit_scene();
//		}
//	}
//
//	if (this->right_team_ != NULL)
//	{
//		for (TravPeakTeam::TeamerMap::iterator iter = this->right_team_->__teamer_map.begin();
//				iter != this->right_team_->__teamer_map.end(); ++iter)
//		{
//			JUDGE_CONTINUE(this->offline_set_.count(iter->first) > 0);
//
//			MapPlayerEx *offline_player = iter->second.__offline_player;
//			JUDGE_CONTINUE(offline_player != NULL);
//
//			offline_player->offline_exit_scene();
//		}
//	}

	MAP_MONITOR->unbind_scene(this->space_id(), this->scene_id());
	TRVL_PEAK_MONITOR->recycle_scene(this);
}

TravPeakTeam *TrvlPeakScene::fetch_team(const int side)
{
	if (side == SIDE_LEFT)
	{
		return this->left_team_;
	}
	else
	{
		return this->right_team_;
	}
}

void TrvlPeakScene::set_team(TravPeakTeam *team, const int side)
{
	if (side == SIDE_LEFT)
	{
		this->left_team_ = team;
	}
	else
	{
		this->right_team_ = team;
	}
}

void TrvlPeakScene::insert_die_player(const Int64 role_id)
{
	this->die_rold_set_.insert(role_id);
}

bool TrvlPeakScene::is_has_die_player(const Int64 role_id)
{
	return (this->die_rold_set_.find(role_id) != this->die_rold_set_.end());
}

bool TrvlPeakScene::is_prepare_fight(void)
{
	return this->peak_timer_.state_ == FIGHT_PREPARE;
}

bool TrvlPeakScene::is_fighting(void)
{
    return this->peak_timer_.state_ == FIGHT_BEING;
}

bool TrvlPeakScene::is_finish_fight(void)
{
    return this->peak_timer_.state_ == FIGHT_FINISH;
}

void TrvlPeakScene::set_win_team(const Int64 team_id)
{
	this->win_team_id_ = team_id;
}

Int64 TrvlPeakScene::calc_win_team_id(void)
{
	int left_alive_amount = 0, right_alive_amount = 0;
	int left_fight_force = this->left_team_->total_force();
	int right_fight_force = this->right_team_->total_force();

	bool is_all_die = true;
	for (TravPeakTeam::TeamerMap::iterator iter = this->left_team_->__teamer_map.begin();
	        iter != this->left_team_->__teamer_map.end(); ++iter)
	{
		TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;

		JUDGE_CONTINUE(this->is_has_die_player(teamer_info.__teamer_id) == false);

		++left_alive_amount;
		is_all_die = false;
	}
	if (is_all_die == true)
		return this->right_team_->__team_id;

	is_all_die = true;
	for (TravPeakTeam::TeamerMap::iterator iter = this->right_team_->__teamer_map.begin();
		    iter != this->right_team_->__teamer_map.end(); ++iter)
	{
		TravPeakTeam::TravPeakTeamer &teamer_info = iter->second;

		JUDGE_CONTINUE(this->is_has_die_player(teamer_info.__teamer_id) == false);

		++right_alive_amount;
		is_all_die = false;
	}
	if (is_all_die == true)
		return this->left_team_->__team_id;

	// 存活人数多的赢
	if (left_alive_amount > right_alive_amount)
	    return this->left_team_->__team_id;
	else if (left_alive_amount < right_alive_amount)
	    return this->right_team_->__team_id;

	// 平均战力低的赢
	if (left_fight_force < right_fight_force)
	    return this->left_team_->__team_id;
	else if (left_fight_force > right_fight_force)
	    return this->right_team_->__team_id;

	// 都相等则随机
	return ((rand() % 2) == 1 ? this->left_team_->__team_id : this->right_team_->__team_id);
}

void TrvlPeakScene::finish_fight_earlier(void)
{
	JUDGE_RETURN(this->is_finish_fight() == false, ;);

	this->peak_timer_.cancel_timer();
	this->peak_timer_.state_ = FIGHT_FINISH;

	Int64 win_team_id = this->calc_win_team_id();
	this->set_win_team(win_team_id);

	TravPeakTeam *win_team = NULL, *loss_team = NULL;
	if (this->left_team_->__team_id == win_team_id)
	{
	    win_team = this->left_team_;
	    loss_team = this->right_team_;
	}
	else
	{
	    win_team = this->right_team_;
	    loss_team = this->left_team_;
	}
	TRVL_PEAK_MONITOR->update_fight_after_team_record(win_team, loss_team);

	this->notify_fight_result(win_team, loss_team);

	int quit_time = this->conf()["quit_time"].asInt();
	this->peak_timer_.schedule_timer(quit_time);
}



