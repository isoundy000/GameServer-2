/*
 * BaseChannel.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "BaseChannel.h"
#include "ChannelAgency.h"
#include "Block_Buffer.h"

BaseChannel::BaseChannel()
{
	reset();
}

BaseChannel::~BaseChannel()
{

}

void BaseChannel::reset(void)
{
	//this->record_vec_.clear();
	this->cancel_timer();
	if (this->agency_ != NULL)
	{
		for (RecordList::iterator iter = this->record_list_.begin();
				iter != this->record_list_.end(); ++iter)
		{
			this->agency_->push_record(*iter);
		}
		for (RecordList::iterator iter = this->voice_list_.begin();
				iter != this->voice_list_.end(); ++iter)
		{
			this->agency_->push_record(*iter);
		}
	}
	this->record_list_.clear();
	this->voice_list_.clear();
	this->player_sid_set_.clear();
	this->interval_tick_ = Time_Value::zero;
	this->agency_ = NULL;
	this->channel_id_ = 0;
}

int BaseChannel::type(void)
{
	return GTT_CHAT_CHANNEL;
}

int BaseChannel::handle_timeout(const Time_Value &tv)
{
	return this->notify();
}

int BaseChannel::schedule_timer(const Time_Value &interval)
{
	this->interval_tick_ = interval;
	return GameTimer::schedule_timer(interval);
}

int BaseChannel::cancel_timer(void)
{
	return GameTimer::cancel_timer();
}

int BaseChannel::init(ChannelAgency *agency,int64_t channel_id)
{
	this->reset();
	this->channel_id_ = channel_id;
	this->agency_ = agency;
	return 0;
}

int BaseChannel::stop(void)
{
	this->cancel_timer();

	if (NULL != this->agency_)
	{
		RecordList::iterator iter = this->record_list_.begin();
		for (; iter!=this->record_list_.end(); ++iter)
		{
			this->push_record(*iter);
		}
		this->record_list_.clear();
		for (RecordList::iterator iter = this->voice_list_.begin();
				iter != this->voice_list_.end(); ++iter)
		{
			this->push_record(*iter);
		}
		this->voice_list_.clear();
	}

//	this->reset();
	return 0;
}

int BaseChannel::push_record(ChatRecord *record)
{
	return this->agency()->push_record(record);
}

ChatRecord* BaseChannel::pop_record(void)
{
	return this->agency()->pop_record();
}

int BaseChannel::send_record(ChatRecord *record)
{
	JUDGE_RETURN(record != NULL, -1);
	this->record_list_.push_back(record);
	return 0;
}

int BaseChannel::notify(void)
{
	RecordList::iterator iter_r = this->record_list_.begin();
	for (; iter_r!=this->record_list_.end(); ++iter_r)
	{
		BIntSet::iterator iter_p = this->player_sid_set_.begin();
		for (; iter_p!=this->player_sid_set_.end(); ++iter_p)
		{
			(*iter_r)->__sid = *iter_p;
			this->agency_->channel_notify(this->channel_type(),*iter_r);
		}

		MSG_USER(hyk_chat--PLAYER_SID_SET:%d---, this->player_sid_set_.size());
		this->push_record(*iter_r);
	}

	this->record_list_.clear();
	return 0;
}

const int64_t BaseChannel::channel_id(void)
{
	return this->channel_id_;
}

void BaseChannel::set_channel_id(int64_t id)
{
	this->channel_id_ = id;
}

int BaseChannel::bind_sid(int sid)
{
	if(this->find_sid(sid)==0)
	{
		return -1;
	}
	this->player_sid_set_.insert(sid);
	return 0;
}

int BaseChannel::unbind_sid(int sid)
{
	return this->player_sid_set_.erase(sid);
}

int BaseChannel::find_sid(int sid)
{
	BIntSet::iterator iter = this->player_sid_set_.find(sid);
	if(iter!=this->player_sid_set_.end())
	{
		return 0;
	}
	return -1;
}

int BaseChannel::channel_client_amount(void)
{
    return int(this->player_sid_set_.size());
}

void BaseChannel::set_agency(ChannelAgency *agency)
{
	this->agency_ = agency;
}

ChannelAgency *BaseChannel::agency(void)
{
	return this->agency_;
}

int BaseChannel::push_voice(ChatRecord* record,bool history/*=false*/)
{
	if (voice_list_.size() >= 100)
	{
		ChatRecord *back = voice_list_.back();
		this->push_record(back);
		voice_list_.pop_back();
	}
	if(history)
	{
		voice_list_.push_back(record);
	}
	else
	{
		ChatRecord *r = this->pop_record();
		r->copy(record);
		voice_list_.push_front(r);
	}
	return 0;
}
ChatRecord* BaseChannel::get_voice(int64_t id)
{
	for(RecordList::iterator iter = this->voice_list_.begin();
			iter!=this->voice_list_.end();iter++)
	{
		if((*iter)->__voice_id==id)
		{
			return *iter;
		}
	}
	return NULL;
}
