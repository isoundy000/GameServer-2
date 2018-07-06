/*
 * MediaGiftConfig.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: root
 */

#include "MediaGiftConfig.h"
#include "BackMediaGift.h"
#include "MapMonitor.h"

MediaGiftConfig::UpdateConfigTimer::UpdateConfigTimer(void):
	parent_(0)
{ /*NULL*/ }

MediaGiftConfig::UpdateConfigTimer::~UpdateConfigTimer(void)
{ /*NULL*/ }

void MediaGiftConfig::UpdateConfigTimer::set_parent(MediaGiftConfig *parent)
{
    this->parent_ = parent;
}

int MediaGiftConfig::UpdateConfigTimer::type(void)
{
    return GTT_ML_ONE_MINUTE;
}

int MediaGiftConfig::UpdateConfigTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->parent_ != 0, 0);
    JUDGE_RETURN(MAP_MONITOR->is_has_travel_scene() == false, 0);
    this->parent_->update_gift_config();
    return 0;
}

MediaGiftConfig::MediaGiftConfig():
		last_update_tick_(0)
{
	// TODO Auto-generated constructor stub

	this->update_timer_.set_parent(this);
	this->gift_map_.clear();
}

MediaGiftConfig::~MediaGiftConfig() {
	// TODO Auto-generated destructor stub
}

void MediaGiftConfig::media_gift_start(void)
{
	this->update_gift_config();
	this->update_timer_.schedule_timer(Time_Value(Time_Value::MINUTE));
	MSG_USER("MediaGiftConfig...");
}

void MediaGiftConfig::media_gift_end(void)
{
	this->update_timer_.cancel_timer();
}

void MediaGiftConfig::update_gift_config(void)
{
	BackMediaGift::update_gift_config(this->gift_map_, this->last_update_tick_);
}

int MediaGiftConfig::fetch_gift_config(int gift_sort, MediaGiftDef* &gift_conf)
{
	JUDGE_RETURN(this->gift_map_.count(gift_sort) > 0, -1);
	gift_conf = &(this->gift_map_[gift_sort]);

	return 0;
}

const MediaGiftDefMap& MediaGiftConfig::media_gift_def_map(void)
{
	return this->gift_map_;
}
