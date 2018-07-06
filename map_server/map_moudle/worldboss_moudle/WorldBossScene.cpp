/*
 * WorldBossScene.cpp
 *
 *  Created on: 2016年9月29日
 *      Author: lyw
 */

#include "WorldBossScene.h"
#include "WorldBossSystem.h"
#include "ProtoDefine.h"
#include "MapPlayerEx.h"
#include "AIManager.h"
#include "MapMonitor.h"
#include "TrvlWbossMonitor.h"

#define LAST_POCKET_TIME 60

WorldBossScene::BossItem::BossItem()
{
	this->sort_ = 0;
	this->index_ = 0;
	this->level_ = 0;
	this->status_ = 0;
	this->die_tick_ = 0;
	this->killer_ 	= 0;
	this->kill_award_ = 0;
	this->send_pocket_ = 0;
	this->total_blood_ = 0;
	this->cur_blood_ = 0;
	this->killer_name_.clear();
	this->name_.clear();
	this->loaction_.reset();
	this->buff_on_.clear();
}

WorldBossScene::PlayerHurtInfo::PlayerHurtInfo()
{
	this->__amount_hurt = 0;
	this->__player_id = 0;
	this->__league_index = 0;
	this->__tick = 0;
	this->__rank = 0;
	this->__player_name.clear();
}

WorldBossScene::PocketPlayer::PocketPlayer()
{
	this->player_id_ = 0;
	this->tick_ = 0;
	this->is_get_ = 0;
	this->get_num_ = 0;
	this->get_sort_ = 0;
}

WorldBossScene::DicePlayer::DicePlayer()
{
	this->player_id_ = 0;
	this->tick_ = 0;
	this->num_ = 0;
	this->name_.clear();
}

WorldBossScene::BloodLine::BloodLine()
{
	this->blood_ 	= 0;
	this->is_shout_ = 0;
	this->shout_id_ = 0;
}

WorldBossScene::WBossTimer::WBossTimer(void)
{
	this->scene_ = NULL;
}

WorldBossScene::WBossTimer::~WBossTimer(void)
{ /*NULL*/ }

void WorldBossScene::WBossTimer::set_script_scene(WorldBossScene *scene)
{
    this->scene_ = scene;
}

int WorldBossScene::WBossTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int WorldBossScene::WBossTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

//    Int64 cur_time = ::time(NULL);
//    if (cur_time >= this->scene_->refresh_tick_)
//    {
//    	if (this->scene_->check_can_generate() == true)
//    	{
//    		this->scene_->generate_boss();
//    	}
//    }

    this->scene_->send_dice_award();
    this->scene_->sort_rank();

    return 0;
}

WorldBossScene::DiceTimer::DiceTimer(void)
{
	this->scene_ = NULL;
}

WorldBossScene::DiceTimer::~DiceTimer(void)
{ /*NULL*/ }

void WorldBossScene::DiceTimer::set_script_scene(WorldBossScene *scene)
{
    this->scene_ = scene;
}

int WorldBossScene::DiceTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int WorldBossScene::DiceTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

    this->scene_->notify_play_dice();
    return 0;
}

WorldBossScene::BossTimer::BossTimer(void)
{
	this->scene_ = NULL;
}

WorldBossScene::BossTimer::~BossTimer(void)
{ /*NULL*/ }

void WorldBossScene::BossTimer::set_script_scene(WorldBossScene *scene)
{
    this->scene_ = scene;
}

int WorldBossScene::BossTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int WorldBossScene::BossTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene_ != NULL, -1);

    this->scene_->check_can_generate();
    return 0;
}

WorldBossScene::WorldBossScene() {
	// TODO Auto-generated constructor stub
	this->wboss_timer_.set_script_scene(this);
	this->dice_timer_.set_script_scene(this);
	this->boss_timer_.set_script_scene(this);
}

WorldBossScene::~WorldBossScene() {
	// TODO Auto-generated destructor stub
}

