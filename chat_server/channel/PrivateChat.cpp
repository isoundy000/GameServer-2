/*
 * PrivateChat.cpp
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#include "PrivateChat.h"
#include "ChannelAgency.h"
#include "TransactionMonitor.h"
#include "ChatMonitor.h"

#define MAX_OFFLINE_RECORD 10
#define MAX_RECORD_RETURN_PER_TIMES 10
#define SAVE_INTERVAL 60*1

PrivateChat::PrivateChat()
{
	reset();
}

PrivateChat::~PrivateChat()
{

}

void PrivateChat::reset(void)
{
	BaseChannel::reset();
//	this->offline_record_.clear();
//	this->offline_record_bak_.clear();
	this->data_change_ = false;
	this->is_supend_ = false;
	this->black_map_.clear();
	this->friend_map_.clear();
	this->last_save_ = Time_Value::gettimeofday();

	BaseChannel::reset();
}

int PrivateChat::init(ChannelAgency *agency,int64_t channel_id,bool offline)
{
	BaseChannel::init(agency,channel_id);
//	this->offline_record_.clear();

	if(offline)
	{
		this->is_supend_ = true;
	}
	return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),TRANS_CHAT_LOAD_PRIVATE_HISTORY,
			DB_CHAT_PRIVATE,this,NULL,this->agency()->logic_unit(),0,TRANS_CHAT_LOAD_PRIVATE_HISTORY);
}

int PrivateChat::stop(void)
{
//	this->cancel_timer();
//
//	if (NULL != this->agency())
//	{
//		RecordList::iterator iter = this->record_list_.begin();
//		for (; iter != this->record_list_.end(); ++iter)
//		{
//			this->push_record(*iter);
//		}
//	}
//	this->is_supend_ = true;
	this->last_save_-= Time_Value(SAVE_INTERVAL);
	this->data_change_ = true;
	this->save_record();
	return BaseChannel::stop();
}

int PrivateChat::suspend(void)
{
	this->cancel_timer();

	if (NULL != this->agency())
	{
		RecordList::iterator iter = this->record_list_.begin();
		for (; iter != this->record_list_.end(); ++iter)
		{
			this->push_record(*iter);
		}
	}
	this->is_supend_ = true;
	return 0;
}

int PrivateChat::restart(void)
{
	if(this->is_supend_==false)
	{
		return 0;
	}
	this->is_supend_ = false;
	if (this->interval_tick_ == Time_Value::zero)
		this->interval_tick_ = Time_Value(0, 0.5*1000000);
	return this->schedule_timer(this->interval_tick_);
}

int PrivateChat::channel_type(void)
{
	return CHANNEL_PRIVATE;
}

bool PrivateChat::is_suspend(void)
{
	return this->is_supend_;
}

int PrivateChat::notify(void)
{
	if(this->is_suspend()==false)
	{
		RecordList::iterator iter_r = this->record_list_.begin();
		for(;iter_r!=this->record_list_.end();++iter_r)
		{
			this->agency()->channel_notify(this->channel_type(),*iter_r);
//			this->push_history((*iter_r)->__dst_role_id,*iter_r);
		}
		this->record_list_.clear();
	}
	this->save_record();
	return 0;
}

int PrivateChat::notify_offline(void)
{
	this->import_offline_record();

	if(this->record_map_.size() > 0)
	{
		RecordMap::iterator iter = this->record_map_.begin();
		for(; iter != this->record_map_.end(); ++iter)
		{
			RecordList& list = iter->second;
			RecordList::reverse_iterator iter_l = list.rbegin();
			for(; iter_l != list.rend(); ++iter_l)
			{
				this->agency()->channel_notify(this->channel_type(),*iter_l,true);
	//		    this->push_history((*iter_l)->__dst_role_id,*iter_l);

				this->push_record(*iter_l);
			}
			list.clear();
		}
		this->record_map_.clear();
	}

	return 0;
}

int PrivateChat::recv_record(ChatRecord* record)
{
	if(this->is_in_black_list(record->__src_role_id))
	{
		return 0;
	}
	this->push_history(record->__src_role_id, record);
	return 0;
}

int PrivateChat::push_history(int64_t role_id,ChatRecord *record)
{
	RecordList& list = this->record_map_[role_id];
	if (list.size() >= MAX_OFFLINE_RECORD)
	{
		ChatRecord *back = list.back();
		this->push_record(back);
		list.pop_back();
	}
	list.push_front(record);//新的记录放在最前
	this->data_change_ = true;
	return 0;
}

int PrivateChat::clear_history(void)
{
	RecordMap::iterator iter = this->record_map_.begin();
	for(; iter != this->record_map_.end(); ++iter)
	{
		RecordList& list = iter->second;
		RecordList::reverse_iterator iter_l = list.rbegin();
		for(; iter_l != list.rend(); ++iter_l)
		{
//			this->agency()->channel_notify(this->channel_type(),*iter_l,true);
//		    this->push_history((*iter_l)->__dst_role_id,*iter_l);

			this->push_record(*iter_l);
		}
		list.clear();
	}
	this->record_map_.clear();
	return 0;
}

//将从数据库加载上来的数据合并
void PrivateChat::import_offline_record(void)
{
	//将从数据库加载上来的数据合并
	if(this->record_load_list_.size()>0)
	{
		RecordList::iterator iter = this->record_load_list_.begin();
		for(;iter!=this->record_load_list_.end();iter++)
		{
			ChatRecord* r  = *iter;
			if(r->__type==R_VOICE_SAVE)
			{
				this->push_voice(r,true);
				continue;
			}
			int64_t key = r->__src_role_id==this->channel_id()?r->__dst_role_id:r->__src_role_id;
			RecordList& list_import = this->record_map_[key];

			if (list_import.size() >= MAX_OFFLINE_RECORD)
			{
				this->push_record(r);
				break;
			}
			list_import.push_back(r);//导入历史记录放在最后
		}
		this->record_load_list_.clear();
	}
}

int PrivateChat::load_record(ChatRecord* record)
{
	if(this->is_in_black_list(record->__src_role_id))
	{
		this->push_record(record);
		return 0;
	}
	this->record_load_list_.push_back(record);
	return 0;
}

int PrivateChat::save_record(void)
{
	if (Time_Value::gettimeofday().sec() - this->last_save_.sec()>= SAVE_INTERVAL
			&& this->data_change_)
	{
		MSG_USER("###-now time:%ld-time:%ld-bool:%d-&&&",
				Time_Value::gettimeofday().sec(), this->last_save_.sec(), this->data_change_);

		this->last_save_ = Time_Value::gettimeofday();
		this->data_change_ = false;

		this->record_save_list_.clear();
		RecordMap::iterator iter = this->record_map_.begin();
		for(;iter!=this->record_map_.end();iter++)
		{
			RecordList& list = iter->second;
			RecordList::iterator iter_list = list.begin();
			for(;iter_list!=list.end();++iter_list)
			{
				ChatRecord *bak = this->pop_record();
				bak->copy(*iter_list);
				this->record_save_list_.push_back(bak);
				MSG_USER("###-record_map_first:%ld-time:%ld-pop_record addr:%x--src_record addr:%x&&&", iter->first,
						this->last_save_.sec(), bak, *iter_list);
			}
		}
		//语音内容
		for(RecordList::iterator iter=this->voice_list_.begin();
				iter!=this->voice_list_.end();iter++)
		{
			ChatRecord *bak = this->pop_record();
			bak->copy(*iter);
			this->record_save_list_.push_back(bak);
			MSG_USER("###-voice_list_size:%d-time:%ld-chatrecord addr:%x-&&&", this->voice_list_.size(),
					this->last_save_.sec(), bak);
		}
		return TRANSACTION_MONITOR->request_mongo_transaction(this->channel_id(),
				TRANS_CHAT_SAVE_PRIVATE_HISTORY,DB_CHAT_PRIVATE,this,NULL);
	}
	return 0;
}

int PrivateChat::add_black_list(int64_t role_id)
{
	this->black_map_[role_id]=role_id;
	return 0;
}

int PrivateChat::remove_black_list(int64_t role_id)
{
	return this->black_map_.erase(role_id);
}

bool PrivateChat::is_in_black_list(int64_t role_id)
{
	return this->black_map_.count(role_id) > 0;
}

int PrivateChat::add_friend_list(Int64 role_id)
{
	this->friend_map_[role_id] = role_id;
	return 0;
}

int PrivateChat::remove_friend_list(Int64 role_id)
{
	return this->friend_map_.erase(role_id);
}

bool PrivateChat::is_in_friend_list(Int64 role_id)
{
	IntMap::iterator iter = this->friend_map_.find(role_id);
	if(iter!=this->friend_map_.end())
	{
		return true;
	}
	return false;
}

int PrivateChat::get_history(RecordList& list,int64_t role_id,int time_offset)
{
	RecordMap::iterator iter = this->record_map_.find(role_id);
	if (iter != this->record_map_.end())
	{
		RecordList& contian_list = iter->second;
		RecordList::iterator iter_list = contian_list.begin();
		int count = 0;
		for (; iter_list != contian_list.end() && count <= MAX_RECORD_RETURN_PER_TIMES; ++iter_list)
		{
			ChatRecord* r = (*iter_list);
			if((time_offset > 0 && r->__time >= time_offset) ||
					this->is_in_black_list(r->__src_role_id))
			{
				continue;
			}

			count++;
			list.push_front(*iter_list);
		}
	}
	return 0;
}

int PrivateChat::push_voice(ChatRecord* record,bool history)
{
	if (voice_list_.size() >= MAX_OFFLINE_RECORD)
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
	this->data_change_ = true;
	return 0;
}

ChatRecord *PrivateChat::get_voice(int64_t id)
{
	for(RecordList::iterator iter = this->voice_list_.begin();
			iter!=this->voice_list_.end();iter++)
	{
		if((*iter)->__voice_id==id)
		{
			if(this->is_in_black_list((*iter)->__src_role_id)==false)
			{
				return *iter;
			}
			break;
		}
	}
	return NULL;
}
