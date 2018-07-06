/*
 * SMBattleScene.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: lijin
 */

#include "SMBattleScene.h"
#include "SMBattleSystem.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "AIManager.h"
#include "MapMonitor.h"

SMBattleScene::TemItem::TemItem()
{
	this->sort_ = 0;			//key
	this->conf_index_ = 0;
	this->index_ = 0;
	this->owner_camp_ = 0;
	this->reward_id_ = 0;
	this->attack_point_ = 0;
	this->kill_point_ = 0;
	this->shout_refresh_ = 0;
	this->refresh_time_ = 0;
	this->loaction_.reset();
}

SMBattleScene::BattleTimer::BattleTimer(void)
{
	this->state_ = 0;
	this->scene_ = NULL;
}

SMBattleScene::BattleTimer::~BattleTimer(void)
{ /*NULL*/ }

void SMBattleScene::BattleTimer::set_script_scene(SMBattleScene *scene)
{
    this->scene_ = scene;
}

int SMBattleScene::BattleTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int SMBattleScene::BattleTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

    switch (this->state_)
    {
    case 0:
    {
    	//进行
        Int64 now_tick = ::time(NULL);
        this->scene_->add_online_score(now_tick);

        this->scene_->sort_rank();
        this->scene_->notify_all_sm_info();
        this->scene_->check_and_generage_scene_monster();
    	break;
    }

    case 1:
    {
    	//结束
    	MSG_USER("SMBattle Finish");

    	this->scene_->recycle_all_monster();
    	this->scene_->notify_all_player_exit();

    	MAP_MONITOR->unbind_scene(this->scene_->space_id(),
    			GameEnum::SUN_MOON_BATTLE_ID);
    	SM_BATTLE_SYSTEM->recycle_battle(this->scene_);
    	break;
    }
    }

    return 0;
}

SMBattleScene::SMBattleScene()
{
	// TODO Auto-generated constructor stub
	this->battle_timer_.set_script_scene(this);
}

SMBattleScene::~SMBattleScene()
{
	// TODO Auto-generated destructor stub
}

int SMBattleScene::handle_player_hurt(MapPlayer* player, Int64 benefited_attackor, int hurt_value)
{
	this->add_battler_score(benefited_attackor, this->hurt_score_);
	return 0;
}

int SMBattleScene::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	TemItem* tem_item = this->find_tem_by_sort(game_ai->ai_sort());
	JUDGE_RETURN(tem_item != NULL, -1);

	this->add_battler_score(benefited_attackor, tem_item->attack_point_);
	return 0;
}

int SMBattleScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	TemItem* tem_item = this->find_tem_by_sort(game_ai->ai_sort());
	JUDGE_RETURN(tem_item != NULL, -1);

	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(benefited_attackor);
	JUDGE_RETURN(battler != NULL, -1);

	if (tem_item->conf_index_ == 0)
	{
		this->handle_center_tem_die(tem_item, battler);
	}
	else
	{
		this->handle_camp_tem_die(tem_item, battler);
	}

	return 0;
}

void SMBattleScene::handle_center_tem_die(TemItem* tem_item, SMBattlerInfo* battler)
{
	tem_item->index_ = 0;
	this->center_time_.first = 0;

	SMCampInfo& camp_info = this->camp_info_[battler->camp_index_];
	for (LongMap::iterator iter = camp_info.enter_sm_player_.begin();
			iter != camp_info.enter_sm_player_.end(); ++iter)
	{
		this->add_battler_score(iter->first, tem_item->kill_point_);
	}
}

void SMBattleScene::handle_camp_tem_die(TemItem* tem_item, SMBattlerInfo* battler)
{
//	tem_item->owner_camp_ = battler->camp_id();

	SMCampInfo& camp_info = this->camp_info_[battler->camp_index_];
	for (LongMap::iterator iter = camp_info.enter_sm_player_.begin();
			iter != camp_info.enter_sm_player_.end(); ++iter)
	{
		this->add_battler_score(iter->first, tem_item->kill_point_);
	}

//	GameAI* game_ai = AI_PACKAGE->find_object(tem_item->index_);
//	JUDGE_RETURN(game_ai != NULL, ;);
//
//	game_ai->set_camp_id(battler->camp_id());
//	game_ai->ai_detail().league_index_ = battler->camp_id();
//
//	game_ai->fighter_restore_all();
//	game_ai->notify_fight_update(FIGHT_UPDATE_CAMP, battler->camp_id());
}

