/*
 * LeagueRegionFightScene.cpp
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#include "LeagueRegionFightScene.h"
#include "LeagueReginFightSystem.h"
#include "MapPlayer.h"
#include "AIManager.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"

LRFPlayer::LRFPlayer()
{
	LRFPlayer::reset();
}

void LRFPlayer::reset()
{
	this->playerid = 0;
	this->league_id = 0;

	this->flag_reward_ = 0;
	this->history_reward_.clear();

	this->camp_id_ = 0;
	this->fight_score = 0;
	this->resource_score = 0;

	this->hickty_index_ = 0;
	this->hickty_id_ = 0;
	this->attack_ = 0;
	this->defence_ = 0;
	this->health_ = 0;
	this->force_hurt_ = 0;
}

void LRFPlayer::set_hickty_info(const Json::Value& weapon_conf)
{
	this->hickty_index_ = weapon_conf["id"].asInt();
	this->hickty_id_ 	= weapon_conf["hickty_id"].asInt();
	this->attack_ 		= weapon_conf["attack"].asInt();
	this->defence_ 		= weapon_conf["defense"].asInt();
	this->health_ 		= weapon_conf["health"].asInt();
	this->force_hurt_ 	= weapon_conf["force_hurt"].asInt();
}

RegionWarMonster::RegionWarMonster()
{
	this->sort_ = 0;
	this->conf_index_ = -1;
	this->owner_camp_ = 0;
	this->position_index = 0;

	this->index = 0;
	this->kill_player_point_ = 0;
	this->kill_league_point_ = 0;
	this->kill_player_res_score = 0;
	this->attack_player_point = 0;
	this->attack_league_point = 0;
}

LeagueRegionFightScene::RegionWarTimer::RegionWarTimer()
{
	this->state_ = 0;
	this->scene_ = NULL;
}

int LeagueRegionFightScene::RegionWarTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

void LeagueRegionFightScene::RegionWarTimer::set_script_scene(LeagueRegionFightScene *scene)
{
	this->scene_ = scene;
}

int LeagueRegionFightScene::RegionWarTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(this->scene_ != NULL, -1);

	switch (this->state_)
	{
	case 1:
	{
		//生成资源怪
		this->scene_->generate_res(5, GameEnum::CAMP_NONE);
		this->scene_->generate_res(6, GameEnum::CAMP_NONE);
		this->scene_->add_lrf_info_by_flag();
		this->scene_->notify_lrf_fight_info();
		break;
	}
	case 2:
	{
		//回收
		this->scene_->handle_recycle_region();
		break;
	}
	}
	return 0;
}

void LeagueRegionFightScene::reset()
{
	Scene::reset();

	this->win_ = NULL;
	this->lose_ = NULL;
	this->first_info_ = NULL;
	this->second_info_ = NULL;

	this->real_scene = 0;
	this->first_rank_ = 0;
	this->ten_second_ = 0;
	this->second_rank_ = 0;
	this->total_tower_ = 0;
	this->finish_flag_ = 0;
	this->init_resource_ = 0;
	this->center_monster_ = 0;
	this->get_flag_reward_ = 0;
	this->kill_player_score_[0] = 0;
	this->kill_player_score_[1] = 0;

	this->score_reward_.clear();
	this->lrf_player_map_.clear();
	this->tower_monster_map_.clear();
	this->region_monster_map_.clear();
	this->res_monster_map_.clear();
	this->exchange_point_map_.clear();

	this->region_timer.cancel_timer();
}

void LeagueRegionFightScene::init_region_scene(int real_scene, LRFLeagueInfo& first, LRFLeagueInfo& second)
{
	this->real_scene = real_scene;
	this->first_info_ = &first;
	this->second_info_ = &second;

	int space_id = first.space_;
	this->first_rank_ = space_id * 2 - 1;
	this->second_rank_ = space_id * 2;
	this->init_scene(space_id, this->real_scene);

	const Json::Value& conf = this->conf();
	this->kill_player_score_[0] = conf["kill_player_score"][0u].asInt();
	this->kill_player_score_[1] = conf["kill_player_score"][1u].asInt();
	this->get_flag_reward_ 		= conf["get_flag_reward"][0u].asInt();
	this->init_resource_ 		= conf["init_resource"].asInt();

	int total_size = conf["personal_reward"].size();
	for (int i = 0; i < total_size; ++i)
	{
		PairObj obj;
		obj.id_ = conf["personal_reward"][i][0u].asInt();
		obj.value_ = conf["personal_reward"][i][1u].asInt();
		this->score_reward_.push_back(obj);
	}

	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, this->real_scene, this);

	this->init_flags(0, GameEnum::CAMP_NONE);
	this->init_flags(1, GameEnum::CAMP_ONE);
	this->init_flags(2, GameEnum::CAMP_TWO);

	this->generate_tower(3, GameEnum::CAMP_ONE);
	this->generate_tower(4, GameEnum::CAMP_TWO);

	//生成资源怪
	this->generate_res(5, GameEnum::CAMP_NONE);
	this->generate_res(6, GameEnum::CAMP_NONE);

	this->region_timer.set_script_scene(this);
	this->region_timer.state_ = 1;
	this->region_timer.schedule_timer(1);

	//生成资源兑换点
	//this->generate_res_exchange(7);
	//this->generate_res_exchange(8);
}

void LeagueRegionFightScene::init_flags(int type, int camp_id)
{
	Int64 monster_id = AIMANAGER->generate_ai(type, this);
	JUDGE_RETURN(monster_id > 0, ;);

	GameAI* game_ai = this->find_ai(monster_id);
	JUDGE_RETURN(game_ai != NULL, ;);

	RegionWarMonster region_monster;
	region_monster.index = monster_id;
	region_monster.sort_ = game_ai->ai_sort();
	region_monster.conf_index_ = type;
	region_monster.owner_camp_ = camp_id;

	const Json::Value& layout_info = this->layout_index(type);
	if (layout_info.isMember("kill_center_flag_add"))
	{
		const Json::Value& kill_add = layout_info["kill_center_flag_add"];
		region_monster.kill_league_point_ = kill_add[0u].asInt();
		region_monster.kill_player_point_ = kill_add[1u].asInt();
	}

	if (layout_info.isMember("kill_left_flag_add"))
	{
		const Json::Value& kill_add = layout_info["kill_left_flag_add"];
		region_monster.kill_league_point_ = kill_add[0u].asInt();
		region_monster.kill_player_point_ = kill_add[1u].asInt();
	}

	if (layout_info.isMember("kill_right_flag_add"))
	{
		const Json::Value& kill_add = layout_info["kill_right_flag_add"];
		region_monster.kill_league_point_ = kill_add[0u].asInt();
		region_monster.kill_player_point_ = kill_add[1u].asInt();
	}

	if (layout_info.isMember("flag_per_ten_sec_add"))
	{
		GameCommon::json_to_t_vec(region_monster.flag_per_ten_add_,
				layout_info["flag_per_ten_sec_add"]);
	}

	game_ai->set_camp_id(camp_id);
	game_ai->ai_detail().league_index_ = camp_id;

	if (type == 0)
	{
		this->center_monster_ = region_monster.index;
	}
	else if (type == 1)
	{
		this->first_info_->flag_monster_ = region_monster.index;

		int cur_blood = LRF_MONITOR->fetch_league_flag_blood(
				this->first_info_->flag_lvl_);
		game_ai->fight_detail().__blood_max.set_single(cur_blood);
		game_ai->modify_blood_by_levelup(cur_blood);
	}
	else
	{
		this->second_info_->flag_monster_ = region_monster.index;

		int cur_blood = LRF_MONITOR->fetch_league_flag_blood(
				this->second_info_->flag_lvl_);
		game_ai->fight_detail().__blood_max.set_single(cur_blood);
		game_ai->modify_blood_by_levelup(cur_blood);
	}

	this->region_monster_map_[region_monster.index] = region_monster;
	MSG_USER("LRF flags camp %d", camp_id);
}

void LeagueRegionFightScene::generate_tower(int type, int camp_id)
{
	const Json::Value& layout_info = this->layout_index(type);
	const Json::Value& point_json = layout_info["born_coordxy"];

	this->total_tower_ = point_json.size();
	for (int i = 0; i < this->total_tower_; ++i)
	{
		IntPair monster_index(type, layout_info["monster_sort"][i].asInt());

		MoverCoord coordxy;
		coordxy.set_pixel(point_json[i][0u].asInt(), point_json[i][1u].asInt());

		GameAI* game_ai = AIMANAGER->generate_monster_by_scene(monster_index, coordxy, this);
		JUDGE_CONTINUE(game_ai != NULL);

		RegionWarMonster region_monster;
		region_monster.index = game_ai->ai_id();
		region_monster.sort_ = game_ai->ai_sort();
		region_monster.conf_index_ = type;
		region_monster.position_index = i;
		region_monster.owner_camp_ = camp_id;

		if (layout_info.isMember("kill_attack_tower"))
		{
			const Json::Value& kill_add = layout_info["kill_attack_tower"];
			region_monster.kill_league_point_ = kill_add[0u].asInt();
			region_monster.kill_player_point_ = kill_add[1u].asInt();
		}

		if (layout_info.isMember("attack_tower"))
		{
			const Json::Value& kill_add = layout_info["attack_tower"];
			region_monster.attack_league_point = kill_add[0u].asInt();
			region_monster.attack_player_point = kill_add[1u].asInt();
		}

		game_ai->set_camp_id(camp_id);
		game_ai->ai_detail().league_index_ = camp_id;
		this->tower_monster_map_[region_monster.index] = region_monster;
	}

	MSG_USER("LRF tower camp %d", camp_id);
}

void LeagueRegionFightScene::generate_res(int type, int camp_id)
{
	const Json::Value& layout_info = this->layout_index(type);
	const Json::Value& point_json = layout_info["born_coordxy"];

	int current_num = 0;
	for (WarMonsterMap::iterator iter = this->res_monster_map_.begin();
			iter != this->res_monster_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.conf_index_ == type);
		current_num += 1;
	}

	int diff = layout_info["max_exist"].asInt() - current_num;
	JUDGE_RETURN(diff > 0, ;);

	IntPair monster_index(type, layout_info["monster_sort"].asInt());
	for (int i = 0; i < diff; ++i)
	{
		int index = ::rand() % point_json.size();

		MoverCoord coordxy;
		coordxy.set_pixel(point_json[index][0u].asInt(), point_json[index][1u].asInt());

		GameAI* game_ai = AIMANAGER->generate_monster_by_scene(monster_index, coordxy, this);
		JUDGE_CONTINUE(game_ai != NULL);

		RegionWarMonster region_monster;
		region_monster.index = game_ai->ai_id();
		region_monster.sort_ = game_ai->ai_sort();
		region_monster.conf_index_ =  type;
		region_monster.owner_camp_ = camp_id;

		const Json::Value& kill_add = layout_info["kill_defend_monster"];
		region_monster.kill_league_point_ = kill_add[0u].asInt();
		region_monster.kill_player_point_ = kill_add[1u].asInt();
		region_monster.kill_player_res_score = kill_add[2u].asInt();

		game_ai->set_camp_id(camp_id);
		game_ai->ai_detail().league_index_ = camp_id;
		this->res_monster_map_[region_monster.index] = region_monster;
	}
}

void LeagueRegionFightScene::generate_res_exchange(int type)
{
//	const Json::Value layout_info = CONFIG_INSTANCE->scene(CONFIG_INSTANCE->league_regionfight("activity_id").asInt());
//	const Json::Value& npc_json = layout_info["npc"][(type+1)%2];
//
//	BirthRecord birth_record;
//	int monster_sort = npc_json["monster_sort"].asInt();
//	birth_record.reset();
//	birth_record.__birth_coord.set_pixel(npc_json["posX"].asInt(),
//			npc_json["posY"].asInt());
//
//	MoverCoord coordxy = birth_record.__birth_coord;
//	Int64 monster_id = AIMANAGER->generate_monster_by_sort(monster_sort, coordxy, this);
//	RegionWarMonster region_monster;
//	region_monster.index = monster_id;
//	region_monster.sort_ = monster_sort;
//	region_monster.conf_index_ = type;
//	region_monster.loaction_ = coordxy;
//
//	region_monster.owner_camp_ = rank_vecs[(type+1)%2];
//
//	GameAI* game_ai = AI_PACKAGE->find_object(region_monster.index);
//	JUDGE_RETURN(game_ai != NULL, ;);
//
//	if( type == 7 )
//		game_ai->set_camp_id(rank_vecs[1]);
//	else
//		game_ai->set_camp_id(rank_vecs[0]);
//
//	game_ai->ai_detail().league_index_ = type;
//	this->exchange_point_map_[rank_vecs[(type+1)%2]] = region_monster;
}

void LeagueRegionFightScene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);
	this->start_monster_ = true;
}

void LeagueRegionFightScene::stop_region_war()
{
	this->check_handle_region_timeout();
}

int LeagueRegionFightScene::is_fighting_time()
{
	return this->finish_flag_ == false;
}

int LeagueRegionFightScene::is_validate_attack_player(GameFighter* attacker, GameFighter* target)
{
	JUDGE_RETURN(attacker->is_lrf_change_mode() == true, true);
	return target->is_lrf_change_mode();
}

int LeagueRegionFightScene::is_validate_attack_monster(GameFighter* attacker, GameFighter* target)
{
	Int64 target_id = target->mover_id();

	if (this->center_monster_ == target_id)
	{
		return true;
	}
	else if (this->tower_monster_map_.count(target_id) > 0)
	{
		RegionWarMonster& monster = this->tower_monster_map_[target_id];
		if (monster.owner_camp_ == GameEnum::CAMP_ONE)
		{
			JUDGE_RETURN(monster.position_index / 2 == this->first_info_->killed_tower_ / 2, false);
			return true;
		}
		else
		{
			JUDGE_RETURN(monster.position_index / 2 == this->second_info_->killed_tower_ / 2, false);
			return true;
		}
	}
	else if (this->region_monster_map_.count(target_id) > 0)
	{
		RegionWarMonster& monster = this->region_monster_map_[target_id];
		if (monster.owner_camp_ == GameEnum::CAMP_ONE)
		{
			JUDGE_RETURN(this->first_info_->killed_tower_ >= this->total_tower_, false);
			return true;
		}
		else if (monster.owner_camp_ == GameEnum::CAMP_TWO)
		{
			JUDGE_RETURN(this->second_info_->killed_tower_ >= this->total_tower_, false);
			return true;
		}
	}

	return true;
}

int LeagueRegionFightScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	this->handle_league_tower_die(game_ai, benefited_attackor);
	this->handle_league_res_die(game_ai, benefited_attackor);
	this->handle_league_flag_die(game_ai, benefited_attackor);
	return 0;
}

int LeagueRegionFightScene::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	JUDGE_RETURN(this->tower_monster_map_.count(game_ai->ai_id()) > 0, -1);
	JUDGE_RETURN(this->lrf_player_map_.count(benefited_attackor) > 0, -1);

	RegionWarMonster& monster = this->tower_monster_map_[game_ai->ai_id()];
	LRFPlayer& lrf_player = this->lrf_player_map_[benefited_attackor];

	this->add_lrf_player_score(lrf_player, monster.attack_player_point);
	this->add_lrf_league_score(lrf_player.league_id, monster.attack_league_point);

	return 0;
}

int LeagueRegionFightScene::modify_ai_hurt_value(GameAI* game_ai, int src_value, Int64 attackor_id)
{
	MapPlayerEx* player = this->find_player(attackor_id);
	JUDGE_RETURN(player != NULL, src_value);
	JUDGE_RETURN(player->is_lrf_change_mode() == true, src_value);

	JUDGE_RETURN(this->lrf_player_map_.count(attackor_id) > 0, src_value);
	LRFPlayer& lrf_player = this->lrf_player_map_[attackor_id];

	return LeagueRegionFightScene::caculate_lrf_hurt(lrf_player.attack_, game_ai->defence_in_hurt());
}

void LeagueRegionFightScene::handle_league_flag_die(GameAI* game_ai, Int64 benefited_attackor)
{
	JUDGE_RETURN(this->region_monster_map_.count(game_ai->ai_id()) > 0, ;);
	JUDGE_RETURN(this->lrf_player_map_.count(benefited_attackor) > 0, ;);

	RegionWarMonster& monster = this->region_monster_map_[game_ai->ai_id()];
	if (monster.conf_index_ == 0)   //复活点被击杀，此时需要切换复活点的阵营
	{
		LRFPlayer& lrf_player = this->lrf_player_map_[benefited_attackor];
		this->add_lrf_player_score(lrf_player, monster.kill_player_point_);
		this->add_lrf_league_score(lrf_player.league_id, monster.kill_league_point_);

		monster.owner_camp_ = this->fetch_camp_id(lrf_player.league_id);
		this->shout_lrf_center(monster.owner_camp_);
		this->restore_center_flag_blood(monster.owner_camp_);
	}
	else if (monster.conf_index_ == 1)
	{
		this->handle_add_all_leaguer(monster, benefited_attackor);
		this->handle_region_finish(this->second_info_->id_, false);
		this->shout_lrf_win_by_flag(this->second_info_->id_);
		LRF_MONITOR->check_and_handle_all_scene();
	}
	else if (monster.conf_index_ == 2)
	{
		this->handle_add_all_leaguer(monster, benefited_attackor);
		this->handle_region_finish(this->first_info_->id_, false);
		this->shout_lrf_win_by_flag(this->first_info_->id_);
		LRF_MONITOR->check_and_handle_all_scene();
	}
}

void LeagueRegionFightScene::handle_league_tower_die(GameAI* game_ai, Int64 benefited_attackor)
{
	JUDGE_RETURN(this->tower_monster_map_.count(game_ai->ai_id()) > 0, ;);

	RegionWarMonster& monster = this->tower_monster_map_[game_ai->ai_id()];
	this->handle_add_all_leaguer(monster, benefited_attackor);

	if (monster.owner_camp_ == GameEnum::CAMP_ONE)
	{
		this->first_info_->killed_tower_ += 1;
		this->update_player_hickty_level(this->second_info_->id_);
		this->shout_lrf_tower(GameEnum::CAMP_ONE, this->first_info_->killed_tower_);
	}
	else
	{
		this->second_info_->killed_tower_ += 1;
		this->update_player_hickty_level(this->first_info_->id_);
		this->shout_lrf_tower(GameEnum::CAMP_TWO, this->second_info_->killed_tower_);
	}

	this->tower_monster_map_.erase(game_ai->ai_id());
}

void LeagueRegionFightScene::handle_league_res_die(GameAI* game_ai, Int64 benefited_attackor)
{
	JUDGE_RETURN(this->res_monster_map_.count(game_ai->ai_id()) > 0, ;);
	JUDGE_RETURN(this->lrf_player_map_.count(benefited_attackor) > 0, ;);

	LRFPlayer& lrf_player = this->lrf_player_map_[benefited_attackor];
	RegionWarMonster& monster = this->res_monster_map_[game_ai->ai_id()];
	lrf_player.resource_score += monster.kill_player_res_score;

	this->add_lrf_player_score(lrf_player, monster.kill_player_point_);
	this->add_lrf_league_score(lrf_player.league_id, monster.kill_league_point_);
	this->res_monster_map_.erase(monster.index);
}

void LeagueRegionFightScene::check_handle_region_timeout()
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	if (this->first_info_->fight_score_ >= this->second_info_->fight_score_)
	{
		this->handle_region_finish(this->first_info_->id_, true);
	}
	else
	{
		this->handle_region_finish(this->second_info_->id_, true);
	}
}

void LeagueRegionFightScene::handle_region_finish(Int64 win_league, int timeout)
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	this->finish_flag_ = true;
	this->set_lrf_result_info(win_league);
	this->notify_lrf_fight_info();

	this->handle_region_flag_reward(win_league, timeout);
	this->notify_lrf_finish_info(win_league, timeout);
	LRF_MONITOR->set_result_info(this->win_, this->lose_);

	this->recycle_all_monster();
	this->region_timer.state_ = 2;
	this->region_timer.cancel_timer();

	int quit_time = this->conf()["quit_time"].asInt();
	this->region_timer.schedule_timer(quit_time);
}

void LeagueRegionFightScene::handle_region_flag_reward(Int64 win_league, int timeout)
{
	JUDGE_RETURN(timeout == false, ;);

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);

		JUDGE_CONTINUE(player != NULL);
		JUDGE_CONTINUE(player->league_id() == win_league);

		LRFPlayer& lrf_player = this->lrf_player_map_[iter->first];
		lrf_player.flag_reward_ = this->get_flag_reward_;

		player->request_add_reward(this->get_flag_reward_, ADD_FROM_LRF_FLAG);
	}
}

void LeagueRegionFightScene::handle_recycle_region()
{
	this->notify_all_player_exit();
	this->region_timer.cancel_timer();

	MAP_MONITOR->unbind_scene(this->space_id(),
			GameEnum::LEAGUE_REGION_FIGHT_ID);
	LRF_MONITOR->recycle_league_region(this);
}

void LeagueRegionFightScene::handle_add_all_leaguer(const RegionWarMonster& monster, Int64 role)
{
	JUDGE_RETURN(this->lrf_player_map_.count(role) > 0, ;);

	LRFPlayer& player = this->lrf_player_map_[role];
	for (LRFPlayerMap::iterator iter = this->lrf_player_map_.begin();
			iter != this->lrf_player_map_.end(); ++iter)
	{
		LRFPlayer& lrf_player = iter->second;
		JUDGE_CONTINUE(lrf_player.league_id == player.league_id);
		JUDGE_CONTINUE(this->find_player(iter->first) != NULL);

		this->add_lrf_player_score(lrf_player, monster.kill_player_point_);
		this->add_lrf_league_score(lrf_player.league_id, monster.kill_league_point_);
	}
}

void LeagueRegionFightScene::update_player_hickty_level(Int64 league_id)
{
	int level = this->fetch_lrf_weapon_level(league_id);
	for (LRFPlayerMap::iterator iter = this->lrf_player_map_.begin();
			iter != this->lrf_player_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.league_id == league_id);

		const Json::Value& weapon_conf = CONFIG_INSTANCE->lrf_weapon(
				iter->second.hickty_id_, level);
		JUDGE_CONTINUE(weapon_conf.empty() == false);

		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->update_hickty_info(weapon_conf);
	}
}

void LeagueRegionFightScene::shout_lrf_win_by_flag(Int64 win_league)
{
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_int(para_vec, this->win_->rank_);
	GameCommon::push_brocast_para_int(para_vec, this->lose_->rank_);
	GameCommon::push_brocast_para_string(para_vec, this->win_->name_);
	GameCommon::push_brocast_para_string(para_vec, this->lose_->name_);

	string city_name = LRF_MONITOR->fetch_city_name(this->first_rank_);
	GameCommon::push_brocast_para_string(para_vec, city_name);
	GameCommon::push_brocast_para_int(para_vec, this->first_rank_);

	int shout_flag = this->conf()["brocast_get_flag"].asInt();
	GameCommon::announce(this->scene_id(), shout_flag, &para_vec);
}

void LeagueRegionFightScene::shout_lrf_tower(int die_camp, int amount)
{
	LRFLeagueInfo* kill = NULL;
	LRFLeagueInfo* die = NULL;
	BrocastParaVec para_vec;

	if (die_camp == GameEnum::CAMP_TWO)
	{
		kill = this->first_info_;
		die = this->second_info_;
	}
	else
	{
		kill = this->second_info_;
		die = this->first_info_;
	}

	GameCommon::push_brocast_para_int(para_vec, this->first_rank_);
	GameCommon::push_brocast_para_int(para_vec, this->second_rank_);

	GameCommon::push_brocast_para_string(para_vec, kill->name_);
	GameCommon::push_brocast_para_string(para_vec, die->name_);
	GameCommon::push_brocast_para_int(para_vec, die->killed_tower_);
	GameCommon::push_brocast_para_string(para_vec, kill->name_);

	int shout_flag = this->conf()["brocast_destroy_tower"].asInt();
	GameCommon::announce(this->scene_id(), shout_flag, &para_vec);
}

void LeagueRegionFightScene::shout_lrf_center(int win_camp)
{
	LRFLeagueInfo* win = NULL;
	LRFLeagueInfo* lose = NULL;
	BrocastParaVec para_vec;

	if (win_camp == GameEnum::CAMP_ONE)
	{
		win = this->first_info_;
		lose = this->second_info_;
	}
	else
	{
		win = this->second_info_;
		lose = this->first_info_;
	}

	GameCommon::push_brocast_para_int(para_vec, this->first_rank_);
	GameCommon::push_brocast_para_int(para_vec, this->second_rank_);
	GameCommon::push_brocast_para_string(para_vec, win->name_);
	GameCommon::push_brocast_para_string(para_vec, lose->name_);

	int shout_flag = this->conf()["brocast_center_flag"].asInt();
	GameCommon::announce(this->scene_id(), shout_flag, &para_vec);
}

//击杀玩家
void LeagueRegionFightScene::handle_player_kill(Int64 attacked_id, Int64 be_killed_id)
{
	JUDGE_RETURN(this->lrf_player_map_.count(attacked_id) > 0, ;);

	LRFPlayer& lrf_player = this->lrf_player_map_[attacked_id];
	this->add_lrf_player_score(lrf_player, this->kill_player_score_[0]);
	this->add_lrf_league_score(lrf_player.league_id, this->kill_player_score_[1]);
}

void LeagueRegionFightScene::add_lrf_player_score(LRFPlayer& lrf_player, int score)
{
	JUDGE_RETURN(score > 0, ;);

	lrf_player.fight_score += score;
	this->check_reward_lrf_player(lrf_player);
}

void LeagueRegionFightScene::add_lrf_league_score(Int64 league_id, int score)
{
	JUDGE_RETURN(score > 0, ;);

	LRFLeagueInfo* lrf_league = this->fetch_lrf_league(league_id);
	lrf_league->fight_score_ += score;
}

void LeagueRegionFightScene::check_reward_lrf_player(LRFPlayer& lrf_player)
{
	MapPlayerEx* player = this->find_player(lrf_player.playerid);
	JUDGE_RETURN(player != NULL, ;);

	for (PairObjVec::iterator iter = this->score_reward_.begin();
			iter != this->score_reward_.end(); ++iter)
	{
		JUDGE_BREAK(lrf_player.fight_score >= iter->id_);

		int reward_id = iter->value_;
		JUDGE_CONTINUE(lrf_player.history_reward_.count(reward_id) == 0);

		lrf_player.history_reward_[reward_id] = true;
		player->request_add_reward(reward_id, ADD_FROM_LRF_SCORE);
	}
}

void LeagueRegionFightScene::restore_center_flag_blood(int camp)
{
	GameAI* game_ai = this->find_ai(this->center_monster_);
	JUDGE_RETURN(game_ai != NULL, ;);

	game_ai->set_camp_id(camp);
	game_ai->ai_detail().league_index_ = camp;

	game_ai->fighter_restore_all();
	game_ai->notify_fight_update(FIGHT_UPDATE_CAMP, camp);
}

void LeagueRegionFightScene::enter_player(MapPlayer* player)
{
	JUDGE_RETURN(this->lrf_player_map_.count(player->role_id()) == 0, ;);

	LRFPlayer& lrf_player = this->lrf_player_map_[player->role_id()];
	lrf_player.league_id = player->league_id();
	lrf_player.playerid = player->role_id();
	lrf_player.camp_id_ = player->camp_id();
	lrf_player.resource_score += this->init_resource_;
}

void LeagueRegionFightScene::exit_player()
{
}

void LeagueRegionFightScene::add_lrf_info_by_flag()
{
	JUDGE_RETURN(this->finish_flag_ == false, ;);

	this->ten_second_ += 1;
	JUDGE_RETURN(this->ten_second_ >= 10, ;);

	for (WarMonsterMap::iterator iter = this->region_monster_map_.begin();
			iter != this->region_monster_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.conf_index_ > 0);
		this->add_lrf_info_by_flag(iter->second);
	}

	this->ten_second_ = 0;
}

void LeagueRegionFightScene::add_lrf_info_by_flag(const RegionWarMonster& monster)
{
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->lrf_player_map_.count(iter->first) > 0);

		LRFPlayer& lrf_player = this->lrf_player_map_[iter->first];
		JUDGE_CONTINUE(lrf_player.camp_id_ == monster.owner_camp_);

		this->add_lrf_league_score(lrf_player.league_id, monster.flag_per_ten_add_[0]);
		this->add_lrf_player_score(lrf_player, monster.flag_per_ten_add_[1]);
		lrf_player.resource_score += monster.flag_per_ten_add_[2];
	}
}

void LeagueRegionFightScene::notify_lrf_fight_info()
{
	int left_time = LRF_MONITOR->left_activity_time();
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		this->notify_lrf_fight_info(iter->second, left_time);
	}
}

void LeagueRegionFightScene::notify_lrf_fight_info(GameMover* mover, int left_time)
{
	JUDGE_RETURN(mover != NULL, ;);
	JUDGE_RETURN(this->lrf_player_map_.count(mover->mover_id()) > 0, ;);

	LRFLeagueInfo* self = NULL;
	LRFLeagueInfo* enemy = NULL;

	LRFPlayer& lrf_player = this->lrf_player_map_[mover->mover_id()];
	if (lrf_player.league_id == this->first_info_->id_)
	{
		self = this->first_info_;
		enemy = this->second_info_;
	}
	else
	{
		self = this->second_info_;
		enemy = this->first_info_;
	}

	Proto80404006 fight_info;
	fight_info.set_left_time(left_time);
	fight_info.set_camp_id(self->camp_);
	fight_info.set_player_score(lrf_player.fight_score);
	fight_info.set_my_res_score(lrf_player.resource_score);

	IntPair reward = this->fetch_fight_score_reward(lrf_player);
	fight_info.set_reward_id(reward.first);
	fight_info.set_arrive_score(reward.second);

	int hicktylevel = this->fetch_lrf_weapon_level(enemy->id_);
	fight_info.set_crt_hickty_level(hicktylevel);

	fight_info.set_my_league(self->name_);
	fight_info.set_league_score(self->fight_score_);
	fight_info.set_enemy_league(enemy->name_);
	fight_info.set_enemy_league_score(enemy->fight_score_);

	IntPair center_pair = this->fetch_lrf_center_flag(self);
	fight_info.set_center_relive_belongto(center_pair.second);
	fight_info.set_center_relive_crt_blood(center_pair.first);

	IntPair self_pair = this->fetch_lrf_league_flag(self);
	IntPair enemy_pair = this->fetch_lrf_league_flag(enemy);
	fight_info.add_flags_crt_blood(self_pair.first);
	fight_info.add_flags_crt_blood(enemy_pair.first);
	fight_info.add_tower_surplus_num(self_pair.second);
	fight_info.add_tower_surplus_num(enemy_pair.second);

	mover->respond_to_client(ACTIVE_REGION_FIGHT_INFO, &fight_info);
}

//结束通知客户端
void LeagueRegionFightScene::notify_lrf_finish_info(Int64 win_league, int timeout)
{
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->lrf_player_map_.count(iter->first) > 0);

		MapPlayer* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		LRFPlayer& lrf_player = this->lrf_player_map_[iter->first];
		int state = lrf_player.league_id == win_league;

		Proto80404007 finish_info;
		finish_info.set_kill_flag(!timeout);
		finish_info.set_fight_score(lrf_player.fight_score);
		finish_info.set_flag_reward(lrf_player.flag_reward_);

		if (state == true)
		{
			finish_info.set_state(true);
			finish_info.set_rank(this->first_rank_);
		}
		else
		{
			finish_info.set_state(false);
			finish_info.set_rank(this->second_rank_);
		}

		for (IntMap::reverse_iterator iter = lrf_player.history_reward_.rbegin();
				iter != lrf_player.history_reward_.rend(); ++iter)
		{
			finish_info.add_item(iter->first);
		}

		player->respond_to_client(ACTIVE_REGION_FINISH_INFO, &finish_info);
	}
}

void LeagueRegionFightScene::set_lrf_result_info(Int64 win_league)
{
	if (this->first_info_->id_ == win_league)
	{
		this->win_ = this->first_info_;
		this->lose_ = this->second_info_;
	}
	else
	{
		this->win_ = this->second_info_;
		this->lose_ = this->first_info_;
	}

	this->win_->result_ = 1;
	this->lose_->result_ = 2;

	this->set_lrf_result_reward(this->win_);
	this->set_lrf_result_reward(this->lose_);
}

void LeagueRegionFightScene::set_lrf_result_reward(LRFLeagueInfo* league)
{
	JUDGE_RETURN(league != NULL, ;);
	JUDGE_RETURN(league->id_ > 0, ;);

	int rank = 0;
	if (league->result_ == 1)
	{
		rank = this->first_rank_;
	}
	else
	{
		rank = this->second_rank_;
	}

	int reward_id = CONFIG_INSTANCE->league_region(rank)["result_reward"].asInt();
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(this->lrf_player_map_.count(iter->first) > 0);

		LRFPlayer& lrf_player = this->lrf_player_map_[iter->first];
		JUDGE_CONTINUE(lrf_player.league_id == league->id_);

		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		lrf_player.history_reward_[reward_id] = true;
		player->request_add_reward(reward_id, ADD_FROM_LRF_FINISH);
	}
}

LRFPlayer* LeagueRegionFightScene::fetch_lrf_player(Int64 id)
{
	JUDGE_RETURN(this->lrf_player_map_.count(id) > 0, NULL);
	return &this->lrf_player_map_[id];
}

LRFLeagueInfo* LeagueRegionFightScene::fetch_lrf_league(Int64 id)
{
	if (this->first_info_->id_ == id)
	{
		return this->first_info_;
	}
	else
	{
		return this->second_info_;
	}
}

int LeagueRegionFightScene::fetch_camp_id(Int64 league_id)
{
	if (this->first_info_->id_ == league_id)
	{
		return this->first_info_->camp_;
	}
	else
	{
		return this->second_info_->camp_;
	}
}

int LeagueRegionFightScene::fetch_lrf_weapon_level(Int64 league_id)
{
	if (this->first_info_->id_ == league_id)
	{
		return this->first_info_->killed_tower_ + 1;
	}
	else
	{
		return this->second_info_->killed_tower_ + 1;
	}
}

IntPair LeagueRegionFightScene::fetch_lrf_center_flag(LRFLeagueInfo* lrf_league)
{
	IntPair pair;

	GameAI* game_ai = this->find_ai(this->center_monster_);
	JUDGE_RETURN(game_ai != NULL, pair);

	if (game_ai->camp_id() == 0)
	{
		pair.second = 0;
	}
	else if (lrf_league->camp_ == game_ai->camp_id())
	{
		pair.second = 2;
	}
	else
	{
		pair.second = 1;
	}

	pair.first = game_ai->cur_blood_percent(1000);
	return pair;
}

IntPair LeagueRegionFightScene::fetch_lrf_league_flag(LRFLeagueInfo* lrf_league)
{
	IntPair pair;

	GameAI* game_ai = this->find_ai(lrf_league->flag_monster_);
	JUDGE_RETURN(game_ai != NULL, pair);

	pair.first = game_ai->cur_blood_percent(1000);
	pair.second = this->total_tower_ - lrf_league->killed_tower_;
	return pair;
}

IntPair LeagueRegionFightScene::fetch_fight_score_reward(LRFPlayer& lrf_player)
{
	IntPair pair;
	for (PairObjVec::iterator iter = this->score_reward_.begin();
			iter != this->score_reward_.end(); ++iter)
	{
		JUDGE_CONTINUE(lrf_player.fight_score < iter->id_);

		pair.first = iter->value_;
		pair.second = iter->id_;

		return pair;
	}

	int total_size = this->score_reward_.size();
	if (total_size > 0)
	{
		pair.first = this->score_reward_[total_size - 1].value_;
		pair.second = this->score_reward_[total_size - 1].id_;
	}

	return pair;
}

int LeagueRegionFightScene::is_lrf_finish()
{
	return this->finish_flag_ == true;
}

int LeagueRegionFightScene::is_in_center_relive(int camp_id)
{
	JUDGE_RETURN(camp_id > 0, false);
	JUDGE_RETURN(this->region_monster_map_.count(this->center_monster_) > 0, false);

	RegionWarMonster& monster = this->region_monster_map_[this->center_monster_];
	return monster.owner_camp_ == camp_id;
}

int LeagueRegionFightScene::validate_sub_resource(Int64 role, int need_resource)
{
	JUDGE_RETURN(this->lrf_player_map_.count(role) > 0, false);

	LRFPlayer& lrf_player = this->lrf_player_map_[role];
	JUDGE_RETURN(lrf_player.resource_score >= need_resource, false);

	lrf_player.resource_score -= need_resource;
	return true;
}

int LeagueRegionFightScene::lrf_modify_player_blood(LRFPlayer* self, GameFighter* player)
{
	if (player->is_lrf_change_mode() == true)
	{
		LRFPlayer* lrf_player = this->fetch_lrf_player(player->mover_id());
		JUDGE_RETURN(lrf_player != NULL, self->force_hurt_);
		return LeagueRegionFightScene::caculate_lrf_hurt(lrf_player->attack_, self->defence_);
	}
	else
	{
		return self->force_hurt_;
	}
}

int LeagueRegionFightScene::caculate_lrf_hurt(int attack, int defence)
{
	if (attack > defence)
	{
		return attack - defence;
	}
	else
	{
		return std::max<int>(attack / 100, 1);
	}
}