void WorldBossScene::reset()
{
	Scene::reset();

	this->rank_flag_ = 0;
	this->pocket_time_ = 0;
	this->dice_time_ = 0;
	this->cur_player_.clear();
	this->league_name_.clear();

	this->blood_map_.clear();
	this->dice_map_.clear();

	this->wboss_timer_.cancel_timer();
	this->dice_timer_.cancel_timer();
	this->boss_timer_.cancel_timer();

	this->player_hurt_rank_.clear();
	this->player_hurt_map_.clear();
	this->pocket_player_map_.clear();
	this->server_mail_map_.clear();
}

void WorldBossScene::init_wboss_scene(int scene_id, int space_id)
{
	this->init_scene(space_id, scene_id);

	this->start_scene();
	MAP_MONITOR->bind_scene(space_id, scene_id, this);

//	const Json::Value& blood_shout = CONFIG_INSTANCE->world_boss("blood_shout");
	const Json::Value& blood_shout = this->boss_conf_value("blood_shout");
	for (uint i = 0; i < blood_shout.size(); ++i)
	{
		double blood = blood_shout[i][0u].asInt();
		int shout_id = blood_shout[i][1u].asInt();

		BloodLine &blood_line = this->blood_map_[blood];
		blood_line.blood_ = blood;
		blood_line.shout_id_ = shout_id;
		blood_line.is_shout_ = false;
	}
}

void WorldBossScene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	this->wboss_timer_.schedule_timer(1);
}

void WorldBossScene::enter_player(Int64 role_id, int boss_type)
{
	//更新世界boss成就
	MapPlayerEx* player = this->find_player(role_id);
	if (player != NULL && boss_type == GameEnum::ET_WORLD_BOSS)
	{
		int refresh_tick = WORLD_BOSS_SYSTEM->fetch_refresh_tick();
		player->notify_ML_to_update_achievement(GameEnum::WORLD_BOSS, 1, refresh_tick);
	}

	this->update_cur_player(role_id);

	// 通知新进的玩家boss信息
	this->rank_flag_ = true;
	this->sort_rank();
}

void WorldBossScene::exit_player(Int64 role_id)
{
	this->cur_player_.erase(role_id);
}

void WorldBossScene::update_cur_player(Int64 role_id)
{
	JUDGE_RETURN(this->cur_player_.count(role_id) <= 0, ;);
	this->cur_player_[role_id] = true;
}

bool WorldBossScene::is_player_full()
{
	const Json::Value& scene_conf = this->conf();
	int max_player = scene_conf["max_player"].asInt();
	return int(this->cur_player_.size()) >= max_player;
}

int WorldBossScene::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	BossItem &boss_item = this->boss_item_;
	JUDGE_RETURN(game_ai->ai_id() == boss_item.index_, -1);

	MapPlayerEx* player = this->find_player(benefited_attackor);
	JUDGE_RETURN(player != NULL,-1);

	if (this->player_hurt_map_.count(benefited_attackor) == 0)
	{
		PlayerHurtInfo &player_hurt = this->player_hurt_map_[benefited_attackor];
		player_hurt.__player_id = benefited_attackor;
		player_hurt.__league_index = player->league_id();
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

	// 血量播报
	double cur_blood = game_ai->fight_detail().blood_percent(game_ai, 1);
	boss_item.cur_blood_ = cur_blood;

	int shout_id = 0;
	for (BloodLineMap::iterator iter = this->blood_map_.begin();
			iter != this->blood_map_.end(); ++iter)
	{
		BloodLine &blood_line = iter->second;
		JUDGE_CONTINUE(blood_line.is_shout_ == false);
		JUDGE_CONTINUE(cur_blood * 100 <= blood_line.blood_);

		shout_id = blood_line.shout_id_;
		blood_line.is_shout_ = true;
	}
	if (shout_id > 0)
	{
		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, game_ai->ai_detail().__name);
		GameCommon::push_brocast_para_int(para_vec, game_ai->fight_detail().__level);

		if (GameCommon::is_world_boss_scene(this->scene_id()))
		{
			GameCommon::announce(shout_id, &para_vec);
		}
		else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
		{
			GameCommon::trvl_announce(shout_id, &para_vec);
		}
	}

	if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		TRVL_WBOSS_MONITOR->set_boss_status(this->scene_id(), boss_item.status_, boss_item.killer_,
				boss_item.killer_name_, boss_item.cur_blood_);
	}

	return 0;
}

int WorldBossScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	BossItem &boss_item = this->boss_item_;
	JUDGE_RETURN(game_ai->ai_id() == boss_item.index_, -1);

	MapPlayerEx* player = this->find_player(benefited_attackor);
	JUDGE_RETURN(player != NULL,-1);

	const Json::Value& scene_conf = this->conf();
	int mail_id = scene_conf["kill_mail_id"].asInt();
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), boss_item.name_.c_str());

	RewardInfo reward_info = mail_info->add_goods(boss_item.kill_award_);

	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		GameCommon::request_save_mail_content(benefited_attackor, mail_info);
	}
	else
	{
		int sid = TRVL_WBOSS_MONITOR->fetch_server_sid(benefited_attackor);
		if (sid >= 0)
		{
			Proto30400517 inner_proto;
			inner_proto.set_type(1);
			inner_proto.set_mail_id(mail_id);
			inner_proto.set_boss_name(boss_item.name_);

			ProtoTrvlWbossMail *mail_info = inner_proto.add_mail_info();
			mail_info->set_role_id(benefited_attackor);
			mail_info->set_reward_id(boss_item.kill_award_);
			MAP_MONITOR->dispatch_to_logic(sid, &inner_proto);

			MSG_USER("WorldBoss kill reward mail %ld %s %d", benefited_attackor, player->name(), sid);
		}
		else
		{
			MSG_USER("WorldBoss kill reward no gate %ld %s %d", benefited_attackor, player->name(), sid);
		}
	}

	// 伤害排行榜奖励
	this->send_rank_award();

	this->boss_item_.status_ = 0;
	this->boss_item_.killer_ = benefited_attackor;
	this->boss_item_.killer_name_ = player->name();
	this->boss_item_.die_tick_ = ::time(NULL);
	WORLD_BOSS_SYSTEM->set_boss_status(this->scene_id(), this->boss_item_.status_);

	// 击杀播报
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, player->name());
	GameCommon::push_brocast_para_string(para_vec, game_ai->ai_detail().__name);
	GameCommon::push_brocast_para_int(para_vec, game_ai->fight_detail().__level);
	GameCommon::push_brocast_para_reward(para_vec, &reward_info);

//	int shout_id = CONFIG_INSTANCE->world_boss("kill_shout").asInt();
	int shout_id = this->boss_conf_value("kill_shout").asInt();

	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		GameCommon::announce(shout_id, &para_vec);
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		GameCommon::trvl_announce(shout_id, &para_vec);
	}

	return 0;
}

void WorldBossScene::generate_shield(GameAI* game_ai)
{
	IntMap& buff_on = this->boss_item_.buff_on_;
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
		const Json::Value& conf = this->conf();
		const Json::Value& layout_json = conf["layout"][0u];
		const Json::Value& add_buff = layout_json["add_buff"];
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

//int WorldBossScene::notify_shield_info(BasicStatus* status)
//{
//	Proto80401024 respond;
//	respond.set_scene_id(this->scene_id());
//	respond.set_total_blood(status->__view1);
//	respond.set_cur_blood(status->__value1);
//	for (MoverMap::iterator iter = this->player_map_.begin(); iter != this->player_map_.end(); ++iter)
//	{
//		MapPlayerEx* player = this->find_player(iter->first);
//		JUDGE_CONTINUE(player != NULL);
//
//		player->respond_to_client(NOTIFY_CREATE_BOSS_SHIELD, &respond);
//	}
//
//	return 0;
//}

void WorldBossScene::notify_dice_announce()
{
	GameAI *game_ai = this->find_ai(this->boss_item_.index_);
	JUDGE_RETURN(game_ai != NULL, ;);

	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, game_ai->ai_detail().__name);
	GameCommon::push_brocast_para_int(para_vec, game_ai->level());
//	int shout_id = CONFIG_INSTANCE->world_boss("dice_shout").asInt();
	int shout_id = this->boss_conf_value("dice_shout").asInt();

	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		GameCommon::announce(shout_id, &para_vec);
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		GameCommon::trvl_announce(shout_id, &para_vec);
	}
}