void SMBattleScene::reset(void)
{
	Scene::reset();

	this->real_scene_	= 0;
	this->actor_limit_ 	= 0;
	this->score_tick_ 	= 0;
	this->rank_flag_ 	= 0;
	this->hurt_score_	= 0;
	this->kill_score_	= 0;
	this->killed_score_ = 0;

	this->con_kill_score_ 		= 0;
	this->max_con_kill_score_ 	= 0;
	this->center_time_		= IntPair(0, 0);
	this->camp_time_		= IntPair(0, 0);

	this->rank_vec_.clear();
	this->point_reward_.clear();
	this->rank_vec_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	this->point_reward_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);

	this->battle_timer_.state_ = 0;
	this->battle_timer_.cancel_timer();

	this->tem_item_map_.clear();
	this->con_kill_shout_.clear();

	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		this->camp_info_[i].reset();
	}
}

void SMBattleScene::init_sm_scene(int real_scene, int space_id)
{
	this->real_scene_ = real_scene;
	this->init_scene(space_id, this->real_scene_);

	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, this->real_scene_, this);

	const Json::Value& conf = this->conf();
	this->actor_limit_ 			= conf["actor_limit"].asInt();
	this->score_value_ 			= conf["score_value"].asInt();
	this->score_span_time_		= conf["score_span_time"].asInt();
	this->hurt_score_ 			= conf["hurt_score"].asInt();
	this->kill_score_			= conf["kill_score"].asInt();
	this->killed_score_			= conf["killed_score"].asInt();
	this->con_kill_score_ 		= conf["con_kill_score"].asInt();
	this->max_con_kill_score_ 	= conf["max_con_kill_score"].asInt();

	const Json::Value& shout_kill = conf["shout_kill"];
	for (uint i = 0; i < shout_kill.size(); ++i)
	{
		int key = shout_kill[i][0u].asInt();
		int value = shout_kill[i][1u].asInt();
		JUDGE_CONTINUE(key > 0 && value > 0);

		this->con_kill_shout_[key] = value;
	}

	const Json::Value& tem_conf = conf["tem"];
	for (uint i = 0; i < tem_conf.size(); ++i)
	{
		int sort = tem_conf[i]["id"].asInt();

		TemItem& tem_item = this->tem_item_map_[sort];
		tem_item.sort_ 			= sort;
		tem_item.conf_index_ 	= i;
		tem_item.loaction_ 		= GameCommon::fetch_rand_conf_pos(tem_conf[i]["location"]);

		tem_item.reward_id_ 	= tem_conf[i]["reward_id"].asInt();
		tem_item.attack_point_ 	= tem_conf[i]["attack_point"].asInt();
		tem_item.kill_point_ 	= tem_conf[i]["kill_point"].asInt();
		tem_item.shout_refresh_ = tem_conf[i]["shout_refresh"].asInt();
		tem_item.refresh_time_ 	= tem_conf[i]["refresh_time"].asInt();
	}

	const Json::Value& reward_conf = conf["point_reward"];
	for (uint i = 0; i < reward_conf.size(); ++i)
	{
		PairObj obj;
		obj.id_ 	= reward_conf[i][0u].asInt();
		obj.value_  = reward_conf[i][1u].asInt();

		this->point_reward_.push_back(obj);
	}

	this->generate_center_tem();
	this->generate_camp_tem();
}

void SMBattleScene::handle_sm_kill(Int64 attackor, Int64 defener)
{
	SMBattlerInfo* attack_battler = SM_BATTLE_SYSTEM->find_battler(attackor);
	SMBattlerInfo* defence_battler = SM_BATTLE_SYSTEM->find_battler(defener);
	JUDGE_RETURN(attack_battler != NULL && defence_battler != NULL, ;);

	attack_battler->con_kill_num_ = this->find_status_value(attackor,
			BasicStatus::EVENTCUT);
	defence_battler->con_kill_num_ = this->find_status_value(defener,
			BasicStatus::EVENTCUT);

	attack_battler->total_kill_num_ += 1;
	attack_battler->max_con_kill_num_ = std::max<int>(attack_battler->con_kill_num_,
			attack_battler->max_con_kill_num_);

	int attack_score = this->kill_score_;
	attack_score += attack_battler->con_score(this->con_kill_score_,
			this->max_con_kill_score_);
	attack_score += defence_battler->con_score(this->con_kill_score_,
			this->max_con_kill_score_);

	this->add_battler_score(attackor, attack_score);
	this->shout_contine_kill(attack_battler);

	GameFighter* fighter = this->find_fighter(defener);
	if (fighter != NULL)
	{
		fighter->remove_status(BasicStatus::EVENTCUT);
	}

	//更新成就
	MapPlayerEx* player = this->find_player(attackor);
	if (player != NULL)
	{
		player->notify_ML_to_update_achievement(GameEnum::SM_BATTLE_KILL,
				attack_battler->total_kill_num_, 0, GameEnum::ACHIEVE_TYPE_1);
		player->notify_ML_to_update_achievement(GameEnum::SM_BATTLE_KILL,
				1, 0, GameEnum::ACHIEVE_TYPE_2);
	}

	this->add_battler_score(defener, this->killed_score_);
}

