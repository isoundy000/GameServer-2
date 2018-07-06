/*
 * LeagueWarScene.cpp
 *
 *  Created on: 2016年9月19日
 *      Author: lyw
 */

#include "LeagueWarScene.h"
#include "LeagueWarSystem.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "AIManager.h"
#include "MapMonitor.h"

void LeagueWarScene::BirthRecord::reset(void)
{
	this->__birth_coord.reset();
}

LeagueWarScene::LWarMonster::LWarMonster()
{
	this-> sort_ = 0;
	this->conf_index_ = 0;
	this->owner_camp_ = 0;
	this->is_boss_ = 0;
	this->league_index_ = 0;
	this->league_name_ = 0;
	this->index_ = 0;

	this->attack_player_point_ = 0;
	this->attack_league_point_ = 0;
	this->kill_player_point_ = 0;
	this->kill_league_point_ = 0;
	this->kill_resource_point_ = 0;

	this->loaction_.reset();
}

LeagueWarScene::LWarTimer::LWarTimer(void)
{
	this->state_ = 0;
	this->scene_ = NULL;
}

LeagueWarScene::LWarTimer::~LWarTimer(void)
{ /*NULL*/ }

void LeagueWarScene::LWarTimer::set_script_scene(LeagueWarScene *scene)
{
    this->scene_ = scene;
}

int LeagueWarScene::LWarTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int LeagueWarScene::LWarTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

    switch (this->state_)
    {
    case 0:
    {
    	//进行
        Int64 now_tick = ::time(NULL);
        this->scene_->add_online_score(now_tick);

        this->scene_->generate_boss(true);
        this->scene_->generate_monster(1);
        this->scene_->generate_monster(2);
//        this->scene_->active_check_boss_radius();
        this->scene_->sort_rank();
        this->scene_->active_add_buff();
        this->scene_->notify_all_player_info();
    	break;
    }

    case 1:
    {
    	//结束
    	MSG_USER("league war Finish");

    	this->scene_->recycle_all_monster();
    	this->scene_->notify_all_player_exit();

    	MAP_MONITOR->unbind_scene(this->scene_->space_id(),
    			GameEnum::LWAR_SCENE_ID);
    	LEAGUE_WAR_SYSTEM->recycle_lwar(this->scene_);
    	break;
    }
    }

    return 0;
}

LeagueWarScene::LeagueWarScene() {
	// TODO Auto-generated constructor stub
	this->lwar_timer_.set_script_scene(this);
}

LeagueWarScene::~LeagueWarScene() {
	// TODO Auto-generated destructor stub
}

void LeagueWarScene::reset()
{
	Scene::reset();

	this->real_scene_ = 0;
	this->lwar_limit_ = 0;
	this->player_count_ = 0;
	this->max_radius_ = 0;
	this->boss_flag_ = 0;
	this->boss_id_ = 0;

	this->defence_boss_league_score_ = 0;
	this->kill_score_ = 0;
	this->league_extra_award_ = 0;
	this->league_extra_leader_ = 0;
	this->label_award_ 	 = 0;
	this->first_mail_id_ = 0;
	this->other_mail_id_ = 0;
	this->label_mail_id_ = 0;

	this->defence_resource_ = 0;
	this->attack_resource_ = 0;

	this->interval_tick_ = 0;
	this->add_tick_ = 0;
	this->add_league_ = 0;
	this->add_player_ = 0;

	this->defence_league_ = 0;
	this->league_name_.clear();
	this->has_enter_ = 0;

	this->rank_flag_ = 0;
	this->league_hurt_rank_.clear();
	this->league_score_rank_.clear();

	this->lwar_timer_.state_ = 0;
	this->lwar_timer_.cancel_timer();

	this->attack_birth_list_.clear();
	this->defence_birth_list_.clear();

	this->lwar_monster_map_.clear();
	this->lwar_league_map_.clear();
	this->lwar_league_hurt_map_.clear();
}

void LeagueWarScene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	this->lwar_timer_.schedule_timer(1);
	this->start_monster_ = true;
}

void LeagueWarScene::stop_lwar()
{
	this->lwar_timer_.state_ = 1;
	this->lwar_timer_.cancel_timer();

	// 守住boss的帮派获得帮派积分
	if (this->lwar_league_map_.count(this->defence_league_) > 0)
	{
		LWarLeagueInfo &lwar_league = this->lwar_league_map_[this->defence_league_];
		this->add_league_score(lwar_league.league_index_, lwar_league.league_name_,
				this->defence_boss_league_score_);

		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, lwar_league.league_name_);
		int shout_id = CONFIG_INSTANCE->league_war("defence_shout").asInt();
		GameCommon::announce(this->scene_id(), shout_id, &para_vec);
	}

	this->rank_flag_ = true;
	this->sort_rank();

	const Json::Value& league_rank_award = CONFIG_INSTANCE->league_war("league_rank_award");

	Proto30100242 inner_res;
	// 发送奖励给每个战区排名第一的帮派(分第一战区和其他战区)
	int mail_id = this->space_id() == 0 ? this->first_mail_id_ : this->other_mail_id_;

	for (LWarLeagueMap::iterator iter = this->lwar_league_map_.begin();
			iter != this->lwar_league_map_.end(); ++iter)
	{
		LWarLeagueInfo &lwar_league = iter->second;

		// 开服活动
		if (this->space_id() == 0 && lwar_league.rank_ <= 3)
		{
			for (LongMap::iterator it = lwar_league.lwar_player_.begin();
					it != lwar_league.lwar_player_.end(); ++it)
			{
				ProtoActivityLWarRank *activity_lwar = inner_res.add_activity_lwar();
				activity_lwar->set_role_id(it->first);
				activity_lwar->set_rank(lwar_league.rank_);

				if (lwar_league.leader_index_ == it->first)
					activity_lwar->set_is_leader(1);
				else
					activity_lwar->set_is_leader(0);
			}
		}

		//设置第一战区获胜帮派
		if (this->space_id() == 0 && lwar_league.rank_ == 1)
		{
			LEAGUE_WAR_SYSTEM->set_first_space_winner(lwar_league);

			//第一战区帮主称号邮件
//			MailInformation *mail_info = GameCommon::create_sys_mail(this->label_mail_id_);
//			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
//					mail_info->mail_content_.c_str());
//
//			mail_info->add_goods(this->label_award_);
//			GameCommon::request_save_mail_content(lwar_league.leader_index_, mail_info);
		}

		//获取排名奖励
		int award_id = 0;
		for (uint i = 0; i < league_rank_award.size(); ++i)
		{
			int rank_left = league_rank_award[i][0u].asInt();
			int rank_right = league_rank_award[i][1u].asInt();
			JUDGE_CONTINUE(lwar_league.rank_ >= rank_left && lwar_league.rank_ <= rank_right);

			award_id = league_rank_award[i][2u].asInt();
		}
		JUDGE_CONTINUE(award_id > 0);

		Int64 first_win_league = LEAGUE_WAR_SYSTEM->fetch_first_win_league_id();

		for (LongMap::iterator it = lwar_league.enter_lwar_player_.begin();
					it != lwar_league.enter_lwar_player_.end(); ++it)
		{
			MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), lwar_league.rank_);

			int extra_award = 0;
			mail_info->add_goods(award_id);
