/*
 * LeagueSystem.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: peizhibi
 */

#include "LeagueSystem.h"

#include "GameFont.h"
#include "MMOLabel.h"
#include "MMOLeague.h"
#include "GameField.h"
#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "OpenActivitySys.h"
#include "PoolMonitor.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include <mongo/client/dbclient.h>
#include "TransactionMonitor.h"
#include "Transaction.h"

int LeagueSystem::StartTimer::type()
{
	return GTT_LOGIC_ONE_SEC;
}

int LeagueSystem::StartTimer::handle_timeout(const Time_Value &tv)
{
	return LEAGUE_SYSTEM->try_start_boss_scene();
}

int LeagueSystem::SortTimer::type()
{
	return GTT_LOGIC_ONE_HOUR;
}

int LeagueSystem::SortTimer::handle_timeout(const Time_Value &tv)
{
	LEAGUE_SYSTEM->sort_league();
	return 0;
}

LeagueSystem::LeagueSystem()
{
	// TODO Auto-generated constructor stub
}

LeagueSystem::~LeagueSystem()
{
	// TODO Auto-generated destructor stub
}

void LeagueSystem::start()
{
	MMOLeague::load_all_league();

	this->check_league_leader();
	this->sort_league();
	MMOLeague::load_leaguer_fb_flag(this->lfb_flag);

	this->load_league_boss_scene();
	this->start_timer_.schedule_timer(10);
	this->sort_timer_.schedule_timer(1);

	MSG_USER("league system start!");
}

void LeagueSystem::stop()
{
	this->start_timer_.cancel_timer();
	this->sort_timer_.cancel_timer();
	this->save_all_league(true);
	MMOLeague::save_leaguer_fb_flag(this->lfb_flag);

	MSG_USER("league system stop!");
}

void LeagueSystem::fetch_league_set(int joinable, LongVec& league_set)
{
	league_set.reserve(this->league_set_.size());

	if (joinable == true)
	{
		for (FourObjVec::iterator iter = this->league_set_.begin();
				iter != this->league_set_.end(); ++iter)
		{
			League* league = this->find_league(iter->id_);

			JUDGE_CONTINUE(league != NULL);
			JUDGE_CONTINUE(league->league_full() == false);

			league_set.push_back(iter->id_);
		}
	}
	else
	{
		for (FourObjVec::iterator iter = this->league_set_.begin();
				iter != this->league_set_.end(); ++iter)
		{
			league_set.push_back(iter->id_);
		}
	}
}

PoolPackage<League, Int64>* LeagueSystem::league_package()
{
	return &this->league_package_;
}

void LeagueSystem::league_check_everyday(bool force_reset)
{
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);
		JUDGE_CONTINUE(league != NULL);

		league->reset_everyday(force_reset);
//		league->check_leader_offline();
		league->caculate_league_force();
		league->__lstore.apply_his_timeout();
		league->league_boss_.reset_everyday();
		league->sort_rank();

		this->check_and_dismiss_league(league);
	}

	this->sort_league();
	this->save_all_league(false);
	MMOLeague::save_leaguer_fb_flag(this->lfb_flag,0);
	this->lfb_flag.clear();
}

int LeagueSystem::load_league_boss_scene()
{
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end(); ++iter)
	{
		League* league = this->find_league(iter->first);
		JUDGE_CONTINUE(league != NULL);

		if (league->league_boss_.normal_die_tick_ <= 0)
		{
			league->league_boss_.normal_summon_type_ = false;
			league->league_boss_.normal_summon_tick_ = 0;
		}

		if (league->league_boss_.super_die_tick_ <= 0)
		{
			league->league_boss_.super_summon_type_ = false;
			league->league_boss_.super_summon_tick_ = 0;
		}

		this->sync_league_boss_info(league);
		this->league_id_map_[iter->first] = true;
	}

	return 0;
}

