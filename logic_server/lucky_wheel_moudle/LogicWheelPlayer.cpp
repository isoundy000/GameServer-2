/*
 * LogicWheelPlayer.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: lyw
 */

#include "LogicWheelPlayer.h"
#include "LuckyWheelSys.h"
#include "ProtoDefine.h"
#include "LogicMonitor.h"
#include "LogicPlayer.h"

LogicWheelPlayer::LogicWheelPlayer() {
	// TODO Auto-generated constructor stub

}

LogicWheelPlayer::~LogicWheelPlayer() {
	// TODO Auto-generated destructor stub
}

void LogicWheelPlayer::reset()
{
	this->wheel_info_.reset();
}

void LogicWheelPlayer::reset_every_day()
{
	IntMap act_list = LUCKY_WHEEL_SYSTEM->fetch_all_activity();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
				iter->first, this->role_detail().__agent_code);
		JUDGE_CONTINUE(act_detail != NULL);
		JUDGE_CONTINUE(LUCKY_WHEEL_SYSTEM->is_activity_time(iter->first) == true);
		JUDGE_CONTINUE(act_detail->reset_flag_ == true);

		WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(iter->first);
		player_detail->reset_every_day();
	}
}

void LogicWheelPlayer::wheel_player_sign_out()
{
	int interval_tick = this->login_interval_tick();

	WheelPlayerInfo& wheel_info = this->wheel_player_info();
	for (WheelPlayerInfo::PlayerDetailMap::iterator iter = wheel_info.player_detail_map_.begin();
			iter != wheel_info.player_detail_map_.end(); ++iter)
	{
		LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
				iter->first, this->role_detail().__agent_code);
		JUDGE_CONTINUE(act_detail != NULL);
		JUDGE_CONTINUE(LUCKY_WHEEL_SYSTEM->is_activity_time(iter->first) == true);

		iter->second.login_tick_ += interval_tick;
	}
}

int LogicWheelPlayer::login_interval_tick()
{
	Int64 cur_tick 	  = ::time(NULL);
	Int64 login_tick  = this->role_detail().__login_tick;
	int interval_tick = 0;
	int is_today 	  = GameCommon::is_current_day(login_tick);
	if (is_today == true)
		interval_tick = cur_tick - login_tick;
	else
		interval_tick = cur_tick - GameCommon::today_zero();

	return interval_tick;
}

void LogicWheelPlayer::serial_draw_serialize(ProtoSerialObj* obj, int activity_id, int type)
{
	obj->set_sub_type(activity_id);
	switch (activity_id)
	{
		case LuckyWheelActivity::ACT_LUCKY_WHEEL:
		{
			obj->set_serial_type(SUB_MONEY_LUCKY_WHEEL_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_GOLD_BOX:
		{
			obj->set_serial_type(SUB_MONEY_GOLD_BOX_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_ADVANCE_BOX:
		{
			obj->set_serial_type(SUB_MONEY_ADVANCE_BOX_RESET);
			break;
		}
		case LuckyWheelActivity::ACT_LIMIT_BUY:
		{
			obj->set_serial_type(SUB_MONEY_TIME_LIMIT_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_CABINET:
		{
			if (type)
				obj->set_serial_type(SUB_MONEY_REFRESH_CABINET);
			else
				obj->set_serial_type(SUB_MONEY_BUY_CABINET);
			break;
		}
		case LuckyWheelActivity::ACT_ANTIQUES:
		{
			if (type)
				obj->set_serial_type(SUB_MONEY_DRAW_IMMORTAL);
			else
				obj->set_serial_type(SUB_MONEY_RAND_IMMORTAL);
			break;
		}
		case LuckyWheelActivity::ACT_MAZE:
		{
			obj->set_serial_type(SUB_MONET_MAZE_TREASURE_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_NINE_WORD:
		{
			obj->set_serial_type(SUB_MONEY_NINE_WORD_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_LUCKY_EGG:
		{
			obj->set_serial_type(SUB_MONEY_LUCKY_EGG_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_CABINET_DISCOUNT:
		{
			if (type)
				obj->set_serial_type(SUB_MONEY_REFRESH_CABINET_DISCOUNT);
			else
				obj->set_serial_type(SUB_MONEY_BUY_CABINET_DISCOUNT);
			break;
		}
		case LuckyWheelActivity::ACT_FISH:
		{
			if(type == REFRESH_FISH)
				obj->set_serial_type(SUB_MONEY_FISH_REFRESH);
			else
				obj->set_serial_type(SUB_MONEY_GET_FISH);
			break;
		}

		case LuckyWheelActivity::ACT_GODDESS_BLESS:
		{
			obj->set_serial_type(SUB_MONEY_GODDESS_BLESS);
			break;
		}
		default:
			break;
	}
}

void LogicWheelPlayer::serial_reset_serialize(ProtoSerialObj* obj, int activity_id)
{
	obj->set_sub_type(activity_id);
	switch (activity_id)
	{
		case LuckyWheelActivity::ACT_ADVANCE_BOX:
		{
			obj->set_serial_type(SUB_MONEY_LUCKY_WHEEL_ACTIVITY);
			break;
		}
		case LuckyWheelActivity::ACT_NINE_WORD:
		{
			obj->set_serial_type(SUB_MONEY_NINE_WORD_RESET);
			break;
		}
		case LuckyWheelActivity::ACT_LUCKY_EGG:
		{
			obj->set_serial_type(SUB_MONEY_LUCKY_EGG_RESET);
			break;
		}
		default:
			break;
	}
}

void LogicWheelPlayer::player_detail_change(WheelPlayerInfo::PlayerDetail* player_detail,
		LuckyWheelActivity::ActivityDetail* act_detail)
{
	player_detail->wheel_times_ += 1;
	player_detail->open_times_ += 1;
	player_detail->act_score_ += act_detail->add_score_;

	this->check_red_point(player_detail, act_detail);
}

void LogicWheelPlayer::add_player_slot_info(WheelPlayerInfo::PlayerDetail* player_detail,
		LuckyWheelActivity::ActivityDetail* act_detail)
{
	JUDGE_RETURN(player_detail->person_slot_set_.size() <= 0, ;);

	int activity_id = act_detail->activity_id_;
	switch (activity_id)
	{
	case LuckyWheelActivity::ACT_LIMIT_BUY:
	{
		for (LuckyWheelActivity::ActivityDetail::TimeSlotInfoMap::iterator iter = act_detail->time_slot_map_.begin();
				iter != act_detail->time_slot_map_.end(); ++iter)
		{
			LuckyWheelActivity::SlotInfoMap &slot_info_map = iter->second;
			for (LuckyWheelActivity::SlotInfoMap::iterator it = slot_info_map.begin();
					it != slot_info_map.end(); ++it)
			{
				WheelPlayerInfo::PersonSlot person_slot;
				person_slot.time_point_ = iter->first;
				person_slot.slot_id_ = it->first;
				person_slot.buy_times_ = 0;
				player_detail->person_slot_set_.push_back(person_slot);
			}
		}
		break;
	}
	case LuckyWheelActivity::ACT_NINE_WORD:
	case LuckyWheelActivity::ACT_LUCKY_EGG:
	{
		int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
		LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
		for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_map->begin();
				iter != slot_map->end(); ++iter)
		{
			WheelPlayerInfo::PersonSlot person_slot;
			person_slot.slot_id_ = iter->first;
			if (LuckyWheelActivity::ACT_NINE_WORD == activity_id)
			{
				for (int i = 1; i <= LuckyWheelActivity::NINE_WORD_NUM; ++i)
				{
					person_slot.nine_slot_[i] = 0;
				}
			}
			else
			{
				LuckyWheelActivity::SlotInfo &slot_info = iter->second;
				int rand_weight = ::rand() % 100;
				if (rand_weight <= slot_info.the_weight_)
				{
					int color_egg = player_detail->fetch_lucky_egg_open(2);
					if (player_detail->is_first_ <= 1)
					{
						if (color_egg < 1)
						{
							person_slot.is_color_ = true;
						}
					}
					else
					{
						person_slot.is_color_ = true;
					}
				}
			}

			player_detail->person_slot_set_.push_back(person_slot);
		}
		player_detail->is_first_ += 1;

		break;
	}
	default:
		break;
	}
}

WheelPlayerInfo::ItemRecord LogicWheelPlayer::create_record(ItemObj& obj, int reward_mult, int sub_value)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	WheelPlayerInfo::ItemRecord record;
	record.item_id_   	= obj.id_;
	record.amount_ 	    = obj.amount_;
	record.get_time_    = nowtime.sec() * 1000000 + nowtime.usec();
	record.item_bind_   = obj.bind_;
	record.reward_mult_ = reward_mult;
	record.sub_value_ = sub_value;
	return record;
}

void LogicWheelPlayer::record_person(WheelPlayerInfo::PlayerDetail* player_detail,
		int is_record, WheelPlayerInfo::ItemRecord& record)
{
	JUDGE_RETURN(is_record == true, ;);
	player_detail->item_record_.push_back(record);

	// 个人记录排序
	std::sort(player_detail->item_record_.begin(), player_detail->item_record_.end(),
			WheelPlayerInfo::comp_by_time_desc);
}

int LogicWheelPlayer::cal_free_times(WheelPlayerInfo::PlayerDetail* player_detail,
		LuckyWheelActivity::ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail->free_time_.size() > 0, 0);

	int free_times = 0;
	int iterval_tick = this->login_interval_tick();
	int save_login_tick = player_detail->login_tick_;
	for (IntVec::iterator iter = act_detail->free_time_.begin();
			iter != act_detail->free_time_.end(); ++iter)
	{
		JUDGE_CONTINUE(((iterval_tick + save_login_tick) / 60) >= *iter);
		++free_times;
	}

	return free_times;
}

int LogicWheelPlayer::cal_left_free_time(WheelPlayerInfo::PlayerDetail* player_detail,
			LuckyWheelActivity::ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail->free_time_.size() > 0, 0);

	int iterval_tick = this->login_interval_tick();
	int save_login_tick = player_detail->login_tick_;
	int left_free_time = 0;
	for (IntVec::iterator iter = act_detail->free_time_.begin();
			iter != act_detail->free_time_.end(); ++iter)
	{
		int free_min = (*iter) * 60;
		JUDGE_CONTINUE((iterval_tick + save_login_tick) <= free_min);

		left_free_time = free_min - iterval_tick - save_login_tick;
		break;
	}

	return left_free_time;
}

int LogicWheelPlayer::check_red_point(WheelPlayerInfo::PlayerDetail* player_detail,
		LuckyWheelActivity::ActivityDetail* act_detail)
{
	JUDGE_RETURN(player_detail != NULL && act_detail != NULL, 0);
	JUDGE_RETURN(act_detail->red_point_ > 0, 0);

	int is_notify = false;
	if (act_detail->add_score_ > 0)
	{
		GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->score_exchange_map();
		for (GameConfig::ConfigMap::iterator iter = act_map.begin();
				iter != act_map.end(); ++iter)
		{
			const Json::Value &conf = *(iter->second);
			int need_score = conf["cost"].asInt();
			JUDGE_CONTINUE(player_detail->act_score_ >= need_score);

			is_notify = true;
			break;
		}
	}

	if (act_detail->free_time_.size() > 0 && is_notify == false)
	{
		int free_times = this->cal_free_times(player_detail, act_detail);
		int left_free = free_times - player_detail->use_free_;
		if (left_free > 0)
			is_notify = true;
	}

	if (act_detail->activity_id_ == LuckyWheelActivity::ACT_LUCKY_EGG)
	{
		GameConfig::ConfigMap& act_map = CONFIG_INSTANCE->egg_reward_map();
		for (GameConfig::ConfigMap::iterator iter = act_map.begin();
				iter != act_map.end(); ++iter)
		{
			const Json::Value &conf = *(iter->second);
			int need_num = conf["num"].asInt();
			if (player_detail->check_in_reward_location(iter->first) == false
					&& player_detail->open_times_ >= need_num)
			{
				is_notify = true;
				break;
			}
		}
	}

	if (is_notify == true)
		this->inner_notify_assist_event(act_detail->red_point_, 1);
	else
		this->inner_notify_assist_event(act_detail->red_point_, 0);

	return 0;
}

void LogicWheelPlayer::add_lucky_rank_num(WheelPlayerInfo::PlayerDetail* player_detail,
		LuckyWheelActivity::ActivityDetail* act_detail)
{
	int word_num = player_detail->fetch_nine_word_num();
	JUDGE_RETURN(word_num >= LuckyWheelActivity::SIX_WORD_REWARD_NUM, ;);

	int open_slot_num = player_detail->fetch_total_open_num();
	act_detail->update_rank_info(this->role_id(), this->name(), open_slot_num,
			LuckyWheelActivity::RANK_TYPE_LUCKY);
}

void LogicWheelPlayer::add_item(ItemObj &item_obj, SerialObj &obj)
{
	if (item_obj.id_ == GameEnum::ITEM_MONEY_UNBIND_GOLD
			|| item_obj.id_ == GameEnum::ITEM_MONEY_BIND_GOLD)
	{
		Money money;
		if (item_obj.id_ == GameEnum::ITEM_MONEY_UNBIND_GOLD)
			money.__gold = item_obj.amount_;
		else if (item_obj.id_ == GameEnum::ITEM_MONEY_BIND_GOLD)
			money.__bind_gold = item_obj.amount_;

		this->request_add_money(money, obj, true);
	}
	else
	{
		this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);
	}
}

int LogicWheelPlayer::fetch_cabinet_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102011*, request, RETURN_CABINET_INFO);

	int activity_id = request->activity_id();
	if(activity_id == 0)
		activity_id = LuckyWheelActivity::ACT_CABINET;

	return fetch_cabinet_info(activity_id);
}

int LogicWheelPlayer::fetch_reset_cost(int activity_id, int reset_times, IntMap &reset_cost)
{
	JUDGE_RETURN(reset_cost.size() > 0, -1);

	if (activity_id == LuckyWheelActivity::ACT_ADVANCE_BOX)
	{
//		CONDITION_NOTIFY_RETURN(int(reset_cost.size()) >= reset_times, RETURN_RESET_LUCKY_WHEEL,
//				ERROR_SCRIPT_VIP_BUY_MAX);
		if (reset_times > int(reset_cost.size()))
			return ERROR_SCRIPT_VIP_BUY_MAX;
	}

	if (reset_times > int(reset_cost.size()))
		reset_times = int(reset_cost.size());

	return reset_cost[reset_times];
}

int LogicWheelPlayer::fetch_wheel_act_list()
{
	Proto50102015 respond;
	IntMap act_list = LUCKY_WHEEL_SYSTEM->fetch_all_activity();
	for (IntMap::iterator iter = act_list.begin(); iter != act_list.end(); ++iter)
	{
		LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
				iter->first, this->role_detail().__agent_code);
		JUDGE_CONTINUE(act_detail != NULL);

		respond.add_activity_set(iter->first);
	}

	FINER_PROCESS_RETURN(RETURN_LUCKY_WHEEL_LIST_NEW, &respond);
}

int LogicWheelPlayer::fetch_one_wheel_activity(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102002*, request, RETURN_ONE_LUCKY_WHEEL_ACTIVITY);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	return this->fetch_activity_info(activity_detail);
}

int LogicWheelPlayer::fetch_activity_info(LuckyWheelActivity::ActivityDetail* act_detail)
{
	JUDGE_RETURN(act_detail != NULL, 0);

	int activity_id = act_detail->activity_id_;
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_ONE_LUCKY_WHEEL_ACTIVITY, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto50102002 respond;
	respond.set_activity_id(activity_id);
	respond.set_left_gold(act_detail->save_gold_);
	respond.set_my_score(player_detail->act_score_);
	respond.set_left_tick(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));
	respond.set_now_slot_id(player_detail->wheel_times_);
	respond.set_draw_limit(act_detail->draw_limit_ - player_detail->wheel_times_);
	respond.set_nine_word_reward(player_detail->nine_word_reward_);
	respond.set_reward_show(act_detail->six_reward_);
	respond.set_rank_reward_show(act_detail->rank_reward_);
	respond.set_open_times(player_detail->open_times_);
	respond.set_total_recharge(this->role_detail().__recharge_total_gold);

	int reset_times = player_detail->reset_times_ + 1;
	int reset_cost = this->fetch_reset_cost(activity_id, reset_times, act_detail->reset_cost_);
	if (reset_cost >= 0)
		respond.set_reset_cost(reset_cost);

	int word_num = player_detail->fetch_nine_word_num();
	if (word_num < LuckyWheelActivity::SIX_WORD_REWARD_NUM && player_detail->reward_get_ == false)
		respond.set_reward_get(-1);
	else
		respond.set_reward_get(player_detail->reward_get_);

	int total_reset = act_detail->reset_cost_.size();
	int left_reset = total_reset - player_detail->reset_times_;
	respond.set_reset_times(left_reset);

	int free_times = this->cal_free_times(player_detail, act_detail);
	int left_free = free_times - player_detail->use_free_;
	respond.set_free_times(left_free);

	int total_free = act_detail->free_time_.size();
	int draw_free_times = total_free - free_times;
	respond.set_draw_free_times(draw_free_times);

	int refresh_tick = GameCommon::next_day();
	respond.set_refresh_tick(refresh_tick);

	int left_free_time = this->cal_left_free_time(player_detail, act_detail);
	respond.set_left_free_time(left_free_time);

	for (IntVec::iterator iter = player_detail->reward_location_.begin();
			iter != player_detail->reward_location_.end(); ++iter)
	{
		respond.add_reward_location(*iter);
	}

	int num = 0;
	for (WheelPlayerInfo::ItemRecordSet::iterator iter = player_detail->item_record_.begin();
			iter != player_detail->item_record_.end(); ++iter)
	{
		ProtoPersonRecord *person_record = respond.add_person_record();
		this->wheel_info_.record_serialize(person_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_PERSON_RECORD_NUM);
	}

	num = 0;
	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->item_set_.begin();
			iter != act_detail->item_set_.end(); ++iter)
	{
		ProtoServerRecord *server_record = respond.add_server_record();
		LUCKY_WHEEL_SYSTEM->record_serialize(server_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_SERVER_RECORD_NUM);
	}

	fetch_fish_info(&respond);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	if(slot_map != NULL)
	{
		for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_map->begin();
				iter != slot_map->end(); ++iter)
		{
			LuckyWheelActivity::SlotInfo &slot_info = iter->second;
			ProtoSlotInfo *slot = respond.add_slot_info();
			LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, &slot_info, player_detail);
		}
	}



	FINER_PROCESS_RETURN(RETURN_ONE_LUCKY_WHEEL_ACTIVITY, &respond);
}

int LogicWheelPlayer::fetch_recharge_rebate_info()
{
	int activity_id = LuckyWheelActivity::ACT_RECHARGE_REBATE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);

	Proto50102019 respond;
	respond.set_begin_tick(act_detail->save_first_date_);
	respond.set_end_tick(act_detail->save_last_date_);
	FINER_PROCESS_RETURN(RETURN_FETCH_RECHARGE_REBATE, &respond);
}

int LogicWheelPlayer::recharge_rebate(Message* msg)
{
	DYNAMIC_CAST_RETURN(Proto30101111 *, request, msg, -1);

	int gold = request->cur_gold();
	int type = request->recharge_type();
	JUDGE_RETURN(gold > 0 && type > 0, 0);

	int activity_id = LuckyWheelActivity::ACT_RECHARGE_REBATE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);
	JUDGE_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(act_detail) == true, 0);

	int id = CONFIG_INSTANCE->fetch_recharge_id(gold, type);
	JUDGE_RETURN(id > 0, 0);

	const Json::Value& conf = CONFIG_INSTANCE->recharge_json(id);
	JUDGE_RETURN(conf != Json::Value::null, 0);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int rebate_times = conf["rebate_times"].asInt();
	JUDGE_RETURN(player_detail->rebate_map_[id] < rebate_times, 0);

	int get_gold = 0;
	int first_gold = conf["first_gold"].asInt();
	if (player_detail->rebate_map_[id] == 0 && first_gold > 0)
	{
		get_gold = first_gold;
	}
	else
	{
		get_gold = conf["rebate_gold"].asInt();
	}
	player_detail->rebate_map_[id] += 1;

	int mail_id = act_detail->mail_id_;
	JUDGE_RETURN(mail_id > 0, 0);

	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
			mail_info->mail_content_.c_str(), get_gold);
	mail_info->add_money(GameEnum::ITEM_MONEY_UNBIND_GOLD, get_gold);
	GameCommon::request_save_mail_content(this->role_id(), mail_info);

	LogicPlayer* player = this->logic_player();
	MSG_USER("LogicWheelPlayer, recharge reward:%d", get_gold);
	player->update_cornucopia_activity_recharge(get_gold);
	return 0;
}

