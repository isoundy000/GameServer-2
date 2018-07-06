/*
 * MonsterAttackScene.cpp
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#include "MonsterAttackScene.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "MonsterAttackSystem.h"
#include "AIManager.h"

void MonsterAttackScene::BirthRecord::reset(void)
{
	this->__birth_coord.reset();
}

MonsterAttackScene::WaveMonster::WaveMonster()
{
	this->index_ = 0;
	this->wave_id_ = 0;
	this->sort_ = 0;
	this->is_boss_ = 0;
	this->side_ = 0;
	this->loaction_.reset();

	this->kill_award_ = 0;
	this->kill_award_rate_ = 0;
	this->drop_chest_ = 0;
	this->drop_num_ = 0;
	this->drop_chest_rate_ = 0;
}

MonsterAttackScene::MAttackTimer::MAttackTimer(void)
{
	this->state_ = 0;
	this->scene_ = NULL;
}

MonsterAttackScene::MAttackTimer::~MAttackTimer(void)
{ /*NULL*/ }

void MonsterAttackScene::MAttackTimer::set_script_scene(MonsterAttackScene *scene)
{
    this->scene_ = scene;
}

int MonsterAttackScene::MAttackTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int MonsterAttackScene::MAttackTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

    switch (this->state_)
    {
    case 0:
    {
    	//进行
    	this->scene_->notify_generate_boss_skill();
    	this->scene_->clean_monster();
    	this->scene_->process_wave();
    	this->scene_->notify_mattack_finish();
    	this->scene_->notify_mattack_info();
    	this->scene_->notify_generate_buff();
    	break;
    }

    case 1:
    {
    	//结束
    	MSG_USER("monster attack Finish");

    	this->scene_->recycle_all_monster();
    	this->scene_->notify_all_player_exit();

    	MAP_MONITOR->unbind_scene(this->scene_->space_id(),
    			GameEnum::MATTACK_SCENE_ID);
    	MONSTER_ATTACK_SYSTEM->recycle_mattack(this->scene_);
    	break;
    }
    }

    return 0;
}

MonsterAttackScene::MonsterAttackScene() {
	// TODO Auto-generated constructor stub
	this->mattack_timer_.set_script_scene(this);
}

MonsterAttackScene::~MonsterAttackScene() {
	// TODO Auto-generated destructor stub
}

void MonsterAttackScene::reset(void)
{
	Scene::reset();

	this->real_scene_ = 0;
	this->cur_wave_ = 0;
	this->total_wave_ = 0;
	this->boss_id_ = 0;
	this->boss_point_.reset();
	this->interval_tick_ = 0;
	this->refresh_tick_ = 0;
	this->wave_reward_ = 0;
	this->is_boss_wave_ = 0;
	this->clean_flag_ = 0;
	this->clean_interval_ = 0;
	this->clean_tick_ = 0;

	this->mattack_timer_.state_ = 0;
	this->mattack_timer_.cancel_timer();

	this->notify_label_.clear();
	this->birth_list_.clear();
	this->birth_side_map_.clear();
	this->wave_monster_map_.clear();
}

void MonsterAttackScene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	this->mattack_timer_.schedule_timer(1);
}

int MonsterAttackScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	if (game_ai->ai_id() == this->boss_id_)
	{
		this->mattack_timer_.state_ = 1;
		this->mattack_timer_.cancel_timer();

		const Json::Value& scene_conf = this->conf();
		int quit_time = scene_conf["quit_time"].asInt();
		this->mattack_timer_.schedule_timer(quit_time);
	}
	else
	{
		WaveMonster* wave_monster = this->find_monster_by_index(game_ai->ai_id());
		JUDGE_RETURN(wave_monster != NULL, -1);

		//发送奖励
		MapPlayerEx* player = this->find_player(benefited_attackor);
		if (player != NULL && this->cal_award_rate(wave_monster->kill_award_rate_) == true)
		{
			player->request_add_reward(wave_monster->kill_award_, SerialObj(ADD_FROM_MONSTER_ATTACK_KILL));

			//boss击杀播报
			if (wave_monster->is_boss_ == true)
			{
				BrocastParaVec para_vec;
				GameCommon::push_brocast_para_string(para_vec, player->name());
				GameCommon::push_brocast_para_string(para_vec, game_ai->name());
				int shout_id = this->conf()["boss_chest_shout"].asInt();
				GameCommon::announce(this->scene_id(), shout_id, &para_vec);
			}
		}
		//生成宝箱
		if (this->cal_award_rate(wave_monster->drop_chest_rate_) == true)
			this->generate_chest(game_ai, wave_monster->drop_chest_, wave_monster->drop_num_);

		this->wave_monster_map_.erase(game_ai->ai_id());
	}

	return 0;
}