int LeagueSystem::try_start_boss_scene()
{
	for (LongMap::iterator iter = this->league_id_map_.begin();
			iter != this->league_id_map_.end(); ++iter)
	{
		League* league = this->find_league(iter->first);
		JUDGE_CONTINUE(league != NULL);

		this->sync_league_boss_info(league);
	}

	return 0;
}

int LeagueSystem::check_boss_scene_start(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400450*, request, -1);

	this->league_id_map_.erase(request->league_index());
	JUDGE_RETURN(this->league_id_map_.size() <= 0, -1);

	this->start_timer_.cancel_timer();
	return 0;
}

int LeagueSystem::sync_league_boss_info(League* league)
{
	Proto30400450 boss_scene;
	boss_scene.set_league_index(league->league_index_);
	boss_scene.set_flag_lvl(league->flag_lvl_);
	boss_scene.set_name(league->league_name_);
	boss_scene.set_leader(league->leader_name());
	boss_scene.set_force(league->league_force_);
	boss_scene.set_leader_index(league->leader_index_);
	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::LBOSS_SCENE_ID, &boss_scene);
}

int LeagueSystem::sync_leader_to_map(League* league)
{
	JUDGE_RETURN(league != NULL, 0);

	Proto30400457 update_leader;
	update_leader.set_leader_index(league->leader_index_);
	update_leader.set_leader(league->leader_name());
	update_leader.set_league_index(league->league_index_);
	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::LBOSS_SCENE_ID, &update_leader);
}

void LeagueSystem::check_and_dismiss_league(League* league)
{
	JUDGE_RETURN(league != NULL, ;);
	JUDGE_RETURN(league->is_can_dismiss() == true, ;);

	this->dismiss_league(league);
}

void LeagueSystem::dismiss_league(League* league)
{
	JUDGE_RETURN(league != NULL, ;);
	league->dismiss_league();

	MMOLeague::remove_league(league->league_index_);
	this->league_package()->unbind_and_push(league->league_index_, league);

	Proto30400452 req_boss;
	req_boss.set_league_index(league->league_index_);
	LOGIC_MONITOR->dispatch_to_scene(GameEnum::LBOSS_SCENE_ID,&req_boss);
}

Int64 LeagueSystem::fetch_new_league_index()
{
	int64_t league_index = 0;
	JUDGE_RETURN(LOGIC_MONITOR->find_global_value(Global::LEAGUE,
			league_index) == 0, -1);

	league_index += 1;
	MMOLeague::save_league_index((int)league_index);
	LOGIC_MONITOR->bind_global_value(Global::LEAGUE, league_index);

	return league_index;
}

int LeagueSystem::creat_league_lvl(int create_type)
{
	return create_type == 1 ? GameEnum::DEFAULT_LEAGUE_LVL
			: GameEnum::DEFAULT_HGIH_LEAGUE_LVL;
}

int LeagueSystem::max_level()
{
	const Json::Value &league = CONFIG_INSTANCE->league();
	return league["set_up"].size();
}

int LeagueSystem::fetch_league_pos(int league_pos)
{
	if (league_pos == GameEnum::LEAGUE_POS_NONE)
	{
		return GameEnum::LEAGUE_POS_DEPUTY;
	}
	else
	{
		return GameEnum::LEAGUE_POS_NONE;
	}
}

bool LeagueSystem::validate_quit_league(int scene_id)
{
	const Json::Value& set_conf = CONFIG_INSTANCE->scene_set(scene_id);
	return set_conf["league_limit"].asInt() == 0;
}

bool LeagueSystem::player_have_league(Int64 role_index)
{
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);

		JUDGE_CONTINUE(league != NULL);
		JUDGE_CONTINUE(league->validate_member(role_index) == true);

		return true;
	}

	return false;
}

bool LeagueSystem::is_league_name_same(const string& league_name)
{
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);

		JUDGE_CONTINUE(league != NULL);
		JUDGE_CONTINUE(league->league_name_ == league_name);

		return true;
	}

	return false;
}