int LogicWheelPlayer::check_fina_slot_map()
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_INFO,
			ERROR_NO_FUN_ACTIVITY);

	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_IMMORTAL_TREASURE_INFO, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	player_detail->fina_slot_map_.clear();
	IntSet fina_set;
	int size = 1;

	for (int i = 1; i <= std::min((int)player_detail->now_slot_map_.size(), 3); ++i)
	{
		int slot_id = player_detail->now_slot_map_[i];
		JUDGE_CONTINUE(fina_set.count(slot_id) == 0);
		player_detail->fina_slot_map_[size] = player_detail->now_slot_map_[i];
		fina_set.insert(slot_id);
		size++;
	}

	return 0;
}

int LogicWheelPlayer::fetch_immortal_treasures(int type)
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_INFO,
			ERROR_NO_FUN_ACTIVITY);

	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_IMMORTAL_TREASURE_INFO, ERROR_ACTIVITY_TIME_INVALID);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto50102031 respond;
	respond.set_activity_id(activity_id);
	respond.set_left_tick(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));

	int num = 0;
	for (WheelPlayerInfo::ItemRecordSet::iterator iter = player_detail->item_record_.begin();
			iter != player_detail->item_record_.end(); ++iter)
	{
		ProtoPersonRecord *person_record = respond.add_person_record();
		this->wheel_info_.record_serialize(person_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_SERVER_RECORD_NUM);
	}

	num = 0;
	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->item_set_.begin();
			iter != act_detail->item_set_.end(); ++iter)
	{
		ProtoServerRecord *server_record = respond.add_server_record();
		LUCKY_WHEEL_SYSTEM->record_serialize(server_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_SERVER_RECORD_NUM);
	}

	for (uint i = 1; i <= player_detail->now_slot_map_.size(); ++i)
	{
		int slot_id = player_detail->now_slot_map_[i];
		JUDGE_CONTINUE(slot_id > 0);
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
		ProtoSlotInfo *slot = respond.add_slot_info();
		LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, slot_info, player_detail);
	}

	//最终奖励
	ProtoThreeObj* info = respond.mutable_other_info();
	int scale = act_detail->draw_same_scale_[3 - player_detail->fina_slot_map_.size()];
	info->set_id(scale);
	info->set_value(type);

//	for (int i = 0; i < player_detail->fina_slot_map_.size(); ++i)
//	{
//		int slot_id = player_detail->now_slot_map_[i];
//		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
//		ProtoItem *item = respond.add_fina_item();
//		item->set_id(slot_info->item_obj_.item_id_);
//		item->set_amount(slot_info->item_obj_.item_amount_ * scale);
//		item->set_bind(slot_info->item_obj_.item_bind_);
//	}


	FINER_PROCESS_RETURN(RETURN_IMMORTAL_TREASURE_INFO, &respond);
}

int LogicWheelPlayer::fetch_immortal_treasures_reward()
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_IMMORTAL_TREASURE_REWARD, ERROR_ACTIVITY_TIME_INVALID);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail->now_slot_map_[1] > 0,RETURN_IMMORTAL_TREASURE_REWARD,
			ERROR_CLIENT_OPERATE);
	SerialObj obj(ADD_FROM_IMMORTAL_TREASURES, activity_id);
	int scale = act_detail->draw_same_scale_[3 - player_detail->fina_slot_map_.size()];


	int i = 1;
	if ((int)player_detail->fina_slot_map_.size() == 2 && player_detail->now_slot_map_[2] == player_detail->now_slot_map_[3])
	{
		i = 2;
	}

	int slot_id = player_detail->fina_slot_map_[i];
	LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
	ItemObj item;
	item.id_		= 	slot_info->item_obj_.id_;
	item.amount_ 	=	slot_info->item_obj_.amount_ * scale;
	item.bind_		= 	slot_info->item_obj_.bind_;
	scale = 1;
	this->request_add_item(obj, item.id_, item.amount_, item.bind_);

	LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
				this->role_id(), item);

	WheelPlayerInfo::ItemRecord record = this->create_record(item);
	this->record_person(player_detail, slot_info->person_record_, record);

	player_detail->now_slot_map_.clear();
	player_detail->fina_slot_map_.clear();
	this->fetch_immortal_treasures(4);
	FINER_PROCESS_NOTIFY(RETURN_IMMORTAL_TREASURE_REWARD);
}

int LogicWheelPlayer::draw_immortal_treasures_begin()
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
//	CONDITION_NOTIFY_RETURN(player_detail->now_slot_map_.size() == 0, RETURN_IMMORTAL_TREASURE_DRAW,
//			ERROR_TASK_UNSUBMIT_EXP);

	//TODO:需要查找key == 1为0的原因
	for (IntMap::iterator iter = player_detail->now_slot_map_.begin();
			iter != player_detail->now_slot_map_.end(); ++iter)
	{
		CONDITION_NOTIFY_RETURN(iter->second <= 0, RETURN_IMMORTAL_TREASURE_DRAW,
				ERROR_TASK_UNSUBMIT_EXP);
	}

	player_detail->now_slot_map_.clear();

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_IMMORTAL_TREASURE_DRAW);
	inner.set_amount(0);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(act_detail->draw_cost_);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id, 1);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::draw_immortal_treasures_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
//	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	player_detail->now_slot_map_.clear();

	this->refresh_immortal_treasures_info();

	//格子顺序调整
	int max_weight = 999999;
	int fina_id = 1;
	for (uint i = 1; i <= player_detail->now_slot_map_.size(); ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day,
				player_detail->now_slot_map_[i]);

		if (slot_info->item_show_weight_ < max_weight)
		{
			fina_id = i;
			max_weight = slot_info->item_show_weight_;
		}
	}
	std::swap(player_detail->now_slot_map_[1], player_detail->now_slot_map_[fina_id]);
	this->check_fina_slot_map();

//	this->fetch_immortal_treasures(2);
	Proto50102032 respond;
	for (uint i = 1; i <= player_detail->now_slot_map_.size(); ++i)
	{
		int slot_id = player_detail->now_slot_map_[i];
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
		ProtoSlotInfo *slot = respond.add_slot_info();
		LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, slot_info, player_detail);
	}
	respond.set_activity_id(activity_id);

	int reward_id = act_detail->draw_return_;
	this->logic_player()->request_add_reward(reward_id, ADD_FROM_IMMORTAL_TREASURES_RETURN);

	//最终奖励
	ProtoThreeObj* info = respond.mutable_other_info();
	int scale = act_detail->draw_same_scale_[3 - player_detail->fina_slot_map_.size()];
	info->set_id(scale);
	info->set_value(0);

	FINER_PROCESS_RETURN(RETURN_IMMORTAL_TREASURE_DRAW, &respond);
}

int LogicWheelPlayer::rand_immortal_treasures_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102033*, request, RETURN_IMMORTAL_TREASURE_RAND);

	int id = request->id();
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_RAND,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int check_bool = false;
	if (player_detail->now_slot_map_[1] == player_detail->now_slot_map_[2] &&
			player_detail->now_slot_map_[1] == player_detail->now_slot_map_[3])
		check_bool = true;

	CONDITION_NOTIFY_RETURN(check_bool == false, RETURN_IMMORTAL_TREASURE_RAND,
			ERROR_TASK_UNSUBMIT_EXP);

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_IMMORTAL_TREASURE_RAND);
	inner.set_amount(id + 100);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(act_detail->refresh_cost_);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id, 0);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::rand_immortal_treasures_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_RAND,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail->now_slot_map_[1] > 0,RETURN_IMMORTAL_TREASURE_RAND,
			ERROR_CLIENT_OPERATE);

	int id = request->amount() - 100;
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
//	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

	this->refresh_immortal_treasures_info_by_id(id);
	this->check_fina_slot_map();

//	this->fetch_immortal_treasures(3);
	Proto50102033 respond;

	for (uint i = 1; i <= player_detail->now_slot_map_.size(); ++i)
	{
		int slot_id = player_detail->now_slot_map_[i];
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
		ProtoSlotInfo *slot = respond.add_slot_info();
		LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, slot_info, player_detail);
	}

	respond.set_activity_id(activity_id);
	respond.set_id(id);

	int reward_id = act_detail->draw_return_;
	this->logic_player()->request_add_reward(reward_id, ADD_FROM_IMMORTAL_TREASURES_RETURN);
	FINER_PROCESS_RETURN(RETURN_IMMORTAL_TREASURE_RAND, &respond);

}

int LogicWheelPlayer::refresh_immortal_treasures_info()
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	IntMap maybe_show_id_set;
	IntMap must_show_id_set;
	IntSet fina_show_set;
	IntMap show_pro_map;
	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

		player_detail->now_slot_map_.clear();

		LuckyWheelActivity::SlotInfoMap::iterator iter = slot_map->begin();
		for (; iter != slot_map->end(); ++iter)
		{
			int slot_id = iter->first;
			JUDGE_CONTINUE(slot_id > 0);
			LuckyWheelActivity::SlotInfo &slot_info = iter->second;
			if(slot_info.item_min_times_ == 0 && slot_info.item_max_times_ == 0)
			{
				int size = maybe_show_id_set.size();
				maybe_show_id_set[size] = slot_id;
			}
			else
			{
				if (slot_info.item_min_times_ <= player_detail->group_show_times_map_[slot_id])
				{
					int size = maybe_show_id_set.size();
					maybe_show_id_set[size] = slot_id;
				}
				if (slot_info.item_max_times_ <= player_detail->group_show_times_map_[slot_id])
				{
					int size = must_show_id_set.size();
					must_show_id_set[size] = slot_id;
				}
			}
		}

		//必先

		for (int i = 0; i < std::min((int)must_show_id_set.size(), 3); ++i)
		{
			player_detail->now_slot_map_[i + 1] = must_show_id_set[i];
			int id = must_show_id_set[i];
			maybe_show_id_set.erase(id);
			fina_show_set.insert(id);
		}

		//必先次数消除
		for (uint i = 0; i < must_show_id_set.size(); ++i)
		{
			int slot_id = must_show_id_set[i];
			JUDGE_CONTINUE(slot_id > 0);
			player_detail->group_show_times_map_[slot_id] = 0;
		}

		//可先的概率分布
		int sum = 0;
		for (uint i = 0; i < maybe_show_id_set.size(); ++i)
		{
			int slot_id = maybe_show_id_set[i];
			JUDGE_CONTINUE(slot_id > 0);
			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
			sum += slot_info->group_weight_;
			show_pro_map[i] = sum;
		}

		while (player_detail->now_slot_map_.size() < 3)
		{
			int rand_num = std::rand() % sum;
			int id = 0;
			for (uint i = 0; i < show_pro_map.size(); ++i)
			{
				if (show_pro_map[i] >= rand_num)
				{
					id = maybe_show_id_set[i];
					break;
				}
			}
			JUDGE_CONTINUE(id > 0);
			JUDGE_CONTINUE(fina_show_set.count(id) == 0);
			int size = player_detail->now_slot_map_.size();
			player_detail->now_slot_map_[size + 1] = id;
			fina_show_set.insert(id);

		}
		//没有出现的刷新次数处理
		LuckyWheelActivity::SlotInfoMap::iterator it2 = slot_map->begin();
		for (; it2 != slot_map->end(); ++it2)
		{
			int slot_id = it2->first;
			JUDGE_CONTINUE(slot_id > 0);
			LuckyWheelActivity::SlotInfo &slot_info = it2->second;
			JUDGE_CONTINUE(slot_info.item_min_times_ > 0 && slot_info.item_max_times_ > 0);
			if (fina_show_set.count(slot_id) == 0)
				player_detail->group_show_times_map_[slot_id]++;
			else
				player_detail->group_show_times_map_[slot_id] = 0;
		}

	return 0;
}

int LogicWheelPlayer::refresh_immortal_treasures_info_by_id(int refresh_id)
{
	int activity_id = LuckyWheelActivity::ACT_ANTIQUES;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_IMMORTAL_TREASURE_RAND,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	IntMap maybe_show_id_set;
	IntMap must_show_id_set;
	IntSet fina_show_set;
	IntMap show_pro_map;
	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

		int slot_a = 0, slot_b = 0, slot_self = player_detail->now_slot_map_[refresh_id];
		int fina_id = 0;
		switch (refresh_id)
		{
		case 1:
			slot_a = player_detail->now_slot_map_[2];
			slot_b = player_detail->now_slot_map_[3];
			break;
		case 2:
			slot_a = player_detail->now_slot_map_[1];
			slot_b = player_detail->now_slot_map_[3];
			break;
		case 3:
			slot_a = player_detail->now_slot_map_[1];
			slot_b = player_detail->now_slot_map_[2];
			break;
		}
		LuckyWheelActivity::SlotInfo* slot_info_a = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_a);
		LuckyWheelActivity::SlotInfo* slot_info_b = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_b);

		if (slot_a == slot_b)
		{
			if (player_detail->three_same_show_times_map_[slot_a] >= slot_info_a->three_same_times_
					&& slot_info_a->three_same_times_ > 0)
			{
				fina_id = slot_a;
			}
			else
			{
				int size = maybe_show_id_set.size();
				maybe_show_id_set[size] = slot_a;
				LuckyWheelActivity::SlotInfoMap::iterator it = slot_map->begin();
				for (; it != slot_map->end(); ++it)
				{
					int slot_id = it->first;
					JUDGE_CONTINUE(slot_id > 0);
					LuckyWheelActivity::SlotInfo* slot_info = &(it->second);

					if(slot_info->two_same_times_ == 0 && slot_info->three_same_times_ == 0)
					{
						int size = maybe_show_id_set.size();
						maybe_show_id_set[size] = slot_id;
					}
				}

				//随机
				int sum = 0;
				for (uint i = 0; i < maybe_show_id_set.size(); ++i)
				{
					int slot_id = maybe_show_id_set[i];
					JUDGE_CONTINUE(slot_id > 0);
					LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
					sum += slot_info->group_weight_;
					show_pro_map[i] = sum;
				}
				do
				{
					int rand_num = std::rand() % sum;
					for (uint i = 0; i < show_pro_map.size(); ++i)
					{
						if (show_pro_map[i] >= rand_num)
						{
							fina_id = maybe_show_id_set[i];
							break;
						}
					}
				}while (slot_self == fina_id);
			}

			if (slot_info_a->three_same_times_ > 0)
			{
				if (fina_id == slot_a)
					player_detail->three_same_show_times_map_[fina_id] = 0;
				else
					player_detail->three_same_show_times_map_[fina_id]++;
			}
		}
		//slot_a != slot_b
		else
		{
			if (player_detail->two_same_show_times_map_[slot_a] >= slot_info_a->two_same_times_ && slot_info_a->two_same_times_ != 0)
			{
				fina_id = slot_a;
				player_detail->two_same_show_times_map_[slot_a] = 0;
			}
			if (player_detail->two_same_show_times_map_[slot_b] >= slot_info_b->two_same_times_&& slot_info_b->two_same_times_ != 0)
			{
				player_detail->two_same_show_times_map_[slot_a] = 0;
				if (fina_id == 0)
				{
					fina_id = slot_b;
				}
				else
				{
					if (slot_info_b->item_show_weight_ > slot_info_a->item_show_weight_)
					{
						fina_id = slot_b;
					}
				}
			}
			if (fina_id == 0)
			{
				int size = maybe_show_id_set.size();
				maybe_show_id_set[size] = slot_a;
				maybe_show_id_set[size + 1] = slot_b;
				LuckyWheelActivity::SlotInfoMap::iterator it = slot_map->begin();
				for (; it != slot_map->end(); ++it)
				{
					int slot_id = it->first;
					JUDGE_CONTINUE(slot_id > 0);
					LuckyWheelActivity::SlotInfo* slot_info = &(it->second);
					JUDGE_CONTINUE(slot_id != slot_a && slot_id != slot_b);

					if(slot_info->two_same_times_ == 0 && slot_info->three_same_times_ == 0)
					{
						int size = maybe_show_id_set.size();
						maybe_show_id_set[size] = slot_id;
					}
				}

				//随机
				int sum = 0;
				for (uint i = 0; i < maybe_show_id_set.size(); ++i)
				{
					int slot_id = maybe_show_id_set[i];
					LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
					sum += slot_info->group_weight_;
					show_pro_map[i] = sum;
				}
				do
				{
					int rand_num = std::rand() % sum;
					for (uint i = 0; i < show_pro_map.size(); ++i)
					{
						if (show_pro_map[i] >= rand_num)
						{
							fina_id = maybe_show_id_set[i];
							break;
						}
					}
				}while (slot_self == fina_id);

				if (slot_info_a->two_same_times_ > 0)
				{
					if (fina_id == slot_a)
						player_detail->two_same_show_times_map_[slot_a] = 0;
					else
						player_detail->two_same_show_times_map_[slot_a]++;
				}

				if (slot_info_b->two_same_times_ > 0)
				{
					if (fina_id == slot_b)
						player_detail->two_same_show_times_map_[slot_b] = 0;
					else
						player_detail->two_same_show_times_map_[slot_b]++;
				}

			}
		}
	player_detail->now_slot_map_[refresh_id] = fina_id;

	return 0;
}

int LogicWheelPlayer::fetch_maze_treasures(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102021*, request, RETURN_MAZE_TREASURE_INFO);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_INFO,
			ERROR_NO_FUN_ACTIVITY);

	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_MAZE_TREASURE_INFO, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	for (uint i = 1; i <= act_detail->finish_free_time_.size(); ++i)
	{
		JUDGE_BREAK(act_detail->finish_all_times_ >= act_detail->finish_free_time_[i - 1]);
		if (player_detail->free_times_map_[i] == 0)
			player_detail->free_times_map_[i] = 1;

		if (player_detail->free_times_map_[i] == 1)
		{
			player_detail->maze_free_ ++;
			player_detail->free_times_map_[i] = 2;
		}
	}

	Proto50102021 respond;
	respond.set_activity_id(activity_id);
	respond.set_left_tick(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));
	respond.set_now_slot_id(player_detail->slot_index_);
	respond.set_free_times(player_detail->maze_free_);
	respond.set_bless(player_detail->bless_);
	respond.set_max_bless(act_detail->show_bless_);
	respond.set_one_cost(act_detail->draw_cost_);

	int total_slot = act_detail->slot_fina_id_ - player_detail->slot_index_;
	for (ThreeObjVec::iterator iter = act_detail->total_cost_.begin();
			iter != act_detail->total_cost_.end(); ++iter)
	{
		JUDGE_CONTINUE(total_slot >= iter->id_ && total_slot <= iter->tick_);
		respond.set_total_cost(iter->value_);
		break;
	}

	int free_left_times = 0;
	for (uint i = 0; i < act_detail->finish_free_time_.size(); ++i)
	{
		if (act_detail->finish_free_time_[i] > act_detail->finish_all_times_)
		{
			free_left_times = act_detail->finish_free_time_[i] - act_detail->finish_all_times_;
			break;
		}
	}

	respond.set_act_finish_time((int)act_detail->finish_all_times_);
	respond.set_left_free_times(free_left_times);

	int num = 0;
	for (WheelPlayerInfo::ItemRecordSet::iterator iter = player_detail->item_record_.begin();
			iter != player_detail->item_record_.end(); ++iter)
	{
		ProtoPersonRecord *person_record = respond.add_person_record();
		this->wheel_info_.record_serialize(person_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_SERVER_RECORD_NUM);
	}

	num = 0;
	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->item_set_.begin();
			iter != act_detail->item_set_.end(); ++iter)
	{
		ProtoServerRecord *server_record = respond.add_server_record();
		LUCKY_WHEEL_SYSTEM->record_serialize(server_record, *iter);

		++num;
		JUDGE_BREAK(num < MAX_SERVER_RECORD_NUM);
	}

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
//	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	for (int i = 1; i <= act_detail->slot_fina_id_; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
		ProtoSlotInfo *slot = respond.add_slot_info();
		LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, slot_info, player_detail, i);
	}

	FINER_PROCESS_RETURN(RETURN_MAZE_TREASURE_INFO, &respond);
}

