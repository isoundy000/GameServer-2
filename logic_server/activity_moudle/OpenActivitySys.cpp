/*
 * LogicOpenActivity.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: jinxing
 */

#include "GameCommon.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "OpenActivitySys.h"
#include "MMOOpenActivity.h"


OpenActivitySys::OpenActivitySys()
{
	this->cur_open_day_ = 0;
	this->cur_combine_day_ = 0;
	this->cur_main_act_ = 0;
}

OpenActivitySys::~OpenActivitySys()
{
}

int OpenActivitySys::after_load_activity(DBShopMode* shop_mode)
{
	return 0;
}

int OpenActivitySys::start(void)
{
	this->init_open_activity();
	this->init_return_activity();
	this->init_combine_activity();
	this->init_combine_return_activity();

	this->update_activity_sys();
	MMOOpenActivity::load_open_activity_sys(this);

	this->after_load_db_update();
	MSG_USER("OpenActivitySys start...");

	return 0;
}

int OpenActivitySys::stop(int test)
{
	this->save_open_activity_sys();
	return 0;
}

int OpenActivitySys::midnight_handle_timeout(int test_day, int act_type)
{
	this->send_lwar_activity_rank_award();
	this->send_cornucopia_mail_reward();
	this->init_cornucopia_act();
	this->handle_act_finish();
	this->update_activity_sys(test_day, act_type);
	MMOOpenActivity::save_open_activity_sys(this);
	return 0;
}

int OpenActivitySys::cur_open_day()
{
	return this->cur_open_day_;
}

int OpenActivitySys::cur_combine_day()
{
	return this->cur_combine_day_;
}

int OpenActivitySys::validate_open_day(int day)
{
	return this->cur_open_day_ == day;
}

int OpenActivitySys::validate_combine_day(int day)
{
	return this->cur_combine_day_ == day;
}

int OpenActivitySys::main_act_type()
{
	return this->cur_main_act_;
}

int OpenActivitySys::is_open_activity_id(int act_index)
{
	return CONFIG_INSTANCE->open_activity_map().find(act_index);
}

int OpenActivitySys::is_return_activity_id(int act_index)
{
	return CONFIG_INSTANCE->return_activity_map().find(act_index);
}

int OpenActivitySys::is_combine_activity_id(int act_index)
{
	return CONFIG_INSTANCE->combine_activity_map().find(act_index);
}

int OpenActivitySys::is_combine_return_activity(int act_index)
{
	return CONFIG_INSTANCE->combine_return_activity_map().find(act_index);
}

int OpenActivitySys::is_open_activity()
{
	return this->main_act_type() == GameEnum::MAIN_ACT_OPEN;
}

int OpenActivitySys::is_return_activity()
{
	return this->main_act_type() == GameEnum::MAIN_ACT_RETURN;
}

int OpenActivitySys::is_combine_activity()
{
	return this->main_act_type() == GameEnum::MAIN_ACT_COMBINE;
}

int OpenActivitySys::is_combine_return_activity()
{
	return this->main_act_type() == GameEnum::MAIN_ACT_C_RETURN;
}

void OpenActivitySys::init_open_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->open_activity_map();
	this->init_all_activity(act_map);
}

void OpenActivitySys::init_return_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->return_activity_map();
	this->init_all_activity(act_map);
}

void OpenActivitySys::init_combine_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->combine_activity_map();
	this->init_all_activity(act_map);
}

void OpenActivitySys::init_combine_return_activity()
{
	GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->combine_return_activity_map();
	this->init_all_activity(act_map);
}

void OpenActivitySys::init_all_activity(GameConfig::ConfigMap& act_map)
{
	GameConfig::ConfigMap::iterator iter = act_map.begin();
	for (; iter != act_map.end(); ++iter)
	{
		this->add_new_item(iter->first, *(iter->second));
	}
}

void OpenActivitySys::init_cur_day_act()
{
	this->act_list_.clear();

	for (BackSetActDetail::ActTypeItemSet::iterator
			iter = this->act_type_item_set_.begin();
			iter != this->act_type_item_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->special_notify_ <= 1);

		if (CONFIG_INSTANCE->do_combine_server())
		{
			JUDGE_CONTINUE(this->is_combine_activity_id(iter->act_index_) == true
					|| this->is_combine_return_activity(iter->act_index_) == true);
			JUDGE_CONTINUE(iter->open_time_.count(this->cur_combine_day_) > 0);
		}
		else
		{
			JUDGE_CONTINUE(this->is_open_activity_id(iter->act_index_) == true
					|| this->is_return_activity_id(iter->act_index_) == true);
			JUDGE_CONTINUE(iter->open_time_.count(this->cur_open_day_) > 0);
		}

		this->act_list_[iter->act_index_] = true;
	}
}

