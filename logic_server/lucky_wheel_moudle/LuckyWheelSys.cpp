/*
 * LuckyWheelSys.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: lyw
 */

#include "LuckyWheelSys.h"
#include "MMOLuckyWheel.h"
#include "GameCommon.h"
#include "BackField.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"
#include "JYBackActivitySys.h"
#include "TrvlRechargeMonitor.h"

#include <mongo/client/dbclient.h>

int LuckyWheelSys::BackActivityTimer::type()
{
	return GTT_LOGIC_ONE_MINUTE;
}

int LuckyWheelSys::BackActivityTimer::handle_timeout(const Time_Value &tv)
{
	LUCKY_WHEEL_SYSTEM->request_load_activity();
	return 0;
}


int LuckyWheelSys::ShopRefreshTimer::type()
{
	return GTT_LOGIC_ONE_SEC;
}

int LuckyWheelSys::ShopRefreshTimer::handle_timeout(const Time_Value &tv)
{
	LUCKY_WHEEL_SYSTEM->refresh_cabinet_info(LuckyWheelActivity::ACT_CABINET);
	LUCKY_WHEEL_SYSTEM->refresh_cabinet_info(LuckyWheelActivity::ACT_CABINET_DISCOUNT);
	return 0;
}

void LuckyWheelSys::ShopRefreshTimer::set_act_type(int act_type)
{
	this->act_type_ = act_type;
}


LuckyWheelSys::BrocastTimer::BrocastTimer()
{
	this->state_ = 0;
}

int LuckyWheelSys::BrocastTimer::type()
{
	return GTT_LOGIC_ONE_MINUTE;
}

int LuckyWheelSys::BrocastTimer::handle_timeout(const Time_Value &tv)
{
	switch(this->get_state())
	{
	case STATE_BROCAST:
	{
		LUCKY_WHEEL_SYSTEM->handle_limit_buy_shout();
		break;
	}
	case STATE_FLICKER:
	{
		LUCKY_WHEEL_SYSTEM->handle_limit_buy_flicker();
		break;
	}
	case STATE_END:
	{
		break;
	}
	default:
		break;
	}

	LUCKY_WHEEL_SYSTEM->start_limit_buy_brocast();

	return 0;
}

void LuckyWheelSys::BrocastTimer::set_state(int state)
{
	this->state_ = state;
}

int LuckyWheelSys::BrocastTimer::get_state()
{
	return this->state_;
}

LuckyWheelSys::LuckyWheelSys()
{
	LuckyWheelSys::reset();
}

LuckyWheelSys::~LuckyWheelSys() {
	// TODO Auto-generated destructor stub
}

void LuckyWheelSys::reset()
{
	this->act_map_.clear();
	this->act_timer_.cancel_timer();
	this->brocast_timer_.cancel_timer();
}

void LuckyWheelSys::activity_brocast(int shout_id, ServerItemInfo &item_info)
{
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, item_info.player_name_);
	GameCommon::push_brocast_para_item_info(para_vec, item_info.item_id_,
			item_info.item_bind_, item_info.amount_);
	GameCommon::announce(shout_id, &para_vec);
}

int LuckyWheelSys::start(void)
{
	MMOLuckyWheel::load_lucky_wheel_system(this);
	this->request_load_activity();
	this->check_combine_reset_act();
	this->update_activity_list();
	this->act_timer_.schedule_timer(1);

	MSG_USER("LuckyWheelSys start, activity_num: %d", this->act_map_.size());

	return 0;
}

int LuckyWheelSys::stop(void)
{
//	MMOLuckyWheel::save_lucky_wheel_system(this);
	this->save_activity();

	this->act_timer_.cancel_timer();
	this->brocast_timer_.cancel_timer();

	MSG_USER("LuckyWheelSys stop!");
	return 0;
}

int LuckyWheelSys::midnight_handle_timeout()
{
	this->update_activity_list();
	this->reset_every_day();
	this->start_limit_buy_brocast();

	return 0;
}

int LuckyWheelSys::reset_every_day()
{
	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		ActivityDetail &act_detail = iter->second;
		this->act_end_send_mail(&act_detail);
		this->rand_get_slot_map(&act_detail);
		this->act_reset_every_day(&act_detail);
	}

	return 0;
}

int LuckyWheelSys::request_load_activity()
{
//	return LOGIC_MONITOR->db_load_mode_begin(TRANS_LOAD_LUCKY_WHEEL);
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, 0);

	shop_mode->recogn_ = TRANS_LOAD_LUCKY_WHEEL;
	shop_mode->sub_value_ = 1;
	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
