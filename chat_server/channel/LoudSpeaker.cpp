/*
 * LoudSpeaker.cpp
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#include "LoudSpeaker.h"
#include "ChannelAgency.h"
#include "TransactionMonitor.h"

#define MAX_CHAT_VECTOR 10240
#define SAVE_INTERVAL 60*1
#define RECORD_PER_TIME 2

LoudSpeaker::LoudSpeaker()
{
	reset();
}

LoudSpeaker::~LoudSpeaker()
{

}

void LoudSpeaker::reset(void)
{
	this->wr_index_ = 0;
	this->rd_index_ = 0;
	this-> data_change_ = false;
	this->last_save_ = Time_Value::gettimeofday();
	this->record_list_bak_.clear();
	BaseChannel::reset();
}

int LoudSpeaker::init(ChannelAgency *agency,int64_t channel_id)
{
	return BaseChannel::init(agency,channel_id);
	return this->load_record();
}

int LoudSpeaker::stop(void)
{
	this->save_record();
	return BaseChannel::stop();
}

int LoudSpeaker::channel_type(void)
{
	return CHANNEL_LOUDSPEAKER;
}

int LoudSpeaker::notify(void)
{
	for(int i=0;i< RECORD_PER_TIME;++i)
	{
		if(this->record_list_.empty()==false)
		{
			ChatRecord *record = this->record_list_.front();
			if (NULL != record)
			{
				BIntSet::iterator iter_p = this->player_sid_set_.begin();
				for(;iter_p!=this->player_sid_set_.end();++iter_p)
				{
					record->__sid = *iter_p;
					this->agency()->channel_notify(this->channel_type(),record);
				}
				this->record_list_.pop_front();
				this->push_record(record);
			}
			continue;
		}
		break;
	}
	this->save_record();
	return 0;
}

int LoudSpeaker::record_size(void)
{
	return this->record_list_.size();
}

int LoudSpeaker::load_record(void)
{
	return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),TRANS_CHAT_LOAD_LOUDSPEAKER,
			DB_CHAT_LOUDSPEAKER,this,NULL,this->agency()->logic_unit(),0,TRANS_CHAT_LOAD_LOUDSPEAKER);
}

int LoudSpeaker::save_record(void)
{
	if(!(Time_Value::gettimeofday().sec()-this->last_save_.sec()>=SAVE_INTERVAL
		&&	this->data_change_ ) || this->record_list_.size()<=0)
	{
		return 0;
	}
	this->last_save_ = Time_Value::gettimeofday();
	this->data_change_ = false;

	RecordList::iterator iter = this->record_list_.begin();
	for(;iter!=this->record_list_.end();++iter)
	{
		ChatRecord *tmp = this->pop_record();
		tmp->copy(*iter);
		this->record_list_bak_.push_back(tmp);
	}
	return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),TRANS_CHAT_SAVE_LOUDSPEAKER,
			DB_CHAT_LOUDSPEAKER,this,NULL);
}