void MonsterAttackScene::stop_mattack()
{
	this->mattack_timer_.state_ = 1;
	this->mattack_timer_.cancel_timer();

	const Json::Value& scene_conf = this->conf();
	int quit_time = scene_conf["quit_time"].asInt();
	this->mattack_timer_.schedule_timer(quit_time);
	MSG_USER("MonsterAttack %d", this->space_id());
}

void MonsterAttackScene::init_mattack_scene(int real_scene, int space_id)
{
	this->cur_wave_ = 0;
	this->real_scene_ = real_scene;
	this->init_scene(space_id, this->real_scene_);

	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, this->real_scene_, this);

	const Json::Value& conf = this->conf();
	this->total_wave_ 	  	= conf["wave"].asInt();
	this->interval_tick_  	= conf["first_refresh"].asInt();
	this->clean_interval_ 	= conf["first_refresh"].asInt();
	this->clean_tick_ 	  	= ::time(NULL);
	this->refresh_tick_   	= ::time(NULL);

	this->generate_boss();
}

void MonsterAttackScene::clean_monster()
{
	Int64 cur_time = ::time(NULL);
	JUDGE_RETURN(cur_time >= this->clean_interval_ + this->clean_tick_, ;);

	for(WaveMonsterMap::iterator iter = this->wave_monster_map_.begin();
			iter != this->wave_monster_map_.end(); ++iter)
	{
		//生成宝箱
		WaveMonster &wave_monster = iter->second;
		GameAI *game_ai = this->find_ai(wave_monster.index_);
		JUDGE_CONTINUE(game_ai != NULL);

		if (this->cal_award_rate(wave_monster.drop_chest_rate_) == true)
			this->generate_chest(game_ai, wave_monster.drop_chest_, wave_monster.drop_num_);

		this->recycle_one_mover(iter->first);
	}
	this->wave_monster_map_.clear();
	this->clean_tick_ = ::time(NULL);
	this->clean_flag_ = false;
}

void MonsterAttackScene::notify_mattack_finish()
{
	JUDGE_RETURN(this->cur_wave_ >= this->total_wave_, ;);
	JUDGE_RETURN(this->wave_monster_map_.size() <= 0, ;);

	this->mattack_timer_.state_ = 1;
	this->mattack_timer_.cancel_timer();

	const Json::Value& scene_conf = this->conf();
	int quit_time = scene_conf["quit_time"].asInt();
	this->mattack_timer_.schedule_timer(quit_time);
}

void MonsterAttackScene::init_monster_layout(int wave_id)
{
	const Json::Value& conf = this->conf();
	const Json::Value& layout_json = conf["layout"];
	JUDGE_RETURN(layout_json != Json::Value::null, ;);

	int index = 1;
	uint layout_id = wave_id + 1;
	for (uint i = wave_id * 3 + 1; i <= layout_id * 3; ++i)
	{
		BirthRecordList &birth_list = this->birth_side_map_[index];
		BirthRecord birth_record;

		const Json::Value& layout_info = layout_json[i];
		const Json::Value& point_coordxy = layout_info["point_coordxy"];
		for (uint j = 0; j < point_coordxy.size(); ++j)
		{
			birth_record.reset();
			birth_record.__birth_coord.set_pixel(point_coordxy[j][0u].asInt(),
					point_coordxy[j][1u].asInt());
			birth_list.push_back(birth_record);
		}

		this->interval_tick_ = layout_info["fresh"].asInt();
		this->clean_interval_ = layout_info["fresh"].asInt();
		++index;
	}

	this->refresh_tick_ = ::time(NULL);
}

void MonsterAttackScene::generate_boss(bool shout)
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf != Json::Value::null, ;);

	const Json::Value& general_json = conf["layout"][0u];
	const Json::Value& point_coordxy = general_json["point_coordxy"];
	int monster_sort = general_json["monster_sort"].asInt();

	MoverCoord coordxy;
	coordxy.set_pixel(point_coordxy[0u].asInt(), point_coordxy[1u].asInt());

	IntPair monser_index(0, monster_sort);
	GameAI* game_ai = AIMANAGER->generate_monster_by_scene(monser_index, coordxy, this);
	JUDGE_RETURN(game_ai != NULL, ;);

	this->boss_id_ = game_ai->ai_id();
	this->boss_point_ = coordxy;

	game_ai->set_camp_id(this->fetch_player_camp() + 1);
	game_ai->ai_detail().league_index_ = this->fetch_player_camp() + 1;
	game_ai->notify_fight_update(FIGHT_UPDATE_CAMP, this->fetch_player_camp() + 1);
}