//	return 0;
}

int LuckyWheelSys::after_load_activity_done(DBShopMode* shop_mode)
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

		LuckyWheelActivity::ActivityDetail* act_detail = this->fetch_act_detail_by_type(act_type);
		if (act_detail == NULL)
		{
			LuckyWheelActivity::ActivityDetail &new_detail = this->act_detail_map_[act_type];
			this->add_new_item(new_detail, res);
		}
		else
		{
			this->update_item(act_detail, res);
		}
	}

	MSG_USER("after_load_activity_done, size : %d", type_map.size());

	int res1 = this->check_update_activity();
	int res2 = this->check_delete_activity(type_map);
	this->start_limit_buy_brocast();
	JUDGE_RETURN(res1 == true || res2 == true, 0);

	MSG_USER("after_load_activity_done, update_activity_list!");
	this->update_activity_list();
//	this->init_shop_timer();
	if(this->get_refresh_timer(LuckyWheelActivity::ACT_CABINET).is_registered() == false)
		this->refresh_cabinet_info(LuckyWheelActivity::ACT_CABINET);

	if(this->get_refresh_timer(LuckyWheelActivity::ACT_CABINET_DISCOUNT).is_registered() == false)
		this->refresh_cabinet_info(LuckyWheelActivity::ACT_CABINET_DISCOUNT);
	return 0;
}

int LuckyWheelSys::check_update_activity()
{
	int update_flag = false;
	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		ActivityDetail &act_detail = iter->second;
		int date_type = act_detail.back_date_type_;
		Int64 first_date = act_detail.back_first_date_;
		Int64 last_date = act_detail.back_last_date_;
		if (this->is_activity_time(act_detail.activity_id_, date_type, first_date, last_date)
				!= this->is_activity_time(act_detail.activity_id_)
				|| act_detail.back_date_type_ != act_detail.save_date_type_)
		{
			act_detail.new_act_reset();
			update_flag = true;
			this->rand_get_slot_map(&act_detail);
		}
		else if (act_detail.save_first_date_ != first_date || act_detail.save_last_date_ != last_date)
		{
			act_detail.save_first_date_ = first_date;
			act_detail.save_last_date_  = last_date;
			update_flag = true;
		}
	}
	return update_flag;
}

int LuckyWheelSys::check_delete_activity(IntMap &type_map)
{
	IntVec mark_set;
	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(type_map.count(iter->first) <= 0);
		mark_set.push_back(iter->first);
	}
	JUDGE_RETURN(mark_set.size() > 0, false);

	for (IntVec::iterator iter = mark_set.begin(); iter != mark_set.end(); ++iter)
	{
		int activity_id = *iter;
		if (activity_id == LuckyWheelActivity::ACT_CABINET || activity_id == LuckyWheelActivity::ACT_CABINET_DISCOUNT)
			this->get_refresh_timer(activity_id).cancel_timer();

		this->act_detail_map_.erase(*iter);
	}

	return true;
}

int LuckyWheelSys::start_limit_buy_brocast()
{
	this->brocast_timer_.cancel_timer();

	int activity_id = ACT_LIMIT_BUY;
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, 0);
	JUDGE_RETURN(this->is_activity_time(act_detail) == true, 0);
	JUDGE_RETURN(this->fetch_next_time_point(act_detail) > 0, 0);

	int start_tick = this->fetch_start_tick(act_detail);
	if (act_detail->brocast_time_ > 0 && start_tick  >= act_detail->brocast_time_)
	{
		this->brocast_timer_.schedule_timer(start_tick - act_detail->brocast_time_);
		this->brocast_timer_.set_state(STATE_BROCAST);
	}
	else if (act_detail->flicker_time_ > 0)
	{
		if (start_tick >= act_detail->flicker_time_)
		{
			this->brocast_timer_.schedule_timer(start_tick - act_detail->flicker_time_);
			this->brocast_timer_.set_state(STATE_FLICKER);
		}
		else
		{
			this->brocast_timer_.schedule_timer(start_tick + 1); //延迟1秒
			this->brocast_timer_.set_state(STATE_END);
		}
	}

	return 0;
}

int LuckyWheelSys::handle_limit_buy_shout()
{
	int activity_id = ACT_LIMIT_BUY;
	ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, 0);

	GameCommon::announce(act_detail->act_shout_);
	return 0;
}

