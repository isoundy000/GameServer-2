/*
 * LeagueBoss.cpp
 *
 *  Created on: 2016年8月5日
 *      Author: lyw
 */

#include "LeagueBoss.h"
#include "AIManager.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "LeagueMonitor.h"
#include "ProtoDefine.h"

LeagueBoss::LBossItem::LBossItem()
{
	this->id_ = 0;
	this->sort_ = 0;
	this->status_ = 0;
	this->summon_type_ = 0;
	this->flag_level_ = 0;
	this->direct_award_ = 0;
	this->index_ = 0;
	this->league_index_ = 0;
	this->total_blood_ = 0;
	this->loaction_.reset();
	this->buff_on_.clear();
}

LeagueBoss::PlayerHurtInfo::PlayerHurtInfo()
{
	this->__player_id = 0;
	this->__tick = 0;
	this->__amount_hurt = 0;
	this->__rank = 0;
}

LeagueBoss::BOSSTimer::BOSSTimer(void)
{
	this->scene_ = NULL;
}

LeagueBoss::BOSSTimer::~BOSSTimer(void)
{ /*NULL*/ }

void LeagueBoss::BOSSTimer::set_script_scene(LeagueBoss *scene)
{
    this->scene_ = scene;
}

int LeagueBoss::BOSSTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int LeagueBoss::BOSSTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->scene_ != NULL, -1);

	Int64 cur_time = ::time(NULL);
	if (cur_time >= this->scene_->refresh_tick_)
	{
		this->scene_->refresh_scene();
	}
	if (cur_time >= this->scene_->exp_tick_)
	{
		this->scene_->notify_add_player_exp();
	}
	this->scene_->sort_rank();
	this->scene_->send_exp_and_refresh_tick();

	return 0;
}

LeagueBoss::LeagueBoss() {
	// TODO Auto-generated constructor stub
	this->rank_flag_ = 0;
	this->refresh_flag_ = 0;
	this->exp_interval_ = 0;
	this->refresh_tick_ = 0;
	this->exp_tick_ = 0;
	this->boss_timer_.scene_ = this;
}

LeagueBoss::~LeagueBoss() {
	// TODO Auto-generated destructor stub
}

void LeagueBoss::reset()
{
	Scene::reset();

	this->rank_flag_ = 0;
	this->refresh_flag_ = 0;
	this->exp_interval_ = 0;
	this->refresh_tick_ = 0;
	this->exp_tick_ = 0;

	this->player_hurt_rank_.clear();
	this->chest_map_.clear();
	this->boss_timer_.cancel_timer();
	this->player_hurt_map_.clear();
	this->player_exp_map_.clear();

	this->scene_detail_.__scene_id = GameEnum::LBOSS_SCENE_ID;
	this->scene_detail_.__space_id = this->hash_id();
}

int LeagueBoss::reset_scene_info()
{
	this->rank_flag_ = 0;
	this->player_hurt_rank_.clear();
	this->chest_map_.clear();
	this->player_hurt_map_.clear();

	return 0;
}

void LeagueBoss::init_league_boss_scene(int flag_lvl)
{
	int scene_id = GameEnum::LBOSS_SCENE_ID;
	this->init_scene(this->hash_id(), scene_id);
	this->start_scene();
	MAP_MONITOR->bind_scene(this->hash_id(), scene_id, this);

	this->lboss_item_.flag_level_ = flag_lvl;
}