uint LeagueSystem::max_member(int league_lvl)
{
	const Json::Value &league_item = this->league_set_up(league_lvl);
	JUDGE_RETURN(league_item != Json::Value::null, 0);

	const Json::Value& conf = CONFIG_INSTANCE->cur_servers_list();
	string agent_name = conf["agent_name"].asString();
	int agent_code = CONFIG_INSTANCE->agent_code(agent_name);

	const Json::Value& league_ids = CONFIG_INSTANCE->const_set_conf("league_ids")["arr"];
	if (league_ids.empty() == false)
	{
		for (uint i = 0; i < league_ids.size(); ++i)
		{
			JUDGE_CONTINUE(agent_code == league_ids[i].asInt());
			return league_item["special_count"].asInt();
		}
	}

	return league_item["max_count"].asInt();
}

uint LeagueSystem::max_league_pos_count(int league_lvl, int league_pos)
{
	const Json::Value &league_item = this->league_set_up(league_lvl);
	JUDGE_RETURN(league_item != Json::Value::null, 0);

	switch(league_pos)
	{
	case GameEnum::LEAGUE_POS_NONE:
	{
		return league_item["max_count"].asInt() - GameEnum::LEAGUE_LEADER_COUNT;
	}

	case GameEnum::LEAGUE_POS_EXCELLENT:
	{
		return league_item["excellent_count"].asInt();
	}

	case GameEnum::LEAGUE_POS_ELDER:
	{
		return league_item["elder_count"].asInt();
	}

	case GameEnum::LEAGUE_POS_DEPUTY:
	{
		return league_item["deputy_count"].asInt();
	}

	case GameEnum::LEAGUE_POS_LEADER:
	{
		return GameEnum::LEAGUE_LEADER_COUNT;
	}
	}

	return 0;
}

void LeagueSystem::erase_all_apply(long role_index)
{
	for (FourObjVec::iterator iter = this->league_set_.begin();
			iter != this->league_set_.end(); ++iter)
	{
		League* league = this->find_league(iter->id_);
		JUDGE_CONTINUE(league != NULL);
		league->erase_applier(role_index);
	}
}

int LeagueSystem::send_league_escort_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100234*, request, -1);

	for (int i = 0; i < request->role_set_size(); ++i)
	{
		this->send_league_escort_reward(request->role_set(i));
	}

	return 0;
}

int LeagueSystem::send_league_escort_reward(const ProtoThreeObj& three_obj)
{
	Proto30100224 add_info;
	add_info.set_role_id(three_obj.id());
	add_info.set_league_id(three_obj.tick());
	add_info.set_add_contri(three_obj.value());
	return this->add_league_member_contri(&add_info);
}

int LeagueSystem::add_league_flag_exp(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100230*, request, -1);

	int add_exp = request->add_flag_exp();
	Int64 league_index = request->league_index();
	League* league = this->find_league(league_index);
	JUDGE_RETURN(league != NULL, -1);

	league->flag_exp_ += add_exp;

	return 0;
}

int LeagueSystem::send_league_member_reward(MailInformation* send_mail,
		Int64 league_index, int reward)
{
	League* league = this->find_league(league_index);
	JUDGE_RETURN(league != NULL, -1);

	std::string src_mail_content = send_mail->mail_content_;
	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		char mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1];
		::snprintf(mail_content, GameEnum::MAX_MAIL_CONTENT_LENGTH, src_mail_content.c_str(), reward);
		mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH] = '\0';

		send_mail->mail_content_ = mail_content;
		GameCommon::request_save_mail(iter->first, send_mail, false);

		Proto30100224 add_info;
		add_info.set_role_id(iter->first);
		add_info.set_league_id(league_index);
		add_info.set_add_contri(reward);

		this->add_league_member_contri(&add_info);
	}

	return 0;
}