void MonsterAttackScene::process_wave()
{
	JUDGE_RETURN(this->cur_wave_ < this->total_wave_, ;);

	Int64 cur_time = ::time(NULL);
	JUDGE_RETURN(cur_time >= this->interval_tick_ + this->refresh_tick_, ;);

	this->birth_side_map_.clear();
	this->award_all_player();
	this->init_monster_layout(this->cur_wave_);
	this->is_boss_wave_ = this->fetch_is_boss_wave(this->cur_wave_ + 1);

	int is_shout = false;
	for (uint i = 1; i <= 3; ++i)
	{
		this->generate_monster(i, is_shout);
	}

	this->cur_wave_ += 1;
}

void MonsterAttackScene::award_all_player()
{
	const Json::Value& conf = this->conf();
	const Json::Value& award_wave = conf["award_wave"];
	for (uint i = 0; i < award_wave.size(); ++i)
	{
		const Json::Value& award_info = award_wave[i];
		JUDGE_CONTINUE((this->cur_wave_ + 1) == award_info[0u].asInt());

		this->wave_reward_ = award_info[1u].asInt();
		for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
		{
			MapPlayerEx* player = this->find_player(iter->first);
			if (player != NULL)
			{
				player->request_add_reward(award_info[1u].asInt(), SerialObj(ADD_FROM_MONSTER_ATTACK_WAVE_AWARD));
			}
		}
	}
}

void MonsterAttackScene::generate_monster(uint side, int& is_shout)
{
	const Json::Value& conf = this->conf();
	const Json::Value& layout_json = conf["layout"];
	JUDGE_RETURN(layout_json.empty() == false, ;);

	uint wave = this->cur_wave_ * 3 + side;
	const Json::Value& layout_info = layout_json[wave];

	int once_num 	 = layout_info["once"].asInt(),
		monster_sort = layout_info["monster_sort"].asInt();

	const Json::Value &monster_json = CONFIG_INSTANCE->monster(monster_sort);
	JUDGE_RETURN(monster_json.empty() == false, ;);

	BirthRecordList &birth_list = this->birth_side_map_[side];
	const Json::Value& point_json = layout_info["point_coordxy"];
	for (int i = 0; i < once_num; ++i)
	{
		MoverCoord coordxy;
		if (birth_list.size() <= 0)
		{
			int index = rand() % point_json.size();
			coordxy.set_pixel(point_json[index][0u].asInt(), point_json[index][1u].asInt());
		}
		else
		{
			BirthRecord &birth_record = birth_list.front();
			coordxy = birth_record.__birth_coord;
			birth_list.pop_front();
		}

		IntPair monser_index((int)wave, monster_sort);
		GameAI* game_ai = AIMANAGER->generate_monster_by_scene(monser_index, coordxy, this);
		JUDGE_CONTINUE(game_ai != NULL);

		game_ai->set_camp_id(this->fetch_monster_camp() + 1);
		game_ai->set_aim_object(this->boss_id_);

		WaveMonster &wave_monster 	  = this->wave_monster_map_[game_ai->ai_id()];
		wave_monster.index_ 		  = game_ai->ai_id();
		wave_monster.sort_ 			  = monster_sort;
		wave_monster.wave_id_ 		  = this->cur_wave_ + 1;
		wave_monster.side_ 			  = side;
		wave_monster.loaction_ 		  = coordxy;
		wave_monster.is_boss_ 		  = layout_info["is_boss"].asInt();
		wave_monster.kill_award_ 	  = layout_info["kill_award"].asInt();
		wave_monster.kill_award_rate_ = layout_info["kill_award_rate"].asInt();
		wave_monster.drop_chest_ 	  = layout_info["drop_chest"].asInt();
		wave_monster.drop_num_ 		  = layout_info["drop_num"].asInt();
		wave_monster.drop_chest_rate_ = layout_info["drop_chest_rate"].asInt();

		JUDGE_CONTINUE(is_shout == false && wave_monster.is_boss_ == true);

		int shout_id = conf["boss_appear_shout"].asInt();
		GameCommon::announce(this->scene_id(), shout_id);
		is_shout = true;
	}
}

void MonsterAttackScene::generate_chest(GameAI* game_ai, int sort, int num)
{
	JUDGE_RETURN(this->player_map_.size() > 0, ;);

	for (int i = 0; i < num; ++i)
	{
		MoverCoord coordxy = this->rand_coord(game_ai->location(), 3, game_ai);
		AIMANAGER->generate_monster_by_sort(sort, coordxy, this);
	}
}

