/*
 * MLHiddenTreasure.cpp
 *
 *  Created on: 2016年12月6日
 *      Author: lyw
 */

#include "MLHiddenTreasure.h"
#include "ProtoDefine.h"

MLHiddenTreasure::MLHiddenTreasure() {
	// TODO Auto-generated constructor stub

}

MLHiddenTreasure::~MLHiddenTreasure() {
	// TODO Auto-generated destructor stub
}

void MLHiddenTreasure::hi_treasure_send_mail(Int64 refresh_tick)
{
	Int64 cur_time = ::time(NULL);
	IntVec reward_set;
	int mail_id = 0;
	int interval_day = GameCommon::day_interval(cur_time, refresh_tick) + 1;
	for (int i = 0; i < interval_day; ++i)
	{
		int day = this->hi_treasure_.day_ + i;
		const Json::Value& treasure_json = CONFIG_INSTANCE->hidden_treasure_json(day);
		JUDGE_CONTINUE(treasure_json != Json::Value::null);

		int reward_id = treasure_json["reward_id"].asInt();
		if ((day == this->hi_treasure_.day_ && this->hi_treasure_.get_status_ == false)
				|| day > this->hi_treasure_.day_)
		{
			reward_set.push_back(reward_id);
			mail_id = treasure_json["mail_id"].asInt();
		}
	}
	JUDGE_RETURN(mail_id > 0 && reward_set.size() > 0, ;);
	{
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), int(reward_set.size()));
		for (IntVec::iterator iter = reward_set.begin(); iter != reward_set.end(); ++iter)
		{
			mail_info->add_goods(*iter);
		}
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}

}

void MLHiddenTreasure::reset(void)
{
	this->hi_treasure_.reset();
}

void MLHiddenTreasure::active_hi_treasure()
{
	JUDGE_RETURN(this->hi_treasure_.open_ == NOT_OPEN, ;);

	this->hi_treasure_.open_ = HAS_OPEN;
	this->hi_treasure_.set_next_day_info();
	this->fetch_hi_treasure_info();
}

void MLHiddenTreasure::hi_treasure_next_day()
{
	JUDGE_RETURN(this->hi_treasure_.open_ == HAS_OPEN, ;);

	Int64 cur_time = ::time(NULL);
	if (this->hi_treasure_.refresh_tick_ <= cur_time)
	{
		this->hi_treasure_send_mail(this->hi_treasure_.refresh_tick_);
		this->hi_treasure_.set_next_day_info();
		this->fetch_hi_treasure_info();
	}
}

int MLHiddenTreasure::fetch_hi_treasure_info()
{
	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();

	Proto51401501 respond;
	respond.set_open(hi_treasure.open_);

	if (hi_treasure.open_ == HAS_OPEN)
	{
		respond.set_day(hi_treasure.day_);
		respond.set_get_status(hi_treasure.get_status_);

		int refresh = GameCommon::next_day();
		respond.set_refresh(refresh);

		for (IntMap::iterator iter = hi_treasure.buy_map_.begin();
				iter != hi_treasure.buy_map_.end(); ++iter)
		{
			ProtoPairObj* buy_status = respond.add_buy_status();
			buy_status->set_obj_id(iter->first);
			buy_status->set_obj_value(iter->second);
		}
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_HIDDEN_TREASURE_INFO, &respond);
}

int MLHiddenTreasure::fetch_hi_treasure_reward()
{
	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();
	CONDITION_NOTIFY_RETURN(hi_treasure.open_ == HAS_OPEN, RETURN_FETCH_HIDDEN_TREASURE_REWARD,
			ERROR_HIDDEN_TREASURE_NOT_OPEN);
	CONDITION_NOTIFY_RETURN(hi_treasure.get_status_ == false,
			RETURN_FETCH_HIDDEN_TREASURE_REWARD, ERROR_HAS_GET_REWARD);

	const Json::Value& treasure_json = CONFIG_INSTANCE->hidden_treasure_json(hi_treasure.day_);
	CONDITION_NOTIFY_RETURN(treasure_json != Json::Value::null,
			RETURN_FETCH_HIDDEN_TREASURE_REWARD, ERROR_CONFIG_NOT_EXIST);

	int reward_id = treasure_json["reward_id"].asInt();
	SerialObj obj(ADD_FROM_HIDDEN_TREASURE_GET, hi_treasure.day_);
	this->add_reward(reward_id, obj);

	hi_treasure.get_status_ = true;
	this->fetch_hi_treasure_info();

	FINER_PROCESS_NOTIFY(RETURN_FETCH_HIDDEN_TREASURE_REWARD);
}