int LeagueSystem::update_martial_role_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100227*, request, -1);

	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(request->role_id(), player) == 0)
	{
		Proto31401701 lable_info;
		lable_info.set_label_id(request->label_id());
		LOGIC_MONITOR->dispatch_to_scene(player, &lable_info);
	}
	else
	{
		MMOLabel::save_player_label(request->role_id(), request->label_id());
	}

	return 0;
}

int LeagueSystem::add_league_member_contri(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100224*, request, -1);
	return this->add_league_member_contri(request->league_id(),
			request->role_id(), request->add_contri());
}

int LeagueSystem::add_league_member_contri(Int64 league_id, Int64 role_id, int add_contri)
{
	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(role_id, player) == 0)
	{
		// online
		player->add_league_member_contri(add_contri);
	}
	else
	{
		// offline
		League* league = this->find_league(league_id);
		JUDGE_RETURN(league != NULL, -1);

		LeagueMember* member = league->league_member(role_id);
		JUDGE_RETURN(member != NULL, -1);

		member->offline_contri_ += add_contri;
		league->add_league_resource(member->role_index_, add_contri);
	}

	return 0;
}

int LeagueSystem::add_league_member_contri(League* league, Int64 role_id, int add_contri)
{
	LogicPlayer* player = NULL;
	if (LOGIC_MONITOR->find_player(role_id, player) == 0)
	{
		// online
		player->add_league_member_contri(add_contri);
	}
	else
	{
		// offline
		LeagueMember* member = league->league_member(role_id);
		JUDGE_RETURN(member != NULL, -1);

		member->offline_contri_ += add_contri;
		league->add_league_resource(member->role_index_, add_contri);
	}

	return 0;
}

void LeagueSystem::sort_league()
{
	this->league_set_.clear();

	// fetch
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);
		JUDGE_CONTINUE(league != NULL);

		FourObj obj;
		obj.id_ = iter->first;
		obj.tick_ = league->create_tick_;

		obj.first_value_ = league->league_force_;
		obj.second_value_ = league->league_lvl_;

		this->league_set_.push_back(obj);
	}

	// sort
	std::sort(this->league_set_.begin(), this->league_set_.end(),
			GameCommon::four_comp_by_desc);

	// set rank info
	int rank_index = 1;
	int record_time = Time_Value::gettimeofday().sec();
	for (FourObjVec::iterator iter = this->league_set_.begin();
			iter != this->league_set_.end(); ++iter)
	{
		League* league = this->find_league(iter->id_);
		JUDGE_CONTINUE(league != NULL);

		league->league_rank_ = rank_index;

		// 记录排行流水
		std::string &league_name = league->league_name_;
		SERIAL_RECORD->record_rank(iter->id_, league_name,
				SERIAL_RANK_TYPE_LEAGUE, rank_index, record_time,
				league->league_lvl_, league->league_force_);

		rank_index += 1;
	}
}

int LeagueSystem::check_league_leader()
{
	BLongMap league_map = this->league_package()->fetch_index_map();

	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);
		JUDGE_CONTINUE(league != NULL);

		league->check_leader_validate();
		JUDGE_CONTINUE(league->member_map_.empty() == true);

		MMOLeague::remove_league(league->league_index_);
		this->league_package()->unbind_and_push(league->league_index_, league);
	}

	return 0;
}

int LeagueSystem::handle_league_fb_finish(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100229*, request, -1);
	return 0;
}

int LeagueSystem::handle_league_boss_finish(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100240*, request, -1);

	League* league = this->find_league(request->league_index());
	JUDGE_RETURN(league != NULL, -1);

	if (request->summon_type() == 1)
	{
		league->league_boss_.normal_summon_type_ = false;
		league->league_boss_.normal_die_tick_ = ::time(NULL);
	}
	else
	{
		league->league_boss_.super_summon_type_ = false;
		league->league_boss_.super_die_tick_ = ::time(NULL);
	}
	MMOLeague::save_league(league);

	return 0;
}