int LeagueBoss::generate_boss()
{
	this->recycle_all_monster();

	const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(this->lboss_item_.id_);
	JUDGE_RETURN(boss_info != Json::Value::null, ERROR_CONFIG_ERROR);

	int boss_id = boss_info["boss_id"].asInt();
	const Json::Value& boss_pos = CONFIG_INSTANCE->league_boss("boss_pos");

	MoverCoord coordxy;
	coordxy.set_pixel(boss_pos[0u].asInt(), boss_pos[1u].asInt());

	Int64 monster_id = AIMANAGER->generate_monster_by_sort(boss_id, coordxy, this);
	JUDGE_RETURN(monster_id > 0, -1);

	GameAI *game_ai = AIMANAGER->ai_package()->find_object(monster_id);
	JUDGE_RETURN(game_ai != NULL, -1);

	LBossItem &lboss_item = this->lboss_item_;
	lboss_item.sort_ = boss_id;
	lboss_item.index_ = monster_id;
	lboss_item.status_ = 1;
	lboss_item.direct_award_ = boss_info["direct_award"].asInt();
	lboss_item.total_blood_ = game_ai->fight_detail().__blood_total_i(game_ai);
	lboss_item.loaction_ = coordxy;

	if (boss_info.isMember("add_buff"))
	{
		const Json::Value& add_buff = boss_info["add_buff"];
		for (uint i = 0; i < add_buff.size(); ++i)
		{
			int buff_status = add_buff[i]["status"].asInt();
			lboss_item.buff_on_[buff_status] = false;
		}
	}
	int during_tick = CONFIG_INSTANCE->league_boss("boss_exit_time").asInt();
	this->refresh_tick_ = ::time(NULL) + during_tick;
	this->refresh_flag_ = true;
	this->rank_flag_ = true;
	this->exp_tick_ = ::time(NULL) + this->exp_interval_;

	return 0;
}

void LeagueBoss::generate_shield(GameAI* game_ai)
{
	IntMap& buff_on = this->lboss_item_.buff_on_;
	int blood_status = 0;
	for (IntMap::iterator iter = buff_on.begin(); iter != buff_on.end(); ++iter)
	{
		if (iter->second == false && blood_status < iter->first)
			blood_status = iter->first;
	}
	JUDGE_RETURN(blood_status > 0, ;);

	double cur_blood = game_ai->fight_detail().blood_percent(game_ai, 100);
	if (cur_blood <= blood_status)
	{
		const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(this->lboss_item_.id_);
		const Json::Value& add_buff = boss_info["add_buff"];
		for (uint i = 0; i < add_buff.size(); ++i)
		{
			JUDGE_CONTINUE(blood_status == add_buff[i]["status"].asInt());

			const Json::Value& buff_info = add_buff[i];
			int value = buff_info["value"].asInt();
			int last = buff_info["last"].asInt();
			int reduce = buff_info["reduce"].asInt();
			game_ai->insert_defender_status(game_ai, BasicStatus::SHIELD, 0, last, 0, value, reduce, 0, value);
			buff_on[blood_status] = true;

			BasicStatus* c_status = NULL;
			JUDGE_RETURN(game_ai->find_status(BasicStatus::SHIELD, c_status) == 0, ;);

			this->notify_shield_info(c_status);
			break;
		}
	}
}

//int LeagueBoss::notify_shield_info(BasicStatus* status)
//{
//	Proto80400387 respond;
//	respond.set_total_blood(status->__view1);
//	respond.set_cur_blood(status->__value1);
//	for (MoverMap::iterator iter = this->player_map_.begin(); iter != this->player_map_.end(); ++iter)
//	{
//		MapPlayerEx* player = this->find_player(iter->first);
//		JUDGE_CONTINUE(player != NULL);
//
//		player->respond_to_client(ACTIVE_SEND_LBOSS_SHIELD_INFO, &respond);
//	}
//
//	return 0;
//}

void LeagueBoss::run_boss_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	this->exp_interval_ = CONFIG_INSTANCE->league_boss("player_exp_tick").asInt();
	this->boss_timer_.schedule_timer(1);
}

void LeagueBoss::refresh_scene()
{
	JUDGE_RETURN(this->refresh_flag_ == true, ;);

	this->recycle_all_monster();
	this->reset_scene_info();
	this->player_exp_map_.clear();
	this->refresh_flag_ = false;
}