void SMBattleScene::notify_all_sm_info()
{
	Proto80100501 respond;
	this->make_up_other_info(&respond);

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		this->make_up_self_info(iter->first, &respond);
		iter->second->respond_to_client(ACTIVE_SM_BATTLE_SCENE_INFO, &respond);
	}
}

void SMBattleScene::shout_contine_kill(SMBattlerInfo* battler)
{
	JUDGE_RETURN(this->con_kill_shout_.count(battler->con_kill_num_) > 0, ;);

	int shout_id = this->con_kill_shout_[battler->con_kill_num_];
	JUDGE_RETURN(shout_id > 0, ;);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, battler->name_);
	GameCommon::push_brocast_para_int(para_vec, battler->con_kill_num_);
	GameCommon::announce(this->scene_id(), shout_id, &para_vec);
}

void SMBattleScene::generate_center_tem(bool shout)
{
	this->center_time_.first = -1;

	TemItem* tem_item = this->find_tem_by_conf_index(0);
	JUDGE_RETURN(tem_item != NULL, ;);

	tem_item->index_ = AIMANAGER->generate_monster_by_sort(
			tem_item->sort_, tem_item->loaction_, this);

	this->center_time_.second = tem_item->refresh_time_;
	JUDGE_RETURN(shout == true, ;);

	GameCommon::announce(this->scene_id(), tem_item->shout_refresh_);
}

void SMBattleScene::generate_camp_tem()
{
	int total_size = this->tem_item_map_.size();
	for (int i = 1; i < total_size; ++i)
	{
		TemItem* tem_item = this->find_tem_by_conf_index(i);
		JUDGE_CONTINUE(tem_item != NULL);

		this->camp_time_.second = tem_item->refresh_time_;
		tem_item->index_ = AIMANAGER->generate_monster_by_sort(
				tem_item->sort_, tem_item->loaction_, this);
	}
}

void SMBattleScene::recycle_camp_tem()
{
	int total_size = this->tem_item_map_.size();
	for (int i = 1; i < total_size; ++i)
	{
		TemItem* tem_item = this->find_tem_by_conf_index(i);
		JUDGE_CONTINUE(tem_item != NULL);

		this->recycle_one_mover(tem_item->index_);
		tem_item->index_ = 0;
	}
}

void SMBattleScene::register_player(SMBattlerInfo* battler)
{
	battler->state_ = 1;
	battler->tick_ 	= ::time(NULL);

	SMCampInfo& camp_info = this->camp_info_[battler->camp_index_];
	camp_info.sm_player_[battler->id_] = battler->force_;
}

void SMBattleScene::unregister_player(Int64 role_id)
{

}

void SMBattleScene::enter_player(SMBattlerInfo* battler)
{
	battler->state_ = 2;
	battler->tick_ = ::time(NULL);

	SMCampInfo& camp_info = this->camp_info_[battler->camp_index_];
	camp_info.enter_sm_player_[battler->id_] = battler->force_;

	this->attender_map_[battler->id_] = true;
}

void SMBattleScene::exit_player(SMBattlerInfo* battler)
{
	battler->state_ = 3;
	battler->tick_ = ::time(NULL);
}

SMBattleScene::TemItem* SMBattleScene::find_tem_by_sort(int sort)
{
	JUDGE_RETURN(this->tem_item_map_.count(sort) > 0, NULL);
	return &this->tem_item_map_[sort];
}

SMBattleScene::TemItem* SMBattleScene::find_tem_by_conf_index(int index)
{
	for (TemItemMap::iterator iter = this->tem_item_map_.begin();
			iter != this->tem_item_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.conf_index_ == index);
		return &iter->second;
	}

	return NULL;
}