//			if (this->space_id() == 0 && lwar_league.rank_ == 1)
			if (first_win_league > 0 && first_win_league == iter->first)
			{
				if (lwar_league.leader_index_ == it->first)
				{
					extra_award = this->league_extra_leader_;
					mail_info->add_goods(this->league_extra_leader_);
				}
				else
				{
					extra_award = this->league_extra_award_;
//					mail_info->add_goods(this->league_extra_award_);
				}
			}
			GameCommon::request_save_mail_content(it->first, mail_info);

			// 记录个人奖励id
			LWarRoleInfo *role_info = LEAGUE_WAR_SYSTEM->find_lwar(it->first);
			JUDGE_CONTINUE(role_info != NULL);

			IntMap &reward_map = role_info->reward_map_;
			++reward_map[award_id];
			if (extra_award > 0)
				++reward_map[extra_award];

			JUDGE_CONTINUE(lwar_league.leader_index_ == it->first);
			++reward_map[this->label_award_];
		}
	}

	if (this->space_id() == 0)
	{
		//开服活动
		MAP_MONITOR->dispatch_to_logic(&inner_res);

		//第一战区获胜帮众称号
		Int64 first_win_league = LEAGUE_WAR_SYSTEM->fetch_first_win_league_id();
		Proto30100231 league_inner;
		league_inner.set_league_id(first_win_league);
		MAP_MONITOR->dispatch_to_logic(&league_inner);
	}

	// 结算
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		Proto80400386 respond;
		LEAGUE_WAR_SYSTEM->fetch_first_space_winner(&respond);

		for (LWarLeagueMap::iterator it = this->lwar_league_map_.begin();
				it != this->lwar_league_map_.end(); ++it)
		{
			LWarLeagueInfo &lwar_league = it->second;
			JUDGE_CONTINUE(lwar_league.rank_ <= 3);

			ProtoLeagueRankInfo *league_rank = respond.add_league_rank();
			league_rank->set_league_index(lwar_league.league_index_);
			league_rank->set_league_name(lwar_league.league_name_);
			league_rank->set_rank(lwar_league.rank_);
		}

		LWarRoleInfo *role_info = LEAGUE_WAR_SYSTEM->find_lwar(iter->first);
		JUDGE_CONTINUE(role_info != NULL);

		IntMap &reward_map = role_info->reward_map_;
		for (IntMap::iterator it = reward_map.begin();
				it != reward_map.end(); ++it)
		{
			ProtoItemId *item_list = respond.add_item_list();
			item_list->set_id(it->first);
			item_list->set_amount(it->second);
		}
		iter->second->respond_to_client(ACTIVE_LWAR_END_INFO, &respond);
	}

	int quit_time = CONFIG_INSTANCE->league_war("quit_time").asInt();
	this->lwar_timer_.schedule_timer(quit_time);
}

void LeagueWarScene::init_lwar_scene(int real_scene, int space_id)
{
	this->real_scene_ = real_scene;
	this->init_scene(space_id, this->real_scene_);

	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, this->real_scene_, this);

	this->lwar_limit_ = CONFIG_INSTANCE->league_war("max_player").asInt();
	this->max_radius_ = CONFIG_INSTANCE->league_war("boss_radius").asInt();
	this->defence_boss_league_score_ = CONFIG_INSTANCE->league_war("defend_boss_add").asInt();
	this->kill_score_ = CONFIG_INSTANCE->league_war("kill_score").asInt();
	this->league_extra_award_ = CONFIG_INSTANCE->league_war("league_extra_award").asInt();
	this->league_extra_leader_ = CONFIG_INSTANCE->league_war("league_extra_leader").asInt();
	this->label_award_ 	 = CONFIG_INSTANCE->league_war("label_award").asInt();
	this->first_mail_id_ = CONFIG_INSTANCE->league_war("first_mail_id").asInt();
	this->other_mail_id_ = CONFIG_INSTANCE->league_war("other_mail_id").asInt();
	this->label_mail_id_ = CONFIG_INSTANCE->league_war("label_mail_id").asInt();

	this->init_monster_layout(1);
	this->init_monster_layout(2);
}

