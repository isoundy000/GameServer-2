/*
 * LeagueChannel.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "LeagueChannel.h"
#include "ChannelAgency.h"
#include "ChatPlayer.h"
#include "TransactionMonitor.h"

#define MAX_HISTORY 100
#define MAX_HISTORY_RETURN_ONCE 10
#define SAVE_INTERVAL  60*3
#define CHANNEL_TIME_INTERVAL 0.5*1000000

LeagueChannel::LeagueChannel()
{
	reset();
}

LeagueChannel::~LeagueChannel()
{

}

void LeagueChannel::reset(void)
{
	if (this->agency_ != NULL)
	{
		for (RecordList::iterator iter = this->history_.begin();
				iter != this->history_.end(); ++iter)
		{
			this->agency_->push_record(*iter);
		}
		for (RecordList::iterator iter = this->history_for_save_.begin();
				iter != this->history_for_save_.end(); ++iter)
		{
			this->agency_->push_record(*iter);
		}
	}
	this->history_.clear();
	this->history_for_save_.clear();
	this->data_change_ = false;
	this->last_save_ = Time_Value::gettimeofday();

	BaseChannel::reset();
}

int LeagueChannel::init(ChannelAgency *agency,int64_t channel_id)
{
	BaseChannel::init(agency,channel_id);
	this->history_.clear();
	return 0;//this->load_history();
}

int LeagueChannel::start(void)
{
	return this->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
}

int LeagueChannel::stop(void)
{
	this->save_history();
	BaseChannel::stop();
	return 0;
}

int LeagueChannel::channel_type(void)
{
	return CHANNEL_LEAGUE;
}

int LeagueChannel::notify(void)
{
	RecordList::iterator iter_r = this->record_list_.begin();
	for(;iter_r!=this->record_list_.end();++iter_r)
	{
		BIntSet::iterator iter_p = this->player_sid_set_.begin();
		for(;iter_p!=this->player_sid_set_.end();++iter_p)
		{
			(*iter_r)->__sid = *iter_p;
			this->agency()->channel_notify(this->channel_type(),*iter_r);
		}
		this->push_history(*iter_r);
	}
	this->record_list_.clear();
	this->save_history();
	return 0;
}

int LeagueChannel::push_history(ChatRecord *record)
{
	this->data_change_ = true;

	if(record->__type==R_VOICE_SAVE)
	{
		this->push_voice(record,true);
		return 0;
	}

	if(this->history_.size()>=MAX_HISTORY)
	{
		ChatRecord *tmp = this->history_.back();
		this->push_record(tmp);
		this->history_.pop_back();
	}
	this->history_.push_front(record);
	return 0;
}

int LeagueChannel::notify_offline(ChatPlayer *player)
{
	JUDGE_RETURN(player != NULL, -1);
	JUDGE_RETURN(player->league_id() > 0, -1);

	RecordList list;
	this->get_history(list, player->role_detail().__last_sign_out);
	MSG_USER("%d-%d-%s-%d", list.size(), player->role_detail().__last_sign_out,
			player->name(), Time_Value::gettimeofday().sec() - player->role_detail().__last_sign_out);
	JUDGE_RETURN(list.size() > 0, -1);

	RecordList::iterator it = list.begin();
	for(; it != list.end(); ++it)
	{
		JUDGE_CONTINUE((*it) != NULL);
		(*it)->__sid = player->client_sid();
		this->agency()->channel_notify(this->channel_type(), *it);
	}

	return 0;
}

int LeagueChannel::get_history(std::list<ChatRecord*>& list,int time_offset)
{
	int max_msg = 0;
	RecordList::iterator iter = this->history_.begin();
	for (; iter != this->history_.end() && max_msg < MAX_HISTORY_RETURN_ONCE; ++iter)
	{
		ChatRecord* record = *iter;
		if(time_offset>0 && record->__time <= time_offset)
		{
			continue;
		}
		JUDGE_CONTINUE(record->__src_role_id != 0);
		MSG_USER("timeoffset:%d- record_time:%d- sender_id:%ld -size:%d",
				time_offset, record->__time, record->__src_role_id, list.size());
//		std::string content(record->__buffer->get_read_ptr(),
//				record->__buffer->readable_bytes());
//		MSG_DEBUG(%s, content.c_str());
		list.push_front(record);
		++max_msg;
	}
	return 0;
}

int LeagueChannel::load_history(void)
{
	return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),TRANS_CHAT_LOAD_LEAGUE_HISTORY,
			DB_CHAT_LEAGUE,this,NULL,this->agency()->logic_unit(),0,TRANS_CHAT_LOAD_LEAGUE_HISTORY);
}

int LeagueChannel::save_history(void)
{
	if(Time_Value::gettimeofday().sec()-this->last_save_.sec()>=SAVE_INTERVAL
		&&	this->data_change_ )
	{
		this->data_change_ = false;
		this->last_save_ = Time_Value::gettimeofday();

		this->history_for_save_.clear();
		RecordList::iterator iter = this->history_.begin();
		for (; iter != this->history_.end(); ++iter)
		{
			ChatRecord *bak = this->pop_record();
			bak->copy(*iter);
			this->history_for_save_.push_back(bak);
		}
		//语音内容
		for (RecordList::iterator iter = this->voice_list_.begin();
				iter != this->voice_list_.end(); iter++)
		{
			ChatRecord *bak = this->pop_record();
			bak->copy(*iter);
			this->history_for_save_.push_back(bak);
		}
		return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),TRANS_CHAT_SAVE_LEAGUE_HISTORY,
					DB_CHAT_LEAGUE,this,&this->agency()->league_channel_pool());
	}
	return 0;
}