void WorldBossScene::notify_play_dice()
{
	const Json::Value& conf = this->conf();
	int dice_time = conf["dice_time"].asInt();
	Int64 cur_time = ::time(NULL);
	int pass_time = cur_time - this->dice_time_;
	if (pass_time <= dice_time)
	{
		if (this->dice_timer_.is_registered() == false)
		{
			this->dice_timer_.schedule_timer(1);
		}
		return ;
	}

	this->dice_map_.clear();

	Proto80401023 respond;
	respond.set_scene_id(this->scene_id());
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		player->respond_to_client(ACTIVE_CREATE_DICE_PLAY, &respond);
	}

	this->dice_time_ = ::time(NULL);
	this->dice_timer_.cancel_timer();
}

int WorldBossScene::get_dice_num(Int64 role_id, Proto50401024* respond)
{
	JUDGE_RETURN(this->dice_map_.count(role_id) == 0, ERROR_HAS_PLAY_DICE);

	MapPlayerEx* player = this->find_player(role_id);
	JUDGE_RETURN(player != NULL,ERROR_PLAYER_OFFLINE);

	const Json::Value& conf = this->conf();
	int dice_time = conf["dice_time"].asInt();

	Int64 cur_time = ::time(NULL);
	int pass_time = cur_time - this->dice_time_;
	JUDGE_RETURN(pass_time <= dice_time, ERROR_TIME_IS_OVER);

	// 保证投到的数字不一致
	while(true)
	{
		int get_num = std::rand()%98 + 1;
		int get_flag = true;
		for (DicePlayerMap::iterator iter = this->dice_map_.begin();
				iter != this->dice_map_.end(); ++iter)
		{
			DicePlayer &dice_player = iter->second;
			if (dice_player.num_ == get_num)
				get_flag = false;
		}
		if (get_flag == true || this->dice_map_.size() >= 99)
		{
			respond->set_my_num(get_num);

			DicePlayer &dice_player = this->dice_map_[role_id];
			dice_player.player_id_ = role_id;
			dice_player.num_ = get_num;
			dice_player.tick_ = ::time(NULL);
			dice_player.name_ = player->role_name();
			break;
		}
	}

	int max_num = 0;
	Int64 max_role_id = 0;
	std::string max_role_name;
	for (DicePlayerMap::iterator iter = this->dice_map_.begin();
			iter != this->dice_map_.end(); ++iter)
	{
		DicePlayer &dice_player = iter->second;
		if (max_num < dice_player.num_)
		{
			max_num = dice_player.num_;
			max_role_id = dice_player.player_id_;
			max_role_name = dice_player.name_;

			if (max_num == 99)
				break;
		}
	}

	// 主推所有玩家最大点数
	Proto80401025 active_res;
	active_res.set_max_num(max_num);
	active_res.set_max_num_role(max_role_id);
	active_res.set_max_num_name(max_role_name);
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		MapPlayerEx* all_player = this->find_player(iter->first);
		JUDGE_CONTINUE(all_player != NULL);

		all_player->respond_to_client(ACTIVE_SEND_MAX_DICE_NUM, &active_res);
	}

	return 0;
}

void WorldBossScene::send_dice_award()
{
	JUDGE_RETURN(this->dice_map_.size() > 0, ;);

	const Json::Value& conf = this->conf();
	int dice_time = conf["dice_time"].asInt();
	Int64 cur_time = ::time(NULL);
	JUDGE_RETURN(cur_time >= this->dice_time_+dice_time, ;);

	int max_num = 0;
	Int64 max_role_id = 0;
	for (DicePlayerMap::iterator iter = this->dice_map_.begin();
			iter != this->dice_map_.end(); ++iter)
	{
		DicePlayer &dice_player = iter->second;
		if (max_num < dice_player.num_)
		{
			max_num = dice_player.num_;
			max_role_id = dice_player.player_id_;

			if (max_num == 99)
				break;
		}
	}

	int dice_award = conf["dice_award"].asInt();
	MapPlayerEx* player = this->find_player(max_role_id);
	if (player != NULL)
	{
		//online
		player->request_add_reward(dice_award, SerialObj(ADD_FROM_WBOSS_DICE_MAX));
	}

	this->dice_map_.clear();
}