int SMBattleScene::is_finish()
{
	return this->battle_timer_.state_ == 1;
}

int SMBattleScene::is_player_full()
{
	int total_player = 0;

	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		total_player += this->camp_info_[i].total_player();
	}

	return total_player >= this->actor_limit_;
}

int SMBattleScene::fetch_player_rank(Int64 role_id)
{
	if (this->rank_map_.count(role_id) > 0)
	{
		return this->rank_map_[role_id];
	}
	else if (this->rank_map_.empty() == false)
	{
		return this->rank_map_.size();
	}
	else
	{
		return 1;
	}
}

int SMBattleScene::fetch_camp_index()
{
	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		JUDGE_CONTINUE(this->camp_info_[i].total_player() == 0);
		return i;
	}

	IntPair pair;
	pair.first = 0;
	pair.second = INT_MAX;

	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		int total_force = this->camp_info_[i].total_force();
		JUDGE_CONTINUE(total_force < pair.second);

		pair.first = i;
		pair.second = total_force;
	}

	return pair.first;
}

void SMBattleScene::make_up_rank_info(Int64 role_id, Proto50400803* respond)
{
	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(role_id);
	JUDGE_RETURN(battler != NULL, ;);

	int total = this->rank_vec_.size();
	for (int i = 0; i < total; ++i)
	{
		SMBattlerInfo* rank_battler = SM_BATTLE_SYSTEM->find_battler(
				this->rank_vec_[i].id_);
		JUDGE_CONTINUE(rank_battler != NULL);

		ProtoSMBattleRankRec* proto = respond->add_rank();
		rank_battler->make_up_rank(i + 1, proto);

//		int event_cut = this->find_status_value(
//				rank_battler->id_, BasicStatus::EVENTCUT);
//		proto->set_max_kill(event_cut);
	}
}

void SMBattleScene::make_up_self_info(Int64 role_id, Proto80100501* respond)
{
	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(role_id);
	JUDGE_RETURN(battler != NULL, ;);

	respond->set_self_kill(battler->total_kill_num_);
	respond->set_self_score(battler->score_);
	respond->set_self_camp(battler->camp_id());
}

void SMBattleScene::make_up_other_info(Proto80100501* respond)
{
	respond->set_zone(this->space_id() + 1);
	respond->set_left_time(SM_BATTLE_SYSTEM->sm_battle_left_time());

	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		respond->add_camp_scores(this->camp_info_[i].score_);
	}
}

void SMBattleScene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	this->battle_timer_.schedule_timer(1);
	this->start_monster_ = true;
}

void SMBattleScene::stop_battle()
{
	this->battle_timer_.state_ = 1;
	this->battle_timer_.cancel_timer();

	this->rank_flag_ = true;
	this->sort_rank();

	Proto80100502 respond;

	//top three
	int total = std::min<int>(3, this->rank_vec_.size());
	for (int i = 0; i < total; ++i)
	{
		const ThreeObj& obj = this->rank_vec_[i];

		SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(obj.id_);
		JUDGE_CONTINUE(battler != NULL);

		ProtoSMBattleRankRec* rec = respond.add_top_three();
		battler->make_up_rank(i + 1, rec);
	}

	//reward
	const Json::Value& scene_conf = this->conf();
	int label_id = scene_conf["label_reward"].asInt();;

	for (LongMap::iterator iter = this->rank_map_.begin();
			iter != this->rank_map_.end(); ++iter)
	{
		SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(iter->first);
		JUDGE_CONTINUE(battler != NULL);

		SMCampInfo& camp_info = this->camp_info_[battler->camp_index_];
		MapPlayerEx* player = this->find_player(iter->first);

		int camp_reward = scene_conf["camp_reward"][camp_info.rank_ - 1].asInt();
		if (player != NULL)
		{
			respond.set_camp_rank(camp_info.rank_);
			respond.set_self_rank(iter->second);
			respond.set_self_kill(battler->total_kill_num_);
			respond.set_self_reward(battler->score_);
			respond.set_camp_reward(camp_reward);
			player->respond_to_client(ACTIVE_SM_BATLLE_RESULT, &respond);
		}

		{
			// mail
		    int font_id = scene_conf["mail_id"].asInt();
		    MailInformation *mail_info = GameCommon::create_sys_mail(font_id);
		    ::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
		            mail_info->mail_content_.c_str(), this->space_id() + 1, camp_info.rank_,
		            battler->score_, iter->second);

		    mail_info->add_goods(GameEnum::ITEM_ID_EXPLOIT, battler->score_);
		    mail_info->add_goods(GameEnum::ITEM_ID_EXPLOIT, camp_reward);

		    if (iter->second == 1 && this->space_id() == 0)
		    {
		    	mail_info->label_id_ = label_id;
		    }

		    GameCommon::request_save_mail_content(iter->first, mail_info);
		}
	}

	int quit_time = scene_conf["quit_time"].asInt();
	this->battle_timer_.schedule_timer(quit_time);
	MSG_USER("SMBattle %d %d", this->space_id(), this->rank_map_.size());
}