void OpenActivitySys::save_open_activity_sys()
{
	MMOOpenActivity::save_open_activity_sys(this);
	MSG_USER("OpenActivitySys ...");
}

void OpenActivitySys::handle_act_finish()
{
	for (IntMap::iterator iter = this->act_list_.begin();
			iter != this->act_list_.end(); ++iter)
	{
		BackSetActDetail::ActTypeItem* act_t_item = this->find_item(iter->first);
		JUDGE_CONTINUE(act_t_item != NULL);
		this->handle_act_finish(act_t_item);
	}

	for (IntMap::iterator iter = this->act_list_.begin();
			iter != this->act_list_.end(); ++iter)
	{
		BackSetActDetail::ActTypeItem* act_t_item = this->find_item(iter->first);
		JUDGE_CONTINUE(act_t_item != NULL);
		JUDGE_CONTINUE(act_t_item->day_clear_ == 1);
		act_t_item->reset_everyday();
	}
}

void OpenActivitySys::handle_act_finish(BackSetActDetail::ActTypeItem* act_t_item)
{
	JUDGE_RETURN(act_t_item != NULL, ;);
	JUDGE_RETURN(act_t_item->mail_id_ > 0, ;);

	int record = false;
	switch (act_t_item->cond_type_)
	{
	case BackSetActDetail::COND_TYPE_1:
	{
		//战骑类冲榜
		act_t_item->sort_t_sub_map();
		break;
	}
	case BackSetActDetail::COND_TYPE_4:
	{
		//消费排行榜
		JUDGE_RETURN(CONFIG_INSTANCE->combine_server_days() >= 4, ;);
		act_t_item->sort_t_sub_map_b();
	}
	}

	if (record == true)
	{
		Int64 record_time = ::time(NULL);
		for (ThreeObjMap::iterator iter = act_t_item->t_sub_map_.begin();
				iter != act_t_item->t_sub_map_.end(); ++iter)
		{
			MSG_USER("activity rank %d %d %d %ld %s",
					act_t_item->act_index_, iter->second.sub_, iter->second.value_,
					iter->second.id_, iter->second.name_.c_str());

			SERIAL_RECORD->record_rank(iter->first, iter->second.name_,
					act_t_item->act_index_, iter->second.sub_, record_time,
					iter->second.value_);
		}
	}

	BackSetActDetail::ActItemSet::iterator reward_iter = act_t_item->act_item_set_.begin();
	for (; reward_iter != act_t_item->act_item_set_.end(); ++reward_iter)
	{
		switch (reward_iter->reward_type_)
		{
		case 0:
		{
			this->send_undrawed_player_reward(act_t_item, *reward_iter);
			break;
		}
		case 1:
		{
			this->send_rank_player_reward(act_t_item, *reward_iter);
			break;
		}
		}
	}
}

void OpenActivitySys::update_activity_sys(int cur_day, int act_type)
{
	int open_days = 0;
	if (CONFIG_INSTANCE->do_combine_server())
	{
		open_days = CONFIG_INSTANCE->const_set("combine_activity_days");
	}
	else
	{
		open_days = CONFIG_INSTANCE->const_set("open_activity_days");
	}

	if (cur_day == -1)
	{
		this->cur_open_day_ = CONFIG_INSTANCE->open_server_days();
		this->cur_combine_day_ = CONFIG_INSTANCE->combine_server_days();
		this->cur_main_act_ = CONFIG_INSTANCE->main_activity_type();
	}
	else if (cur_day <= open_days)
	{
		if (CONFIG_INSTANCE->do_combine_server())
		{
			this->cur_combine_day_ = cur_day;
			this->cur_main_act_ = GameEnum::MAIN_ACT_COMBINE;
		}
		else
		{
			this->cur_open_day_ = cur_day;
			this->cur_main_act_ = GameEnum::MAIN_ACT_OPEN;
		}
	}
	else
	{
		if (CONFIG_INSTANCE->do_combine_server())
		{
			this->cur_combine_day_ = cur_day;
			this->cur_main_act_ = GameEnum::MAIN_ACT_C_RETURN;
		}
		else
		{
			this->cur_open_day_ = cur_day;
			this->cur_main_act_ = GameEnum::MAIN_ACT_RETURN;
		}
	}

	this->init_cur_day_act();
	MSG_USER("OpenActivitySys %d %d %d", this->cur_open_day_, this->cur_combine_day_, this->cur_main_act_);
}

void OpenActivitySys::after_load_db_update()
{
	this->init_cur_day_act();
	this->load_rank_combine_activity_consume();
	this->load_cornucopia_act();
}