int WorldBossScene::get_my_rank_info(Int64 role_id, Proto50401025* respond)
{
	JUDGE_RETURN(this->player_hurt_map_.count(role_id) > 0, -1);

	PlayerHurtInfo& player_hurt = this->player_hurt_map_[role_id];

	ProtoWorldBossRank* self_rank = respond->mutable_self_rank();
	self_rank->set_role_id(role_id);
	self_rank->set_role_name(player_hurt.__player_name);
	self_rank->set_rank(player_hurt.__rank);
	double hurt_precent = (double)player_hurt.__amount_hurt / this->boss_item_.total_blood_;
	self_rank->set_score(hurt_precent);

	return 0;
}

void WorldBossScene::check_can_generate()
{
	const Json::Value& conf = this->conf();
	int dice_time = conf["dice_time"].asInt();

	Int64 cur_time = ::time(NULL);
	int pass_time = cur_time - this->dice_time_;
	if (pass_time <= dice_time && this->boss_timer_.is_registered() == false)
	{
		this->boss_timer_.schedule_timer(1);
	}
	else
	{
		this->generate_boss();
	}
}

void WorldBossScene::generate_boss()
{
	{
		Int64 boss_id = this->boss_item_.index_;
		GameAI *game_ai = this->find_ai(boss_id);
		if (game_ai != NULL)
		{
			this->recycle_one_mover(boss_id);
		}
	}

	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf != Json::Value::null, ;);

	const Json::Value& layout_json = conf["layout"][0u];
	const Json::Value& point_coordxy = layout_json["center_coordxy"];
	int monster_sort = layout_json["monster_sort"].asInt();

	MoverCoord coordxy;
	coordxy.set_pixel(point_coordxy[0u].asInt(), point_coordxy[1u].asInt());

	IntPair monser_index(0, monster_sort);
	GameAI* game_ai = AIMANAGER->generate_monster_by_scene(monser_index, coordxy, this);
	JUDGE_RETURN(game_ai != NULL, ;);

	BossItem &boss_item = this->boss_item_;
	boss_item.sort_ = monster_sort;
	boss_item.index_ = game_ai->ai_id();
	boss_item.status_ = 1;
	boss_item.send_pocket_ = false;
	boss_item.level_ = game_ai->fight_detail().__level;
	boss_item.die_tick_ = 0;
	boss_item.killer_ = 0;
	boss_item.loaction_ = coordxy;
	boss_item.kill_award_ = conf["final_awards"].asInt();
	boss_item.name_ = game_ai->ai_detail().__name;
	boss_item.total_blood_ = game_ai->fight_detail().__blood_total_i(game_ai);
	boss_item.cur_blood_ = game_ai->fight_detail().blood_percent(game_ai, 1);

	if (layout_json.isMember("add_buff"))
	{
		const Json::Value& add_buff = layout_json["add_buff"];
		for (uint i = 0; i < add_buff.size(); ++i)
		{
			int buff_status = add_buff[i]["status"].asInt();
			boss_item.buff_on_[buff_status] = false;
		}
	}

	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		WORLD_BOSS_SYSTEM->set_boss_status(this->scene_id(), boss_item.status_);
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		TRVL_WBOSS_MONITOR->set_boss_status(this->scene_id(), boss_item.status_, boss_item.killer_,
				boss_item.killer_name_, boss_item.cur_blood_);
	}

	for (BloodLineMap::iterator iter = this->blood_map_.begin();
			iter != this->blood_map_.end(); ++iter)
	{
		BloodLine &blood_line = iter->second;
		blood_line.is_shout_ = false;
	}

	this->rank_flag_ = true;
	this->pocket_time_ = 0;
	this->dice_time_ = 0;
	this->league_name_.clear();
	this->player_hurt_rank_.clear();
	this->player_hurt_map_.clear();
	this->pocket_player_map_.clear();
	this->boss_timer_.cancel_timer();
}