MoverCoord LeagueBoss::fetch_enter_pos()
{
	const Json::Value& enter_conf = CONFIG_INSTANCE->league_boss("enter_pos");
	MoverCoord enter_pos;
	enter_pos.set_pixel(enter_conf[0u].asInt(), enter_conf[1u].asInt());

	return enter_pos;
}

int LeagueBoss::fetch_flag_level(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400454*, request, -1);

	this->lboss_item_.flag_level_ = request->flag_lvl();
	return 0;
}

void LeagueBoss::summon_league_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400451*, request, ;);

	this->reset_scene_info();

	this->lboss_item_.id_ = request->boss_index();
	this->lboss_item_.summon_type_ = request->summon_type();
	this->lboss_item_.league_index_	= request->league_index();

	this->generate_boss();
}

int LeagueBoss::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	LBossItem &lboss_item = this->lboss_item_;
	JUDGE_RETURN(game_ai->ai_id() == lboss_item.index_, -1);

	MapPlayerEx* player = this->find_player(benefited_attackor);
	JUDGE_RETURN(player != NULL,-1);

	if (this->player_hurt_map_.count(benefited_attackor) == 0)
	{
		PlayerHurtInfo &player_hurt = this->player_hurt_map_[benefited_attackor];
		player_hurt.__player_id = benefited_attackor;
		player_hurt.__amount_hurt = hurt_value;
		player_hurt.__player_name = player->role_name();
		player_hurt.__tick = ::time(NULL);
	}
	else
	{
		PlayerHurtInfo &player_hurt = this->player_hurt_map_[benefited_attackor];
		player_hurt.__amount_hurt += hurt_value;
		player_hurt.__tick = ::time(NULL);
	}
	this->rank_flag_ = true;
	this->generate_shield(game_ai);

	return 0;
}

int LeagueBoss::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	LBossItem &lboss_item = this->lboss_item_;
	if (game_ai->ai_id() == lboss_item.index_)
	{
		this->league_boss_finish(game_ai);
	}
	else
	{
		this->erase_ai(game_ai->ai_id());
	}

	return 0;
}

int LeagueBoss::league_boss_finish(GameAI* game_ai)
{
	JUDGE_RETURN(this->lboss_item_.status_ == 1, -1);

	// 爆宝箱
	const Json::Value& boss_info = CONFIG_INSTANCE->boss_info(this->lboss_item_.id_);
	if (boss_info != Json::Value::null)
	{
		for (int i = 0; i < boss_info["drop_num"].asInt(); i++)
		{

			MoverCoord coordxy = this->rand_coord(game_ai->location(), boss_info["radius"].asInt(), game_ai);
			Int64 chest_id = AIMANAGER->generate_monster_by_sort(boss_info["drop_award"].asInt(), coordxy, this);
			JUDGE_RETURN(chest_id > 0, -1);

			this->chest_map_[chest_id] = true;
		}
	}

	//发送直接奖励
	for (MoverMap::iterator iter = this->player_map_.begin(); iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->request_add_reward(this->lboss_item_.direct_award_, SerialObj(ADD_FROM_LEAGUE_BOSS_DIRECT));
	}

	this->lboss_item_.status_ = 0;

	Proto30100240 finish_info;
	finish_info.set_league_index(this->lboss_item_.league_index_);
	finish_info.set_summon_type(this->lboss_item_.summon_type_);
	finish_info.set_boss_index(this->lboss_item_.id_);
	MAP_MONITOR->dispatch_to_logic(&finish_info);

	return 0;
}

void LeagueBoss::erase_ai(Int64 ai_id)
{
	this->chest_map_.erase(ai_id);
}

void LeagueBoss::enter_player(Int64 role_id)
{
	// 通知新进的玩家boss信息
//	JUDGE_RETURN(this->player_hurt_map_.size() > 0 || this->lboss_item_.status_ >= 1, ;);

	this->rank_flag_ = true;
	this->sort_rank();
}

int LeagueBoss::fetch_lboss_status()
{
	return this->lboss_item_.status_;
}