void LeagueWarScene::init_monster_layout(uint type)
{
	const Json::Value layout_info = this->fetch_layout(type);
	const Json::Value& point_json = layout_info["point_coordxy"];
	BirthRecord birth_record;
	for (uint i = 0; i < point_json.size(); ++i)
	{
		birth_record.reset();
		birth_record.__birth_coord.set_pixel(point_json[i][0u].asInt(),
				point_json[i][1u].asInt());

		if (type == 1)
			this->attack_birth_list_.push_back(birth_record);
		else
			this->defence_birth_list_.push_back(birth_record);
	}
}

int LeagueWarScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	LWarMonster* lwar_monster = this->find_monster_by_index(game_ai->ai_id());
	JUDGE_RETURN(lwar_monster != NULL, -1);

	LWarRoleInfo* lwar_role = LEAGUE_WAR_SYSTEM->find_lwar(benefited_attackor);
	JUDGE_RETURN(lwar_role != NULL, -1);

	if (lwar_monster->is_boss_ == 1)
	{
		this->handle_boss_die(lwar_monster, lwar_role);
	}
	else
	{
		this->handle_monster_die(lwar_monster, lwar_role);

		MoverCoord coordxy = game_ai->birth_coord();
		BirthRecord birth_record;
		birth_record.__birth_coord = coordxy;
		if (lwar_monster->conf_index_ == 1)
			this->attack_birth_list_.push_back(birth_record);
		else
			this->defence_birth_list_.push_back(birth_record);
	}

	return 0;
}

int LeagueWarScene::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	LWarMonster* lwar_monster = this->find_monster_by_index(game_ai->ai_id());
	JUDGE_RETURN(lwar_monster != NULL, -1);

	LWarRoleInfo* lwar_role = LEAGUE_WAR_SYSTEM->find_lwar(benefited_attackor);
	JUDGE_RETURN(lwar_role != NULL, -1);

	if (lwar_monster->is_boss_ == true)
	{
		this->add_player_score(benefited_attackor, lwar_monster->attack_player_point_);
		this->add_league_score(lwar_role->league_index_, lwar_role->league_name_, lwar_monster->attack_league_point_);
		this->add_league_hurt(lwar_role->league_index_, lwar_role->league_name_, hurt_value);

		this->rank_flag_ = true;
	}
	return 0;
}

void LeagueWarScene::handle_boss_die(LWarMonster* lwar_monster, LWarRoleInfo* lwar_role)
{
	for (LWarLeagueMap::iterator iter = this->lwar_league_map_.begin();
			iter != this->lwar_league_map_.end(); ++iter)
	{
		bool add_flag = false;
		LWarLeagueInfo &lwar_league = iter->second;
		for (LongMap::iterator it = lwar_league.enter_lwar_player_.begin();
				it != lwar_league.enter_lwar_player_.end(); ++it)
		{
			// 增加个人积分
			LWarRoleInfo* role_info = LEAGUE_WAR_SYSTEM->find_lwar(it->first);
			JUDGE_CONTINUE(role_info != NULL);

			int camp_index = role_info->camp_id();
			if (camp_index == TEMP_ATTACK + 1)
			{
				this->add_player_score(it->first, lwar_monster->kill_player_point_);
				add_flag = true;
			}
			else
			{
				this->change_camp_index(it->first, TEMP_ATTACK + 1);
				role_info->camp_index_ = TEMP_ATTACK;
			}
		}
		//增加帮派积分
		JUDGE_CONTINUE(add_flag == true);
		this->add_league_score(iter->first,lwar_league.league_name_, lwar_monster->kill_league_point_);
	}

	// 帮派伤害排名增加帮派积分
	Int64 top_league = 0;
	string league_name;
	const Json::Value hurt_rank_awards = CONFIG_INSTANCE->league_war("hurt_rank_awards");
	for (LWarLeagueHurtMap::iterator iter = this->lwar_league_hurt_map_.begin();
			iter != this->lwar_league_hurt_map_.end(); ++iter)
	{
		LWarLeagueHurt &league_hurt = iter->second;
		int rank = league_hurt.rank_;
		for (uint i = 0; i < hurt_rank_awards.size(); ++i)
		{
			int hight_rank = hurt_rank_awards[i][0u].asInt();
			int low_rank = hurt_rank_awards[i][1u].asInt();
			int rank_score = hurt_rank_awards[i][2u].asInt();

			JUDGE_CONTINUE(rank >=hight_rank && rank <= low_rank);

			this->add_league_score(league_hurt.league_index_, league_hurt.league_name_, rank_score);
		}
		JUDGE_CONTINUE(rank == 1);

		top_league = iter->first;
		league_name = league_hurt.league_name_;
	}
	// clear league hurt rank
	this->lwar_league_hurt_map_.clear();

	LWarLeagueInfo &lwar_league = this->lwar_league_map_[top_league];
	for (LongMap::iterator iter = lwar_league.enter_lwar_player_.begin();
			iter != lwar_league.enter_lwar_player_.end(); ++iter)
	{
		this->change_camp_index(iter->first, TEMP_DEFENCE + 1);
		LWarRoleInfo* role_info = LEAGUE_WAR_SYSTEM->find_lwar(iter->first);
		role_info->camp_index_ = TEMP_DEFENCE;
	}
	this->set_camp_info(top_league, league_name);

	this->attack_resource_ = 0;
	this->defence_resource_ = 0;
	this->boss_id_ = 0;
	this->erase_monster_map(lwar_monster->index_);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_int(para_vec, this->space_id()+1);
	GameCommon::push_brocast_para_string(para_vec, lwar_league.league_name_);
	GameCommon::push_brocast_para_string(para_vec, lwar_league.league_name_);
	int shout_id = CONFIG_INSTANCE->league_war("kill_boss_shout").asInt();
	GameCommon::announce(this->scene_id(), shout_id, &para_vec);
}