void WorldBossScene::generate_red_pocket()
{
	JUDGE_RETURN(this->player_hurt_map_.size() > 0, ;);

	const Json::Value& conf = this->conf();
	int pocket_award_id = conf["pocket_award"].asInt();
	int pocket_base 	= conf["pocket_base"].asInt();
	JUDGE_RETURN(pocket_award_id > 0 && pocket_base > 0, ;);

	Int64 top_player = 0;
	Int64 league_index = 0;
	std::string name;
	for (PlayerHurtMap::iterator iter = this->player_hurt_map_.begin();
			iter != this->player_hurt_map_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = iter->second;
		JUDGE_CONTINUE(player_hurt.__rank == 1);

		top_player = iter->first;
		league_index = player_hurt.__league_index;
		name = player_hurt.__player_name;
		break;
	}

	if (league_index <= 0)
	{
		PocketPlayer& pocket_player = this->pocket_player_map_[top_player];
		pocket_player.player_id_ = top_player;
		pocket_player.is_get_ = 0;
	}
	else
	{
		for (MoverMap::iterator iter = this->player_map_.begin(); iter != this->player_map_.end(); ++iter)
		{
			MapPlayerEx* map_player = this->find_player(iter->first);
			JUDGE_CONTINUE(map_player != NULL);
			JUDGE_CONTINUE(map_player->league_id() == league_index);

			PocketPlayer& pocket_player = this->pocket_player_map_[iter->first];
			pocket_player.player_id_ = top_player;
			pocket_player.is_get_ = 0;
		}
	}
	int total_money = pocket_base * this->pocket_player_map_.size();
	total_money = total_money > 0 ? total_money : pocket_base;

	MapPlayerEx* player = this->find_player(top_player);
	if (league_index > 0 || player != NULL)
	{
		// 通知生成红包
		Proto80401022 respond;
		respond.set_role_name(name);
		respond.set_scene_id(this->scene_id());
		respond.set_total_money(total_money);
		respond.set_total_player(this->pocket_player_map_.size());
		MSG_USER("Proto80401022: %s", respond.Utf8DebugString().c_str());

		for (PocketPlayerMap::iterator iter = this->pocket_player_map_.begin();
				iter != this->pocket_player_map_.end(); ++iter)
		{
			MapPlayerEx* map_player = this->find_player(iter->first);
			map_player->respond_to_client(ACTIVE_CREATE_RED_POCKET, &respond);
		}
	}

	this->pocket_time_ = ::time(0);

	// 红包播报
	int shout_id = 0;
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, name);
	GameCommon::push_brocast_para_string(para_vec, this->boss_item_.name_);
	GameCommon::push_brocast_para_int(para_vec, this->boss_item_.level_);
	if (league_index > 0)
	{
		GameCommon::push_brocast_para_string(para_vec, this->league_name_);
		GameCommon::push_brocast_para_int(para_vec, total_money);
//		shout_id = CONFIG_INSTANCE->world_boss("pocket_shout").asInt();
		shout_id = this->boss_conf_value("pocket_shout").asInt();
	}
	else
	{
//		shout_id = CONFIG_INSTANCE->world_boss("pocket_shout_no_league").asInt();
		shout_id = this->boss_conf_value("pocket_shout_no_league").asInt();
	}

	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		GameCommon::announce(shout_id, &para_vec);
	}
	else if (GameCommon::is_trvl_wboss_scene(this->scene_id()))
	{
		GameCommon::trvl_announce(shout_id, &para_vec);
	}

	this->boss_item_.send_pocket_ = true;
}