void SMBattleScene::add_online_score(Int64 now_tick)
{
	JUDGE_RETURN(this->score_tick_ < now_tick, ;);
	this->score_tick_ = now_tick + this->score_span_time_;

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		this->add_battler_score(iter->first, this->score_value_);
	}
}

void SMBattleScene::add_battler_score(Int64 role_id, int score)
{
	JUDGE_RETURN(score > 0, ;);

	SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(role_id);
	JUDGE_RETURN(battler != NULL, ;);

	battler->score_ += score;
	battler->tick_ = ::time(NULL);

	this->camp_info_[battler->camp_index_].score_ += score;
	this->camp_info_[battler->camp_index_].tick_ = ::time(NULL);

	this->check_reward_battler(battler);
	this->rank_flag_ = true;
}

void SMBattleScene::check_reward_battler(SMBattlerInfo* battler)
{
	JUDGE_RETURN(battler != NULL, ;);

	MapPlayerEx* player = this->find_player(battler->id_);
	JUDGE_RETURN(player != NULL, ;);

	for (PairObjVec::iterator iter = this->point_reward_.begin();
			iter != this->point_reward_.end(); ++iter)
	{
		JUDGE_BREAK(battler->score_ >= iter->id_);

		int reward_id = iter->value_;
		JUDGE_CONTINUE(battler->reward_map_.count(reward_id) == 0);

		battler->reward_map_[reward_id] = true;
		player->request_add_reward(reward_id, SerialObj(
				ADD_FROM_SM_BATTLE, battler->score_, reward_id));
	}
}

void SMBattleScene::check_and_generage_scene_monster()
{
	this->camp_time_.first += 1;
	if (this->camp_time_.first >= this->camp_time_.second)
	{
		this->camp_time_.first = 0;
		this->recycle_camp_tem();
		this->generate_camp_tem();
	}

	if (this->center_time_.first >= 0)
	{
		this->center_time_.first += 1;
	}

	if (this->center_time_.first >= this->center_time_.second)
	{
		this->generate_center_tem(true);
	}
}

void SMBattleScene::sort_rank()
{
	JUDGE_RETURN(this->rank_flag_ == true, ;);

	this->rank_flag_ = false;
	this->rank_vec_.clear();
	this->rank_map_.clear();

	//person rank
	for (LongMap::iterator iter = this->attender_map_.begin();
			iter != this->attender_map_.end(); ++iter)
	{
		SMBattlerInfo* battler = SM_BATTLE_SYSTEM->find_battler(iter->first);
		JUDGE_CONTINUE(battler != NULL);

		ThreeObj obj;
		obj.id_ = battler->id_;
		obj.value_ = battler->score_;
		obj.tick_ = battler->score_tick_;

		this->rank_vec_.push_back(obj);
	}

	int rank = 1;
	std::sort(this->rank_vec_.begin(), this->rank_vec_.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->rank_vec_.begin();
			iter != this->rank_vec_.end(); ++iter)
	{
		this->rank_map_[iter->id_] = rank;
		++rank;
	}

	//camp rank
	ThreeObjVec camp_rank_vec;
	for (int i = 0; i < TOTAL_CAMP; ++i)
	{
		ThreeObj obj;
		obj.id_ = i;
		obj.value_ = this->camp_info_[i].score_;
		obj.tick_ = this->camp_info_[i].tick_;

		camp_rank_vec.push_back(obj);
	}

	rank = 1;
	std::sort(camp_rank_vec.begin(), camp_rank_vec.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = camp_rank_vec.begin();
			iter != camp_rank_vec.end(); ++iter)
	{
		int index = iter->id_;
		this->camp_info_[index].rank_ = rank;
		++rank;
	}
}