void LeagueWarScene::handle_monster_die(LWarMonster* lwar_monster, LWarRoleInfo* lwar_role)
{
	this->add_player_score(lwar_role->id_, lwar_monster->kill_player_point_);
	this->add_league_score(lwar_role->league_index_, lwar_role->league_name_, lwar_monster->kill_league_point_);

	if (lwar_role->camp_id() == lwar_monster->owner_camp_)
	{
		if (lwar_monster->conf_index_ == 1)
			this->attack_resource_ += lwar_monster->kill_resource_point_;
		else
			this->defence_resource_ += lwar_monster->kill_resource_point_;
	}

	this->erase_monster_map(lwar_monster->index_);
}

void LeagueWarScene::handle_player_kill(Int64 attacked_id, Int64 be_killed_id)
{
	this->add_player_score(attacked_id, this->kill_score_);
}

void LeagueWarScene::add_player_score(Int64 role_id, int score)
{
	JUDGE_RETURN(score > 0, ;);

	// 记录个人奖励id
	LWarRoleInfo *role_info = LEAGUE_WAR_SYSTEM->find_lwar(role_id);
	JUDGE_RETURN(role_info != NULL, ;);

	role_info->score_ += score;
	role_info->tick_ = ::time(NULL);

	IntMap &reward_map = role_info->reward_map_;
	IntMap &record_map = role_info->record_map_;

	const Json::Value &player_score_awards = CONFIG_INSTANCE->league_war("player_score_awards");
	for (uint i = 0; i < player_score_awards.size(); ++i)
	{
		int need_score = player_score_awards[i][0u].asInt();
		int reward_id = player_score_awards[i][1u].asInt();

		if(role_info->score_ >= need_score && record_map.count(need_score) == 0)
		{
			record_map[need_score] = true;
			++(reward_map[reward_id]);

			MapPlayerEx* player = this->find_player(role_id);
			if (player != NULL)
			{
				//online
				player->request_add_reward(reward_id, SerialObj(ADD_FROM_LWAR_ROLE_SCORE));
			}
		}
	}
	this->rank_flag_ = true;
}

void LeagueWarScene::add_league_score(Int64 league_index, string league_name, int score)
{
	JUDGE_RETURN(score > 0, ;);

	if (this->lwar_league_map_.count(league_index) == 0)
	{
		LWarLeagueInfo &lwar_league = this->lwar_league_map_[league_index];
		lwar_league.league_index_ = league_index;
		lwar_league.league_name_ = league_name;
		lwar_league.score_ = score;
		lwar_league.tick_ = ::time(NULL);
		lwar_league.space_id_ = this->space_id();
	}
	else
	{
		LWarLeagueInfo &lwar_league = this->lwar_league_map_[league_index];
		lwar_league.score_ += score;
		lwar_league.tick_ = ::time(NULL);
	}

	LEAGUE_WAR_SYSTEM->update_leauge_score(this->lwar_league_map_[league_index]);
}

void LeagueWarScene::add_league_hurt(Int64 league_index, string league_name, int hurt_value)
{
	JUDGE_RETURN(hurt_value > 0, ;);

	if (this->lwar_league_hurt_map_.count(league_index) == 0)
	{
		LWarLeagueHurt &league_hurt = this->lwar_league_hurt_map_[league_index];
		league_hurt.league_index_ = league_index;
		league_hurt.league_name_ = league_name;
		league_hurt.hurt_value_ = hurt_value;
		league_hurt.tick_ = ::time(NULL);
	}
	else
	{
		LWarLeagueHurt &league_hurt = this->lwar_league_hurt_map_[league_index];
		league_hurt.hurt_value_ += hurt_value;
		league_hurt.tick_ = ::time(NULL);
	}
}

void LeagueWarScene::enter_player(LWarRoleInfo* lwar_role)
{
	lwar_role->state_ = 2;
	lwar_role->tick_  = ::time(NULL);

	this->player_count_ += 1;
	this->has_enter_ = 1;

	if (this->lwar_league_map_.count(lwar_role->league_index_) == 0)
	{
		this->serialize(lwar_role, this->lwar_league_map_[lwar_role->league_index_]);
	}
	LWarLeagueInfo &lwar_league = this->lwar_league_map_[lwar_role->league_index_];
	lwar_league.enter_lwar_player_[lwar_role->id_] = lwar_role->league_index_;
	lwar_league.tick_ = ::time(NULL);
	lwar_league.space_id_ = this->space_id();
}

void LeagueWarScene::exit_player(LWarRoleInfo* lwar_role)
{
	lwar_role->state_ = 3;
	lwar_role->tick_  = ::time(NULL);

	this->player_count_ -= 1;

	JUDGE_RETURN(this->lwar_league_map_.count(lwar_role->league_index_) > 0, ;);

	LWarLeagueInfo &lwar_league = this->lwar_league_map_[lwar_role->league_index_];
	lwar_league.enter_lwar_player_.erase(lwar_role->id_);
}

void LeagueWarScene::update_league_info(Int64 league_id, Int64 leader_id, string leader)
{
	JUDGE_RETURN(this->lwar_league_map_.count(league_id) > 0, ;);

	LWarLeagueInfo &lwar_league = this->lwar_league_map_[league_id];
	lwar_league.leader_index_ = leader_id;
	lwar_league.leader_name_ = leader;
}

void LeagueWarScene::register_player(LWarRoleInfo* lwar_role)
{
	lwar_role->state_ = 1;
	lwar_role->tick_  = ::time(NULL);

	if (this->lwar_league_map_.count(lwar_role->league_index_) == 0)
	{
		this->serialize(lwar_role, this->lwar_league_map_[lwar_role->league_index_]);
	}
	LWarLeagueInfo &lwar_league = this->lwar_league_map_[lwar_role->league_index_];
	lwar_league.lwar_player_[lwar_role->id_] = lwar_role->league_index_;
	lwar_league.tick_ = ::time(NULL);
	lwar_league.space_id_ = this->space_id();
}