int LogicWheelPlayer::draw_maze_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102022*, request, RETURN_MAZE_TREASURE_DRAW);

	int activity_id = LuckyWheelActivity::ACT_MAZE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int is_total = request->is_total();

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_MAZE_TREASURE_DRAW);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id, 1);

	ProtoMoney *money = inner.mutable_money();
	if (is_total == true)
	{
		int num = act_detail->slot_fina_id_ - player_detail->slot_index_;
		for (ThreeObjVec::iterator iter = act_detail->total_cost_.begin();
				iter != act_detail->total_cost_.end(); ++iter)
		{
			JUDGE_CONTINUE(num >= iter->id_ && num <= iter->tick_);
			money->set_gold(iter->value_);
			break;
		}

		inner.set_is_all(is_total);
	}
	else if (player_detail->maze_free_ > 0)
	{
		player_detail->maze_free_ --;
		return this->draw_maze_end(&inner);
	}
	else
	{
		money->set_gold(act_detail->draw_cost_);
	}
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::draw_maze_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);

	int is_total = request->is_all();
	Proto50102022 respond;
	respond.set_is_total(is_total);

	int rand_num = std::rand() % 6 + 1;
	if ((rand_num > act_detail->slot_fina_id_ - player_detail->slot_index_) || is_total == true)
	{
		rand_num = act_detail->slot_fina_id_ - player_detail->slot_index_;
	}

	int bless_num = 1;
	if (is_total == true)
	{
		for (ThreeObjVec::iterator iter = act_detail->bless_total_.begin();
				iter != act_detail->bless_total_.end(); ++iter)
		{
			JUDGE_CONTINUE(rand_num >= iter->id_ && rand_num <= iter->tick_);
			bless_num = iter->value_;
		}
	}

	if (act_detail->bless_interval_.size() == 2)
	{
		int before_bless = player_detail->bless_;
		for(int i = 0; i < bless_num; ++i)
		{
			int bless = GameCommon::rand_value(act_detail->bless_interval_[0], act_detail->bless_interval_[1]);
			int rate = ::rand() % 100;
			int mult = act_detail->mult_rate_ >= rate ? 2 : 1;
			player_detail->bless_ += bless * mult;
			JUDGE_CONTINUE(is_total != true);

			respond.set_mult(mult);
		}

		int add_bless = player_detail->bless_ - before_bless;
		respond.set_bless(add_bless);
	}

	respond.set_activity_id(activity_id);
	respond.set_rand_num(rand_num);
	respond.set_pre_slot_id(player_detail->slot_index_ + 1);
	respond.set_now_slot_id(player_detail->slot_index_ + rand_num);
	SerialObj obj(ADD_FROM_MAZE_TREASURE, activity_id);

	for (int i = player_detail->slot_index_ + 1; i <= player_detail->slot_index_ + rand_num; ++i)
	{
		JUDGE_CONTINUE(i != 0);
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);

		switch (slot_info->slot_type_)
		{
		case 1:
		{
			this->draw_maze_msg_info(respond, i, i);
			break;
		}
		case 2:
		{
			int fina_slot_id = this->draw_maze_type_work(i, 2);
			if (fina_slot_id > 0) this->draw_maze_msg_info(respond, fina_slot_id, i);
			this->draw_maze_times_work(fina_slot_id, 2);
			break;
		}
		case 3:
		{
			int fina_slot_id = this->draw_maze_type_work(i, 3);
			if (fina_slot_id > 0) this->draw_maze_msg_info(respond, fina_slot_id, i);
			this->draw_maze_times_work(fina_slot_id, 3);
			break;
		}
		}
	}
	player_detail->slot_index_ += rand_num;
	if (player_detail->slot_index_ == act_detail->slot_fina_id_)
	{
		player_detail->slot_index_ = 0;
		act_detail->finish_all_times_++;
		//免费次数传闻
		if (act_detail->finish_free_shout_index_ < (int)act_detail->finish_free_time_.size())
		{
			if (act_detail->finish_all_times_ >= act_detail->finish_free_time_[act_detail->finish_free_shout_index_])
			{
				act_detail->finish_free_shout_index_++;
				BrocastParaVec para_vec;
				GameCommon::push_brocast_para_int(para_vec,
						act_detail->finish_free_time_[act_detail->finish_free_shout_index_ - 1]);

				int shout_id = act_detail->finish_free_shout_id_;
				GameCommon::announce(shout_id, &para_vec);
			}
		}
		//刷新面板
		Proto10102021 info;
		info.set_activity_id(LuckyWheelActivity::ACT_MAZE);
		LogicMonitor::PlayerMap &player_map = LOGIC_MONITOR->player_map();
		for (LogicMonitor::PlayerMap::iterator iter = player_map.begin();
				iter != player_map.end(); ++iter)
		{
			LogicPlayer* player = iter->second;
	        player->fetch_maze_treasures(&info);
		}

		player_detail->slot_item_id_.clear();
		player_detail->slot_item_num_.clear();
	}
	//刷新面板
	Proto10102021 info;
	info.set_activity_id(LuckyWheelActivity::ACT_MAZE);
	this->fetch_maze_treasures(&info);

	FINER_PROCESS_RETURN(RETURN_MAZE_TREASURE_DRAW, &respond);
}

int LogicWheelPlayer::draw_maze_times_work(int slot_id, int slot_type)
{
	int activity_id = LuckyWheelActivity::ACT_MAZE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	int slot_size = (int)slot_map->size();	//一共多少个格子
	int begin = act_detail->slot_fina_id_ + 1;
	for (int i = begin; i < slot_size; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
		JUDGE_CONTINUE(slot_info->item_min_times_ > 0 && slot_info->item_max_times_ > 0 && i != slot_id
				&& slot_info->slot_type_ == slot_type);

		if (slot_info->slot_type_ == 2)
			player_detail->group_show_times_map_[i] ++;
		else
			player_detail->group_show_times_map_fina_[i] ++;
	}

	LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
	if (slot_info->slot_type_ == 2)
		player_detail->group_show_times_map_[slot_id] = 0;
	else
		player_detail->group_show_times_map_fina_[slot_id] = 0;

	return 0;
}

int LogicWheelPlayer::draw_maze_type_work(int slot_id, int type)
{
	int activity_id = LuckyWheelActivity::ACT_MAZE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	if (slot_id == act_detail->slot_fina_id_ && player_detail->bless_ >= act_detail->get_bless_)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(
				act_detail, cur_day, act_detail->bless_slot_id_);
		if (slot_info != NULL)
			return act_detail->bless_slot_id_;
	}

	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

	int slot_size = (int)slot_map->size();	//一共多少个格子
	int fina_slot_id = 0, fina_slot_level = 99999999, may_show_size = 0;
	IntMap may_show_list;
	for (int i = act_detail->slot_fina_id_ + 1; i < slot_size; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
		JUDGE_CONTINUE(slot_info->slot_type_ == type);
		if (type == 2)
		{
			if (player_detail->group_show_times_map_[slot_info->slot_id_] >=  slot_info->item_min_times_)
			{
				may_show_size++;
				may_show_list[may_show_size] = slot_info->slot_id_;
			}
		}
		if (type == 3)
		{
			if (player_detail->group_show_times_map_fina_[slot_info->slot_id_] >=  slot_info->item_min_times_)
			{
				may_show_size++;
				may_show_list[may_show_size] = slot_info->slot_id_;
			}
		}
		JUDGE_CONTINUE(slot_info->item_min_times_ != 0 && slot_info->item_max_times_ != 0);

		if (type == 2)
		{
			if (player_detail->group_show_times_map_[slot_info->slot_id_] >=  slot_info->item_max_times_)
			{
				if (slot_info->item_show_weight_ < fina_slot_level)
				{
					fina_slot_id = slot_info->slot_id_;
					fina_slot_level = slot_info->item_show_weight_;
				}
			}
		}

		if (type == 3)
		{
			if (player_detail->group_show_times_map_fina_[slot_info->slot_id_] >=  slot_info->item_max_times_)
			{
				if (slot_info->item_show_weight_ < fina_slot_level)
				{
					fina_slot_id = slot_info->slot_id_;
					fina_slot_level = slot_info->item_show_weight_;
				}
			}
		}
	}

	JUDGE_RETURN(fina_slot_id == 0 && fina_slot_level == 99999999, fina_slot_id);

	//算可先的概率
	IntMap pro_map;
	int pro_sum = 0;
	for (int i = 1; i <= may_show_size; ++i)
	{
		int id =  may_show_list[i];
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, id);

		pro_sum += slot_info->group_weight_;
		pro_map[i] = pro_sum;
	}

	int fina_num = std::rand() % pro_sum;
	for (int i = 1; i <= may_show_size; ++i)
	{
		int id =  may_show_list[i];
		if (fina_num <= pro_map[i]) return id;
	}

	return 0;
}

int LogicWheelPlayer::draw_maze_msg_info(Proto50102022 &respond, int slot_id, int pre_slot_id)
{
	int activity_id = LuckyWheelActivity::ACT_MAZE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_MAZE_TREASURE_DRAW,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);

	int item_id = slot_info->item_obj_.id_;
	int item_num = slot_info->item_obj_.amount_;
	int item_bind = slot_info->item_obj_.bind_;

	if (player_detail->reward_scale_ > 0)
	{
		item_num *= player_detail->reward_scale_ / GameEnum::RAMA_ID_BASE_NUM;
		player_detail->reward_scale_ = 0;
	}
	SerialObj obj(ADD_FROM_MAZE_TREASURE, activity_id);
	ItemObj item_obj(item_id, item_num, item_bind);

	//处理倍数
	if (item_id == item_num && item_num == item_bind)
	{
		player_detail->reward_scale_ = item_id;
	}
	else
	{
		this->request_add_item(obj, item_id, item_num, item_bind);
	}

	//清空祝福值
	if (slot_info->clean_bless_ == true)
		player_detail->bless_ = 0;

	//贵重物品传闻
	if (slot_info->is_shout_)
	{
		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, this->logic_player()->role_detail().__name);
		GameCommon::push_brocast_para_string(para_vec, slot_info->item_name_);
		GameCommon::push_brocast_para_int(para_vec, item_num);

		int shout_id = act_detail->treasure_shout_id_;
		GameCommon::announce(shout_id, &para_vec);
	}

	player_detail->slot_item_id_[pre_slot_id] = item_id;
	player_detail->slot_item_num_[pre_slot_id] = item_num;

	LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
				this->role_id(), item_obj);

	WheelPlayerInfo::ItemRecord record = this->create_record(item_obj);
	this->record_person(player_detail, slot_info->person_record_, record);
	ProtoPersonRecord *person_record = respond.add_record();
	this->wheel_info_.record_serialize(person_record, record);

	return 0;
}

int LogicWheelPlayer::cabinet_sign_in(int activity_id)
{
	JUDGE_RETURN(LUCKY_WHEEL_SYSTEM->check_is_cabinet_activity(activity_id) == 0, -1);
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	if (player_detail->refresh_tick_ < act_detail->last_refresh_tick_)
	{
		//this->cabinet_refresh_work(activity_id);
		this->rand_slot_item(activity_id, LuckyWheelActivity::SHOP_SLOT_NUM);
	}

	return 0;
}

int LogicWheelPlayer::fetch_cabinet_info(int activity_id)
{

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_INFO,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto50102011 respond;
	respond.set_refresh_time(LUCKY_WHEEL_SYSTEM->get_refresh_timer(activity_id).left_second());
	respond.set_act_left_time(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));
	respond.set_act_refresh_times(act_detail->refresh_times_);
	respond.set_refresh_cost(act_detail->refresh_cost_);
	respond.set_activity_id(activity_id);
	for (int i = 0; i < LuckyWheelActivity::SHOP_SLOT_NUM; ++i)
	{
		WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[i];

		ProtoShopInfo* shop_info = respond.add_item_list();
		shop_info->set_is_buy(shop_slot.is_buy_);
		shop_info->set_cost(shop_slot.item_price_);
		shop_info->set_is_rarity(shop_slot.is_rarity_);
		shop_info->set_cost_pre(shop_slot.item_price_pre_);
		ProtoItem* item = shop_info->mutable_item();
		shop_slot.item_.serialize(item);
	}

	for (int i = 0; i < (int)act_detail->refresh_reward_list_.size(); ++i)
	{
		if (act_detail->refresh_times_ >= act_detail->refresh_reward_list_[i].refresh_times_)
		{
			if (this->vip_detail().__vip_level < act_detail->refresh_reward_list_[i].vip_level_)
			{
				respond.add_refresh_status_list(NONE_REWARD);
			}
			else if	(player_detail->refresh_reward_map_[i] == 0)
			{
				respond.add_refresh_status_list(HAVE_REWARD);
			}
			else if (player_detail->refresh_reward_map_[i] == 1)
			{
				respond.add_refresh_status_list(GONE_REWARD);
			}
		}
		else
		{
			respond.add_refresh_status_list(NONE_REWARD);
		}
	}

	FINER_PROCESS_RETURN(RETURN_CABINET_INFO, &respond);
}

int LogicWheelPlayer::cabinet_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102013*, request, RETURN_CABINET_BUY);

	int activity_id = request->activity_id();
	if(activity_id == 0)
		activity_id = LuckyWheelActivity::ACT_CABINET;

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_BUY,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int slot_id = request->slot_id();
	int is_all = request->is_all();
	Money cost;
	if (is_all)
	{
		for (int i = 0; i < LuckyWheelActivity::SHOP_SLOT_NUM; ++i)
		{
			WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[i];
			JUDGE_CONTINUE(shop_slot.is_buy_ == 0);
			Money temp(shop_slot.item_price_);
			cost += temp;
		}
	}
	else
	{
		WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[slot_id];
		if (shop_slot.is_buy_ == 0)
		{
			Money temp(shop_slot.item_price_);
			cost += temp;
		}
	}


	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_CABINET_BUY);
	inner.set_is_all(is_all);
	inner.set_amount(0);
	inner.set_slot_id(slot_id);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(cost.__gold);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::cabinet_buy_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_BUY,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int slot_id = request->slot_id();
	int is_all = request->is_all();
	SerialObj obj(ADD_FROM_CABINET_ACTIVITY, activity_id);
	if (is_all)
	{
		for (int i = 0; i < LuckyWheelActivity::SHOP_SLOT_NUM; ++i)
		{
			WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[i];
			JUDGE_CONTINUE(shop_slot.is_buy_ == 0);
			shop_slot.is_buy_ = 1;
			ItemObj item_obj(shop_slot.item_.id_, shop_slot.item_.amount_, shop_slot.item_.bind_);
			this->request_add_item(obj, shop_slot.item_.id_, shop_slot.item_.amount_, shop_slot.item_.bind_);

			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, shop_slot.day_, shop_slot.slot_id_);
			LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
						this->role_id(), item_obj);
		}
	}
	else
	{
		WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[slot_id];
		if (shop_slot.is_buy_ == 0)
		{
			shop_slot.is_buy_ = 1;
			ItemObj item_obj(shop_slot.item_.id_, shop_slot.item_.amount_, shop_slot.item_.bind_);
			this->request_add_item(obj, shop_slot.item_.id_, shop_slot.item_.amount_, shop_slot.item_.bind_);

			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, shop_slot.day_, shop_slot.slot_id_);
			LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
						this->role_id(), item_obj);
		}
	}
	this->fetch_cabinet_info(activity_id);

	int is_all_buy = 1;
	for (int i = 0; i < LuckyWheelActivity::SHOP_SLOT_NUM; ++i)
	{
		WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[i];
		JUDGE_CONTINUE(shop_slot.is_buy_ == 0);
		is_all_buy = 0;
		break;
	}

	if (is_all_buy)
	{
		int mail_id = act_detail->mail_id_;
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str());
		mail_info->add_goods(act_detail->refresh_reward_);
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}

	Proto50102013 respond;
	respond.set_status(1);
	respond.set_activity_id(activity_id);
	FINER_PROCESS_RETURN(RETURN_CABINET_BUY, &respond);
}

int LogicWheelPlayer::cabinet_refresh_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102012*, request, RETURN_CABINET_REFRESH);

	int activity_id = request->activity_id();
	if(activity_id == 0)
		activity_id = LuckyWheelActivity::ACT_CABINET;

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_REFRESH,
			ERROR_NO_FUN_ACTIVITY);
//	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);


	int times = request->times();

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_CABINET_REFRESH);
	inner.set_amount(times);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(act_detail->refresh_cost_);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id, 1);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::cabinet_refresh_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_REFRESH,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	//this->cabinet_refresh_work(activity_id, 1);
	if(this->rand_slot_item(activity_id, LuckyWheelActivity::SHOP_SLOT_NUM) == 0)
	{
		act_detail->refresh_times_ ++;
	}
	int times = request->amount() - 1;
	int is_refresh = 1;

	for (int i = 0; i < LuckyWheelActivity::SHOP_SLOT_NUM; ++i)
	{
		if (player_detail->shop_slot_map_[i].is_rarity_)
		{
			is_refresh = 0;
			break;
		}
	}

	if (is_refresh && times > 0)
	{
		Proto31400044 inner;
		inner.set_activity_id(activity_id);
		inner.set_recogn(RETURN_CABINET_REFRESH);
		inner.set_amount(times);
		ProtoMoney *money = inner.mutable_money();
		money->set_gold(act_detail->refresh_cost_);

		ProtoSerialObj* serial_obj = inner.mutable_obj();
		this->serial_draw_serialize(serial_obj, activity_id, 1);

		return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
	}
	else
	{
		return this->fetch_cabinet_info(activity_id);
	}

	return 0;
}

int LogicWheelPlayer::cabinet_refresh_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102014*, request, RETURN_CABINET_REWARDS);

	int activity_id = request->activity_id();
	if(activity_id == 0)
		activity_id = LuckyWheelActivity::ACT_CABINET;

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_CABINET_REWARDS,
			ERROR_NO_FUN_ACTIVITY);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int type = request->type();
	int level_limit = act_detail->refresh_reward_list_[type].vip_level_;
	int times_limit = act_detail->refresh_reward_list_[type].refresh_times_;

	CONDITION_NOTIFY_RETURN(this->vip_detail().__vip_level >= level_limit, RETURN_CABINET_REWARDS,
			ERROR_VIP_NOT_ENOUGH);

	CONDITION_NOTIFY_RETURN(act_detail->refresh_times_ >= times_limit, RETURN_CABINET_REWARDS,
			ERROR_ACT_NO_REWARD );

	CONDITION_NOTIFY_RETURN(player_detail->refresh_reward_map_[type] == 0, RETURN_CABINET_REWARDS,
			ERROR_REWARD_DRAWED );

	SerialObj obj(ADD_FROM_CABINET_ACTIVITY, activity_id);
	player_detail->refresh_reward_map_[type] = 1;
	this->request_add_reward(act_detail->refresh_reward_list_[type].refresh_reward_, obj);

	Proto50102014 respond;
	respond.set_type(type);
	respond.set_status(1);
	respond.set_activity_id(activity_id);
	FINER_PROCESS_RETURN(RETURN_CABINET_REWARDS, &respond);
}