int MLHiddenTreasure::buy_hi_treasure_item(Message *msg)
{
	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();
	CONDITION_NOTIFY_RETURN(hi_treasure.open_ == HAS_OPEN, RETURN_HIDDEN_TREASURE_BUY,
			ERROR_HIDDEN_TREASURE_NOT_OPEN);

	DYNAMIC_CAST_RETURN(Proto11401503 *, request, msg, -1);

	int buy_id = request->map_id();
	CONDITION_NOTIFY_RETURN(hi_treasure.buy_map_.count(buy_id) > 0,
			RETURN_HIDDEN_TREASURE_BUY, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(hi_treasure.buy_map_[buy_id] == false,
			RETURN_HIDDEN_TREASURE_BUY, ERROR_TREASURE_BUY_LIMIT);

	const Json::Value& treasure_json = CONFIG_INSTANCE->hidden_treasure_json(hi_treasure.day_);
	CONDITION_NOTIFY_RETURN(treasure_json != Json::Value::null,
			RETURN_HIDDEN_TREASURE_BUY, ERROR_CONFIG_NOT_EXIST);

	const Json::Value& cost_item = treasure_json["cost_item"];
	int item_id   = cost_item[buy_id-1][0u].asInt();
	int amount    = cost_item[buy_id-1][1u].asInt();
	int item_bind = cost_item[buy_id-1][2u].asInt();
	int pay_mon   = cost_item[buy_id-1][3u].asInt();

	Money money(pay_mon);
	GameCommon::adjust_money(money, this->own_money());
	SerialObj obj1(SUB_MONEY_HIDDEN_TREASURE_BUY, hi_treasure.day_, buy_id);
	int ret = this->pack_money_sub(money, obj1);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_HIDDEN_TREASURE_BUY, ret);

	SerialObj obj2(ADD_FROM_HIDDEN_TREASURE_GET, hi_treasure.day_, buy_id);
	this->insert_package(obj2, item_id, amount, item_bind);

	hi_treasure.buy_map_[buy_id] = true;
	this->fetch_hi_treasure_info();

	FINER_PROCESS_NOTIFY(RETURN_HIDDEN_TREASURE_BUY);
}

int MLHiddenTreasure::sync_transfer_hi_treasure(int scene_id)
{
	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();
	JUDGE_RETURN(hi_treasure.open_ >= HAS_OPEN, 0);

	Proto31400145 sync_info;
	sync_info.set_day(hi_treasure.day_);
	sync_info.set_open(hi_treasure.open_);
	sync_info.set_get_status(hi_treasure.get_status_);
	sync_info.set_refresh_tick(hi_treasure.refresh_tick_);

	for (IntMap::iterator iter = hi_treasure.buy_map_.begin();
			iter != hi_treasure.buy_map_.end(); ++iter)
	{
		ProtoPairObj* buy_status = sync_info.add_buy_map();
		buy_status->set_obj_id(iter->first);
		buy_status->set_obj_value(iter->second);
	}

	return this->send_to_other_logic_thread(scene_id, sync_info);
}

int MLHiddenTreasure::read_transfer_hi_treasure(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400145 *, request, msg, -1);

	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();
	hi_treasure.day_ = request->day();
	hi_treasure.open_ = request->open();
	hi_treasure.get_status_ = request->get_status();
	hi_treasure.refresh_tick_ = request->refresh_tick();

	for (int i = 0; i < request->buy_map_size(); ++i)
	{
		const ProtoPairObj &buy_status = request->buy_map(i);
		hi_treasure.buy_map_[buy_status.obj_id()] = buy_status.obj_value();
	}

	return 0;
}

HiddenTreasureDetail& MLHiddenTreasure::hi_treasure_detail()
{
	return this->hi_treasure_;
}

void MLHiddenTreasure::test_hi_treasure_reset()
{
	HiddenTreasureDetail& hi_treasure = this->hi_treasure_detail();
	hi_treasure.reset();
	hi_treasure.open_ = HAS_OPEN;
	hi_treasure.set_next_day_info();
	this->fetch_hi_treasure_info();
}

void MLHiddenTreasure::test_set_hi_treasure_day(int day)
{
	JUDGE_RETURN(day > 0, ;);

	this->hi_treasure_.refresh_tick_ -= day * 86400;
	this->hi_treasure_next_day();
	this->fetch_hi_treasure_info();
}