void OpenActivitySys::load_rank_combine_activity_consume()
{
	JUDGE_RETURN(LOGIC_OPEN_ACT_SYS->is_combine_activity() == true, );

	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(
			BackSetActDetail::C_ACT_CONSUM_RANK, CONFIG_INSTANCE->combine_server_days());
	JUDGE_RETURN(act_t_item != NULL, ;);

	act_t_item->sort_t_sub_map_b();
}

void OpenActivitySys::send_rank_player_reward(BackSetActDetail::ActTypeItem* act_t_item, ActItem& act_item)
{
//	JUDGE_RETURN(act_t_item->cond_type_ == BackSetActDetail::COND_TYPE_1, ;);

	ThreeObjMap player_map;
	for (ThreeObjMap::iterator iter = act_t_item->t_sub_map_.begin();
			iter != act_t_item->t_sub_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second.sub_ >= act_item.cond_value_[0] &&
				iter->second.sub_ <= act_item.cond_value_[1]);
		player_map[iter->first] = iter->second;
	}

	MailInformation* mail_info = GameCommon::create_sys_mail(act_t_item->mail_id_);
	mail_info->add_goods(act_item.reward_id_);

	string src_content = mail_info->mail_content_;
    for (ThreeObjMap::iterator iter = player_map.begin(); iter != player_map.end(); ++iter)
    {
    	int value = 0;
    	if (act_t_item->cond_type_ == BackSetActDetail::COND_TYPE_1)
    	{
    		value = iter->second.tick_;
    	}
    	else
    	{
    		value = iter->second.value_;
    	}

    	mail_info->mail_content_ = src_content;
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), iter->second.sub_, value);

        GameCommon::request_save_mail_content(iter->first, mail_info, false);
        MSG_USER("Rank Reward %ld, %d", iter->first, iter->second.sub_, value);
    }

    mail_info->recycle_self();
}

void OpenActivitySys::send_undrawed_player_reward(BackSetActDetail::ActTypeItem* act_t_item, ActItem& act_item)
{
	LongMap player_map;
	switch (act_t_item->cond_type_)
	{
	case BackSetActDetail::COND_TYPE_0:
	case BackSetActDetail::COND_TYPE_1:
//	case BackSetActDetail::COND_TYPE_4:
	{
		player_map = act_item.drawed_map_;
		break;
	}
	case BackSetActDetail::COND_TYPE_2:
	{
		JUDGE_RETURN(act_item.arrive() == true, ;);
		player_map = act_item.drawed_map_;
		break;
	}
	}

    MailInformation* mail_info = GameCommon::create_sys_mail(act_t_item->mail_id_);
    ::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
    		mail_info->mail_title_.c_str(), act_t_item->act_title_.c_str());
    mail_info->mail_title_ = mail_info->makeup_content_;

    ::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
    		mail_info->mail_content_.c_str(), act_t_item->act_title_.c_str());
    mail_info->add_goods(act_item.reward_id_);

    for (LongMap::iterator iter = player_map.begin(); iter != player_map.end(); ++iter)
    {
    	JUDGE_CONTINUE(iter->second == 0);
        GameCommon::request_save_mail_content(iter->first, mail_info, false);
    }

    mail_info->recycle_self();
}

int OpenActivitySys::update_lwar_activity_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100242*, request, -1);

	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_item_by_day(
			PairObj(BackSetActDetail::F_ACT_LEAGUE_WAR,this->cur_open_day_), 1);
//	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_open_item(F_ACT_LEAGUE_WAR);
	JUDGE_RETURN(s_act_t_item != NULL, -1);
	JUDGE_RETURN(s_act_t_item->special_notify_ <= 0, -1);
	JUDGE_RETURN(::time(NULL) > CONFIG_INSTANCE->open_server_date(), -1);

	BackSetActDetail::ActItemSet::iterator reward_iter = s_act_t_item->act_item_set_.begin();
	for (; reward_iter != s_act_t_item->act_item_set_.end(); ++reward_iter)
	{
		ActItem& act_item = *reward_iter;
		act_item.drawed_map_.clear();
		for (int i = 0; i < request->activity_lwar_size(); ++i)
		{
			const ProtoActivityLWarRank& lwar_rank = request->activity_lwar(i);
			int rank = lwar_rank.rank();
			Int64 role_id = lwar_rank.role_id();
			int is_leader = lwar_rank.is_leader();
			if (rank >= act_item.cond_value_[0] && rank <= act_item.cond_value_[1])
				act_item.drawed_map_[role_id] = rank;

			if (is_leader == true)
			{
				rank += 100;
				if (rank >= act_item.cond_value_[0] && rank <= act_item.cond_value_[1])
					act_item.drawed_map_[role_id] = (rank - 100);
			}
		}
	}
	s_act_t_item->special_notify_ = 1;

	return 0;
}