void LeagueBoss::notify_add_player_exp()
{
	JUDGE_RETURN(LEAGUE_MONITOR->is_boss_time() == true, ;);

	for (MoverMap::iterator iter = this->player_map_.begin(); iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		int level = player->level();
		int add_exp = CONFIG_INSTANCE->role_level(0, level)["blessing_exp"].asInt();

		uint flag_lv = this->lboss_item_.flag_level_;
		double flag_add_wish = 0;
		const Json::Value &flag_info = CONFIG_INSTANCE->league_flag(flag_lv);
		if (flag_info != Json::Value::null)
		{
			flag_add_wish = flag_info["flag_add_wish"].asInt();
		}
		add_exp = add_exp * (1 + flag_add_wish / 10000);
		player->modify_element_experience(add_exp, SerialObj(EXP_FROM_LEAGUE_BOSS));

		this->player_exp_map_[iter->first] += add_exp;
	}
	this->exp_tick_ = ::time(NULL) + this->exp_interval_;
}

void LeagueBoss::send_exp_and_refresh_tick()
{
	JUDGE_RETURN(LEAGUE_MONITOR->is_boss_time() == true, ;);

	Proto80400388 respond;
	respond.set_refresh_tick(LEAGUE_MONITOR->activity_left_tick());

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		int add_exp = this->player_exp_map_[iter->first];
		respond.set_add_exp(add_exp);

		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(ACTIVE_LBOSS_REFRESH_TICK, &respond);
	}
}

void LeagueBoss::sort_rank()
{
	JUDGE_RETURN(this->rank_flag_ == true, ;);

	this->rank_flag_ = false;
	this->player_hurt_rank_.clear();

	for (PlayerHurtMap::iterator iter = this->player_hurt_map_.begin();
			iter != this->player_hurt_map_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = iter->second;

		ThreeObj obj;
		obj.id_ 	= iter->first;
		obj.tick_ 	= player_hurt.__tick;
		obj.value_ 	= player_hurt.__amount_hurt;

		this->player_hurt_rank_.push_back(obj);
	}

	Proto80400384 respond;
	respond.set_boss_id(this->lboss_item_.index_);
	respond.set_status(this->lboss_item_.status_);
	GameAI *game_ai = this->find_ai(this->lboss_item_.index_);
	if (game_ai == NULL || this->lboss_item_.status_ == 0)
	{
		respond.set_cur_blood(0);
	}
	else
	{
		respond.set_cur_blood(game_ai->fight_detail().blood_percent(game_ai, 100));
	}

	std::sort(this->player_hurt_rank_.begin(), this->player_hurt_rank_.end(),
			GameCommon::three_comp_by_desc);

	int rank = 1;
	for (ThreeObjVec::iterator iter = this->player_hurt_rank_.begin();
		iter != this->player_hurt_rank_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = this->player_hurt_map_[iter->id_];
		player_hurt.__rank = rank;

		ProtoLScoreInfo* rank_item = respond.add_role_set();
		rank_item->set_role_rank(rank);
		rank_item->set_role_name(player_hurt.__player_name);

		int role_hurt = (player_hurt.__amount_hurt / this->lboss_item_.total_blood_) * 100;
		rank_item->set_role_hurt(role_hurt);

		rank += 1;
	}

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		if (this->player_hurt_map_.count(iter->first) > 0)
		{
			PlayerHurtInfo& player_hurt = this->player_hurt_map_[iter->first];
			ProtoLScoreInfo* my_rank = respond.mutable_my_rank();
			my_rank->set_role_rank(player_hurt.__rank);
			my_rank->set_role_name(player_hurt.__player_name);
			int role_hurt = (player_hurt.__amount_hurt / this->lboss_item_.total_blood_) * 100;
			my_rank->set_role_hurt(role_hurt);
		}
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(ACTIVE_LEAGUE_BOSS_SCORE, &respond);
	}
}