int WorldBossScene::get_red_pocket_award(Int64 role_id, Proto50401023* respond)
{
	const Json::Value& conf = this->conf();
	int pocket_base = conf["pocket_base"].asInt();
	int pocket_time = conf["pocket_time"].asInt();
	if (pocket_time <= 0)
	{
		pocket_time = LAST_POCKET_TIME;
	}

	Int64 cur_time = ::time(NULL);
	int pass_time = cur_time - this->pocket_time_;
	JUDGE_RETURN(pass_time <= pocket_time, ERROR_TIME_IS_OVER);
	JUDGE_RETURN(this->pocket_player_map_.count(role_id) > 0, ERROR_NOT_SAME_LEAGUE);

	PocketPlayer& pocket_player = this->pocket_player_map_[role_id];
	JUDGE_RETURN(pocket_player.is_get_ == 0, ERROR_HAS_GET_POCKET);

	int pocket_size = 0;
	int pocket_money = 0;
	int get_money = 0;
	for (PocketPlayerMap::iterator iter = this->pocket_player_map_.begin();
			iter != this->pocket_player_map_.end(); ++iter)
	{
		PocketPlayer& player = this->pocket_player_map_[iter->first];
		if (player.is_get_ == 1)
		{
			pocket_size += 1;
			get_money += player.get_num_;
		}
	}

	if ((this->pocket_player_map_.size() - pocket_size) == 1)
	{
		pocket_money = this->pocket_player_map_.size() * pocket_base - get_money;
	}
	else if ((pocket_size + 1) % 2 == 1)
	{
		double number = std::rand() % 10 + 5;
		double rand_num = number / 10;
		pocket_money = pocket_base * rand_num;
	}
	else
	{
		int last_get = 0;
		int player_sort = 0;
		for (PocketPlayerMap::iterator iter = this->pocket_player_map_.begin();
				iter != this->pocket_player_map_.end(); ++iter)
		{
			PocketPlayer& player = this->pocket_player_map_[iter->first];
			if (player_sort < player.get_sort_)
			{
				player_sort = player.get_sort_;
				last_get = player.get_num_;
			}
		}
		pocket_money = pocket_base * 2 - last_get;
	}

	pocket_player.tick_ = ::time(NULL);
	pocket_player.is_get_ = 1;
	pocket_player.get_num_ = pocket_money;
	pocket_player.get_sort_ = pocket_size + 1;

	respond->set_get_money(pocket_money);

	return 0;
}

void WorldBossScene::sort_rank()
{
	JUDGE_RETURN(this->rank_flag_ == true, ;);

	this->rank_flag_ = false;
	this->player_hurt_rank_.clear();

	// player hurt rank
	for (PlayerHurtMap::iterator iter = this->player_hurt_map_.begin();
			iter != this->player_hurt_map_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = iter->second;

		ThreeObj obj;
		obj.id_ = player_hurt.__player_id;
		obj.tick_ = player_hurt.__tick;
		obj.value_ = player_hurt.__amount_hurt;

		this->player_hurt_rank_.push_back(obj);
	}

	// 主推排行榜信息
	Proto80401021 respond;
	respond.set_scene_id(this->scene_id());
	respond.set_boss_id(this->boss_item_.index_);
	respond.set_status(this->boss_item_.status_);
	respond.set_refresh_tick(WORLD_BOSS_SYSTEM->real_refresh_tick());

	GameAI *game_ai = this->find_ai(this->boss_item_.index_);
	if (game_ai == NULL || this->boss_item_.status_ == 0)
	{
		respond.set_left_blood(0);
	}
	else if(game_ai != NULL)
	{
		respond.set_left_blood(game_ai->fight_detail().blood_percent(game_ai, 100));
	}


	int rank = 1;
	std::sort(this->player_hurt_rank_.begin(), this->player_hurt_rank_.end(), GameCommon::three_comp_by_desc);

	for (ThreeObjVec::iterator iter = this->player_hurt_rank_.begin();
			iter != this->player_hurt_rank_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = this->player_hurt_map_[iter->id_];
		player_hurt.__rank = rank;
		++rank;

		ProtoWorldBossRank *rank_list = respond.add_rank_list();
		rank_list->set_role_id(player_hurt.__player_id);
		rank_list->set_role_name(player_hurt.__player_name);
		rank_list->set_rank(player_hurt.__rank);
		double hurt_precent = (double)player_hurt.__amount_hurt / this->boss_item_.total_blood_;
		rank_list->set_score(hurt_precent);

		JUDGE_CONTINUE(player_hurt.__rank == 1);
		MapPlayerEx* player = this->find_player(iter->id_);
		JUDGE_CONTINUE(player != NULL);

		this->league_name_ = player->role_detail().__league_name;
	}

	this->notify_all_player_msg(ACTIVE_SEND_WORLD_BOSS_RANK, &respond);

	JUDGE_RETURN(this->boss_item_.status_ == 0 && this->boss_item_.send_pocket_ == false, ;);

	// 通知抢红包
	this->generate_red_pocket();
}