int LogicWheelPlayer::cabinet_refresh_work(int activity_id, int type, int shop_slot_num, int shop_last_group)
{
	//JUDGE_RETURN(LUCKY_WHEEL_SYSTEM->check_is_cabinet_activity(activity_id) == 0, -1);
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_REQUEST_TIME_LIMIT,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

	//必先 和 可先
	IntSet show_set;
	IntSet must_show_set;

	IntSet fina_group_slot_set;		//最终随机格子集合
	IntMap fina_group_times_map;	//最终分组个数集合
	int last_group_num = shop_slot_num;	//补漏组个数

	for (int i = 0; i < shop_last_group - 1; ++i)
	{
		JUDGE_CONTINUE(player_detail->group_refresh_times_map_[i] >= act_detail->group_no_show_list_[i]);
		int group_id = i + 1;
		if (player_detail->group_refresh_times_map_[i] >= act_detail->group_show_list_[i])
		{
			must_show_set.insert(group_id);
		}
		else
		{
			show_set.insert(group_id);
		}
	}

	//必先个数
	IntMap show_limit_pro_map; //必先个数概率分布
	int pro_sum = 0;
	for (int i = 0; i < (int) act_detail->group_pro_list_.size(); ++i)
	{
		pro_sum += act_detail->group_pro_list_[i];
		show_limit_pro_map[i] = pro_sum;
	}

	Time_Value nowtime = Time_Value::gettimeofday();
	std::srand(nowtime.sec() + nowtime.usec());

	for (IntSet::iterator it = must_show_set.begin(); it != must_show_set.end(); ++it)
	{
		int rand_num = std::rand() % pro_sum;
		int rand_times = 0;
		for (int i = 0; i < (int) act_detail->group_limit_list_.size(); ++i)
		{
			if (show_limit_pro_map[i] >= rand_num)
			{
				rand_times = i + 1;
				break;
			}
		}
		int group_id = *it;
		fina_group_times_map[group_id] = rand_times;
		last_group_num -= rand_times;
	}

	//必先组号

	for (IntSet::iterator it = must_show_set.begin(); it != must_show_set.end(); ++it)
	{
		int group_id = *it;
		int times = fina_group_times_map[group_id];

		IntMap must_show_group_pro_map;
		pro_sum = 0;
		for (int i = 1; i <= (int)slot_map->size(); ++i)
		{
			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
			JUDGE_CONTINUE(group_id == slot_info->group_id_);
			pro_sum += slot_info->group_weight_;
			must_show_group_pro_map[i] = pro_sum;
		}
		if (pro_sum > 0)
		{
			for (int j = 0; j < times; ++j)
			{
				int slot_id = 0;
				int rand_times = 0;
				while (rand_times < 50)
				{
					++rand_times;
					int rand_num = std::rand() % pro_sum;

					for (int i = 1; i <= (int) slot_map->size(); ++i)
					{
						LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
						JUDGE_CONTINUE(group_id == slot_info->group_id_);

						if (must_show_group_pro_map[i] >= rand_num)
						{
							slot_id = i;
							break;
						}
					}

					if (fina_group_slot_set.count(slot_id) == 0)
					{
						MSG_USER("break OK,cur slot_id:%d, must_num:%d, rand_num:%d, rand_times:%d", slot_id, must_show_group_pro_map[slot_id], rand_num,  rand_times);
						break;
					}
					MSG_USER("no break,cur slot_id:%d, must_num:%d, rand_num:%d, rand_times:%d", slot_id, must_show_group_pro_map[slot_id + 1], rand_num, rand_times);
				}
				JUDGE_CONTINUE(slot_id > 0);
				fina_group_slot_set.insert(slot_id);
			}
		}
	}
	//可先
	IntMap show_group_pro_map;
	pro_sum = 0;
	for (int i = 1; i <= (int)slot_map->size(); ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
		JUDGE_CONTINUE(show_set.count(slot_info->group_id_) || slot_info->group_id_ == shop_last_group);
		pro_sum += slot_info->group_weight_;
		show_group_pro_map[i] = pro_sum;
	}

	if (pro_sum > 0)
	{
		int times = last_group_num;
		for (int j = 0; j < times; ++j)
		{
			int slot_id = 0;
			while (1)
			{
				int rand_num = std::rand() % pro_sum;

				for (int i = 1; i <= (int) slot_map->size(); ++i)
				{
					LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
					JUDGE_CONTINUE(show_set.count(slot_info->group_id_) || slot_info->group_id_ == shop_last_group);

					if (show_group_pro_map[i] >= rand_num)
					{
						slot_id = i;
						break;
					}
				}

				if (fina_group_slot_set.count(slot_id) == 0) break;
			}
			JUDGE_CONTINUE(slot_id > 0);
			fina_group_slot_set.insert(slot_id);
			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
			int group_id = slot_info->group_id_;
			fina_group_times_map[group_id]++;
			last_group_num--;

			JUDGE_CONTINUE(group_id != shop_last_group);//补漏组没有限制
			if (fina_group_times_map[group_id] > act_detail->group_limit_list_[group_id])
			{
				fina_group_times_map[group_id]--;
				fina_group_slot_set.erase(slot_id);
				last_group_num++;
			}
		}
	}
	//补漏组概率分布
	IntMap last_group_pro_map;
	pro_sum = 0;
	for (int i = 1; i <= (int)slot_map->size(); ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
		JUDGE_CONTINUE(slot_info->group_id_ == shop_last_group);
		pro_sum += slot_info->group_weight_;
		last_group_pro_map[i] = pro_sum;
	}

	if (pro_sum > 0)
	{
		int times = last_group_num;
		for (int j = 0; j < times; ++j)
		{
			int slot_id = 0;
			while (1)
			{
				int rand_num = std::rand() % pro_sum;

				for (int i = 1; i <= (int) slot_map->size(); ++i)
				{
					LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, i);
					JUDGE_CONTINUE(slot_info->group_id_ == shop_last_group);

					if (last_group_pro_map[i] >= rand_num)
					{
						slot_id = i;
						break;
					}
				}

				if (fina_group_slot_set.count(slot_id) == 0) break;
			}

			fina_group_slot_set.insert(slot_id);
			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
			int group_id = slot_info->group_id_;
			fina_group_times_map[group_id]++;
		}
	}

	//刷新
	int i = 0;
	for (IntSet::iterator it = fina_group_slot_set.begin(); it != fina_group_slot_set.end(); ++it, ++i)
	{
		int slot_id = *it;
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
		player_detail->shop_slot_map_[i].slot_id_ = slot_id;
		player_detail->shop_slot_map_[i].is_buy_ = 0;
		player_detail->shop_slot_map_[i].group_id_ = slot_info->group_id_;
		player_detail->shop_slot_map_[i].is_cast_ = slot_info->is_cast_;
		player_detail->shop_slot_map_[i].is_rarity_ = slot_info->is_rarity_;
		player_detail->shop_slot_map_[i].item_price_ = slot_info->item_price_;
		player_detail->shop_slot_map_[i].item_price_pre_ = slot_info->now_cost_;
		player_detail->shop_slot_map_[i].day_ = cur_day;
		player_detail->shop_slot_map_[i].item_ = ItemObj(slot_info->item_obj_.id_, slot_info->item_obj_.amount_,
				slot_info->item_obj_.bind_);

	}

	for (int i = 1; i <= shop_last_group; ++i)
	{
		if (fina_group_times_map[i] > 0)
		{
			player_detail->group_refresh_times_map_[i - 1] = 0;
		}
		else
		{
			player_detail->group_refresh_times_map_[i - 1]++;
		}
	}

	player_detail->refresh_tick_ = Time_Value::gettimeofday().sec();
	if (type) act_detail->refresh_times_ ++;

	return 0;
}

int LogicWheelPlayer::fetch_time_limit_info()
{
	int activity_id = LuckyWheelActivity::ACT_LIMIT_BUY;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto50102006 respond;
	respond.set_activity_id(activity_id);
	respond.set_left_tick(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));
	respond.set_now_time_point(LUCKY_WHEEL_SYSTEM->fecth_now_time_point(act_detail));
	respond.set_limit_end_tick(LUCKY_WHEEL_SYSTEM->fetch_limit_end_tick(act_detail));
	respond.set_start_tick(LUCKY_WHEEL_SYSTEM->fetch_start_tick(act_detail));
	respond.set_buy_times(player_detail->fetch_buy_times());

	for (LuckyWheelActivity::ActivityDetail::TimeSlotInfoMap::iterator iter = act_detail->time_slot_map_.begin();
			iter != act_detail->time_slot_map_.end(); ++iter)
	{
		ProtoTimeLimitInfo *time_limit_info = respond.add_time_limit_info();
		int time_point_tick = LUCKY_WHEEL_SYSTEM->cal_day_time(iter->first);
		time_limit_info->set_time_point(iter->first);
		time_limit_info->set_time_point_tick(time_point_tick);

		LuckyWheelActivity::SlotInfoMap &slot_info_map = iter->second;
		for (LuckyWheelActivity::SlotInfoMap::iterator it = slot_info_map.begin();
				it != slot_info_map.end(); ++it)
		{
			LuckyWheelActivity::SlotInfo &slot_info = it->second;
			ProtoSlotInfo *slot = time_limit_info->add_slot_info();
			LUCKY_WHEEL_SYSTEM->slot_info_serialize(slot, &slot_info, player_detail);
			WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(iter->first, it->first);
			JUDGE_CONTINUE(person_slot != NULL);

			int person_left_limit = slot_info.person_limit_ - person_slot->buy_times_;
			slot->set_person_left_limit(person_left_limit);
		}
	}

	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->item_set_.begin();
			iter != act_detail->item_set_.end(); ++iter)
	{
		ProtoServerRecord *server_record = respond.add_server_record();
		LUCKY_WHEEL_SYSTEM->record_serialize(server_record, *iter);
	}

	for (IntVec::iterator iter = player_detail->reward_location_.begin();
			iter != player_detail->reward_location_.end(); ++iter)
	{
		respond.add_reward_location(*iter);
	}

	FINER_PROCESS_RETURN(RETURN_REQUEST_TIME_LIMIT, &respond);
}

int LogicWheelPlayer::time_limit_item_buy_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102007*, request, RETURN_TIME_LIMIT_BUY);

	JUDGE_RETURN(this->add_validate_operate_tick() == true, 0);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_TIME_LIMIT_BUY,
			ERROR_NO_FUN_ACTIVITY);

	int limit_end_tick = LUCKY_WHEEL_SYSTEM->fetch_limit_end_tick(act_detail);
	CONDITION_NOTIFY_RETURN(limit_end_tick > 0, RETURN_TIME_LIMIT_BUY,
			ERROR_NOT_IN_BUY_TIME);

	int slot_id = request->slot_id();
	int time_point = LUCKY_WHEEL_SYSTEM->fecth_now_time_point(act_detail);
	LuckyWheelActivity::SlotInfo *slot_info = LUCKY_WHEEL_SYSTEM->fetch_limit_time_slot(act_detail, time_point, slot_id);
	CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_TIME_LIMIT_BUY, ERROR_CLIENT_OPERATE);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(time_point, slot_id);
	CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_TIME_LIMIT_BUY, ERROR_SERVER_INNER);

	CONDITION_NOTIFY_RETURN(slot_info->server_limit_ > slot_info->server_buy_,
			RETURN_TIME_LIMIT_BUY, ERROR_SERVER_LIMIT);
	CONDITION_NOTIFY_RETURN(slot_info->person_limit_ > person_slot->buy_times_,
			RETURN_TIME_LIMIT_BUY, ERROR_PERSON_LIMIT);

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_TIME_LIMIT_BUY);
	inner.set_time_point(time_point);
	inner.set_slot_id(slot_id);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(slot_info->now_cost_);
	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::time_limit_item_buy_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_TIME_LIMIT_BUY,
			ERROR_NO_FUN_ACTIVITY);

	int time_point = request->time_point();
	int slot_id = request->slot_id();
	LuckyWheelActivity::SlotInfo *slot_info = LUCKY_WHEEL_SYSTEM->fetch_limit_time_slot(act_detail, time_point, slot_id);
	CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_TIME_LIMIT_BUY, ERROR_CLIENT_OPERATE);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(time_point, slot_id);
	CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_TIME_LIMIT_BUY, ERROR_SERVER_INNER);

	slot_info->server_buy_ += 1;
	person_slot->buy_times_ += 1;

	SerialObj obj(ADD_FROM_TIME_LIMIT_ACTIVITY, activity_id);
	ItemObj item_obj = slot_info->item_obj_;
	this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);

	LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
				this->role_id(), item_obj);

	FINER_PROCESS_NOTIFY(RETURN_TIME_LIMIT_BUY);
}

int LogicWheelPlayer::fetch_activity_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102008*, request, RETURN_TIME_LIMIT_REWARD);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_TIME_LIMIT_REWARD,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_TIME_LIMIT_REWARD, ERROR_ACTIVITY_TIME_INVALID);

	int location_id = request->location_id();

	int ret = false;
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	for (IntVec::iterator iter = player_detail->reward_location_.begin();
			iter != player_detail->reward_location_.end(); ++iter)
	{
		JUDGE_CONTINUE(location_id == *iter);
		ret = true;
	}
	CONDITION_NOTIFY_RETURN(ret == false, RETURN_TIME_LIMIT_REWARD, ERROR_AWARD_HAS_GET);

	if (activity_id == LuckyWheelActivity::ACT_LIMIT_BUY)
	{
		const Json::Value &limit_json = CONFIG_INSTANCE->time_limit_reward(location_id);
		CONDITION_NOTIFY_RETURN(limit_json != Json::Value::null, RETURN_TIME_LIMIT_REWARD,
				ERROR_CONFIG_NOT_EXIST);

		int buy_times = player_detail->fetch_buy_times();
		CONDITION_NOTIFY_RETURN(buy_times >= limit_json["num"].asInt(), RETURN_TIME_LIMIT_REWARD,
				ERROR_AWARD_CAN_NOT_GET);

		int item_id = limit_json["item_id"].asInt();
		int amount = limit_json["amount"].asInt();
		int item_bind = limit_json["item_bind"].asInt();
		SerialObj obj(ADD_FROM_TIME_LIMIT_REWARD, activity_id);
		this->request_add_item(obj, item_id, amount, item_bind);
	}
	else if (activity_id == LuckyWheelActivity::ACT_LUCKY_EGG)
	{
		const Json::Value &egg_json = CONFIG_INSTANCE->egg_reward(location_id);
		CONDITION_NOTIFY_RETURN(egg_json != Json::Value::null, RETURN_TIME_LIMIT_REWARD,
				ERROR_CONFIG_NOT_EXIST);

		int open_times = player_detail->open_times_;
		CONDITION_NOTIFY_RETURN(open_times >= egg_json["num"].asInt(), RETURN_TIME_LIMIT_REWARD,
				ERROR_AWARD_CAN_NOT_GET);

		int reward_id = egg_json["reward_id"].asInt();
		SerialObj obj(ADD_FROM_LUCKY_EGG_REWARD, activity_id);
		this->request_add_reward(reward_id, obj);
	}

	player_detail->reward_location_.push_back(location_id);

	Proto50102008 respond;
	respond.set_location_id(location_id);
	respond.set_activity_id(activity_id);
	FINER_PROCESS_RETURN(RETURN_TIME_LIMIT_REWARD, &respond);
}

int LogicWheelPlayer::fetch_wedding_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102009*, request, RETURN_REQUEST_COUPLE_ACT);

	int page = request->page();
	JUDGE_RETURN(page > 0, 0);

	int activity_id = LuckyWheelActivity::ACT_COUPLE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto30400508 inner;
	inner.set_page(page);
	inner.set_label_reward(act_detail->label_reward_);
	inner.set_rank_reward(act_detail->rank_reward_);
	inner.set_label_get(player_detail->label_get_);
	inner.set_reward_get(player_detail->rank_get_);
	inner.set_role_id(this->role_id());
	inner.set_left_tick(GameCommon::next_day());

	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::TRVL_WEDDING_SCENE_ID, &inner);
}

int LogicWheelPlayer::fetch_wedding_act_reward_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102010*, request, RETURN_FETCH_COUPLE_ACT_REWARD);

	int activity_id = LuckyWheelActivity::ACT_COUPLE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_COUPLE_ACT_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	Proto30400510 inner;
	inner.set_type(request->type());
	inner.set_role_id(this->role_id());
	return LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_WEDDING_SCENE_ID, &inner);
}

int LogicWheelPlayer::fetch_wedding_act_reward_end(int type, int rank)
{
	int activity_id = LuckyWheelActivity::ACT_COUPLE;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_COUPLE_ACT_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	if (type == 1)
	{
		CONDITION_NOTIFY_RETURN(player_detail->label_get_ == false, RETURN_FETCH_COUPLE_ACT_REWARD,
				ERROR_COUPLE_ACT_HAS_GET_REWARD);

		player_detail->label_get_ = true;
		this->request_add_reward(act_detail->label_reward_, ADD_FROM_COUPLE_ACT);

		act_detail->erase_role_mail_reward(this->role_id(), act_detail->label_reward_);
	}
	else
	{
		CONDITION_NOTIFY_RETURN(player_detail->rank_get_ == false, RETURN_FETCH_COUPLE_ACT_REWARD,
				ERROR_COUPLE_ACT_HAS_GET_REWARD);
		CONDITION_NOTIFY_RETURN(rank <= act_detail->rank_limit_, RETURN_FETCH_COUPLE_ACT_REWARD,
				ERROR_COUPLE_ACT_HAS_GET_REWARD);

		player_detail->rank_get_ = true;
		this->request_add_reward(act_detail->rank_reward_, ADD_FROM_COUPLE_ACT);

		act_detail->erase_role_mail_reward(this->role_id(), act_detail->rank_reward_);
	}

	Proto50102010 respond;
	respond.set_type(type);
	FINER_PROCESS_RETURN(RETURN_FETCH_COUPLE_ACT_REWARD, &respond);
}

int LogicWheelPlayer::fetch_recharge_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102051*, request, RETURN_FETCH_RECHARGE_RANK);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_RECHARGE_RANK,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(activity_id == LuckyWheelActivity::ACT_DAILY_GET
			|| activity_id == LuckyWheelActivity::ACT_DAILY_COST,
			RETURN_FETCH_RECHARGE_RANK, ERROR_CLIENT_OPERATE);

	Proto30400512 inner;
	inner.set_page(request->page());

	if (activity_id == LuckyWheelActivity::ACT_DAILY_GET)
		inner.set_type(1);
	else
		inner.set_type(2);

	MSG_USER("Proto30400512: %s", inner.Utf8DebugString().c_str());
	LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_RECHARGE_SCENE_ID, &inner);
	return 0;
}

int LogicWheelPlayer::update_recharge_rank(int type, int money)
{
	int activity_id = 0;
	if (type == 1)
	{
		activity_id = LuckyWheelActivity::ACT_DAILY_GET;
	}
	else
	{
		activity_id = LuckyWheelActivity::ACT_DAILY_COST;
	}

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, 0);

	Proto30400511 inner;
	inner.set_type(type);
	inner.set_amount(money);

	ProtoServer* server = inner.mutable_server_info();
	ProtoTeamer* teamer = inner.mutable_self_info();

	this->role_detail().serialize(server, true);
	this->role_detail().make_up_teamer_info(this->role_id(), teamer);

	return LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_RECHARGE_SCENE_ID, &inner);
}

int LogicWheelPlayer::test_send_recharge_mail()
{
	Proto30400514 inner;
	LOGIC_MONITOR->dispatch_to_scene(this, GameEnum::TRVL_RECHARGE_SCENE_ID, &inner);
	return 0;
}

int LogicWheelPlayer::request_open_nine_word_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102016*, request, RETURN_OPEN_NINE_WORD);

	int activity_id = LuckyWheelActivity::ACT_NINE_WORD;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_OPEN_NINE_WORD,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_OPEN_NINE_WORD, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail->fetch_nine_word_num() < LuckyWheelActivity::NINE_WORD_NUM,
			RETURN_OPEN_NINE_WORD, ERROR_NINE_WORD_IS_ALL);

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_type(request->type());
	inner.set_column(request->column());
	inner.set_num(request->num());

	ProtoMoney *money = inner.mutable_money();
	if (request->type() == true)
	{
		money->set_gold(act_detail->ten_cost_);
	}
	else
	{
		int column = request->column();
		int num = request->num();
		CONDITION_NOTIFY_RETURN(column > 0 && column <= int(player_detail->person_slot_set_.size()),
				RETURN_OPEN_NINE_WORD, ERROR_CLIENT_OPERATE);
		CONDITION_NOTIFY_RETURN(num > 0 && num <= LuckyWheelActivity::NINE_WORD_NUM,
				RETURN_OPEN_NINE_WORD, ERROR_CLIENT_OPERATE);

		WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(0, column);
		CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_OPEN_NINE_WORD, ERROR_SERVER_INNER);
		CONDITION_NOTIFY_RETURN(person_slot->is_find_word() == false, RETURN_OPEN_NINE_WORD,
				ERROR_COLUMN_FIND_WORD);

		int word = person_slot->nine_slot_[num];
		CONDITION_NOTIFY_RETURN(word <= 0, RETURN_OPEN_NINE_WORD, ERROR_SLOT_IS_OPEN);

		money->set_gold(act_detail->draw_cost_);
	}

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id);

	LOGIC_MONITOR->dispatch_to_scene(this, &inner);

	return 0;
}

