/*
 * DailyActSys.cpp
 *
 *  Created on: 2017年3月21日
 *      Author: lyw
 */

#include "DailyActSys.h"
#include "MMOLuckyWheel.h"
#include "GameCommon.h"
#include "BackField.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"
#include "ScriptStruct.h"

#include <mongo/client/dbclient.h>

int DailyActSys::BackActivityTimer::type()
{
	return GTT_LOGIC_ONE_MINUTE;
}

int DailyActSys::BackActivityTimer::handle_timeout(const Time_Value &tv)
{
	DAILY_ACT_SYSTEM->request_load_activity();
	DAILY_ACT_SYSTEM->total_double_update_flag();
	return 0;
}

DailyActSys::DailyActSys() {
	// TODO Auto-generated constructor stub
	DailyActSys::reset();
}

DailyActSys::~DailyActSys() {
	// TODO Auto-generated destructor stub
}

void DailyActSys::reset()
{
	this->has_update_ = 0;
	this->act_map_.clear();
	this->act_timer_.cancel_timer();
}

void DailyActSys::start()
{
	MMOLuckyWheel::load_daily_act_system(this);
	this->request_load_activity();
	this->update_activity_list();

	this->act_timer_.schedule_timer(1);
	MSG_USER("DailyActSys start");
}

void DailyActSys::stop()
{
	this->save_activity();
	this->act_timer_.cancel_timer();
}

int DailyActSys::request_load_activity()
{
//	return LOGIC_MONITOR->db_load_mode_begin(TRANS_LOAD_DAILY_ACT);
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, 0);

	shop_mode->recogn_ = TRANS_LOAD_LUCKY_WHEEL;
	shop_mode->sub_value_ = 2;
	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
}

int DailyActSys::after_load_activity_done(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL, -1);

	IntMap type_map;

	for (BSONVec::iterator iter = shop_mode->output_argv_.bson_vec_->begin();
			iter != shop_mode->output_argv_.bson_vec_->end(); ++iter)
	{
		BSONObj res = *iter;
		JUDGE_CONTINUE(res.isEmpty() == false);

		int act_type = res[DBBackWonderfulActivity::ACTIVITY_TYPE].numberInt();
		type_map[act_type] = true;
		DailyDetail* daily_detail = this->fetch_daily_detail_by_type(act_type);
		if (daily_detail == NULL)
		{
			DailyDetail &new_detail = this->daily_detail_map_[act_type];
			this->add_new_item(new_detail, res);
		}
		else
		{
			this->update_item(daily_detail, res);
		}
	}

	int res1 = this->check_update_activity();
	int res2 = this->check_delete_activity(type_map);
	JUDGE_RETURN(res1 == true || res2 == true, 0);

	this->update_activity_list();

	return 0;
}

int DailyActSys::check_update_activity()
{
	int update_flag = false;
	for (DailyDetailMap::iterator iter = this->daily_detail_map_.begin();
			iter != this->daily_detail_map_.end(); ++iter)
	{
		DailyDetail &daily_detail = iter->second;
		if (daily_detail.refresh_tick_ != daily_detail.save_refresh_tick_)
		{
			daily_detail.new_act_reset();
			update_flag = true;
		}
	}

	return update_flag;
}

int DailyActSys::check_delete_activity(IntMap &type_map)
{
	IntVec mark_set;
	for (DailyDetailMap::iterator iter = this->daily_detail_map_.begin();
			iter != this->daily_detail_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(type_map.count(iter->first) <= 0);
		mark_set.push_back(iter->first);
	}
	JUDGE_RETURN(mark_set.size() > 0, false);

	for (IntVec::iterator iter = mark_set.begin(); iter != mark_set.end(); ++iter)
	{
		this->daily_detail_map_.erase(*iter);
	}

	return true;
}

int DailyActSys::update_activity_list()
{
	this->act_map_.clear();
	for (DailyDetailMap::iterator iter = this->daily_detail_map_.begin();
			iter != this->daily_detail_map_.end(); ++iter)
	{
		DailyDetail &daily_detail = iter->second;
		JUDGE_CONTINUE(daily_detail.is_activity_time(true) == true);

		this->act_map_[daily_detail.activity_id_] = true;
	}
	MSG_USER("act_map_size: %d", this->act_map_.size());

	//进行全服玩家更新
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::const_iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->fetch_daily_act_list();
	}

	this->update_total_double();

	return 0;
}

int DailyActSys::midnight_handle_timeout()
{
	this->update_activity_list();

	return 0;
}

IntMap& DailyActSys::fetch_all_activity()
{
	return this->act_map_;
}

void DailyActSys::save_activity()
{
	MMOLuckyWheel::save_daily_act_system(this);
}

int DailyActSys::total_double_update_flag()
{
	JUDGE_RETURN(this->has_update_ == false, 0);

	this->update_total_double();
	return 0;
}

int DailyActSys::check_total_double_is_update(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30400917 *, request, msg, -1);

	this->has_update_ = true;
	return 0;
}

void DailyActSys::update_total_double()
{
	Proto30400917 inner;

	DailyDetail *daily_detail = this->fetch_daily_detail(ACT_TOTAL_DOUBLE);
	if (daily_detail != NULL)
	{
		int script_type = this->fetch_script_type(daily_detail->value1_);
		if (script_type != 0)
			inner.add_value_set(script_type);
		script_type = this->fetch_script_type(daily_detail->value2_);
		if (script_type != 0)
			inner.add_value_set(script_type);
	}

	LOGIC_MONITOR->dispatch_to_scene(GameEnum::LEGEND_TOP_SCENE, &inner);
}

int DailyActSys::fetch_script_type(int value)
{
	switch (value)
	{
	case DOUBLE_ADVANCE:
	{
		return GameEnum::SCRIPT_T_ADVANCE;
	}
	case DOUBLE_RAMA:
	{
		return GameEnum::SCRIPT_T_RAMA;
	}
	case DOUBLE_STORY:
	{
		return GameEnum::SCRIPT_T_STORY;
	}
	case DOUBLE_SWORD:
	{
		return GameEnum::SCRIPT_T_SWORD_TOP;
	}
	case DOUBLE_COUPLE:
	{
		return GameEnum::SCRIPT_T_COUPLES;
	}
	default:
		break;
	}

	return 0;
}