void WorldBossScene::send_rank_award()
{
	JUDGE_RETURN(this->player_hurt_map_.size() > 0, ;);

	this->server_mail_map_.clear();

	BossItem &boss_item = this->boss_item_;
	const Json::Value& scene_conf = this->conf();
	const Json::Value& rank_awards = scene_conf["rank_awards"];

	int mail_id = scene_conf["rank_mail_id"].asInt();

	for (PlayerHurtMap::iterator iter = this->player_hurt_map_.begin();
			iter != this->player_hurt_map_.end(); ++iter)
	{
		PlayerHurtInfo& player_hurt = iter->second;
		int rank = player_hurt.__rank;
		int award_id = 0;
		for (uint i = 0; i < rank_awards.size(); ++i)
		{
			int low_num = rank_awards[i][0u].asInt();
			int high_num = rank_awards[i][1u].asInt();
			if (rank >= low_num && rank <= high_num)
			{
				award_id = rank_awards[i][2u].asInt();
				break;
			}
		}
		JUDGE_CONTINUE(award_id > 0);

		if (GameCommon::is_world_boss_scene(this->scene_id()))
		{
			MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), boss_item.name_.c_str(), rank);

			mail_info->add_goods(award_id);
			GameCommon::request_save_mail_content(iter->first, mail_info);
		}
		else
		{
			int sid = TRVL_WBOSS_MONITOR->fetch_server_sid(iter->first);
			if (sid <= 0)
			{
				MSG_USER("WorldBoss rank reward no gate %ld %d", iter->first, sid);
				continue;
			}

			MSG_USER("WorldBoss rank reward mail %ld %d", iter->first, sid);

			ThreeObjMap &server_player_map = this->server_mail_map_[sid];
			ThreeObj &player_obj = server_player_map[iter->first];
			player_obj.id_ = iter->first;
			player_obj.value_ = award_id;
			player_obj.sub_ = rank;
		}
	}

	JUDGE_RETURN(this->server_mail_map_.size() > 0, ;);

	for (ServerMailMap::iterator iter = this->server_mail_map_.begin();
			iter != this->server_mail_map_.end(); ++iter)
	{
		int sid = iter->first;
		ThreeObjMap &server_player_map = iter->second;

		Proto30400517 inner_proto;
		inner_proto.set_type(2);
		inner_proto.set_mail_id(mail_id);
		inner_proto.set_boss_name(boss_item.name_);

		for (ThreeObjMap::iterator it = server_player_map.begin();
				it != server_player_map.end(); ++it)
		{
			ThreeObj &player_obj = it->second;

			ProtoTrvlWbossMail *mail_info = inner_proto.add_mail_info();
			mail_info->set_role_id(player_obj.id_);
			mail_info->set_reward_id(player_obj.value_);
			mail_info->set_rank(player_obj.sub_);
		}

		MSG_USER("WorldBoss rank reward proto %d %s", sid, inner_proto.Utf8DebugString().c_str());

		MAP_MONITOR->dispatch_to_logic(sid, &inner_proto);
	}
}

void WorldBossScene::request_send_rank()
{
	this->rank_flag_ = true;
	this->sort_rank();
}

const Json::Value& WorldBossScene::boss_conf_value(const char* item_name)
{
	if (GameCommon::is_world_boss_scene(this->scene_id()))
	{
		return CONFIG_INSTANCE->world_boss(item_name);
	}
	else
	{
		return CONFIG_INSTANCE->trvl_wboss(item_name);
	}
}