void MonsterAttackScene::notify_generate_buff()
{
	const Json::Value& conf = this->conf();
	const Json::Value& buff_wave = conf["buff_wave"];
	for (uint i = 0; i < buff_wave.size(); ++i)
	{
		int wave = buff_wave[i][0u].asInt();
		JUDGE_CONTINUE(this->cur_wave_ >= wave);

		int notify_flag = false;
		int label_id = buff_wave[i][2u].asInt();
		if (this->notify_label_.count(label_id) <= 0)
		{
			notify_flag = true;
			this->notify_label_[label_id] = true;
		}
		Proto80401032 notify_info;
		notify_info.set_notify_id(i+1);

		int buff_id = buff_wave[i][1u].asInt();
		const Json::Value &buff_json = CONFIG_INSTANCE->buff(buff_id);
//		int buff_type = buff_json["buff_type"].asInt();
		int value = buff_json["value"].asInt();
		int percent = buff_json["percent"].asInt();
		for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
		{
			MapPlayerEx *player = 0;
			if (this->monitor()->find_player(iter->first, player) != 0)
					continue;

			BasicStatus* c_status = NULL;
			if (player->find_status(buff_id, c_status) != 0)
			{
				player->insert_defender_status(player, buff_id, 0, 86400, 0, value, percent);
			}
			JUDGE_CONTINUE(notify_flag == true);

			player->respond_to_client(NOTIFY_MATTACK_BUFF_INFO, &notify_info);
		}
		JUDGE_CONTINUE(notify_flag == true);

		// 斗志播报
		BrocastParaVec para_vec;
		int shout_id = buff_wave[i][3u].asInt();
		MAttackLabelRecord* label_record = MONSTER_ATTACK_SYSTEM->find_label_record(label_id);
		if (label_record != NULL)
			GameCommon::push_brocast_para_string(para_vec, label_record->role_name_);
		else
			GameCommon::push_brocast_para_string(para_vec, "");
		GameCommon::announce(this->scene_id(), shout_id, &para_vec);
	}
}

void MonsterAttackScene::notify_generate_boss_skill()
{
	Int64 cur_time = ::time(NULL);
	JUDGE_RETURN(cur_time >= this->clean_interval_ + this->clean_tick_ - 1, ;); //提前1s
	JUDGE_RETURN(this->wave_monster_map_.size() > 0 && this->clean_flag_ == false, ;);

	this->clean_flag_ = true;

	const Json::Value& conf = this->conf();
	Proto80401033 respond;
	respond.set_boss_id(this->boss_id_);
	respond.set_buff_id(conf["skill_id"].asInt());
	this->notify_all_player_msg(NOTIFY_MATTACK_BOSS_SKILL, &respond);
}

void MonsterAttackScene::register_player(MAttackInfo* mattack)
{
	mattack->state_ = 1;
	mattack->tick_ 	= ::time(NULL);
}

void MonsterAttackScene::unregister_player(Int64 role_id)
{

}

void MonsterAttackScene::enter_player(MAttackInfo* mattack)
{
	mattack->state_ = 2;
	mattack->tick_ = ::time(NULL);
}

void MonsterAttackScene::exit_player(MAttackInfo* mattack)
{
	mattack->state_ = 3;
	mattack->tick_ = ::time(NULL);
}

int MonsterAttackScene::fetch_player_camp()
{
	return CAMP_PLAYER;
}

int MonsterAttackScene::fetch_monster_camp()
{
	return CAMP_MONSTER;
}

int MonsterAttackScene::cal_award_rate(int rate)
{
	int rand_rate = std::rand() % 9999 + 1;
	if (rand_rate > rate)
		return false;
	else
		return true;
}

int MonsterAttackScene::fetch_is_boss_wave(int wave_id)
{
	const Json::Value& conf = this->conf();
	const Json::Value& layout_json = conf["layout"];
	JUDGE_RETURN(layout_json != Json::Value::null, 0);
	JUDGE_RETURN(this->total_wave_ > wave_id, 0);

	uint layout_id = wave_id;
	for (uint i = layout_id * 3 + 1; i <= (layout_id + 1) * 3; ++i)
	{
		const Json::Value& layout_info = layout_json[i];
		int is_boss = layout_info["is_boss"].asInt();
		JUDGE_RETURN(is_boss == false, true);
	}

	return 0;
}

MonsterAttackScene::WaveMonster* MonsterAttackScene::find_monster_by_index(Int64 index)
{
	JUDGE_RETURN(this->wave_monster_map_.count(index) > 0, NULL);
	return &this->wave_monster_map_[index];
}

void MonsterAttackScene::notify_mattack_info()
{
	Int64 cur_time = ::time(NULL);
	int left_time = MONSTER_ATTACK_SYSTEM->mattack_left_time();
	int next_attack_time = this->interval_tick_ + this->refresh_tick_ - cur_time;

	Proto80401031 respond;
	respond.set_cur_wave(this->cur_wave_);
	respond.set_total_wave(this->total_wave_);
	respond.set_left_time(left_time);
	respond.set_next_attack_time(next_attack_time);
	respond.set_is_boss(this->is_boss_wave_);

	ProtoItemId *item = respond.add_item();
	item->set_id(this->wave_reward_);
	item->set_amount(1);

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_SEND_MATTACK_INFO, &respond);
	}
}