int LogicWheelPlayer::request_open_nine_word_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_OPEN_NINE_WORD,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto50102016 respond;
	respond.set_type(request->type());

	SerialObj obj(ADD_FROM_NINE_WORD_OPEN_SLOT, activity_id);

	if (request->type() == true)
	{
		int total_num = 0;
		for (WheelPlayerInfo::PersonSlotSet::iterator iter = player_detail->person_slot_set_.begin();
				iter != player_detail->person_slot_set_.end(); ++iter)
		{
			WheelPlayerInfo::PersonSlot &person_slot = *iter;
			JUDGE_CONTINUE(person_slot.is_find_word() == false);

			ProtoSlotInfo *proto_slot = respond.add_slot_info();
			proto_slot->set_slot_id(person_slot.slot_id_);

			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, 1, person_slot.slot_id_);
			JUDGE_CONTINUE(slot_info != NULL);

			ItemObj item_obj = slot_info->item_obj_;

			int open_num = person_slot.open_num() - 1;
			for (IntMap::iterator it = person_slot.nine_slot_.begin();
					it != person_slot.nine_slot_.end(); ++it)
			{
				++open_num;
				JUDGE_CONTINUE(it->second <= LuckyWheelActivity::SLOT_NOT_OPEN);

				ProtoPairObj *pair_obj = proto_slot->add_pair_info();
				pair_obj->set_obj_id(it->first);

				++total_num;
				int break_flag = false;
				if (act_detail->check_is_open_word(open_num) == true)
				{
					it->second = LuckyWheelActivity::SLOT_IS_WORD;
					pair_obj->set_obj_value(it->second);
					break_flag = true;
				}
				else
				{
					it->second = LuckyWheelActivity::SLOT_NOT_WORD;
					pair_obj->set_obj_value(it->second);
				}

				//点开格子奖励
				this->add_item(item_obj, obj);

				JUDGE_BREAK(break_flag == false);
			}

			this->add_lucky_rank_num(player_detail, act_detail);
		}

		act_detail->update_rank_info(this->role_id(), this->name(), total_num);

		//全部开启奖励
		SerialObj obj(ADD_FROM_NINE_WORD_ALL_OPEN, activity_id);
		this->request_add_reward(act_detail->lighten_reward_, ADD_FROM_NINE_WORD_ALL_OPEN);
	}
	else
	{
		respond.set_column(request->column());
		respond.set_num(request->num());
		WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(0, request->column());
		CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_OPEN_NINE_WORD, ERROR_SERVER_INNER);

		int word_num = person_slot->open_num();
		if (act_detail->check_is_open_word(word_num) == true)
		{
			person_slot->nine_slot_[request->num()] = LuckyWheelActivity::SLOT_IS_WORD;
			respond.set_is_num(LuckyWheelActivity::SLOT_IS_WORD);

			this->add_lucky_rank_num(player_detail, act_detail);
		}
		else
		{
			person_slot->nine_slot_[request->num()] = LuckyWheelActivity::SLOT_NOT_WORD;
			respond.set_is_num(LuckyWheelActivity::SLOT_NOT_WORD);
		}

		//点开格子奖励
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, 1, request->column());
		if (slot_info != NULL)
		{
			ItemObj item_obj = slot_info->item_obj_;
			this->add_item(item_obj, obj);
		}

		act_detail->update_rank_info(this->role_id(), this->name(), 1);
	}

	FINER_PROCESS_RETURN(RETURN_OPEN_NINE_WORD, &respond);
}

int LogicWheelPlayer::fetch_nine_word_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102017*, request, RETURN_FETCH_NINE_WORD_REWARD);

	int activity_id = LuckyWheelActivity::ACT_NINE_WORD;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_NINE_WORD_REWARD,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_FETCH_NINE_WORD_REWARD, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int word_num = player_detail->fetch_nine_word_num();

	int reward_type = request->reward_type();
	if (reward_type == 1)
	{
		CONDITION_NOTIFY_RETURN(player_detail->reward_get_ == false, RETURN_FETCH_NINE_WORD_REWARD,
				ERROR_COUPLE_ACT_HAS_GET_REWARD);

		CONDITION_NOTIFY_RETURN(word_num >= LuckyWheelActivity::SIX_WORD_REWARD_NUM,
				RETURN_FETCH_NINE_WORD_REWARD, ERROR_REWARD_GET_LIMIT);

		player_detail->reward_get_ = true;
		this->request_add_reward(act_detail->six_reward_, ADD_FROM_LIGHTEN_SIX_WORD);
	}
	else
	{
		int json_id = player_detail->nine_word_reward_ + 1;
		const Json::Value &word_reward = CONFIG_INSTANCE->word_reward(json_id);
		CONDITION_NOTIFY_RETURN(word_reward != Json::Value::null, RETURN_FETCH_NINE_WORD_REWARD,
				ERROR_CONFIG_NOT_EXIST);

		int need_amount = word_reward["amount"].asInt();
		CONDITION_NOTIFY_RETURN(word_num >= need_amount, RETURN_FETCH_NINE_WORD_REWARD,
				ERROR_REWARD_GET_LIMIT);

		int reward_id = word_reward["reward_id"].asInt();
		this->request_add_reward(reward_id, ADD_FROM_NINE_WORD_ACT_REWARD);

		player_detail->nine_word_reward_ += 1;
	}

	Proto50102017 respond;
	respond.set_reward_type(reward_type);
	respond.set_nine_word_reward(player_detail->nine_word_reward_);
	FINER_PROCESS_RETURN(RETURN_FETCH_NINE_WORD_REWARD, &respond);
}

int LogicWheelPlayer::fetch_act_rank_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102018*, request, RETURN_FETCH_ACT_RANK_INFO);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_ACT_RANK_INFO,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_FETCH_ACT_RANK_INFO, ERROR_ACTIVITY_TIME_INVALID);

	int page = request->page();
	CONDITION_NOTIFY_RETURN(page > 0, RETURN_FETCH_ACT_RANK_INFO, ERROR_CLIENT_OPERATE);

	PageInfo page_info;

	Proto50102018 respond;
	respond.set_activity_id(activity_id);
	respond.set_rank_type(request->rank_type());

	if (request->rank_type() == LuckyWheelActivity::RANK_TYPE_NUM)
	{
		GameCommon::game_page_info(page_info, page, act_detail->rank_num_map_.size(),
				LuckyWheelActivity::RANK_PAGE);
	}
	else
	{
		GameCommon::game_page_info(page_info, page, act_detail->rank_lucky_map_.size(),
				LuckyWheelActivity::RANK_PAGE);
	}
	respond.set_cur_page(page_info.cur_page_);
	respond.set_total_page(page_info.total_page_);

	for (int i = page_info.start_index_ + 1; i <= ::std::min(page_info.total_count_,
			page_info.start_index_ + LuckyWheelActivity::RANK_PAGE); ++i)
	{
		Int64 role_id = 0;
		if (request->rank_type() == LuckyWheelActivity::RANK_TYPE_NUM)
		{
			JUDGE_CONTINUE(int(act_detail->player_rank_vec_.size()) >= i);
			role_id = act_detail->player_rank_vec_[i-1].id_;
		}
		else
		{
			JUDGE_CONTINUE(int(act_detail->lucky_rank_vec_.size()) >= i);
			role_id = act_detail->lucky_rank_vec_[i-1].id_;
		}

		LuckyWheelActivity::OneRankInfo *rank_info = act_detail->fetch_rank_info(role_id, request->rank_type());
		ProtoActRankInfo *proto_rank = respond.add_rank_info();
		rank_info->serialize(proto_rank);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_ACT_RANK_INFO, &respond);
}

int LogicWheelPlayer::request_open_lucky_egg_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102041*, request, RETURN_OPEN_LUCKY_EGG);

	int activity_id = LuckyWheelActivity::ACT_LUCKY_EGG;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_OPEN_LUCKY_EGG,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_OPEN_LUCKY_EGG, ERROR_ACTIVITY_TIME_INVALID);

	int slot_id = request->slot_id();
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_slot_id(slot_id);
	int use_money = 0;

	if (slot_id <= 0)
	{
		int open_egg = player_detail->fetch_lucky_egg_open(1);
		int total_size = player_detail->person_slot_set_.size();
		CONDITION_NOTIFY_RETURN(open_egg < total_size, RETURN_OPEN_LUCKY_EGG, ERROR_ALL_EGG_IS_OPEN);

		use_money = act_detail->draw_cost_ * (total_size - open_egg);
	}
	else
	{
		WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(0, slot_id);
		CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_OPEN_LUCKY_EGG, ERROR_SERVER_INNER);
		CONDITION_NOTIFY_RETURN(person_slot->item_.validate() == false,
				RETURN_OPEN_LUCKY_EGG, ERROR_SLOT_IS_OPEN);

		int free_time = this->cal_free_times(player_detail, act_detail);
		if (free_time > player_detail->use_free_)
		{
			player_detail->use_free_ += 1;
			this->request_open_lucky_egg_end(&inner);

			return 0;
		}
		else
		{
			use_money = act_detail->draw_cost_;
		}
	}

	ProtoMoney *money = inner.mutable_money();
	money->set_gold(use_money);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id);

	LOGIC_MONITOR->dispatch_to_scene(this, &inner);
	return 0;
}

int LogicWheelPlayer::request_open_lucky_egg_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	int slot_id = request->slot_id();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_OPEN_LUCKY_EGG, ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	SerialObj obj(ADD_FROM_LUCKY_EGG_OPEN, activity_id);

	Proto50102041 respond;
	respond.set_slot_id(slot_id);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);

	if (slot_id <= 0)
	{
		for (WheelPlayerInfo::PersonSlotSet::iterator iter = player_detail->person_slot_set_.begin();
				iter != player_detail->person_slot_set_.end(); ++iter)
		{
			WheelPlayerInfo::PersonSlot &person_slot = *iter;
			JUDGE_CONTINUE(person_slot.item_.validate() == false);

			LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, person_slot.slot_id_);
			JUDGE_CONTINUE(slot_info != NULL);

			person_slot.item_ = act_detail->fetch_lucky_egg_slot_reward(slot_info, person_slot.is_color_, true);
			this->add_item(person_slot.item_, obj);

			WheelPlayerInfo::ItemRecord record = this->create_record(person_slot.item_);
			this->record_person(player_detail, slot_info->person_record_, record);

			ProtoSlotInfo *proto_slot = respond.add_slot_info();
			LUCKY_WHEEL_SYSTEM->slot_info_serialize(proto_slot, slot_info, player_detail);

			this->player_detail_change(player_detail, act_detail);
		}
	}
	else
	{
		WheelPlayerInfo::PersonSlot *person_slot = player_detail->fetch_person_slot(0, slot_id);
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, person_slot->slot_id_);
		CONDITION_NOTIFY_RETURN(person_slot != NULL, RETURN_OPEN_LUCKY_EGG, ERROR_SERVER_INNER);
		CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_OPEN_LUCKY_EGG, ERROR_SERVER_INNER);

		int is_money = false;
		ProtoMoney money = request->money();
		if (money.gold() > 0)
			is_money = true;

		person_slot->item_ = act_detail->fetch_lucky_egg_slot_reward(slot_info, person_slot->is_color_, is_money);
		this->add_item(person_slot->item_, obj);

		WheelPlayerInfo::ItemRecord record = this->create_record(person_slot->item_);
		this->record_person(player_detail, slot_info->person_record_, record);

		ProtoSlotInfo *proto_slot = respond.add_slot_info();
		LUCKY_WHEEL_SYSTEM->slot_info_serialize(proto_slot, slot_info, player_detail);

		this->player_detail_change(player_detail, act_detail);
	}

//	this->fetch_activity_info(act_detail);

	FINER_PROCESS_RETURN(RETURN_OPEN_LUCKY_EGG, &respond);
}

int LogicWheelPlayer::draw_award_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102003*, request, RETURN_DRAW_LUCKY_WHEEL_REWARD);

	int activity_id = request->activity_id();
	int amount = request->amount();
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int free_time = this->cal_free_times(player_detail, act_detail);

	if (act_detail->draw_limit_ > 0)
	{
		CONDITION_NOTIFY_RETURN(player_detail->wheel_times_ + amount <= act_detail->draw_limit_,
				RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_LIMIT_TIMES);
	}

	int left_count = 0;
	if (activity_id == LuckyWheelActivity::ACT_ADVANCE_BOX)
	{
		int slot_num = player_detail->wheel_times_ + 1;
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, 1, slot_num);
		CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_LIMIT_TIMES);

		LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, 1);
		for(int i = player_detail->wheel_times_ + 1; slot_map->count(i) > 0; ++i)
		{
			left_count++;
		}
	}

	int type = request->type();

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_amount(amount);
	inner.set_type(type);
	if (LUCKY_WHEEL_SYSTEM->is_need_cost(act_detail) == true &&
			free_time <= player_detail->use_free_)
	{
		ProtoMoney *money = inner.mutable_money();
		if (amount > 1)
			money->set_gold(act_detail->ten_cost_);
		else
			money->set_gold(act_detail->draw_cost_);

		if (activity_id == LuckyWheelActivity::ACT_ADVANCE_BOX)
		{
			if(type > 0)
			{
				CONDITION_NOTIFY_RETURN(left_count > 0, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_LIMIT_TIMES);
				money->set_gold(act_detail->draw_cost_ * left_count);
			}
		}


		if(activity_id == LuckyWheelActivity::ACT_GOLD_BOX)
		{
			//元宝宝匣类型
			if (type > 0)
			{
				CONDITION_NOTIFY_RETURN(type <= int(act_detail->draw_type_list_.size()), RETURN_DRAW_LUCKY_WHEEL_REWARD,
						ERROR_CONFIG_NOT_EXIST);
				money->set_gold(act_detail->draw_type_list_[type - 1]);
			}
		}

		ProtoSerialObj* serial_obj = inner.mutable_obj();
		this->serial_draw_serialize(serial_obj, activity_id);

		LOGIC_MONITOR->dispatch_to_scene(this, &inner);
	}
	else if (free_time > player_detail->use_free_)
	{
		if (activity_id == LuckyWheelActivity::ACT_ADVANCE_BOX)
		{
			if(type > 0)
			{
				int left_time = free_time - player_detail->use_free_;
				ProtoMoney *money = inner.mutable_money();
				int gold = (left_count - left_time) * act_detail->draw_cost_;
				money->set_gold(gold);
				ProtoSerialObj* serial_obj = inner.mutable_obj();
				this->serial_draw_serialize(serial_obj, activity_id);

				return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
			}
			else
			{
				player_detail->use_free_ += 1;
			}
		}
		else
		{
			player_detail->use_free_ += 1;
		}
		this->draw_award_done(&inner);
	}

	return 0;
}

int LogicWheelPlayer::draw_award_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	switch (activity_id)
	{
	case LuckyWheelActivity::ACT_LUCKY_WHEEL:
	{
		return this->draw_lucky_wheel_award_done(msg);
	}
	case LuckyWheelActivity::ACT_ADVANCE_BOX:
	{
		return this->draw_advance_box_award_done(msg);
	}
	case LuckyWheelActivity::ACT_GOLD_BOX:
	{
		return this->draw_gold_box_award_done(msg);
	}
	case LuckyWheelActivity::ACT_LIMIT_BUY:
	{
		return this->time_limit_item_buy_done(msg);
	}
	case LuckyWheelActivity::ACT_CABINET:
	case LuckyWheelActivity::ACT_CABINET_DISCOUNT:
	{
		if (request->amount() == 0)
			return this->cabinet_buy_end(msg);
		else
			return this->cabinet_refresh_end(msg);
	}
	case LuckyWheelActivity::ACT_ANTIQUES:
	{
		if (request->amount() == 0)
			return this->draw_immortal_treasures_end(msg);
		else
			return this->rand_immortal_treasures_end(msg);
	}
	case LuckyWheelActivity::ACT_MAZE:
	{
		return this->draw_maze_end(msg);
	}
	case LuckyWheelActivity::ACT_NINE_WORD:
	{
		return this->request_open_nine_word_end(msg);
	}
	case LuckyWheelActivity::ACT_LUCKY_EGG:
	{
		return this->request_open_lucky_egg_end(msg);
	}
	case LuckyWheelActivity::ACT_FISH:
	{
		return this->fetch_fish_end(msg);
	}
	case LuckyWheelActivity::ACT_GODDESS_BLESS:
	{
		return this->goddess_bless_operator_end(msg);
	}
	default:
		break;
	}
	return 0;
}

int LogicWheelPlayer::draw_lucky_wheel_award_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	int draw_amount = request->amount();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(activity_detail != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(draw_amount > 0, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_SERVER_INNER);

	Proto50102003 respond;
	respond.set_activity_id(activity_id);
	respond.set_amount(draw_amount);
	for (int i = 1; i <= draw_amount; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->rand_get_slot(activity_id, player_detail->wheel_times_);
		CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_SERVER_INNER);

		ItemObj item_obj = LUCKY_WHEEL_SYSTEM->fetch_slot_reward(slot_info, player_detail->wheel_times_);
		SerialObj obj(ADD_FROM_LUCKY_WHEEL_REWARD, activity_id);
		int add_money = LUCKY_WHEEL_SYSTEM->add_pool_money(activity_detail, slot_info);
		int gold_percent = 0;
		if (add_money > 0)
		{
			Money money(add_money);
			this->request_add_money(money, obj, false);
			item_obj.id_	= GameEnum::ITEM_MONEY_UNBIND_GOLD;
			item_obj.amount_= add_money;
			gold_percent = slot_info->pool_percent_;
		}
		else
		{
			this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);
		}

		LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(activity_detail,
				slot_info, this->role_id(), item_obj, gold_percent);

		this->player_detail_change(player_detail, activity_detail);

		WheelPlayerInfo::ItemRecord record = this->create_record(item_obj, gold_percent);
		this->record_person(player_detail, slot_info->person_record_, record);

		respond.add_slot_set(slot_info->slot_id_);
		ProtoPersonRecord *person_record = respond.add_record();
		this->wheel_info_.record_serialize(person_record, record);
	}

	FINER_PROCESS_RETURN(RETURN_DRAW_LUCKY_WHEEL_REWARD, &respond);
}

int LogicWheelPlayer::draw_advance_box_award_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

	int type = request->type();
	Proto50102003 respond;
	respond.set_activity_id(activity_id);

	//特殊写
	int free_time = this->cal_free_times(player_detail, activity_detail);
	if(type > 0 && free_time > player_detail->use_free_)
		player_detail->use_free_ += free_time;

	LuckyWheelActivity::SlotInfoMap *slot_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(activity_detail, 1);
	for(int i = player_detail->wheel_times_ + 1; slot_map->count(i) > 0; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(activity_detail, 1, i);
		CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_SERVER_INNER);

		int rand_amount = LUCKY_WHEEL_SYSTEM->fetch_rand_amount(slot_info);
		CONDITION_NOTIFY_RETURN(rand_amount > 0, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_CONFIG_ERROR);

		SerialObj obj(ADD_FROM_ADVANCE_BOX_REWARD, activity_id);
		ItemObj item_obj = slot_info->item_obj_;
		item_obj.amount_ *= rand_amount;
		this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);

		LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(activity_detail, slot_info,
					this->role_id(), item_obj);

		this->player_detail_change(player_detail, activity_detail);

		WheelPlayerInfo::ItemRecord record = this->create_record(item_obj, slot_info->is_precious_);
		this->record_person(player_detail, slot_info->person_record_, record);

		respond.add_slot_set(slot_info->slot_id_);

		ProtoPersonRecord *person_record = respond.add_record();
		this->wheel_info_.record_serialize(person_record, record);

		JUDGE_BREAK(type > 0);
	}

	FINER_PROCESS_RETURN(RETURN_DRAW_LUCKY_WHEEL_REWARD, &respond);
}

int LogicWheelPlayer::draw_gold_box_award_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);

	int activity_id = request->activity_id();
	int draw_amount = request->amount();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(draw_amount > 0, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_SERVER_INNER);

	Proto50102003 respond;
	respond.set_activity_id(activity_id);
	respond.set_amount(draw_amount);
	for (int i = 1; i <= draw_amount; ++i)
	{
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->gold_box_rand_get_slot(activity_id, request->type());
		CONDITION_NOTIFY_RETURN(slot_info != NULL, RETURN_DRAW_LUCKY_WHEEL_REWARD, ERROR_SERVER_INNER);

		int amount = request->money().gold() * slot_info->reward_mult_ / GameEnum::ES_ACT_BASE_NUM;
		int item_id = GameEnum::ITEM_MONEY_BIND_GOLD;
		SerialObj obj(ADD_FROM_GOLD_BOX_REWARD, activity_id);

		this->request_add_item(obj, item_id, amount, 1);

		ItemObj item_obj(item_id, amount);
		LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(activity_detail, slot_info,
				this->role_id(), item_obj, slot_info->reward_mult_);

		this->player_detail_change(player_detail, activity_detail);

		WheelPlayerInfo::ItemRecord record = this->create_record(item_obj, slot_info->reward_mult_);
		this->record_person(player_detail, slot_info->person_record_, record);

		respond.add_slot_set(slot_info->slot_id_);
		ProtoPersonRecord *person_record = respond.add_record();
		this->wheel_info_.record_serialize(person_record, record);
	}

	FINER_PROCESS_RETURN(RETURN_DRAW_LUCKY_WHEEL_REWARD, &respond);
}