int LeagueSystem::handle_rename_league_finish(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100243*, request, -1);

	Int64 league_id = request->league_index();
	League* league = this->find_league(league_id);
	JUDGE_RETURN(league != NULL, -1);

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer *player = NULL;
		if(LOGIC_MONITOR->find_player(iter->first, player) == 0)
		{
			player->fetch_league_info();
		}
	}

	return 0;
}

const Json::Value& LeagueSystem::league_shop(int shop_type)
{
	if (shop_type == GameEnum::LEAGUE_SHOP_ITEM)
	{
		return CONFIG_INSTANCE->league()["shop_item"];
	}
	else
	{
		return CONFIG_INSTANCE->league()["shop_equip"];
	}
}

const Json::Value& LeagueSystem::league_set_up(uint league_lvl)
{
	const Json::Value &league_conf = CONFIG_INSTANCE->league("set_up");
	JUDGE_RETURN(league_lvl > 0 && league_lvl <= league_conf.size(),
			Json::Value::null);

	return league_conf[league_lvl - 1];
}

League* LeagueSystem::find_league(Int64 league_index)
{
	return this->league_package()->find_object(league_index);
}

void LeagueSystem::save_all_league(int direct_save)
{
	BLongMap league_map = this->league_package()->fetch_index_map();
	for (BLongMap::iterator iter = league_map.begin(); iter != league_map.end();
			++iter)
	{
		League* league = this->find_league(iter->first);
		MMOLeague::save_league(league, direct_save);
	}
}

void LeagueSystem::save_league(const Int64 league_id)
{
	League *league = this->find_league(league_id);

	if (league != NULL)
		MMOLeague::save_league(league, false);
}

void LeagueSystem::set_league_war_label(Int64 league_index)
{
	League* league = this->find_league(league_index);
	JUDGE_RETURN(league != NULL, ;);

	int label_id = CONFIG_INSTANCE->league_war("label_id").asInt();

	Proto31401701 request;
	request.set_label_id(label_id);

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		LogicPlayer* player = NULL;
		if (LOGIC_MONITOR->find_player(iter->first, player) == 0)
		{
			LOGIC_MONITOR->dispatch_to_scene(player, &request);
		}
		else
		{
			MMOLabel::save_player_label(iter->first, label_id, true);
		}
	}
}

int LeagueSystem::league_size(void)
{
    return this->league_set_.size();
}

LeagueStore* LeagueSystem::find_lstore(Int64 league_index)
{
	League* league = NULL;
	league = find_league(league_index);
	JUDGE_RETURN(league != NULL,NULL);

	return &league->__lstore;
}


int LeagueSystem::notify_lescort_state(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30100238 *, req, msg, -1);
    Int64 league_id = req->league_id();
    int state = req->state();
    League* league = LEAGUE_SYSTEM->find_league(league_id);
    JUDGE_RETURN(league != NULL,-1);

    if( state == 1 )
    {
        for(League::MemberMap::iterator iter = league->member_map_.begin(); iter != league->member_map_.end(); ++iter)
        {
        	LogicPlayer* player = NULL;
        	LOGIC_MONITOR->find_player(iter->first,player);
        	JUDGE_CONTINUE(player != NULL);
        	player->respond_to_client(NOTIFY_LESCORT_SUCESS);
        	player->respond_to_client(NOTIFY_LESCORT_DISAPPEAR);
        }
    }
    else if( state == 0 )
    {
        for(League::MemberMap::iterator iter = league->member_map_.begin(); iter != league->member_map_.end(); ++iter)
        {
        	LogicPlayer* player = NULL;
        	LOGIC_MONITOR->find_player(iter->first,player);
        	JUDGE_CONTINUE(player != NULL);
        	player->respond_to_client(NOTIFY_LESCORT_FAILED);
        	player->respond_to_client(NOTIFY_LESCORT_DISAPPEAR);
        }
    }
    return 0;
}