int LuckyWheelSys::handle_limit_buy_flicker()
{
	return 0;
}

int LuckyWheelSys::handle_flicker_end()
{
	return 0;
}

int LuckyWheelSys::is_need_cost(ActivityDetail* activity_detail)
{
	JUDGE_RETURN(activity_detail != NULL, false);
	JUDGE_RETURN(activity_detail->draw_cost_ > 0 || activity_detail->draw_type_list_.size() > 0, false);

	return true;
}

int LuckyWheelSys::add_pool_money(ActivityDetail* activity_detail, SlotInfo* slot_info)
{
	JUDGE_RETURN(activity_detail != NULL && slot_info != NULL, 0);
	JUDGE_RETURN(slot_info->pool_percent_ > 0, 0);

	int add_money = activity_detail->save_gold_ * slot_info->pool_percent_ / 100;
	activity_detail->save_gold_ -= add_money;
	activity_detail->save_gold_ = (activity_detail->save_gold_ >= activity_detail->low_gold_) ?
			activity_detail->save_gold_ : activity_detail->low_gold_;

	return add_money;
}

int LuckyWheelSys::draw_refresh_activity_info(ActivityDetail* act_detail, SlotInfo* slot_info,
			Int64 role_id, ItemObj& obj, int reward_mult)
{
	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, 0);
	JUDGE_RETURN(act_detail != NULL && slot_info != NULL, 0);

	act_detail->save_gold_ += act_detail->add_gold_;
	if (slot_info->server_record_ == true)
	{
		LuckyWheelActivity::ServerItemInfo item_info;
		item_info.player_id_ 	= player->role_id();
		item_info.player_name_ 	= player->name();
		item_info.get_time_ 	= ::time(NULL);
		item_info.item_bind_	= obj.bind_;
		item_info.amount_ 		= obj.amount_;
		item_info.item_id_ 		= obj.id_;
		item_info.reward_mult_  = reward_mult;
		act_detail->item_set_.push_back(item_info);

		// 全服记录排序
		std::sort(act_detail->item_set_.begin(), act_detail->item_set_.end(),
				LuckyWheelActivity::comp_by_time_desc);

		//全服播报
		if (slot_info->is_shout_ == true)
			this->activity_brocast(act_detail->shout_id_, item_info);
	}

	return 0;
}

int LuckyWheelSys::check_combine_reset_act()
{
	int combine_first = LOGIC_MONITOR->combine_first();
	JUDGE_RETURN(combine_first == true, 0);

	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		ActivityDetail &act_detail = iter->second;
		JUDGE_CONTINUE(act_detail.combine_reset_act_ == true);

		act_detail.combine_reset();
//		this->rand_get_slot_map(&act_detail);
	}

	return 0;
}

int LuckyWheelSys::update_activity_list()
{
	this->act_map_.clear();
	for (ActivityDetailMap::iterator iter = this->act_detail_map_.begin();
			iter != this->act_detail_map_.end(); ++iter)
	{
		ActivityDetail &act_detail = iter->second;

		JUDGE_CONTINUE(this->is_activity_time(&act_detail) == true);
		this->act_map_[act_detail.activity_id_] = true;
		MSG_USER("update_activity_list, activity_id: %d", act_detail.activity_id_);
	}

	//进行全服玩家更新
	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::const_iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->fetch_wheel_act_list();
	}

	return 0;
}

IntMap& LuckyWheelSys::fetch_all_activity()
{
	return this->act_map_;
}