int LogicWheelPlayer::lucky_wheel_exchange(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102004*, request, RETURN_LUCKY_WHEEL_EXCHANGE);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(activity_detail != NULL, RETURN_LUCKY_WHEEL_EXCHANGE,
			ERROR_NO_FUN_ACTIVITY);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_LUCKY_WHEEL_EXCHANGE, ERROR_ACTIVITY_TIME_INVALID);

	int exchange_id = request->exchange_id();
	const Json::Value &exchange_json = CONFIG_INSTANCE->score_exchange(exchange_id);
	CONDITION_NOTIFY_RETURN(exchange_json != Json::Value::null, RETURN_LUCKY_WHEEL_EXCHANGE,
			ERROR_CONFIG_NOT_EXIST);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int need_score = exchange_json["cost"].asInt();
	CONDITION_NOTIFY_RETURN(player_detail->act_score_ >= need_score,
			RETURN_LUCKY_WHEEL_EXCHANGE, ERROR_PACK_SCORE_AMOUNT);

	int item_id = exchange_json["item_id"].asInt();
	int amount = exchange_json["amount"].asInt();
	int item_bind = exchange_json["item_bind"].asInt();
	SerialObj obj(ADD_FROM_LUCKY_WHEEL_EXCHANGE, activity_id);
	this->request_add_item(obj, item_id, amount, item_bind);

	player_detail->act_score_ -= need_score;
	this->check_red_point(player_detail, activity_detail);

	Proto50102004 respond;
	respond.set_activity_id(activity_id);
	respond.set_cur_score(player_detail->act_score_);
	FINER_PROCESS_RETURN(RETURN_LUCKY_WHEEL_EXCHANGE, &respond);
}

int LogicWheelPlayer::request_activity_reset_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102005*, request, RETURN_RESET_LUCKY_WHEEL);

	int activity_id = request->activity_id();
	LuckyWheelActivity::ActivityDetail* activity_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(activity_detail != NULL, 0);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_RESET_LUCKY_WHEEL, ERROR_ACTIVITY_TIME_INVALID);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int reset_times = player_detail->reset_times_ + 1;
	int cost = this->fetch_reset_cost(activity_id, reset_times, activity_detail->reset_cost_);
	CONDITION_NOTIFY_RETURN(cost >= 0, RETURN_RESET_LUCKY_WHEEL, ERROR_SCRIPT_VIP_BUY_MAX);

	Proto31400045 inner;
	inner.set_activity_id(activity_id);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(cost);
	ProtoSerialObj *serial_obj = inner.mutable_obj();
	this->serial_reset_serialize(serial_obj, activity_id);

	if (activity_id == LuckyWheelActivity::ACT_NINE_WORD)
	{
		int word_num = player_detail->fetch_nine_word_num();
		if (word_num >= LuckyWheelActivity::NINE_WORD_NUM)
		{
			this->request_activity_reset_done(&inner);
			return 0;
		}
	}

	if (cost > 0)
		LOGIC_MONITOR->dispatch_to_scene(this, &inner);
	else
		this->request_activity_reset_done(&inner);

	return 0;
}

int LogicWheelPlayer::request_activity_reset_done(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400045*, request, -1);

	int activity_id = request->activity_id();
	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	player_detail->request_reset();

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	this->fetch_activity_info(act_detail);

	Proto50102005 respond;
	respond.set_activity_id(activity_id);
	FINER_PROCESS_RETURN(RETURN_RESET_LUCKY_WHEEL, &respond);
}

WheelPlayerInfo &LogicWheelPlayer::wheel_player_info()
{
	return this->wheel_info_;
}

WheelPlayerInfo::PlayerDetail* LogicWheelPlayer::fetch_player_detail(int activity_id)
{
	if (this->wheel_info_.player_detail_map_.count(activity_id) <= 0)
	{
		WheelPlayerInfo::PlayerDetail &player_detail = this->wheel_info_.player_detail_map_[activity_id];
		player_detail.activity_id_ = activity_id;
	}

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	if (act_detail != NULL)
	{
		WheelPlayerInfo::PlayerDetail &player_detail = this->wheel_info_.player_detail_map_[activity_id];
		if (player_detail.reset_tick_ != act_detail->reset_tick_)
		{
			player_detail.restart_reset();
			player_detail.reset_tick_ = act_detail->reset_tick_;
		}

		if (act_detail->is_combine_reset_ == true && act_detail->combine_reset_self_ == true
				&& player_detail.combine_reset_ == false)
		{
			player_detail.restart_reset();
			player_detail.combine_reset_ = true;
		}

		this->add_player_slot_info(&player_detail, act_detail);
	}

	return &(this->wheel_info_.player_detail_map_[activity_id]);
}

void LogicWheelPlayer::test_reset_activity(int activity_id)
{
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, ;);
	JUDGE_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true, ;);

	act_detail->test_reset();
	LUCKY_WHEEL_SYSTEM->rand_get_slot_map(act_detail);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	player_detail->restart_reset();
}

int LogicWheelPlayer::fetch_gashapon_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102060*, request, -1);
	int activity_id = request->activity_id();

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_FETCH_GASHAPON_INFO, ERROR_NOT_JOIN_TIME);
	CONDITION_NOTIFY_RETURN(act_detail != NULL && player_detail != NULL,
			RETURN_FETCH_GASHAPON_INFO, ERROR_CLIENT_OPERATE);

	int can_buy_times = this->logic_player()->role_detail().today_can_buy_times_;
	Proto50102060 respond;
	respond.set_activity_id(activity_id);
	respond.set_draw_free_times(can_buy_times);
	respond.set_left_tick(LUCKY_WHEEL_SYSTEM->fetch_left_tick(activity_id));
	respond.set_today_buy_times(this->logic_player()->role_detail().today_total_buy_times_);
	respond.set_today_recharge_gold(this->logic_player()->role_detail().today_recharge_gold_);
	int num = 0;

	for (WheelPlayerInfo::ItemRecordSet::iterator iter = player_detail->item_record_.begin();
			iter != player_detail->item_record_.end(); ++iter)
	{
		ProtoPersonRecord *person_record = respond.add_person_record();
		this->wheel_info_.record_serialize(person_record, *iter);

		++num;
		JUDGE_BREAK(num < act_detail->person_record_count_);
	}

	num = 0;
	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->item_set_.begin();
			iter != act_detail->item_set_.end(); ++iter)
	{
		ProtoServerRecord *server_record = respond.add_server_record();
		LUCKY_WHEEL_SYSTEM->record_serialize(server_record, *iter);

		++num;
		JUDGE_BREAK(num < act_detail->server_record_count_);
	}

	//this->inner_notify_assist_event(act_detail->red_point_, can_buy_times > 0);

	FINER_PROCESS_RETURN(RETURN_FETCH_GASHAPON_INFO, &respond);
}

int LogicWheelPlayer::update_gashapon_recharge(int gold)
{
	int activity_id = LuckyWheelActivity::ACT_GASHAPON;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1;);
	JUDGE_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true, -1;);

	JUDGE_RETURN(act_detail->draw_cost_ > 0, -1;);

	int total_gashapon_count = gold / act_detail->draw_cost_;
	int cur_gashapon_count = this->logic_player()->role_detail().today_total_buy_times_;
	int valid_count = total_gashapon_count - cur_gashapon_count;

	if(valid_count > 0)
	{
		this->logic_player()->role_detail().today_can_buy_times_ = valid_count;

	}

	Proto10102060 msg;
	msg.set_activity_id(activity_id);
	return this->fetch_gashapon_info(&msg);
}

int LogicWheelPlayer::fetch_gashapon_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102061*, request, -1);
	int activity_id = request->activity_id();
	CONDITION_NOTIFY_RETURN(LUCKY_WHEEL_SYSTEM->is_activity_time(activity_id) == true,
			RETURN_FETCH_GASHAPON_INFO, ERROR_NOT_JOIN_TIME);

	int today_can_buy_times = this->logic_player()->role_detail().today_can_buy_times_;
	JUDGE_RETURN(today_can_buy_times > 0, -1;);

	int times = request->count();
	JUDGE_RETURN(times <= today_can_buy_times && times > 0, -1;);

	Proto50102061 respond;
	respond.set_activity_id(activity_id);

	for(int i = 0; i < times; ++i)
	{
		WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);

		CONDITION_NOTIFY_RETURN(player_detail != NULL,
				RETURN_DRAW_GASHAPON_REWARD, ERROR_CLIENT_OPERATE);

		int item_count = 1;
		//this->cabinet_refresh_work(activity_id, 0, item_count);
		this->rand_slot_item(activity_id, item_count);

		CONDITION_NOTIFY_RETURN(player_detail->shop_slot_map_.size() > 0,
				RETURN_DRAW_GASHAPON_REWARD, ERROR_INDEX_INVALID);

		WheelPlayerInfo::ShopSlot &shop_slot = player_detail->shop_slot_map_[item_count - 1];

		LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
				activity_id, this->role_detail().__agent_code);

		CONDITION_NOTIFY_RETURN(act_detail != NULL,
				RETURN_DRAW_GASHAPON_REWARD, ERROR_CLIENT_OPERATE);

		ProtoItem* item = respond.add_item();;
		shop_slot.item_.serialize(item);

		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, shop_slot.day_, shop_slot.slot_id_);
		LUCKY_WHEEL_SYSTEM->draw_refresh_activity_info(act_detail, slot_info,
					this->role_id(), slot_info->item_obj_);

		SerialObj obj(ADD_FROM_GASHAPON_ACTIV, activity_id);
		this->request_add_item(obj, shop_slot.item_.id_, shop_slot.item_.amount_, shop_slot.item_.bind_);

		WheelPlayerInfo::ItemRecord record = this->create_record(shop_slot.item_, slot_info->is_precious_);
		this->record_person(player_detail, slot_info->person_record_, record);
	}

	this->logic_player()->role_detail().today_total_buy_times_ += times;
	this->logic_player()->role_detail().today_can_buy_times_ -= times;
	MSG_USER("today_buy_times:%d, today_can_buy_times:%d",
			this->logic_player()->role_detail().today_total_buy_times_,
			this->logic_player()->role_detail().today_can_buy_times_);

	FINER_PROCESS_RETURN(RETURN_CABINET_INFO, &respond);
}


int LogicWheelPlayer::fetch_gem_activity_info()
{
	int activity_id = LuckyWheelActivity::ACT_GEM_SYNTHESIS;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	int reward_day = 0;
	LuckyWheelActivity::GemSynthesisInfoVec *gem_syn_info_vec = NULL;
	LuckyWheelActivity::ActivityDetail::DayGemSynInfoMap& day_gem_syn_info_map = act_detail->day_gem_synthesis_info_map_;
	for (LuckyWheelActivity::ActivityDetail::DayGemSynInfoMap::iterator iter =
			day_gem_syn_info_map.begin(); iter != day_gem_syn_info_map.end(); ++iter)
	{
		if (cur_day < iter->first)
			break;
		reward_day = iter->first;
		gem_syn_info_vec = &(iter->second);
	}
	JUDGE_RETURN(gem_syn_info_vec != NULL && reward_day != 0, -1);

	int act_remain_time = act_detail->back_last_date_ - ::time(NULL);
	if (act_remain_time < 0)
		act_remain_time = 0;

	Int64 role_id = this->role_id();
	LuckyWheelActivity::RoleMailInfo &role_mail_info = act_detail->gem_role_mail_map_[role_id];
	IntMap& reward_map =  role_mail_info.reward_map_;

	Proto50100268 respond;
	for (IntMap::iterator iter = reward_map.begin();
			iter != reward_map.end(); ++iter)
	{
		ProtoGemInfo *gem_info_ptr = respond.add_reward_gem_info();
		JUDGE_CONTINUE(0 != gem_info_ptr);
		gem_info_ptr->set_gem_id(iter->first);
		gem_info_ptr->set_gem_amount(iter->second);
	}
	respond.set_is_act_open(act_detail->open_flag_);
	respond.set_day(reward_day);
	respond.set_remain_time(act_remain_time);
	FINER_PROCESS_RETURN(RETURN_GEM_SYNTHESIS_INFO, &respond);
}

int LogicWheelPlayer::gem_synthesis(Message* msg)
{
	//这个函数已废弃
	return 0;
}

int LogicWheelPlayer::draw_gem_synthesis_reward()
{
	int activity_id = LuckyWheelActivity::ACT_GEM_SYNTHESIS;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	Int64 role_id = this->role_id();
	LuckyWheelActivity::RoleMailInfo &role_mail_info = act_detail->gem_role_mail_map_[role_id];
	IntMap& reward_map =  role_mail_info.reward_map_;
	JUDGE_RETURN(reward_map.size() > 0, -1);

	int mail_id = act_detail->mail_id_;
	JUDGE_RETURN(mail_id > 0, -1);

	MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);
	if(mail_info == NULL)
	{
		MSG_USER("LogicWheelPlayer draw_gem_synthesis_reward:Create mail obj error");
	}
	else
	{
		for (IntMap::iterator iter = reward_map.begin();
				iter != reward_map.end(); ++iter)
		{
			int item_id = iter->first;
			int item_amount = iter->second;
			::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
					mail_info->mail_content_.c_str());
			mail_info->add_goods(ItemObj(item_id, item_amount));
		}
		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}

	reward_map.clear();
	LuckyWheelActivity::RoleMailMap::iterator pos = act_detail->gem_role_mail_map_.find(role_id);
	if (pos != act_detail->gem_role_mail_map_.end())
		act_detail->gem_role_mail_map_.erase(pos);

	Proto50100270 respond;
	for (IntMap::iterator iter = reward_map.begin();
			iter != reward_map.end(); ++iter)
	{
		ProtoGemInfo *gem_info_ptr = respond.add_reward_gem_info();
		JUDGE_CONTINUE(0 != gem_info_ptr);
		gem_info_ptr->set_gem_id(iter->first);
		gem_info_ptr->set_gem_amount(iter->second);
	}

	FINER_PROCESS_RETURN(RETURN_GEM_SYNTHESIS_REWARD, &respond);
	return 0;
}


int LogicWheelPlayer::inner_gem_synthesis_pack_operate(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400058*, request, -1);

	int synthesis_item_id = request->synthesis_item_id();
	JUDGE_RETURN(synthesis_item_id != 0, -1);

	int activity_id = LuckyWheelActivity::ACT_GEM_SYNTHESIS;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	LuckyWheelActivity::GemSynthesisInfoVec *gem_syn_info_vec = NULL;
	LuckyWheelActivity::ActivityDetail::DayGemSynInfoMap& day_gem_syn_info_map = act_detail->day_gem_synthesis_info_map_;
	for (LuckyWheelActivity::ActivityDetail::DayGemSynInfoMap::iterator iter =
			day_gem_syn_info_map.begin(); iter != day_gem_syn_info_map.end(); ++iter)
	{
		if (cur_day < iter->first)
			break;
		gem_syn_info_vec = &(iter->second);
	}
	JUDGE_RETURN(gem_syn_info_vec != NULL, -1);

	for (LuckyWheelActivity::GemSynthesisInfoVec::iterator iter = gem_syn_info_vec->begin();
			iter != gem_syn_info_vec->end(); ++iter)
	{
		if (iter->synthesis_gem_[0] == synthesis_item_id)
		{
			int reward_gem_amount = iter->reward_gem_[1];
			int reward_gem_id = iter->reward_gem_[0];
			Int64 role_id = this->role_id();
			LuckyWheelActivity::RoleMailInfo &role_mail_info = act_detail->gem_role_mail_map_[role_id];
			role_mail_info.reward_map_[reward_gem_id] += reward_gem_amount;
		}
	}

	this->fetch_gem_activity_info();

	return 0;
}

int LogicWheelPlayer::fetch_bless_activity_info(void)
{
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			LuckyWheelActivity::ACT_GODDESS_BLESS, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_GODDESS_BLESS);
	JUDGE_RETURN(player_detail != NULL, -1);

	int surplus_time = act_detail->back_last_date_ - ::time(NULL);
	if (surplus_time < 0)
		surplus_time = 0;

	Proto50102067 respond;
	respond.set_bless_value(player_detail->bless_value);
	respond.set_surplus_time(surplus_time);

	for (LuckyWheelActivity::ServerItemSet::iterator iter = act_detail->bless_reward_set_.begin();
			iter != act_detail->bless_reward_set_.end(); ++iter)
	{
		ProtoServerRecord *reward_record = respond.add_reward_record_all_server_list();
		JUDGE_CONTINUE(0 != reward_record);
		LUCKY_WHEEL_SYSTEM->record_serialize(reward_record, *iter);
	}

	for (IntMap::iterator iter = player_detail->reward_record_map.begin();
			iter != player_detail->reward_record_map.end(); ++iter)
	{
		ProtoItemId *item_info = respond.add_reward_record_self_list();
		JUDGE_CONTINUE(0 != item_info);
		item_info->set_id(iter->first);
		item_info->set_amount(iter->second);
	}

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_GODDESS_BLESS);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);

	for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin();
			iter != slot_info_map->end(); ++iter)
	{
		ProtoItemExchangeTimes *item_exchange_times = respond.add_item_exchange_times_list();
		JUDGE_CONTINUE(0 != item_exchange_times);

		int item_id = iter->second.item_obj_.id_;
		int current_frequency = player_detail->exchange_item_frequency[item_id];
		item_exchange_times->set_item_id(item_id);
		item_exchange_times->set_times(current_frequency);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_GODDESS_BLESS_INFO, &respond);
}

int LogicWheelPlayer::goddess_bless_operator_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102068*, request, RETURN_GODDESS_BLESS_OPERATOR);
	int frequency = request->bless_number();

	GameConfig::ConfigMap& reward_blessing_conf = CONFIG_INSTANCE->goddess_bless_reward_blessing();
	JUDGE_RETURN(reward_blessing_conf.empty() == false, -1);

	int activity_id = LuckyWheelActivity::ACT_GODDESS_BLESS;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_GODDESS_BLESS);
	JUDGE_RETURN(player_detail != NULL, -1);

	int id = 0;
	for (GameConfig::ConfigMap::iterator iter = reward_blessing_conf.begin();
			iter != reward_blessing_conf.end(); ++iter)
	{
		const Json::Value &reward_blessing_info = *(iter->second);
		int frequency_conf = reward_blessing_info["frequency"].asInt();
		if (frequency_conf == frequency)
		{
			id = iter->first;
			break;
		}
	}
	JUDGE_RETURN(id > 0, -1);

	const Json::Value &reward_blessing_info = reward_blessing_conf[id];
	int need_money = reward_blessing_info["need_money"].asInt();

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_GODDESS_BLESS_OPERATOR);
	inner.set_amount(frequency);
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(need_money);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::goddess_bless_operator(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);
	int activity_id = request->activity_id();
	int frequency = request->amount();

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);

	GameConfig::ConfigMap& reward_blessing_conf = CONFIG_INSTANCE->goddess_bless_reward_blessing();
	JUDGE_RETURN(reward_blessing_conf.empty() == false, -1);

	int id = 0;
	for (GameConfig::ConfigMap::iterator iter = reward_blessing_conf.begin();
			iter != reward_blessing_conf.end(); ++iter)
	{
		const Json::Value &reward_blessing_info = *(iter->second);
		int frequency_conf = reward_blessing_info["frequency"].asInt();
		if (frequency_conf == frequency)
		{
			id = iter->first;
			break;
		}
	}
	JUDGE_RETURN(id > 0, -1);

	const Json::Value &reward_blessing_info = reward_blessing_conf[id];
	int reward_blessing = reward_blessing_info["reward_bless"].asInt();
	player_detail->bless_value += reward_blessing;

	IntMap reward_map;
	for (int i = 0; i < frequency; ++i)
	{
		ItemRate item_rate_obj;
		JUDGE_CONTINUE(fetch_random_reward(item_rate_obj) == 0);
		reward_map[item_rate_obj.item_id] = item_rate_obj.item_num;
	}

	// 奖励物品
	Proto31400062 inner_request;
	for (IntMap::iterator iter = reward_map.begin();
			iter != reward_map.end(); ++iter)
	{
		ProtoItemId *item_info = inner_request.add_item_info_list();
		JUDGE_CONTINUE(0 != item_info);

		int item_id = iter->first;
		int item_amount = iter->second;
		item_info->set_id(item_id);
		item_info->set_amount(item_amount);
	}
	ProtoSerialObj* serial_obj = inner_request.mutable_obj();
	serial_obj->set_serial_type(activity_id);
	serial_obj->set_sub_type(RETURN_GODDESS_BLESS_OPERATOR);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner_request);
}

