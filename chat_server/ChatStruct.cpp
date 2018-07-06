/*
 * ChatStruct.cpp
 *
 * Created on: 2013-01-18 14:21
 *     Author: glendy
 */

#include "ChatStruct.h"
#include "ChatMonitor.h"

void ChatRoleDetail::reset(void)
{
	this->__forbid_type = 0;
	this->__forbid_time = 0;
	this->__sign_in = 0;
	this->__last_sign_out = 0;
	this->__server_flag.clear();
	this->__server_prev.clear();
	this->__server_name.clear();
	this->__travel_area_id = 0;
	this->__league_name.clear();
	BaseRoleInfo::reset();
}

ChatLimit::ChatLimit(void)
{
	ChatLimit::reset();
}

void ChatLimit::reset(void)
{
	this->channel_type_ = 0;
	this->limit_level_ 	= 0;
	this->chat_interval_= 0;
	this->update_tick_ 	= 0;
}

WordCheck::WordCheck(void)
{
	WordCheck::reset();
}

void WordCheck::reset(void)
{
	this->words_.clear();
	this->update_tick_=0;
}

VipLimit::VipTimes::VipTimes()
{
	VipTimes::reset();
}

void VipLimit::VipTimes::reset()
{
	this->vip_lv_ = 0;
	this->channel_times_map_.clear();
}

VipLimit::VipLimit()
{
	VipLimit::reset();
}

void VipLimit::reset()
{
	this->update_tick_ = 0;
	this->time_ = 0;
	this->vip_map_.clear();
}

VipLimit::VipTimes *VipLimit::fetch_vip_times(int vip_lv)
{
	JUDGE_RETURN(this->vip_map_.count(vip_lv) > 0, NULL);
	return &(this->vip_map_[vip_lv]);
}

ChatTimes::ChatTimes(void)
{
	ChatTimes::reset();
}

void ChatTimes::reset(void)
{
	this->role_id_ = 0;
	this->channel_times_.clear();
}

ChatRecord::ChatRecord(void) :
    __src_role_id(0), __dst_role_id(0), __time(0), __type(0),
    __voice_id(0), __voice_len(0), __sid(0)
{
	this->__buffer = CHAT_MONITOR->pop_block();
}
ChatRecord::~ChatRecord(void)
{
	CHAT_MONITOR->push_block(this->__buffer);
}

void ChatRecord::reset(void)
{
	this->__src_role_id = 0;
	this->__dst_role_id = 0;
	this->__time = 0;
	this->__type = 1;
	this->__sid = 0;
//	this->__is_offline = 0;
	this->__voice_id = 0;
	this->__voice_len = 0;
	this->__buffer->reset();
}

void ChatRecord::copy(ChatRecord *record)
{
	this->__src_role_id = record->__src_role_id;
	this->__dst_role_id = record->__dst_role_id;
	this->__time = record->__time;
	this->__type = record->__type;
	this->__voice_len = record->__voice_len;
	this->__voice_id = record->__voice_id;
	this->__sid = record->__sid;
//	this->__is_offline = record->__sid;
	this->__buffer->copy(record->__buffer);
}

FlauntRecord::FlauntRecord(void) : 
    __flaunt_id(0), __flaunt_type(0), __len(0)
{
	this->__buffer = CHAT_MONITOR->pop_block();
}

FlauntRecord::~FlauntRecord(void)
{
	CHAT_MONITOR->push_block(this->__buffer);
}

void FlauntRecord::reset()
{
	this->__flaunt_id = 0;
	this->__flaunt_type = 0;
	this->__buffer->reset();
}