void OpenActivitySys::send_lwar_activity_rank_award()
{
	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_item_by_day(F_ACT_LEAGUE_WAR, 1);
	JUDGE_RETURN(s_act_t_item != NULL, ;);
	JUDGE_RETURN(s_act_t_item->special_notify_ == 1, ;);

	BackSetActDetail::ActItemSet::iterator reward_iter = s_act_t_item->act_item_set_.begin();
	for (; reward_iter != s_act_t_item->act_item_set_.end(); ++reward_iter)
	{
		ActItem& act_item = *reward_iter;
		for (LongMap::iterator iter = act_item.drawed_map_.begin();
				iter != act_item.drawed_map_.end(); ++iter)
		{
			JUDGE_CONTINUE(iter->second > 0);

			int rank = 0;
			if (iter->second < 100)
				rank = iter->second;
			else
				rank = iter->second - 100;

			MailInformation* mail_info = GameCommon::create_sys_mail(s_act_t_item->mail_id_);
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), rank);
			mail_info->add_goods(act_item.reward_id_);
			GameCommon::request_save_mail_content(iter->first, mail_info);

//			mail_info->recycle_self();
		}
		act_item.drawed_map_.clear();
	}
	s_act_t_item->special_notify_ = 2;
}

void OpenActivitySys::test_reset_lwar_activity()
{
	BackSetActDetail::ActTypeItem* s_act_t_item = this->find_item_by_day(F_ACT_LEAGUE_WAR, 1);
	JUDGE_RETURN(s_act_t_item != NULL, ;);

	s_act_t_item->special_notify_ = 0;
}

IntMap& OpenActivitySys::fetch_act_list()
{
	return this->act_list_;
}

BackSetActDetail::ActTypeItem* OpenActivitySys::find_open_item(int type)
{
	int day = this->cur_open_day_;
	return this->find_item_by_day(type, day);
}

void OpenActivitySys::init_cornucopia_act()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	if(act_t_item == NULL)
	{
		act_t_item = LOGIC_OPEN_ACT_SYS->find_item(10901);
		if(act_t_item == NULL)
			return;
		if(act_t_item->open_time_.count(parsed_days - 1) == 0)
			return ;
	}
	act_t_item->server_record_map_.clear();
	act_t_item->cornucopia_server_gold_ = 0;
}

void OpenActivitySys::send_cornucopia_mail_reward()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	if(act_t_item == NULL)
	{
		act_t_item = LOGIC_OPEN_ACT_SYS->find_item(10901);
		if(act_t_item == NULL)
			return;
		if(act_t_item->open_time_.count(parsed_days - 1) == 0)
			return ;
	}
	ServerRecordMap& server_map = act_t_item->server_record_map_;
	ActItemSet& item_set = act_t_item->act_item_set_;
	int item_set_index = 0;
	if((int)item_set.size() < parsed_days)
	{
		item_set_index = item_set.size() - 1;
	}
	else
	{
		item_set_index = parsed_days - 1;
	}
	ServerRecordMap::iterator record_iter = server_map.begin();
	for( ; record_iter != server_map.end(); ++record_iter)
	{
		ServerRecord& record = record_iter->second;
		int reward_mult = record.reward_mult_;
		int gold = record.cornucopia_gold_;

		int mail_gold = gold + gold * reward_mult / 10000;
		JUDGE_CONTINUE(mail_gold > 0);
		MailInformation* mail_info = GameCommon::create_sys_mail(act_t_item->mail_id_);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), gold, record.reward_mult_ * 100 / 10000, mail_gold);

		mail_info->add_goods(item_set[item_set_index].reward_id_, mail_gold, true);
		GameCommon::request_save_mail_content(record.player_id_, mail_info);
	}
}

void OpenActivitySys::load_cornucopia_act()
{
	int parsed_days = CONFIG_INSTANCE->open_server_days();
	BackSetActDetail::ActTypeItem* act_t_item = LOGIC_OPEN_ACT_SYS->find_item_by_day(BackSetActDetail::F_ACT_CORNUCOPIA, parsed_days);
	if(act_t_item == NULL)
	{
		act_t_item = LOGIC_OPEN_ACT_SYS->find_item(10901);
		if(act_t_item == NULL)
			return;
		if(act_t_item->open_time_.count(parsed_days - 1) == 0)
			return ;
	}

	ServerRecordMap& record_map = act_t_item->server_record_map_;
	ServerRecordMap::iterator record_iter = record_map.begin();
	int& server_gold = act_t_item->cornucopia_server_gold_;
	for( ; record_iter != record_map.end(); ++record_iter)
	{
		ServerRecord& record = record_iter->second;
		server_gold += record.cornucopia_gold_;
	}
}