void LeagueWarScene::unregister_player(Int64 role_id)
{

}

int LeagueWarScene::is_player_full()
{
	return this->player_count_ >= this->lwar_limit_;
}

int LeagueWarScene::fetch_camp_index(Int64 league_index)
{
	if (league_index == this->defence_league_)
		return TEMP_DEFENCE;
	else
		return TEMP_ATTACK;
}

void LeagueWarScene::set_camp_info(Int64 league_index, string league_name)
{
	this->defence_league_ = league_index;
	this->league_name_ = league_name;
}

void LeagueWarScene::change_camp_index(Int64 role_id, int camp_index)
{
	GameFighter *fight = this->find_fighter(role_id);
	JUDGE_RETURN(fight != NULL, ;);
	fight->set_camp_id(camp_index);
	fight->notify_fight_update(FIGHT_UPDATE_CAMP, fight->camp_id());
}

int LeagueWarScene::fetch_attack_resource()
{
	return this->attack_resource_;
}

int LeagueWarScene::fetch_defence_resource()
{
	return this->defence_resource_;
}

GameAI* LeagueWarScene::find_space_boss()
{
	JUDGE_RETURN(this->lwar_monster_map_.count(this->boss_id_) > 0, 0);
	LWarMonster &lwar_monster = this->lwar_monster_map_[this->boss_id_];
	JUDGE_RETURN(lwar_monster.is_boss_ == true, 0);

	GameAI* game_ai = AI_PACKAGE->find_object(lwar_monster.index_);
	JUDGE_RETURN(game_ai != NULL, 0);

	return game_ai;
}

int LeagueWarScene::fetch_league_hurt_info(ProtoLWarInfo* lwar_detail)
{
	GameAI *game_ai = this->find_space_boss();
	if (game_ai != NULL)
	{
		int total_blood = game_ai->fight_detail().__blood_total_i(game_ai);
		for (LWarLeagueHurtMap::iterator iter = this->lwar_league_hurt_map_.begin();
				iter != this->lwar_league_hurt_map_.end(); ++iter)
		{
			LWarLeagueHurt &league_hurt = iter->second;

			ProtoLeagueRankInfo *room_league_rank = lwar_detail->add_room_league_rank();
			room_league_rank->set_league_index(league_hurt.league_index_);
			room_league_rank->set_league_name(league_hurt.league_name_);
			room_league_rank->set_rank(league_hurt.rank_);

			double hurt_precent = (double)league_hurt.hurt_value_ / (double)total_blood;
			room_league_rank->set_hurt(hurt_precent);
		}

		BasicStatus* c_status = NULL;
		if (game_ai->find_status(BasicStatus::ATTACK, c_status) == 0)
		{
			int level = 0;
			const Json::Value& add_attack_buff = CONFIG_INSTANCE->league_war("add_attack_buff");
			for (uint i = 0; i < add_attack_buff.size(); ++i)
			{
				int add_num = add_attack_buff[i][1u].asInt();
				if (add_num == c_status->__value1)
				{
					level = (int)i + 1;
					break;
				}
			}
			lwar_detail->set_attack_add(c_status->__value1);
			lwar_detail->set_attack_level(level);
		}
		else
		{
			lwar_detail->set_attack_add(0);
			lwar_detail->set_attack_level(0);
		}
		c_status = NULL;
		if (game_ai->find_status(BasicStatus::DEFENCE, c_status) == 0)
		{
			int level = 0;
			const Json::Value& add_defence_buff = CONFIG_INSTANCE->league_war("add_defence_buff");
			for (uint i = 0; i < add_defence_buff.size(); ++i)
			{
				int add_num = -add_defence_buff[i][1u].asInt();
				if (add_num == c_status->__value1)
				{
					level = (int)i + 1;
					break;
				}
			}
			lwar_detail->set_defence_add(c_status->__value1);
			lwar_detail->set_defence_level(level);
		}
		else
		{
			lwar_detail->set_defence_add(0);
			lwar_detail->set_defence_level(0);
		}
	}

	return 0;
}

int LeagueWarScene::fetch_my_league_hurt(ProtoLWarInfo* lwar_detail, Int64 league_index)
{
	JUDGE_RETURN(this->lwar_league_hurt_map_.count(league_index) > 0, NULL);

	ProtoLeagueRankInfo *room_league_rank = lwar_detail->mutable_room_my_league();

	LWarLeagueHurt &league_hurt = this->lwar_league_hurt_map_[league_index];
	room_league_rank->set_league_index(league_index);
	room_league_rank->set_league_name(league_hurt.league_name_);
	room_league_rank->set_rank(league_hurt.rank_);

	GameAI *game_ai = this->find_space_boss();
	if (game_ai != NULL)
	{
		int total_blood = game_ai->fight_detail().__blood_total_i(game_ai);
		double hurt_precent = (double)league_hurt.hurt_value_ / (double)total_blood;
		room_league_rank->set_hurt(hurt_precent);
	}
	else
	{
		room_league_rank->set_hurt(0);
	}

	return 0;
}

int LeagueWarScene::fetch_league_score_info(LWarLeagueInfo* all_league)
{
	JUDGE_RETURN(this->lwar_league_map_.size() > 0, -1);

	for(LWarLeagueMap::iterator iter = this->lwar_league_map_.begin();
			iter != this->lwar_league_map_.end(); ++iter)
	{
		LWarLeagueInfo &lwar_league = iter->second;
		JUDGE_CONTINUE(lwar_league.league_index_ == all_league->league_index_);

		if (lwar_league.score_ > all_league->score_)
		{
			all_league->score_ = lwar_league.score_;
		}
		all_league->league_name_ = lwar_league.league_name_;
		all_league->tick_ = ::time(NULL);
	}

	return 0;
}