int LogicWheelPlayer::goddess_bless_operator_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400062*, request, -1);

	int activity_id = LuckyWheelActivity::ACT_GODDESS_BLESS;

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);

	GameConfig::ConfigMap& reward_item_conf = CONFIG_INSTANCE->goddess_bless_reward_item();
	JUDGE_RETURN(reward_item_conf.empty() == false, -1);

	for (int i = 0; i < request->item_info_list_size(); ++i)
	{
		const ProtoItemId &item_id_info = request->item_info_list(i);
		int item_id = item_id_info.id();
		int item_amount = item_id_info.amount();

		for (GameConfig::ConfigMap::iterator iter_item_conf = reward_item_conf.begin();
				iter_item_conf != reward_item_conf.end(); ++iter_item_conf)
		{
			const Json::Value &reward_item_info = *(iter_item_conf->second);
			int item_id_conf = reward_item_info["item_id"].asInt();
			int is_rare_conf = reward_item_info["is_rare"].asInt();

			JUDGE_CONTINUE(item_id == item_id_conf);

			player_detail->reward_record_map[item_id] += item_amount;

			JUDGE_CONTINUE(is_rare_conf == 1);

			LuckyWheelActivity::ServerItemInfo server_item_info;
			server_item_info.item_id_ = item_id;
			server_item_info.amount_ = item_amount;
			server_item_info.item_bind_ = GameEnum::ITEM_BIND;
			server_item_info.player_id_ = this->role_id();
			server_item_info.player_name_ = this->role_detail().name();
			server_item_info.get_time_ = ::time(NULL);
			act_detail->bless_reward_set_.push_back(server_item_info);
		}
	}

	this->fetch_bless_activity_info();
	FINER_PROCESS_NOTIFY(RETURN_GODDESS_BLESS_OPERATOR);
}

int LogicWheelPlayer::fetch_random_reward(ItemRate &item_obj)
{
	int activity_id = LuckyWheelActivity::ACT_GODDESS_BLESS;
	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);

	GameConfig::ConfigMap& reward_item_conf = CONFIG_INSTANCE->goddess_bless_reward_item();
	JUDGE_RETURN(reward_item_conf.empty() == false, -1);

	IntMap reward_item;
	for (GameConfig::ConfigMap::iterator iter = reward_item_conf.begin();
			iter != reward_item_conf.end(); ++iter)
	{
		const Json::Value &reward_item_info = *(iter->second);
		int min_frequency = reward_item_info["min_frequency"].asInt();
		int item_id = reward_item_info["item_id"].asInt();
		player_detail->bless_reward_frequency[item_id] += 1;

		int flag = player_detail->bless_reward_possess[item_id];
		if (flag)
		{
			reward_item[iter->first] = true;
			continue;
		}

		int frequency_buf = player_detail->bless_reward_frequency[item_id];
		if (frequency_buf >= min_frequency)
			reward_item[iter->first] = true;
	}

	// 通过加权随机获得奖励的东西
	WeightedRandomInfo weight_rand_info;
	int ret = weight_rand_info.reward_item_to_weighted_set(reward_item, reward_item_conf);
	JUDGE_RETURN(ret != -1, -1);
	int random_item_id = weight_rand_info.weighted_random_operator();
	JUDGE_RETURN(random_item_id != -1, -1);

	int item_amount = 0;
	for (GameConfig::ConfigMap::iterator iter = reward_item_conf.begin();
			iter != reward_item_conf.end(); ++iter)
	{
		const Json::Value &reward_item_info = *(iter->second);
		int item_id = reward_item_info["item_id"].asInt();
		JUDGE_CONTINUE(item_id == random_item_id);

		item_amount = reward_item_info["item_amount"].asInt();
	}
	JUDGE_RETURN(0 == item_amount, -1);

	for (GameConfig::ConfigMap::iterator iter = reward_item_conf.begin();
			iter != reward_item_conf.end(); ++iter)
	{
		const Json::Value &reward_item_info = *(iter->second);
		int max_frequency = reward_item_info["max_frequency"].asInt();
		int item_id = reward_item_info["item_id"].asInt();
		int frequency_buf = player_detail->bless_reward_frequency[item_id];
		JUDGE_CONTINUE(frequency_buf >= max_frequency);

		player_detail->bless_reward_frequency[item_id] = 0;
	}

	item_obj.item_id = random_item_id;
	item_obj.item_num = item_amount;
	return 0;
}

int LogicWheelPlayer::goddess_bless_exchange_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102069*, request, RETURN_GODDESS_BLESS_OPERATOR);
	int exchange_item_id = request->exchange_item_id();

	int activity_id = LuckyWheelActivity::ACT_GODDESS_BLESS;
	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_GODDESS_BLESS);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);


	int id = 0;
	for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin();
			iter != slot_info_map->end(); ++iter)
	{
		int item_id = iter->second.item_obj_.id_;
		if (item_id == exchange_item_id)
			id = iter->first;
	}
	JUDGE_RETURN(id != 0, -1);

	int item_id = (*slot_info_map)[id].item_obj_.id_;
	int frequency_limit = (*slot_info_map)[id].frequency_limit_;
	int current_frequency = player_detail->exchange_item_frequency[item_id];
	JUDGE_RETURN(current_frequency < frequency_limit, -1);

	int item_amount = (*slot_info_map)[id].item_obj_.amount_;
	int need_item_id = (*slot_info_map)[id].need_item_obj_.id_;
	int need_item_amount = (*slot_info_map)[id].need_item_obj_.amount_;

	// 扣紫晶，兑换道具
	Proto31400061 inner_request;
	ProtoItemId *item_info = inner_request.add_item_info_list();
	JUDGE_RETURN(0 != item_info, -1);
	item_info->set_id(item_id);
	item_info->set_amount(item_amount);

	item_info = inner_request.add_item_info_list();
	JUDGE_RETURN(0 != item_info, -1);
	item_info->set_id(need_item_id);
	item_info->set_amount(need_item_amount);

	ProtoSerialObj* serial_obj = inner_request.mutable_obj();
	serial_obj->set_serial_type(activity_id);
	serial_obj->set_sub_type(RETURN_GODDESS_BLESS_EXCHANGE);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner_request);
}

int LogicWheelPlayer::goddess_bless_exchange_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400061*, request, -1);

	int activity_id = LuckyWheelActivity::ACT_GODDESS_BLESS;

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, -1);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_GODDESS_BLESS);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);

	const ProtoItemId &item_id_info = request->item_info_list(1);
	int item_id = item_id_info.id();
	int item_amount = item_id_info.amount();

	for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin();
			iter != slot_info_map->end(); ++iter)
	{
		int item_id_conf = iter->second.item_obj_.id_;
		int is_rare_conf = iter->second.is_rare_;

		JUDGE_CONTINUE(item_id == item_id_conf);
		player_detail->bless_reward_possess[item_id] = true;
		player_detail->reward_record_map[item_id] += item_amount;

		JUDGE_CONTINUE(is_rare_conf == 1);

		LuckyWheelActivity::ServerItemInfo server_item_info;
		server_item_info.item_id_ = item_id;
		server_item_info.amount_ = item_amount;
		server_item_info.item_bind_ = GameEnum::ITEM_BIND;
		server_item_info.player_id_ = this->role_id();
		server_item_info.player_name_ = this->role_detail().name();
		server_item_info.get_time_ = ::time(NULL);
		act_detail->bless_reward_set_.push_back(server_item_info);
	}

	this->fetch_bless_activity_info();
	FINER_PROCESS_NOTIFY(RETURN_GODDESS_BLESS_EXCHANGE);
}

int LogicWheelPlayer::rand_slot_item(int activity_id, int slot_num)
{
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
		activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_REQUEST_TIME_LIMIT,
		ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail* player_detail = this->fetch_player_detail(activity_id);
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);

	//获取组个数，最后一组为补漏
	int group_count = act_detail->group_no_show_list_.size();
	CONDITION_NOTIFY_RETURN(group_count > 0, RETURN_REQUEST_TIME_LIMIT, ERROR_CONFIG_ERROR);

	//必出组集合
	IntSet must_show_set;
	IntSet may_be_show_set;
	//组id从1开始 插入符合次数条件的组
	for (int i = 0; i < group_count; ++i)
	{
		//这里group_no_show_list 和 group_show_list 索引从0开始的
		if (player_detail->group_refresh_times_map_[i] >= act_detail->group_no_show_list_[i])
		{
			if (player_detail->group_refresh_times_map_[i] >= act_detail->group_show_list_[i])
			{
				must_show_set.insert(i + 1);
			}
			else
			{
				may_be_show_set.insert(i + 1);
			}
		}
	}

	//必出个数概率分布
	IntMap show_limit_pro_map;
	int pro_sum = 0;
	for (size_t i = 0; i < act_detail->group_pro_list_.size(); ++i)
	{
		pro_sum += act_detail->group_pro_list_[i];
		show_limit_pro_map[i] = pro_sum;
	}

	//对应组的个数
	IntMap group_item_count_map;
	int sum_group_item_count = 0;
	//必出组插入
	for (IntSet::const_iterator iter = must_show_set.begin(); iter != must_show_set.end(); ++iter)
	{
		//若是补漏组，跳出
		if (*iter == group_count)
			break;
		int rand_num = std::rand() % pro_sum;
		//show_limit_pro_map索引为个数，0代表1个
		for (IntMap::const_iterator limit_iter = show_limit_pro_map.begin(); limit_iter != show_limit_pro_map.end(); ++limit_iter)
		{
			MSG_DEBUG("cur limit:%d, rand_num:%d", limit_iter->second, rand_num);
			if (limit_iter->second >= rand_num)
			{
				group_item_count_map[*iter] = limit_iter->first + 1;
				break;
			}
		}

		int max_rand_times = act_detail->group_limit_list_[*iter - 1];
		if (group_item_count_map[*iter] > max_rand_times)
			group_item_count_map[*iter] = max_rand_times;
		sum_group_item_count += group_item_count_map[*iter];
	}

	//可能出组插入
	for (IntSet::const_iterator iter = may_be_show_set.begin(); iter != may_be_show_set.end(); ++iter)
	{
		//必出组有该组的时候,可出组不出
		if (must_show_set.count(*iter) > 0)
			break;
		CONDITION_NOTIFY_RETURN(act_detail->group_may_be_list_.count(*iter - 1) > 0, RETURN_REQUEST_TIME_LIMIT, ERROR_CONFIG_ERROR);
		int cur_may_be_weight = act_detail->group_may_be_list_[*iter - 1];
		int rand_num = std::rand() % 10000;
		if(rand_num < cur_may_be_weight)
		{
			must_show_set.insert(*iter);
			rand_num = std::rand() % pro_sum;
			MSG_USER("may be no group_count, cur rand_num:%d", rand_num);
			//show_limit_pro_map索引为个数，0代表1个
			for (IntMap::const_iterator limit_iter = show_limit_pro_map.begin(); limit_iter != show_limit_pro_map.end(); ++limit_iter)
			{
				MSG_USER("may be cur limit:%d, rand_num:%d", limit_iter->second, rand_num);
				if (limit_iter->second >= rand_num)
				{
					group_item_count_map[*iter] = limit_iter->first + 1;
					break;
				}
			}
		}

		int max_rand_times = act_detail->group_limit_list_[*iter - 1];
		if (group_item_count_map[*iter] > max_rand_times)
			group_item_count_map[*iter] = max_rand_times;
		sum_group_item_count += group_item_count_map[*iter];
	}


	//如果总个数小于要求的个数，由补漏组提供
	if (sum_group_item_count < slot_num)
	{
		//这里修改了，原本是 int sub = sum_group_item_count < slot_num;
		int sub = slot_num - sum_group_item_count;
		//add
		sum_group_item_count = slot_num;
		group_item_count_map[group_count] += sub;
	}

	//最终插入的格子id
	IntSet last_slot_map;
	if (must_show_set.size() > 0)
	{
		int base_weight = 0;
		//对应物品的权重
		IntMap slot_weight;
		IntMap slot_base_weight;
		for (IntSet::const_iterator iter = must_show_set.begin(); iter != must_show_set.end(); ++iter)
		{
			//如果是补漏组，跳出
			JUDGE_BREAK(*iter != group_count);
			//获取该组的所有物品
			LuckyWheelActivity::SlotInfoVec* slot_info_vec = LUCKY_WHEEL_SYSTEM->fetch_slot_map_by_group(act_detail, *iter, cur_day);
			JUDGE_RETURN(slot_info_vec != NULL, NULL);

			//获取该组的总权重
			int total_weight_by_group = LUCKY_WHEEL_SYSTEM->fetch_slot_map_weight_by_group(act_detail, *iter);
			JUDGE_RETURN(total_weight_by_group >= 0, NULL);

			for (size_t i = 0; i < slot_info_vec->size(); ++i)
			{
				LuckyWheelActivity::SlotInfo *slot_info = &(*slot_info_vec)[i];
				base_weight += slot_info->group_weight_;
				slot_weight[slot_info->slot_id_] = base_weight;
				slot_base_weight[slot_info->slot_id_] = slot_info->group_weight_;
			}

			int cur_group_count = group_item_count_map[*iter];

			IntMap::iterator weight_iter;
			for (int i = 0; i < cur_group_count; ++i)
			{
				if(total_weight_by_group <= 0)
					break;
				int rand_num = std::rand() % total_weight_by_group;
				weight_iter = slot_weight.begin();
				//这里每删除一个slot_weight，重新计算新的权重比例
				for (; weight_iter != slot_weight.end(); )
				{
					if (weight_iter->second >= rand_num)
					{
						last_slot_map.insert(weight_iter->first);
						MSG_DEBUG("erase slot id:%d, weight:%d, rand_weight:%d\n", weight_iter->first, weight_iter->second, rand_num);
						int temp_weight = weight_iter->second;
						slot_weight.erase(weight_iter++);
						IntMap::iterator next_iter = weight_iter;
						total_weight_by_group -= slot_base_weight[weight_iter->first];
						if (next_iter == slot_weight.end())
							break;
						IntMap::iterator pre_iter = next_iter;
						if(pre_iter != slot_weight.begin())
						{
							pre_iter--;
						}

						if (pre_iter == slot_weight.end())
							break;
						for (; next_iter != slot_weight.end(); ++next_iter)
						{
							int next_temp_weight = next_iter->second;
							if(next_iter == slot_weight.begin())
							{
								next_iter->second = next_iter->second - temp_weight;
							}
							else
							{
								next_iter->second = next_iter->second - temp_weight + pre_iter->second;
								++pre_iter;
							}
							temp_weight = next_temp_weight;

						}
						break;
					}
					else
						++weight_iter;
				}
			}
		}
	}

	//这里当随机出来的slotid不足需要随机的物品个数，就在最后一组添加个数
	if ((int)last_slot_map.size() < slot_num)
	{
		//剩余需要插入个数
		int rest = slot_num - last_slot_map.size();

		//获取该组的所有物品
		LuckyWheelActivity::SlotInfoVec* slot_info_vec = LUCKY_WHEEL_SYSTEM->fetch_slot_map_by_group(act_detail, group_count, cur_day);
		JUDGE_RETURN(slot_info_vec != NULL, NULL);

		//获取该组的总权重
		int total_weight_by_group = LUCKY_WHEEL_SYSTEM->fetch_slot_map_weight_by_group(act_detail, group_count);
		JUDGE_RETURN(total_weight_by_group > 0, NULL);

		int base_weight = 0;
		IntMap slot_weight;
		IntMap slot_base_weight;
		for (size_t i = 0; i < slot_info_vec->size(); ++i)
		{
			LuckyWheelActivity::SlotInfo *slot_info = &(*slot_info_vec)[i];
			if(slot_info == NULL)
				continue;
			base_weight += slot_info->group_weight_;
			slot_weight[slot_info->slot_id_] = base_weight;
			slot_base_weight[slot_info->slot_id_] = slot_info->group_weight_;
		}

		LuckyWheelActivity::SlotInfoVec temp(*slot_info_vec);
		IntMap::iterator weight_iter;
		for (int i = 0; i < rest; ++i)
		{
			if(total_weight_by_group <= 0)
				break;
			int rand_num = std::rand() % total_weight_by_group;
			weight_iter = slot_weight.begin();

			for (; weight_iter != slot_weight.end(); )
			{
				if (weight_iter->second >= rand_num)
				{
					last_slot_map.insert(weight_iter->first);
					MSG_DEBUG("last erase slot id:%d, weight:%d, rand_weight:%d\n", weight_iter->first, weight_iter->second, rand_num);
					int temp_weight = weight_iter->second;
					slot_weight.erase(weight_iter++);
					IntMap::iterator next_iter = weight_iter;
					total_weight_by_group -= slot_base_weight[weight_iter->first];
					if (next_iter == slot_weight.end())
						break;
					IntMap::iterator pre_iter = next_iter;
					if(pre_iter != slot_weight.begin())
						pre_iter--;

					if (pre_iter == slot_weight.end())
						break;
					for (; next_iter != slot_weight.end(); ++next_iter)
					{
						int next_temp_weight = next_iter->second;
						if(next_iter == slot_weight.begin())
						{
							next_iter->second = next_iter->second - temp_weight;
						}
						else
						{
							next_iter->second = next_iter->second - temp_weight + pre_iter->second;
							++pre_iter;
						}
						temp_weight = next_temp_weight;

					}
					break;
				}
				else
					++weight_iter;
			}
		}

		//补充随机后，还是不足够的问题，已询问策划
		int rand_count = 50;
		while (rand_count)
		{
			if((int)last_slot_map.size() == slot_num)
				break;
			int index = std::rand() % temp.size();
			if (last_slot_map.count(temp[index].slot_id_)>0)
				continue;
			last_slot_map.insert(temp[index].slot_id_);
			MSG_USER("while rand_count--- slot_id:%d", temp[index].slot_id_);
			rand_count--;
		}
	}

	//刷新
	int i = 0;
	for (IntSet::iterator it = last_slot_map.begin(); it != last_slot_map.end(); ++it, ++i)
	{
		int slot_id = *it;
		LuckyWheelActivity::SlotInfo* slot_info = LUCKY_WHEEL_SYSTEM->fetch_slot_info(act_detail, cur_day, slot_id);
		if(slot_info == NULL)
			continue;
		player_detail->shop_slot_map_[i].is_buy_ = 0;
		player_detail->shop_slot_map_[i].group_id_ = slot_info->group_id_;
		player_detail->shop_slot_map_[i].is_cast_ = slot_info->is_cast_;
		player_detail->shop_slot_map_[i].is_rarity_ = slot_info->is_rarity_;
		player_detail->shop_slot_map_[i].item_price_ = slot_info->item_price_;
		player_detail->shop_slot_map_[i].item_price_pre_ = slot_info->now_cost_;
		player_detail->shop_slot_map_[i].slot_id_ = slot_info->slot_id_;
		player_detail->shop_slot_map_[i].day_ = cur_day;
		player_detail->shop_slot_map_[i].item_ = ItemObj(slot_info->item_obj_.id_, slot_info->item_obj_.amount_,
			slot_info->item_obj_.bind_);
		MSG_USER("rand last item:%d", slot_info->slot_id_);
	}

	for (int i = 0; i < group_count; ++i)
	{
		if (group_item_count_map[i + 1] > 0)
		{
			player_detail->group_refresh_times_map_[i] = 0;
		}
		else
		{
			player_detail->group_refresh_times_map_[i]++;
		}
	}

	player_detail->refresh_tick_ = Time_Value::gettimeofday().sec();

	return 0;
}