void LuckyWheelSys::slot_info_serialize(ProtoSlotInfo* slot, SlotInfo* slot_info,
		WheelPlayerInfo::PlayerDetail* player_detail, int id)
{
	slot->set_index(slot_info->index_);
	slot->set_slot_id(slot_info->slot_id_);
	slot->set_is_precious(slot_info->is_precious_);
	slot->set_pre_cost(slot_info->pre_cost_);
	slot->set_now_cost(slot_info->now_cost_);
	slot->set_person_limit(slot_info->person_limit_);
	slot->set_server_limit(slot_info->server_limit_);
	slot->set_slot_type(slot_info->slot_type_);

	int server_left_limit = slot_info->server_limit_ - slot_info->server_buy_;
	slot->set_server_left_limit(server_left_limit);

	WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(0, slot_info->slot_id_);
	ProtoItem *item = slot->mutable_item();

	if (id > 0 && player_detail->slot_item_id_[id] > 0 && player_detail->slot_item_num_[id] > 0)
	{

		item->set_id(player_detail->slot_item_id_[id]);
		item->set_amount(player_detail->slot_item_num_[id]);
		if (player_detail->slot_item_id_[id] == player_detail->slot_item_num_[id])
		{
			item->set_bind(player_detail->slot_item_id_[id]);
		}
	}
	else
	{
		if (player_detail->activity_id_ == LuckyWheelActivity::ACT_LUCKY_EGG && person_slot != NULL)
		{
			person_slot->item_.serialize(item);
			slot->set_is_precious(person_slot->is_color_);
		}
		else
		{
			ItemObj item_obj = LUCKY_WHEEL_SYSTEM->fetch_slot_reward(slot_info, player_detail->wheel_times_);
			item_obj.serialize(item);
			slot->set_is_precious(slot_info->is_precious_);
		}
	}

	JUDGE_RETURN(person_slot != NULL, ;);

	for (IntMap::iterator iter = person_slot->nine_slot_.begin();
			iter != person_slot->nine_slot_.end(); ++iter)
	{
		ProtoPairObj *pair_obj = slot->add_pair_info();
		pair_obj->set_obj_id(iter->first);
		pair_obj->set_obj_value(iter->second);
	}
}

void LuckyWheelSys::test_set_activity(int activity_id, int date_type, int first_date, int last_date)
{
	ActivityDetail &act_detail = this->act_detail_map_[activity_id];
	act_detail.activity_id_ = activity_id;
	act_detail.limit_time_set_.clear();
	int res = this->fetch_act_base_conf(act_detail);
	JUDGE_RETURN(res == true, ;);

	JUDGE_RETURN(date_type == SERVER_DATE || date_type == REAL_DATE, ;);
	JUDGE_RETURN(first_date > 0 && last_date > 0, ;);

	act_detail.back_date_type_  = date_type;
	act_detail.save_date_type_  = date_type;
	act_detail.act_type_ = 1;
	act_detail.open_flag_ = true;
	act_detail.refresh_tick_ = ::time(NULL);

	if (date_type == SERVER_DATE)
	{
		act_detail.back_first_date_ = first_date;
		act_detail.back_last_date_  = last_date;
		act_detail.save_first_date_ = first_date;
		act_detail.save_last_date_  = last_date;
	}
	else
	{
		int year1 = first_date / 10000;
		int mon1  = (first_date % 10000) / 100;
		int day1  = (first_date % 10000) % 100;

		int year2 = last_date / 10000;
		int mon2  = (last_date % 10000) / 100;
		int day2  = (last_date % 10000) % 100;

		Time_Value first_time;
		Time_Value last_time;
		GameCommon::make_time_value(first_time, year1, mon1, day1, 0, 0);
		GameCommon::make_time_value(last_time, year2, mon2, day2, 0, 0);
		act_detail.back_first_date_ = first_time.sec();
		act_detail.back_last_date_  = last_time.sec();
		act_detail.save_first_date_ = first_time.sec();
		act_detail.save_last_date_  = last_time.sec();
	}
	act_detail.new_act_reset();

	this->update_activity_list();
}

void LuckyWheelSys::save_activity()
{
	MMOLuckyWheel::save_lucky_wheel_system(this);
}

LuckyWheelSys::ShopRefreshTimer& LuckyWheelSys::get_refresh_timer(int act_type)
{
	return this->shop_timer_[act_type];
}

//没有执行
int LuckyWheelSys::init_shop_timer(int activity_id)
{
	LuckyWheelActivity::ActivityDetail* act_detail = this->fetch_act_detail_by_type(activity_id);
	JUDGE_RETURN(act_detail != NULL, 0);
	JUDGE_RETURN(this->get_refresh_timer(activity_id).is_registered() == false, 0);

	this->get_refresh_timer(activity_id).set_act_type(activity_id);
	this->get_refresh_timer(activity_id).cancel_timer();
	this->get_refresh_timer(activity_id).schedule_timer(act_detail->shop_time_);

	return 0;
}

int LuckyWheelSys::refresh_cabinet_info(int activity_id)
{
	LuckyWheelActivity::ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, 0);

	LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
	for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		//player->cabinet_refresh_work(activity_id);
		player->rand_slot_item(activity_id, LuckyWheelActivity::SHOP_SLOT_NUM);
	}

	this->get_refresh_timer(activity_id).cancel_timer();
	this->get_refresh_timer(activity_id).schedule_timer(act_detail->shop_time_);
	act_detail->last_refresh_tick_ = Time_Value::gettimeofday().sec();
	return 0;
}