LeagueWarScene::LWarMonster* LeagueWarScene::find_monster_by_index(Int64 index)
{
	JUDGE_RETURN(this->lwar_monster_map_.count(index) > 0, NULL);
	return &this->lwar_monster_map_[index];
}

int LeagueWarScene::erase_monster_map(Int64 index)
{
	return this->lwar_monster_map_.erase(index);
}

const Json::Value LeagueWarScene::fetch_layout(uint type)
{
	const Json::Value layout_json = CONFIG_INSTANCE->league_war("layout");
	JUDGE_RETURN(layout_json != Json::Value::null, Json::Value::null);

	return layout_json[type];
}

bool LeagueWarScene::is_player_enter()
{
	return this->has_enter_ == true;
}

void LeagueWarScene::test_set_has_enter()
{
	this->has_enter_ = true;
}

void LeagueWarScene::generate_boss(bool shout)
{
	int current_num = 0;
	for (LWarMonsterMap::iterator iter = this->lwar_monster_map_.begin();
			iter != this->lwar_monster_map_.end(); ++iter)
	{
		LWarMonster& lwar_monster = iter->second;
		JUDGE_CONTINUE(lwar_monster.is_boss_ > 0);

		current_num += 1;
	}
	JUDGE_RETURN(current_num <= 0, ;);

	uint generate_type = 0;
	const Json::Value layout_info = this->fetch_layout(generate_type);
	JUDGE_RETURN(layout_info != Json::Value::null, ;);

	int monster_sort = layout_info["monster_sort"].asInt();
	const Json::Value& point_coordxy = layout_info["point_coordxy"];

	MoverCoord coordxy;
	coordxy.set_pixel(point_coordxy[0u].asInt(), point_coordxy[1u].asInt());

	Int64 monster_id = AIMANAGER->generate_monster_by_sort(monster_sort, coordxy, this);
	JUDGE_RETURN(monster_id > 0, ;);

	LWarMonster &lwar_monster = this->lwar_monster_map_[monster_id];
	lwar_monster.index_ = monster_id;
	lwar_monster.sort_ = monster_sort;
	lwar_monster.conf_index_ = generate_type;
	lwar_monster.owner_camp_ = TEMP_DEFENCE + 1;
	lwar_monster.is_boss_ = 1;
	lwar_monster.loaction_ = coordxy;

	this->boss_id_ = monster_id;

	const Json::Value& attack_add = layout_info["attack_boss_add"];
	const Json::Value& kill_add = layout_info["kill_boss_add"];
	lwar_monster.attack_player_point_ = attack_add[0u].asInt();
	lwar_monster.attack_league_point_ = attack_add[1u].asInt();
	lwar_monster.kill_player_point_ = kill_add[0u].asInt();
	lwar_monster.kill_league_point_ = kill_add[1u].asInt();

	const Json::Value boss_add_score = CONFIG_INSTANCE->league_war("boss_add_score");
	this->add_league_ = boss_add_score[0u].asInt();
	this->add_player_ = boss_add_score[1u].asInt();
	this->interval_tick_ = boss_add_score[2u].asInt();

	this->add_tick_ = ::time(NULL) + this->interval_tick_;

	GameAI* game_ai = AI_PACKAGE->find_object(lwar_monster.index_);
	JUDGE_RETURN(game_ai != NULL, ;);
	game_ai->set_camp_id(lwar_monster.owner_camp_);
	game_ai->ai_detail().league_index_ = lwar_monster.owner_camp_;
	if (this->defence_league_ != 0 && this->league_name_.c_str() != NULL)
	{
		game_ai->ai_detail().__name = this->league_name_;
	}
//	game_ai->notify_fight_update(FIGHT_UPDATE_CAMP, lwar_monster.owner_camp_);
	this->boss_flag_ = ::time(NULL) + 1;

	JUDGE_RETURN(shout == true, ;);
//	this->monitor()->announce_world(tem_item->shout_refresh_);
}

void LeagueWarScene::send_boss_camp()
{
	JUDGE_RETURN(this->boss_flag_ > 0 && ::time(NULL) >= this->boss_flag_, ;);
	JUDGE_RETURN(this->lwar_monster_map_.count(this->boss_id_) > 0, ;);

	this->boss_flag_ = 0;

	LWarMonster &lwar_monster = this->lwar_monster_map_[this->boss_id_];
	JUDGE_RETURN(lwar_monster.is_boss_ == 1, ;);

	GameAI* game_ai = this->find_ai(this->boss_id_);
	JUDGE_RETURN(game_ai != NULL, ;);

	game_ai->notify_fight_update(FIGHT_UPDATE_CAMP, lwar_monster.owner_camp_);
}