int LogicWheelPlayer::fetch_fish_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto50102002 *, respond, RETURN_ONE_LUCKY_WHEEL_ACTIVITY);
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			LuckyWheelActivity::ACT_FISH, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL && act_detail->activity_id_ == respond->activity_id(), 0);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_FISH);
	JUDGE_RETURN(player_detail != NULL, 0);

	int fish_type_size = player_detail->fish_info_vec_.size();
	if(fish_type_size <= 0)
	{
		this->init_all_fish();
		fish_type_size = player_detail->fish_info_vec_.size();
		player_detail->refresh_fish_flag_ = false;
	}

	for(int i = 0; i < fish_type_size; ++i)
	{
		FishInfo &fish_info = player_detail->fish_info_vec_[i];
		FishDetail *ptr_detail = respond->add_fish_detail();
		this->serialize_fish_info(&fish_info, ptr_detail);
	}
	//this->test_show_fish_info();
	return 0;
}

int LogicWheelPlayer::init_all_fish()
{
	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_FISH);
	JUDGE_RETURN(player_detail != NULL, 0);

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			LuckyWheelActivity::ACT_FISH, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_GET_FISH,
			ERROR_NO_FUN_ACTIVITY);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_FISH);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);

	player_detail->fish_info_vec_.clear();

	for (LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin();
			iter != slot_info_map->end(); ++iter)
	{
		LuckyWheelActivity::SlotInfo* slot_info = &iter->second;
		int max_count = 0;
		if(slot_info->max_count_.size() > 1)
		{
			int down = slot_info->max_count_[0];
			int up = slot_info->max_count_[1];
			max_count = rand() % (up - down + 1) + down;
		}
		else if(slot_info->max_count_.size() > 0)
			max_count = slot_info->max_count_[0];
		else
			max_count = 0;

		for(int base_count = 0; base_count < max_count; ++base_count)
		{
			FishInfo info;
			if(slot_info->refresh_weight_.size() > 0 && slot_info->layer_.size() > 0)
			{
				int flag = false;
				int min_size = std::min(slot_info->refresh_weight_.size(), slot_info->layer_.size());
				for(int j = 0; j < min_size; ++j)
				{
					JUDGE_CONTINUE(!flag);
					//MSG_DEBUG("up rand_weight:%d, refresh_weight:%d", rand_weight, slot_info->refresh_weight_[j]);
					int total_weight = 10000;
					int rand_weight = rand() % total_weight;
					if(rand_weight < slot_info->refresh_weight_[j])
					{
						info.layer_ = slot_info->layer_[j];
						info.flag_ = false;
						info.type_ = slot_info->fish_type_;
						player_detail->fish_info_vec_.push_back(info);
						flag = true;
						break;
					}
					else
					{
						if(slot_info->other_.size() > 2)
						{
							int zero = 0, one = 1, two = 2;
							info.type_ = slot_info->other_[zero];
							int count = slot_info->other_[one];
							info.layer_ = slot_info->other_[two];
							info.flag_ = false;
							//MSG_DEBUG("up other push_count:%d, type:%d layer:%d", count, info.type_, info.layer_);
							while(count--)
							{
								player_detail->fish_info_vec_.push_back(info);
							}
							base_count++;
							if(base_count >= max_count)
								break;
						}
					}
				}
			}
		}
	}

	//test_show_fish_info();
	return 0;
}

int LogicWheelPlayer::syna_fish_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102063*, request, RETURN_SYNA_FISH_INFO);
	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_FISH);
	JUDGE_RETURN(player_detail != NULL, -1);

	Proto50102063 respond;
	int fish_size = request->fish_detail_size();
	if(fish_size > (int)player_detail->fish_info_vec_.size())
	{
		fish_size = player_detail->fish_info_vec_.size();
	}

	player_detail->fish_info_vec_.clear();
	player_detail->fish_info_vec_.resize(fish_size);
	for(int i = 0; i < fish_size; ++i)
	{
		const FishDetail &fish_detail = request->fish_detail(i);
		FishInfo &info = player_detail->fish_info_vec_[i];
		this->unserialize_fish_info(&info, &fish_detail);
		FishDetail *respond_detail = respond.add_fish_detail();
		this->serialize_fish_info(&info, respond_detail);
	}

	FINER_PROCESS_RETURN(RETURN_SYNA_FISH_INFO, &respond);
}

void LogicWheelPlayer::test_show_fish_info()
{
//	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_FISH);
//	JUDGE_RETURN(player_detail != NULL,);
//	if(player_detail->fish_info_vec_.size() <= 0)
//	{
//		MSG_DEBUG("fish is empty");
//		return ;
//	}
//	for(uint i = 0; i < player_detail->fish_info_vec_.size(); ++i)
//	{
//		FishInfo &info = player_detail->fish_info_vec_[i];
//		MSG_DEBUG("fish type:%d, layer:%d, flag:%d, x:%d, y:%d", info.type_, info.layer_, info.flag_, info.coord_.pos_x(), info.coord_.pos_y());
//	}
}

void LogicWheelPlayer::clean_fish_info()
{
	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(LuckyWheelActivity::ACT_FISH);
	JUDGE_RETURN(player_detail != NULL,);
	player_detail->fish_info_vec_.clear();
	player_detail->act_score_ = 0;
	player_detail->fish_reward_map_.clear();
}

int LogicWheelPlayer::fetch_fish_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102062*, request, RETURN_GET_FISH);
	int activity_id = LuckyWheelActivity::ACT_FISH;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_GET_FISH,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, -1);
	if(player_detail->fish_info_vec_.size() <= 0)
	{
		this->init_all_fish();
		JUDGE_RETURN(player_detail->fish_info_vec_.size() > 0, -1);
	}

	int fish_count = request->index_size();
	int type = request->type();
	int fish_count_flag = false;
	if(type != REFRESH_FISH)
	{
		int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_FISH);
		LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);

		JUDGE_RETURN(slot_info_map != NULL, -1);
		for(int i = 0; i < fish_count; ++i)
		{
			int cur_fish_index = request->index(i);
			JUDGE_RETURN((int)player_detail->fish_info_vec_.size() > cur_fish_index, -1);
			FishInfo &fish_info = player_detail->fish_info_vec_[cur_fish_index];
			if(fish_info.flag_ == true)
			{
				MSG_USER("client fish_info flag = true!!!!!");
				return this->fetch_activity_info(act_detail);
			}
		}

		for(uint i = 0; i < player_detail->fish_info_vec_.size(); ++i)
		{
			FishInfo &fish_info = player_detail->fish_info_vec_[i];
			if(fish_info.flag_ == false)
			{
				fish_count_flag = true;
				break;
			}
		}

		if(fish_count <= 0 && fish_count_flag == false)
			return this->fetch_activity_info(act_detail);
	}

	const Json::Value& type_cost_money = CONFIG_INSTANCE->const_set_conf("fish_cost_info")["arr"];
	JUDGE_RETURN(type < (int)type_cost_money.size(), 0);

	int gold = 0;
	if(type == GET_ALL_FISH)
	{
		int server_count = 0;
		for(uint i = 0; i < player_detail->fish_info_vec_.size(); ++i)
		{
			if(player_detail->fish_info_vec_[i].flag_ == false)
				server_count++;
		}
		if(fish_count > server_count)
			fish_count = server_count;
		gold = type_cost_money[GET_FISH].asInt() * fish_count;
		gold = std::min(gold, type_cost_money[GET_ALL_FISH].asInt());
	}
	else
		gold = type_cost_money[type].asInt();
	JUDGE_RETURN(gold > 0, 0);

	Proto31400044 inner;
	inner.set_activity_id(activity_id);
	inner.set_recogn(RETURN_GET_FISH);
	inner.set_amount(0);
	inner.set_type(type);
	for(int i = 0; i < fish_count; ++i)
	{
		inner.add_fish_index(request->index(i));
	}
	ProtoMoney *money = inner.mutable_money();
	money->set_gold(gold);

	ProtoSerialObj* serial_obj = inner.mutable_obj();
	this->serial_draw_serialize(serial_obj, activity_id, type);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicWheelPlayer::fetch_fish_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400044*, request, -1);
	int activity_id = LuckyWheelActivity::ACT_FISH;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_GET_FISH,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail != NULL, RETURN_GET_FISH,
			ERROR_NO_FUN_ACTIVITY);
	int type = request->type();
	int fish_count = request->fish_index_size();
	int fish_overplus = 0;
	if(type == REFRESH_FISH)
	{
		this->init_all_fish();
		return fetch_activity_info(act_detail);
	}
	else
	{
		for(uint i = 0; i < player_detail->fish_info_vec_.size(); ++i)
		{
			FishInfo &info = player_detail->fish_info_vec_[i];
			if(info.flag_ == false)
				fish_overplus++;
		}

		if(type == GET_ALL_FISH && fish_count > fish_overplus)
			fish_count = fish_overplus;
	}



	//防止作弊
	const Json::Value &fish_limit = CONFIG_INSTANCE->const_set_conf("get_fish_limit")["arr"];
	int zero = 0, one = 1, two = 2;
	int normal_max_fish = fish_limit[zero].asInt();
	int super_max_fish = fish_limit[one].asInt();
	if(type == GET_FISH)
	{
		JUDGE_RETURN(fish_count <= normal_max_fish, -1);
	}
	else if(type == SUPER_GET_FISH)
	{
		JUDGE_RETURN(fish_count <= super_max_fish, -1);
	}

	ItemObjMap show_item_map;
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_FISH);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);
	for(int i = 0; i < fish_count; ++i)
	{
		int cur_fish_index = request->fish_index(i);
		JUDGE_CONTINUE((int)player_detail->fish_info_vec_.size() > cur_fish_index);
		FishInfo &fish_info = player_detail->fish_info_vec_[cur_fish_index];
		for(LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin(); iter != slot_info_map->end(); ++iter)
		{
			LuckyWheelActivity::SlotInfo &slot_info = iter->second;
			if(slot_info.fish_type_ == fish_info.type_ && fish_info.flag_ == false)
			{
				ItemObj item_obj = act_detail->fetch_lucky_egg_slot_reward(&slot_info, true);
				SerialObj obj(ADD_FROM_GET_FISH_REWARD, activity_id);
				this->request_add_item(obj, item_obj.id_, item_obj.amount_, item_obj.bind_);
				show_item_map[item_obj.id_] = item_obj;

				if(slot_info.person_record_)
				{
					WheelPlayerInfo::ItemRecord record;
					record.item_id_ = item_obj.id_;
					record.amount_ = item_obj.amount_;
					record.item_bind_ = item_obj.bind_;
					record.get_time_ = ::time(NULL);
					record.sub_value_ = slot_info.fish_type_;
					this->record_person(player_detail, slot_info.person_record_, record);
				}

				if(slot_info.server_record_)
				{
					LuckyWheelActivity::ServerItemInfo server_item_info;
					server_item_info.item_id_ = item_obj.id_;
					server_item_info.amount_ = item_obj.amount_;
					server_item_info.item_bind_ = item_obj.bind_;
					server_item_info.player_id_ = this->role_id();
					server_item_info.player_name_ = this->role_detail().name();
					server_item_info.get_time_ = ::time(NULL);
					server_item_info.sub_value_ = slot_info.fish_type_;
					LUCKY_WHEEL_SYSTEM->save_server_record(act_detail, &slot_info, server_item_info);
				}

				if(slot_info.score_ > 0)
				{
					player_detail->act_score_ += slot_info.score_;
					MSG_DEBUG("fish_type:%d, player add score:%d, add_score:%d", fish_info.type_, player_detail->act_score_, slot_info.score_);
					this->record_other_serial(FISH_SERIAL, SUB_FISH_SCORE_ADD, player_detail->act_score_, slot_info.score_, fish_info.type_);
				}
				fish_info.flag_ = true;
				break;
			}
			else if(fish_info.flag_ == true)
			{
				MSG_USER("client fish_info flag = true!!!!!");
				return this->fetch_activity_info(act_detail);
			}
		}
	}

	const Json::Value &score_config = CONFIG_INSTANCE->const_set_conf("fish_add_score")["arr"];
	int normal_score = score_config[zero].asInt(),
			super_score = score_config[one].asInt(),
			all_score = score_config[two].asInt();
	//根据捕鱼类型奖励积分
	if(type == GET_FISH)
	{
		player_detail->act_score_ += normal_score;
	}
	else if(type == SUPER_GET_FISH)
	{
		player_detail->act_score_ += super_score;
	}
	else if(type == GET_ALL_FISH)
	{
		player_detail->act_score_ += fish_overplus * all_score;

		//特殊显示
		Proto81400110 respond;
		for (ItemObjMap::const_iterator iter = show_item_map.begin();
				iter != show_item_map.end(); ++iter)
		{
			iter->second.serialize(respond.add_items());
		}

		this->respond_to_client(ACTIVE_PACK_GIFT_SHOW, &respond);
	}

	if(normal_score > 0)
		this->record_other_serial(FISH_SERIAL, SUB_FISH_TYPE_SCORE_ADD, player_detail->act_score_, normal_score, type);
	else if(super_score > 0)
		this->record_other_serial(FISH_SERIAL, SUB_FISH_TYPE_SCORE_ADD, player_detail->act_score_, super_score, type);
	else if(all_score > 0)
		this->record_other_serial(FISH_SERIAL, SUB_FISH_TYPE_SCORE_ADD, player_detail->act_score_, all_score, type);

	Proto50102062 respond;
	respond.set_type(request->type());
	respond.set_ret(0);

	for(uint i = 0; i < player_detail->fish_info_vec_.size(); ++i)
	{
		FishInfo &info = player_detail->fish_info_vec_[i];
		FishDetail *detail = respond.add_fish_detail();
		this->serialize_fish_info(&info, detail);
	}


	this->respond_to_client(RETURN_GET_FISH, &respond);
	return this->fetch_activity_info(act_detail);
}

int LogicWheelPlayer::fetch_fish_score_info(Message* msg)
{
	int activity_id = LuckyWheelActivity::ACT_FISH;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_FISH_SCORE_INFO,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail != NULL, RETURN_FETCH_FISH_SCORE_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_FISH);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);

	Proto50102066 respond;
	int max_count = CONFIG_INSTANCE->fish_type_max_count(cur_day);
	for(int i = 0; i < max_count; ++i)
	{
		const Json::Value *reward_config = CONFIG_INSTANCE->fish_score_reward_by_day(i, cur_day);
		JUDGE_CONTINUE(reward_config != NULL);
		ProtoItemId *item = respond.add_item();
		item->set_id((*reward_config)["item"][0u].asInt());
		item->set_amount((*reward_config)["item"][1].asInt());
		item->set_bind((*reward_config)["item"][2].asInt());
		item->set_cond((*reward_config)["cond"].asInt());
		int max_change_count = (*reward_config)["limit_count"].asInt();
		int cur_change_count = player_detail->fish_reward_map_[(*reward_config)["item"][0u].asInt()];
		respond.add_arrive(cur_change_count);
		respond.add_max_arrive(max_change_count);
		item->set_state(cur_change_count >= max_change_count ? -1 : 0);
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_FISH_SCORE_INFO, &respond);
}

int LogicWheelPlayer::fetch_fish_tips_info(Message* msg)
{
	int activity_id = LuckyWheelActivity::ACT_FISH;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);

	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_FISH_REWARD_INFO,
			ERROR_NO_FUN_ACTIVITY);

	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(LuckyWheelActivity::ACT_FISH);
	LuckyWheelActivity::SlotInfoMap *slot_info_map = LUCKY_WHEEL_SYSTEM->fetch_slot_map(act_detail, cur_day);
	JUDGE_RETURN(slot_info_map != NULL, -1);

	Proto50102064 respond;
	for(LuckyWheelActivity::SlotInfoMap::iterator iter = slot_info_map->begin(); iter != slot_info_map->end(); ++iter)
	{
		LuckyWheelActivity::SlotInfo slot_info = iter->second;
		FishDetail *fish_detail = respond.add_fish_detail();
		fish_detail->set_type(slot_info.fish_type_);
		int item_count = slot_info.special_item_vec_.size();
		JUDGE_CONTINUE(item_count > 0);
		for(int i = 0; i < item_count; ++i)
		{
			LuckyWheelActivity::ChangeReward &reward = slot_info.special_item_vec_[i];
			fish_detail->add_item_id(reward.item_id_);
			fish_detail->add_item_amount(reward.amount_);
		}
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_FISH_REWARD_INFO, &respond);
}

int LogicWheelPlayer::fetch_fish_score_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102065*, request, RETURN_FETCH_FISH_SCORE_REWARD);
	int activity_id = LuckyWheelActivity::ACT_FISH;
	int cur_day = LUCKY_WHEEL_SYSTEM->fetch_cur_day(activity_id);
	int index = request->index();
	const Json::Value *reward_config = CONFIG_INSTANCE->fish_score_reward_by_day(index, cur_day);
	JUDGE_RETURN(reward_config != NULL, ERROR_INDEX_OUT_RANGE);

	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	CONDITION_NOTIFY_RETURN(act_detail != NULL, RETURN_FETCH_FISH_SCORE_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	CONDITION_NOTIFY_RETURN(player_detail != NULL, RETURN_FETCH_FISH_SCORE_REWARD,
			ERROR_NO_FUN_ACTIVITY);

	int ret = false;
	int cond = (*reward_config)["cond"].asInt();
	JUDGE_RETURN(cond > 0, ERROR_CONFIG_ERROR);

	int zero = 0, one = 1, two = 2;
	int max_item_count = (*reward_config)["limit_count"].asInt();
	int id = (*reward_config)["item"][zero].asInt();
	int amount = (*reward_config)["item"][one].asInt();
	int bind = (*reward_config)["item"][two].asInt();

	CONDITION_NOTIFY_RETURN(player_detail->fish_reward_map_[id] + amount <= max_item_count, RETURN_FETCH_FISH_SCORE_REWARD,
			ERROR_ACT_CHANGE_MAX);

	if(player_detail->act_score_ >= cond)
	{
		JUDGE_RETURN((*reward_config)["item"].size() >= 3, ERROR_CONFIG_ERROR);
		player_detail->act_score_ -= cond;
		MSG_DEBUG("player sub score:%d, sub_score:%d",  player_detail->act_score_, cond);
		this->record_other_serial(FISH_SERIAL, SUB_FISH_SCORE_SUB, player_detail->act_score_, cond);

		SerialObj obj(ADD_FROM_FISH_SCORE_CHANGE, activity_id);
		this->request_add_item(obj, id, amount, bind);
		player_detail->fish_reward_map_[id] += amount;
	}

	this->fetch_activity_info(act_detail);
	this->fetch_fish_score_info(NULL);

	Proto50102065 respond;
	respond.set_ret(ret);
	respond.set_index(index);
	FINER_PROCESS_RETURN(RETURN_FETCH_FISH_SCORE_REWARD, &respond);
}

void LogicWheelPlayer::serialize_fish_info(const FishInfo *read_info, FishDetail* write_detail)
{
	JUDGE_RETURN(read_info != NULL && write_detail != NULL, );
	write_detail->set_type(read_info->type_);
	write_detail->set_layer(read_info->layer_);
	write_detail->set_flag(read_info->flag_);
	write_detail->set_pos_x(read_info->coord_.pos_x());
	write_detail->set_pos_y(read_info->coord_.pos_y());
	write_detail->add_item_id(0);
	write_detail->add_item_amount(0);
}

void LogicWheelPlayer::unserialize_fish_info(FishInfo *write_info, const FishDetail* read_detail)
{
	JUDGE_RETURN(write_info != NULL && read_detail != NULL, );
	write_info->type_ = read_detail->type();
	write_info->layer_ = read_detail->layer();
	write_info->flag_ = read_detail->flag();
	write_info->coord_.set_pos(read_detail->pos_x(), read_detail->pos_y());
}

int LogicWheelPlayer::use_prop_add_score(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403203*, request, INNER_USE_PROP_ADD_FISH_SCORE);
	int activity_id = LuckyWheelActivity::ACT_FISH;
	LuckyWheelActivity::ActivityDetail* act_detail = LUCKY_WHEEL_SYSTEM->fetch_activity_detail(
			activity_id, this->role_detail().__agent_code);
	JUDGE_RETURN(act_detail != NULL, ERROR_NO_FUN_ACTIVITY);

	WheelPlayerInfo::PlayerDetail *player_detail = this->fetch_player_detail(activity_id);
	JUDGE_RETURN(player_detail != NULL, ERROR_NO_FUN_ACTIVITY);

	int value = request->value();
	player_detail->act_score_ += value;

	this->record_other_serial(FISH_SERIAL, SUB_FISH_SCORE_USE_PROP, player_detail->act_score_, value);
	return this->fetch_activity_info(act_detail);
}