int LeagueSystem::map_act_fetch_league_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400037*, request, -1);
	//int type = request->rank_type();
	int size = request->size();

	Proto31400037 list_info;
	LongVec league_set;
	LEAGUE_SYSTEM->fetch_league_set(0, league_set);
	int i = 0;
	for (LongVec::iterator iter = league_set.begin(); i < size &&iter != league_set.end() ; ++iter, ++i)
	{
		League* league = this->find_league(*iter);
		JUDGE_CONTINUE(league != NULL);
		ProtoLeagueItem* league_item = list_info.add_rank_list();
		JUDGE_CONTINUE(league_item != NULL);
		league->make_up_league_list_info(0, league_item);
		league_item->set_rank_index(i+1);
	}
	list_info.set_rank_type(RANK_LEAGUE);	//仙盟排行
	return LOGIC_MONITOR->dispatch_to_all_map(&list_info);
}

int LeagueSystem::map_act_fetch_league_rank_info(int size)
{
	Proto31400037 list_info;
	LongVec league_set;
	LEAGUE_SYSTEM->fetch_league_set(0, league_set);
	int i = 0;
	for (LongVec::iterator iter = league_set.begin(); i < size &&iter != league_set.end() ; ++iter, ++i)
	{
		League* league = this->find_league(*iter);
		JUDGE_CONTINUE(league != NULL);
		ProtoLeagueItem* league_item = list_info.add_rank_list();
		JUDGE_CONTINUE(league_item != NULL);
		league->make_up_league_list_info(0, league_item);
		league_item->set_rank_index(i+1);
	}
	list_info.set_rank_type(RANK_LEAGUE);	//仙盟排行
	return LOGIC_MONITOR->dispatch_to_all_map(&list_info);
}

BLongSet& LeagueSystem::league_fb_flag()
{
	return this->lfb_flag;
}

void LeagueSystem::insert_fb_flag(Int64 role_id)
{
	this->lfb_flag.insert(role_id);
	this->sync_fb_flag(role_id);
}

void LeagueSystem::sync_fb_flag(Int64 role_id)
{
	int flag = this->lfb_flag.count(role_id);
	LogicPlayer *player = NULL;
	if(LOGIC_MONITOR->find_player(role_id,player) == 0)
	{
		Proto30400448 req;
		req.set_fb_flag(flag);
		LOGIC_MONITOR->dispatch_to_scene(player,&req);
	}
}

int LeagueSystem::inner_send_lwar_first_win(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100231*, request, -1);

	Int64 league_id = request->league_id();
	League *league = this->find_league(league_id);
	JUDGE_RETURN(league != NULL, 0);

	int label_mail_id = CONFIG_INSTANCE->league_war("label_mail_id").asInt();
	int league_extra_award = CONFIG_INSTANCE->league_war("league_extra_award").asInt();
	int label_award = CONFIG_INSTANCE->league_war("label_award").asInt();

	for (League::MemberMap::iterator iter = league->member_map_.begin();
			iter != league->member_map_.end(); ++iter)
	{
		MailInformation *mail_info = GameCommon::create_sys_mail(label_mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str());

		if (iter->first == league->leader_index_)
		{
			mail_info->add_goods(label_award);
		}
		else
		{
			mail_info->add_goods(league_extra_award);
		}

		GameCommon::request_save_mail_content(iter->first, mail_info);
	}

	return 0;
}

int LeagueSystem::set_region_result(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100111*, request, -1);

	for (int i = 0; i < request->result_list_size(); ++i)
	{
		const ProtoItem& item = request->result_list(i);

		League* league = this->find_league(item.unique_id());
		JUDGE_CONTINUE(league != NULL);

		league->region_rank_ = item.id();
		league->region_tick_ = item.use_tick();
		league->region_leader_reward_ = false;
	}

	return 0;
}