void LeagueWarScene::generate_monster(uint type)
{
	int current_num = 0;
	uint generate_type = type;
	for (LWarMonsterMap::iterator iter = this->lwar_monster_map_.begin();
			iter != this->lwar_monster_map_.end(); ++iter)
	{
		LWarMonster& lwar_monster = iter->second;
		JUDGE_CONTINUE(lwar_monster.conf_index_ == generate_type);

		current_num += 1;
	}

	const Json::Value layout_info = this->fetch_layout(generate_type);
	JUDGE_RETURN(layout_info != Json::Value::null, ;);
	JUDGE_RETURN(current_num < layout_info["max_exist"].asInt(), ;);

	int once_num 	 = layout_info["once"].asInt(),
		max_num 	 = layout_info["max_exist"].asInt(),
		monster_sort = layout_info["monster_sort"].asInt(),
		max_count 	 = layout_info["max_count"].asInt();
	if (max_count == 0)
		max_count = 99999999;
	if (max_num == 0)
		max_num = max_count;

	const Json::Value &monster_json = CONFIG_INSTANCE->monster(monster_sort);
	if (monster_json == Json::Value::null)
	{
		MSG_USER("ERROR generate monster sort error %d %d", this->space_id(), monster_sort);
		return ;
	}

	const Json::Value& point_json = layout_info["point_coordxy"];
	for (int i = 0; i < once_num && (i + current_num) < max_num && i < max_count; ++i)
	{
		MoverCoord coordxy;
		if (type == 1)
		{
			if (this->attack_birth_list_.size() <= 0)
			{
				int index = rand() % point_json.size();
				coordxy.set_pixel(point_json[index][0u].asInt(), point_json[index][1u].asInt());
			}
			else
			{
				BirthRecord &birth_record = this->attack_birth_list_.front();
				coordxy = birth_record.__birth_coord;
				this->attack_birth_list_.pop_front();
			}
		}
		else
		{
			if (this->defence_birth_list_.size() <= 0)
			{
				int index = rand() % point_json.size();
				coordxy.set_pixel(point_json[index][0u].asInt(), point_json[index][1u].asInt());
			} else
			{
				BirthRecord &birth_record = this->defence_birth_list_.front();
				coordxy = birth_record.__birth_coord;
				this->defence_birth_list_.pop_front();
			}
		}

		Int64 monster_id = AIMANAGER->generate_monster_by_sort(monster_sort, coordxy, this);
		JUDGE_CONTINUE(monster_id > 0);

		LWarMonster &lwar_monster = this->lwar_monster_map_[monster_id];
		lwar_monster.index_ = monster_id;
		lwar_monster.sort_ = monster_sort;
		lwar_monster.conf_index_ = generate_type;
		lwar_monster.is_boss_ = 0;
		lwar_monster.loaction_ = coordxy;

		if (generate_type == 1)
			lwar_monster.owner_camp_ = TEMP_ATTACK + 1;
		else if(generate_type == 2)
			lwar_monster.owner_camp_ = TEMP_DEFENCE + 1;

		int set_temp = TEMP_MIDDLE + 1;

		if (layout_info.isMember("kill_attack_monster"))
		{
			const Json::Value& kill_add = layout_info["kill_attack_monster"];
			lwar_monster.kill_player_point_ = kill_add[0u].asInt();
			lwar_monster.kill_league_point_ = kill_add[1u].asInt();
			lwar_monster.kill_resource_point_ = kill_add[2u].asInt();
		}
		else if (layout_info.isMember("kill_defend_monster"))
		{
			const Json::Value& kill_add = layout_info["kill_defend_monster"];
			lwar_monster.kill_player_point_ = kill_add[0u].asInt();
			lwar_monster.kill_league_point_ = kill_add[1u].asInt();
			lwar_monster.kill_resource_point_ = kill_add[2u].asInt();
		}

		GameAI* game_ai = AI_PACKAGE->find_object(lwar_monster.index_);
		JUDGE_RETURN(game_ai != NULL, ;);
		game_ai->set_camp_id(set_temp);
		game_ai->ai_detail().league_index_ = set_temp;
	}
}

void LeagueWarScene::active_add_buff()
{
	JUDGE_RETURN(this->lwar_monster_map_.count(this->boss_id_) > 0, ;);

	LWarMonster &lwar_monster = this->lwar_monster_map_[this->boss_id_];
	JUDGE_RETURN(lwar_monster.is_boss_ == 1, ;);

	GameAI* game_ai = this->find_ai(this->boss_id_);
	JUDGE_RETURN(game_ai != NULL, ;);

	int defence_resource = this->fetch_defence_resource();
	int attack_resource = this->fetch_attack_resource();
	double add_attack = 0;
	double red_defence = 0;
	const Json::Value& add_attack_buff = CONFIG_INSTANCE->league_war("add_attack_buff");
	const Json::Value& add_defence_buff = CONFIG_INSTANCE->league_war("add_defence_buff");
	for (uint i = 0; i < add_attack_buff.size(); ++i)
	{
		int add_resource = add_attack_buff[i][0u].asInt();
		int add_num = add_attack_buff[i][1u].asInt();
		if(defence_resource >= add_resource)
			add_attack = add_num;
	}
	for (uint i = 0; i < add_defence_buff.size(); ++i)
	{
		int add_resource = add_defence_buff[i][0u].asInt();
		int add_num = add_defence_buff[i][1u].asInt();
		if (attack_resource >= add_resource)
			red_defence = -add_num;
	}

	if (add_attack > 0)
	{
		BasicStatus* c_status = NULL;
		if (game_ai->find_status(BasicStatus::ATTACK, c_status) == 0 && add_attack > c_status->__value1)
		{
			game_ai->remove_status(c_status);
			game_ai->insert_defender_status(game_ai, BasicStatus::ATTACK, 0, 86400, 0, add_attack);
		}
		else if(game_ai->find_status(BasicStatus::ATTACK, c_status) != 0)
		{
			game_ai->insert_defender_status(game_ai, BasicStatus::ATTACK, 0, 86400, 0, add_attack);
		}
	}
	if (red_defence < 0)
	{
		BasicStatus* c_status = NULL;
		if (game_ai->find_status(BasicStatus::DEFENCE, c_status) == 0 && red_defence < c_status->__value1)
		{
			game_ai->remove_status(c_status);
			game_ai->insert_defender_status(game_ai, BasicStatus::DEFENCE, 0, 86400, 0, red_defence);
		}
		else if(game_ai->find_status(BasicStatus::DEFENCE, c_status) != 0)
		{
			game_ai->insert_defender_status(game_ai, BasicStatus::DEFENCE, 0, 86400, 0, red_defence);
		}
	}
}