int LuckyWheelSys::update_couple_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400509*, request, -1);
	MSG_USER("update_couple_rank_info,Proto30400509: %s", request->Utf8DebugString().c_str());

	int activity_id = LuckyWheelActivity::ACT_COUPLE;
	LuckyWheelActivity::ActivityDetail* act_detail = this->fetch_activity_detail(activity_id);
	JUDGE_RETURN(act_detail != NULL, 0);

	act_detail->check_in_role_mail(request->player1(), request->rank());
	act_detail->check_in_role_mail(request->player2(), request->rank());

	return 0;
}

int LuckyWheelSys::trvl_wedding_reward_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400510*, request, -1);
	MSG_USER("trvl_wedding_reward_info,Proto30400510: %s", request->Utf8DebugString().c_str());

	Int64 role_id = request->role_id();
	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, 0);

	player->fetch_wedding_act_reward_end(request->type(), request->rank());
	return 0;
}

int LuckyWheelSys::trvl_recharge_rank_mail(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400513*, request, -1);
	MSG_USER("Proto30400513: %s", request->Utf8DebugString().c_str());

	int type = request->type();
	if (type == TrvlRechargeMonitor::TYPE_BACK_RECHAGE)
	{
		return BACK_ACTIVITY_SYS->send_travel_recharge_rank_reward_mail(request);
	}
	const Json::Value &conf = CONFIG_INSTANCE->daily_recharge_rank(type);
	JUDGE_RETURN(conf != Json::Value::null, -1);

	int mail_id = conf["mail_id"].asInt();
	const Json::Value &reward = conf["reward"];
	for (int i = 0; i < request->obj_size(); ++i)
	{
		const ProtoThreeObj &obj = request->obj(i);
		Int64 role_id = obj.id();
		int rank = obj.value();
		int reward_id = 0;
		for (uint j = 0; j < reward.size(); ++j)
		{
			int rank1 = reward[j][0u].asInt();
			int rank2 = reward[j][1u].asInt();
			JUDGE_CONTINUE(rank >= rank1 && rank <= rank2);

			reward_id = reward[j][2u].asInt();
		}
		JUDGE_CONTINUE(reward_id > 0);

		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), rank);
		mail_info->add_goods(reward_id);
		GameCommon::request_save_mail_content(role_id, mail_info);
	}

	return 0;
}

int LuckyWheelSys::trvl_wboss_send_mail(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400517*, request, -1);
	MSG_USER("Proto30400517: %s", request->Utf8DebugString().c_str());

	int type = request->type();
	int mail_id = request->mail_id();
	JUDGE_RETURN(mail_id > 0, -1);

	for (int i = 0; i < request->mail_info_size(); ++i)
	{
		const ProtoTrvlWbossMail &proto_mail = request->mail_info(i);

		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		if (type == 1)
		{
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), request->boss_name().c_str());
		}
		else
		{
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str(), request->boss_name().c_str(),
					proto_mail.rank());
		}

		mail_info->add_goods(proto_mail.reward_id());
		GameCommon::request_save_mail_content(proto_mail.role_id(), mail_info);
	}

	return 0;
}

int LuckyWheelSys::check_is_cabinet_activity(int activity_id)
{
	JUDGE_RETURN(activity_id == LuckyWheelActivity::ACT_CABINET || activity_id == LuckyWheelActivity::ACT_CABINET_DISCOUNT, -1);
	return 0;
}

int LuckyWheelSys::save_server_record(ActivityDetail* act_detail, SlotInfo* slot_info, ServerItemInfo &item_info)
{
	JUDGE_RETURN(act_detail != NULL && slot_info != NULL, 0);

	if (slot_info->server_record_ == true)
	{
		act_detail->item_set_.push_back(item_info);

		// 全服记录排序
		std::sort(act_detail->item_set_.begin(), act_detail->item_set_.end(),
				LuckyWheelActivity::comp_by_time_desc);

		//全服播报
		if (slot_info->is_shout_ == true)
			this->activity_brocast(act_detail->shout_id_, item_info);

//		if(act_detail->treasure_shout_id_ > 0)
//			GameCommon::announce(act_detail->treasure_shout_id_);

	}

	return 0;
}