void LeagueWarScene::active_check_boss_radius()
{
	GameAI *game_ai = this->find_space_boss();
	LWarMonster *lwar_monster = this->find_monster_by_index(this->boss_id_);
	JUDGE_RETURN(game_ai != NULL, ;);
	JUDGE_RETURN(lwar_monster != NULL, ;);

	int radius = this->cal_point_radius(game_ai->mover_detail().__location, lwar_monster->loaction_);
	JUDGE_RETURN(radius > this->max_radius_, ;);

	game_ai->set_moveto_action_coord(lwar_monster->loaction_);
}

int LeagueWarScene::cal_point_radius(MoverCoord coord1, MoverCoord coord2)
{
	int x1 = coord1.pixel_x(), y1 = coord1.pixel_y(),
		x2 = coord2.pixel_x(), y2 = coord2.pixel_y();

	return std::sqrt(pow(x2-x1,2)+pow(y2-y1,2));
}

void LeagueWarScene::serialize(LWarRoleInfo* lwar_role, LWarLeagueInfo& lwar_league)
{
	lwar_league.league_index_ = lwar_role->league_index_;
	lwar_league.leader_index_ = lwar_role->leader_index_;
	lwar_league.league_name_ = lwar_role->league_name_;
	lwar_league.leader_name_ = lwar_role->leader_name_;
	lwar_league.score_ = 0;
}

void LeagueWarScene::notify_all_player_info()
{
	Proto80400385 auto_res;
	auto_res.set_space_id(this->space_id());
	auto_res.set_left_time(LEAGUE_WAR_SYSTEM->lwar_left_time());

	LEAGUE_WAR_SYSTEM->find_open_space(&auto_res);

	GameAI *game_ai = this->find_space_boss();
	if (game_ai != NULL)
	{
		auto_res.set_boss_id(this->boss_id_);
		auto_res.set_boss_name(game_ai->ai_detail().__name);
		double total_blood = game_ai->fight_detail().__blood_total(game_ai);
		auto_res.set_total_blood(total_blood);
		double left_percent = game_ai->fight_detail().blood_percent(game_ai, 1);
		auto_res.set_boss_blood(left_percent);
	}

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		this->make_up_player_info(iter->first, &auto_res);
		iter->second->respond_to_client(ACTIVE_LWAR_SPACE_INFO, &auto_res);
	}
}

void LeagueWarScene::make_up_player_info(Int64 role_id, Proto80400385* respond)
{
	LWarRoleInfo *role_info = LEAGUE_WAR_SYSTEM->find_lwar(role_id);
	JUDGE_RETURN(role_info != NULL, ;);

	LWarLeagueInfo &lwar_league = this->lwar_league_map_[role_info->league_index_];
	respond->set_league_score(lwar_league.score_);
	respond->set_league_rank(lwar_league.rank_);
	respond->set_my_score(role_info->score_);
}

void LeagueWarScene::add_online_score(Int64 now_time)
{
	JUDGE_RETURN(this->add_tick_ <= now_time, ;);

	this->add_tick_ = now_time + this->interval_tick_;

	if (this->lwar_league_map_.count(this->defence_league_) > 0)
	{
		LWarLeagueInfo &lwar_league = this->lwar_league_map_[this->defence_league_];
		for (LongMap::iterator iter = lwar_league.enter_lwar_player_.begin();
				iter != lwar_league.enter_lwar_player_.end(); ++iter)
		{
			LWarRoleInfo* role_info = LEAGUE_WAR_SYSTEM->find_lwar(iter->first);
			JUDGE_CONTINUE(role_info != NULL);

			this->add_player_score(iter->first, this->add_player_);
		}
		this->add_league_score(lwar_league.league_index_,lwar_league.league_name_, this->add_league_);
	}
}

void LeagueWarScene::sort_rank()
{
	JUDGE_RETURN(this->rank_flag_ == true, ;);

	this->rank_flag_ = false;
	this->league_score_rank_.clear();
	this->league_hurt_rank_.clear();

	// league score rank
	for (LWarLeagueMap::iterator iter = this->lwar_league_map_.begin();
			iter != this->lwar_league_map_.end(); ++iter)
	{
		LWarLeagueInfo &lwar_league = iter->second;

		ThreeObj obj;
		obj.id_ = lwar_league.league_index_;
		obj.tick_ = lwar_league.tick_;
		obj.value_ = lwar_league.score_;

		this->league_score_rank_.push_back(obj);
	}

	int rank = 1;
	std::sort(this->league_score_rank_.begin(), this->league_score_rank_.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->league_score_rank_.begin();
			iter != this->league_score_rank_.end(); ++iter)
	{
		LWarLeagueInfo &lwar_league = this->lwar_league_map_[iter->id_];
		lwar_league.rank_ = rank;
		++rank;
	}

	if (this->lwar_league_hurt_map_.count(this->defence_league_) > 0)
			this->lwar_league_hurt_map_.erase(this->defence_league_);

	// league hurt rank
	for (LWarLeagueHurtMap::iterator iter = this->lwar_league_hurt_map_.begin();
			iter != this->lwar_league_hurt_map_.end(); ++iter)
	{
		LWarLeagueHurt &league_hurt = iter->second;

		ThreeObj obj;
		obj.id_ = league_hurt.league_index_;
		obj.tick_ = league_hurt.tick_;
		obj.value_ = league_hurt.hurt_value_;

		this->league_hurt_rank_.push_back(obj);
	}

	rank = 1;
	std::sort(this->league_hurt_rank_.begin(), this->league_hurt_rank_.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->league_hurt_rank_.begin();
			iter != this->league_hurt_rank_.end(); ++iter)
	{
		LWarLeagueHurt &league_hurt = this->lwar_league_hurt_map_[iter->id_];
		league_hurt.rank_ = rank;
		++rank;
	}
}



