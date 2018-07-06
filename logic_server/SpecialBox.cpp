/*
 * SpecialBox.cpp
 *
 *  Created on: May 24, 2017
 *      Author: sai
 */

#include "SpecialBox.h"
#include "LogicMonitor.h"
#include "LogicStruct.h"
#include "LogicPlayer.h"

SpecialBox::SpecialBox()
{
	this->init();
}

SpecialBox::~SpecialBox()
{

}

void SpecialBox::init()
{
	buy_times_ = 0;
	score_ = 0;
	refresh_times_.clear();

	GameConfig::ConfigMap &item_config = CONFIG_INSTANCE->special_box_item();
	for (GameConfig::ConfigMap::iterator iter = item_config.begin(); iter != item_config.end(); ++iter)
	{
		Json::Value &detail_conf = *(iter->second);

		int day		= detail_conf["day"].asInt();
		int slot_id = detail_conf["slot"].asInt();
		SpecialBoxItemMap &slot_map = day_item_[day];
		SpecialBoxItem &slot_info = slot_map[slot_id];

		slot_info.slot_id_ 		 = slot_id;
		slot_info.server_record_ = detail_conf["server_record"].asInt();
		slot_info.is_shout_		 = detail_conf["is_shout"].asInt();
		slot_info.item_obj_.id_ = detail_conf["item"][0u].asInt();
		slot_info.item_obj_.amount_ = detail_conf["item"][1u].asInt();
		slot_info.item_obj_.bind_ = detail_conf["item"][2u].asInt();
		slot_info.min_times_ = detail_conf["item_min_times"].asInt();
		slot_info.max_times_ = detail_conf["item_max_times"].asInt();
		slot_info.weight_ = detail_conf["group_weight"].asInt();
		slot_info.name_ = detail_conf["name"].asString();
	}

	GameConfig::ConfigMap &change_item_config = CONFIG_INSTANCE->special_box_change_item();
	for (GameConfig::ConfigMap::iterator iter = change_item_config.begin(); iter != change_item_config.end(); ++iter)
	{
		Json::Value &detail_conf = *(iter->second);

		int day		= detail_conf["day"].asInt();
		int slot_id = detail_conf["slot"].asInt();
		ChangeItemMap &item_map = day_change_item_[day];
		ChangeItem &detail_item = item_map[slot_id];

		detail_item.slot_id_ = slot_id;
		detail_item.group_ = detail_conf["group"].asInt();
		detail_item.name_ = detail_conf["name"].asString();
		detail_item.page_ = detail_conf["page"].asInt();
		detail_item.score_ = detail_conf["score"].asInt();

		detail_item.change_item_.id_ = detail_conf["item"][0u].asInt();
		detail_item.change_item_.amount_ = detail_conf["item"][1u].asInt();
		detail_item.change_item_.bind_ = detail_conf["item"][2u].asInt();

		detail_item.need_item_.id_ = detail_conf["need_item"][0u].asInt();
		detail_item.need_item_.amount_ = detail_conf["need_item"][1u].asInt();
		detail_item.need_item_.bind_ = detail_conf["need_item"][2u].asInt();
	}
}

void SpecialBox::reset_every_day()
{
	buy_times_ = 0;
}

int SpecialBox::fetch_special_box_info(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	Proto50102073 respond;
	respond.set_buy_times(this->buy_times_);
	const Json::Value &reward_config = CONFIG_INSTANCE->const_set_conf("special_box_key")["arr"];
	int limit_buy_count = reward_config[1].asInt();
	respond.set_max_times(limit_buy_count);
	respond.set_score(this->score_);
	const ServerItemSet &server_record = LOGIC_MONITOR->get_server_record();
	for(ServerItemSet::const_iterator iter = server_record.begin(); iter != server_record.end(); ++iter)
	{
		ProtoServerRecord *record = respond.add_server_record();
		record->set_get_tme(iter->get_time_);
		record->set_item_id(iter->item_id_);
		record->set_player_name(iter->player_name_);
		record->set_player_id(iter->player_id_);
		record->set_amount(iter->amount_);
	}

	FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_INFO, &respond);
}

int SpecialBox::fetch_special_box_reward_check_item(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102071 *, respond, RETURN_SPECIAL_BOX_COST_ITEM);
	int type = respond->type();
	Proto31403204 inner;
	ProtoSerialObj* serial_obj = inner.mutable_serial_obj();
	this->serial_draw_serialize(serial_obj, COST_ITEM);
	const Json::Value &reward_config = CONFIG_INSTANCE->const_set_conf("special_box_key")["arr"];
	const Json::Value &gold_config = CONFIG_INSTANCE->const_set_conf("special_box_gold")["arr"];
	JUDGE_RETURN((int)gold_config.size() > type, ERROR_CLIENT_OPERATE);
	int times = gold_config[type].asInt();
	JUDGE_RETURN(times > 0, ERROR_CLIENT_OPERATE);

	inner.set_item_id(reward_config[0u].asInt());
	inner.set_item_amount(times);
	inner.set_item_bind(true);
	inner.set_type(COST_ITEM);
	inner.set_sub_value(times);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int SpecialBox::fetch_special_box_reward_check_money(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102072 *, respond, RETURN_SPECIAL_BOX_COST_MONEY);
	int type = respond->type();

	Proto31403204 inner;
	ProtoSerialObj* serial_obj = inner.mutable_serial_obj();
	this->serial_draw_serialize(serial_obj, COST_MONEY);
	const Json::Value &reward_config = CONFIG_INSTANCE->const_set_conf("special_box_gold")["arr"];
	int times = reward_config[type].asInt();
	int money = reward_config[type + 3].asInt();
	JUDGE_RETURN(times > 0 && money > 0, ERROR_CLIENT_OPERATE);

	inner.set_money(money);
	inner.set_type(COST_MONEY);
	inner.set_sub_value(times);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int SpecialBox::fetch_special_box_reward_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403204 *, respond, INNER_LUCKY_WHEEL_ALL_COST);
	int cur_day = CONFIG_INSTANCE->open_server_days();
	this->test_print_refresh_list();
	SpecialBoxItemMap &slot_info_map = this->fetch_special_box_item_by_day(cur_day);
	int item_index = -1;

	int ret = 0;
	int index = 0;
	for(SpecialBoxItemMap::iterator iter = slot_info_map.begin(); iter != slot_info_map.end(); ++iter, ++index)
	{
		int cur_refresh_times = this->special_box_get_refresh_times(iter->first);
		if(cur_refresh_times >= iter->second.max_times_ && ret == false)
		{
			item_index = iter->first;
			ret = true;
			this->special_box_set_refresh_times(iter->first, 0);
			continue;
		}
		else if(this->special_box_get_refresh_times(iter->first) >= iter->second.min_times_ && ret == false)
		{
			int rand_weight = rand() % 10000;
			if(rand_weight < iter->second.weight_)
			{
				item_index = iter->first;
				ret = true;
				this->special_box_set_refresh_times(iter->first, 0);
				continue;
			}
			else
			{
				this->special_box_set_refresh_times(iter->first, ++cur_refresh_times);
			}
		}
		else
			this->special_box_set_refresh_times(iter->first, ++cur_refresh_times);
	}


	Proto50102071 item_respond;
	Proto50102072 money_respond;
	if(respond->type() == COST_ITEM)
	{
		item_respond.set_ret(ret);
		if(ret == false)
		{
			FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_COST_ITEM, &item_respond);
		}
	}
	else if(respond->type() == COST_MONEY)
	{
		money_respond.set_ret(ret);
		if(ret == false)
		{
			FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_COST_MONEY, &money_respond);
		}
	}

	SpecialBoxItem &slot_info = slot_info_map[item_index];

	ItemObj &item_obj = slot_info.item_obj_;

	//全服记录
	if(slot_info.server_record_)
	{
		ServerItemRecord server_item_info;
		server_item_info.item_id_ = item_obj.id_;
		server_item_info.amount_ = item_obj.amount_;
		server_item_info.item_bind_ = item_obj.bind_;
		server_item_info.player_id_ = this->role_id();
		server_item_info.player_name_ = this->role_detail().name();
		server_item_info.get_time_ = ::time(NULL);
		LOGIC_MONITOR->save_server_record(slot_info, server_item_info);
	}

	//贵重物品传闻
	if (slot_info.is_shout_)
	{
		BrocastParaVec para_vec;
		GameCommon::push_brocast_para_string(para_vec, this->name());
		GameCommon::push_brocast_para_string(para_vec, slot_info.name_);
		const Json::Value &set_conf = CONFIG_INSTANCE->const_set_conf("special_box_key")["arr"];
		int shout_id = set_conf[3].asInt();
		GameCommon::announce(shout_id, &para_vec);
	}

	SerialObj obj(ADD_FROM_SPECIAL_BOX_REWARD);
	this->request_add_item(obj, slot_info.item_obj_.id_, slot_info.item_obj_.amount_, slot_info.item_obj_.bind_);

	if(respond->type() == COST_ITEM)
	{
		FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_COST_ITEM, &item_respond);
	}
	else if(respond->type() == COST_MONEY)
	{
		FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_COST_MONEY, &money_respond);
	}
	return 0;
}

int SpecialBox::fetch_special_box_change_info(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	Proto50102074 req;
	int cur_day = CONFIG_INSTANCE->open_server_days();
	ChangeItemMap &change_item_map = fetch_special_box_change_item_by_day(cur_day);
	for(ChangeItemMap::iterator item_map_iter = change_item_map.begin();
			item_map_iter != change_item_map.end(); ++item_map_iter)
	{
		ChangeItem &item = item_map_iter->second;
		SpecialBoxChange *change_item = req.add_all_item_set();
		change_item->set_group(item.group_);
		change_item->set_page(item.page_);
		change_item->set_change_item_id(item.need_item_.id_);
		change_item->set_change_item_amount(item.need_item_.amount_);
		change_item->set_change_item_bind(item.need_item_.bind_);
		change_item->set_item_id(item.change_item_.id_);
		change_item->set_item_amount(item.change_item_.amount_);
		change_item->set_item_bind(item.change_item_.bind_);
		change_item->set_index(item.slot_id_);
	}

	FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_CHANGE_INFO, NULL);
}

int SpecialBox::fetch_special_box_change_reward_begin(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102075 *, respond, RETURN_SPECIAL_BOX_CHANGE_REWARD);

	Proto31403204 inner;
	ProtoSerialObj* serial_obj = inner.mutable_serial_obj();
	this->serial_draw_serialize(serial_obj, CHANGE_ITEM);
	int item_index = respond->index();
	int times = respond->times();

	int cur_day = CONFIG_INSTANCE->open_server_days();
	ChangeItemMap &change_item_map = fetch_special_box_change_item_by_day(cur_day);
	JUDGE_RETURN(change_item_map.count(item_index) > 0, ERROR_NO_THIS_ITEM);
	ChangeItem &detail_item = change_item_map[item_index];

	inner.set_item_id(detail_item.need_item_.id_);
	inner.set_item_amount(detail_item.need_item_.amount_ * times);
	inner.set_item_bind(true);
	inner.set_type(CHANGE_ITEM);
	inner.set_sub_value(item_index);

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int SpecialBox::fetch_special_box_change_reward_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403204 *, respond, INNER_LUCKY_WHEEL_ALL_COST);
	int cur_day = CONFIG_INSTANCE->open_server_days();
	int item_index = respond->sub_value();

	ChangeItemMap &change_item_map = fetch_special_box_change_item_by_day(cur_day);
	JUDGE_RETURN(change_item_map.count(item_index) > 0, ERROR_NO_THIS_ITEM);
	ChangeItem &detail_item = change_item_map[item_index];
	int times = respond->item_amount() / detail_item.need_item_.amount_;

	SerialObj obj(ADD_FROM_SPECIAL_BOX_CHANGE_REWARD);
	request_add_item(obj, detail_item.change_item_.id_, detail_item.change_item_.amount_ * times, detail_item.change_item_.bind_);

	Proto50102075 req;
	req.set_ret(respond->ret());
	req.set_index(item_index);
	FINER_PROCESS_RETURN(RETURN_SPECIAL_BOX_CHANGE_REWARD, &req);
}

int SpecialBox::fetch_special_box_buy_key_begin(Message* msg)
{
	JUDGE_RETURN(this->logic_player()->role_level() >= 50, 0);
	MSG_DYNAMIC_CAST_NOTIFY(Proto10102070 *, respond, RETURN_SPECIAL_BOX_BUY_KEY);

	int buy_count = respond->count();
	const Json::Value &reward_config = CONFIG_INSTANCE->const_set_conf("special_box_key")["arr"];
	int limit_buy_count = reward_config[1].asInt();
	CONDITION_NOTIFY_RETURN(buy_times_ + buy_count <= limit_buy_count, RETURN_RESET_LUCKY_WHEEL,
			ERROR_ARENA_MAX_BUY_TIMES);

	int need_gold = reward_config[2].asInt() * buy_count;

	Proto31403204 inner;
	inner.set_money(need_gold);
	inner.set_sub_value(respond->count());
	inner.set_type(BUY_KEY);

	ProtoSerialObj* serial_obj = inner.mutable_serial_obj();
	this->serial_draw_serialize(serial_obj, BUY_KEY);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int SpecialBox::fetch_special_box_buy_key_end(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403204*, request, RETURN_SPECIAL_BOX_BUY_KEY);
	const Json::Value &reward_config = CONFIG_INSTANCE->const_set_conf("special_box_key")["arr"];
	int buy_times = request->sub_value();

	this->buy_times_ += buy_times;

	int key_id = reward_config[0u].asInt();
	SerialObj obj(ADD_FROM_SPECIAL_BOX_REWARD);
	request_add_item(obj, key_id, buy_times);
	FINER_PROCESS_NOTIFY(RETURN_SPECIAL_BOX_BUY_KEY);
}

int SpecialBox::cost_item_or_money_return(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31403204 *, respond, INNER_LUCKY_WHEEL_ALL_COST);

	int ret = respond->ret();
	switch(respond->type())
	{
		case BUY_KEY:
		{
			if(ret == 0)
				this->fetch_special_box_buy_key_end(respond);
			break;
		}
		case COST_ITEM:
		{
			Proto50102071 msg;
			msg.set_ret(ret);
			if(ret == 0)
			{
				int count = respond->sub_value();
				while(count--)
					this->fetch_special_box_reward_end(respond);
			}

			this->respond_to_client(RETURN_SPECIAL_BOX_COST_ITEM, &msg);
			break;
		}
		case COST_MONEY:
		{
			Proto50102072 msg;
			msg.set_ret(ret);
			if(ret == 0)
			{
				int count = respond->sub_value();
				while(count--)
					this->fetch_special_box_reward_end(respond);
			}

			this->respond_to_client(RETURN_SPECIAL_BOX_COST_MONEY, &msg);
			break;
		}
		case CHANGE_ITEM:
		{
			Proto50102075 msg;
			msg.set_ret(ret);
			if(ret == 0)
			{
				int count = respond->sub_value();
				while(count--)
					this->fetch_special_box_change_reward_end(respond);
			}
			this->respond_to_client(RETURN_SPECIAL_BOX_CHANGE_REWARD, &msg);
		}
	}

	return 0;
}

void SpecialBox::clean_special_box_info()
{
	buy_times_ = 0;
	score_ = 0;
	refresh_times_.clear();
}

void SpecialBox::serial_draw_serialize(ProtoSerialObj* obj, int type)
{
	switch(type)
	{
		case BUY_KEY:
		{
			obj->set_serial_type(SUB_MONEY_SPECIAL_BOX_BUY_KEY);
			break;
		}
		case COST_MONEY:
		{
			obj->set_serial_type(SUB_MONEY_SPECIAL_BOX_OPEN_BOX);
			break;
		}
		case COST_ITEM:
		{
			obj->set_serial_type(ITEM_SPECIAL_BOX_COST_ITEM);
			break;
		}
		case CHANGE_ITEM:
		{
			obj->set_serial_type(ITEM_SPECIAL_CHANGE_ITEM);
			break;
		}
	}
}

int SpecialBox::special_box_get_buy_times()
{
	return buy_times_;
}

int SpecialBox::special_box_get_score()
{
	return score_;
}

int SpecialBox::special_box_get_refresh_times(int slot_id)
{
	return refresh_times_[slot_id];
}

void SpecialBox::special_box_set_buy_times(int times)
{
	buy_times_ = times;
}

void SpecialBox::special_box_set_score(int score)
{
	score_ = score;
}

void SpecialBox::special_box_set_refresh_times(int slot_id, int times)
{
	refresh_times_[slot_id] = times;
}

IntMap& SpecialBox::special_box_get_refresh_times_map()
{
	return refresh_times_;
}

SpecialBoxItemMap& SpecialBox::fetch_special_box_item_by_day(int day)
{
	if(day <= 0)
		day = 1;

	if (day_item_.count(day) <= 0)
	{
		int max_day = 0;
		for (DayBoxItemMap::iterator iter = day_item_.begin();
				iter != day_item_.end(); ++iter)
		{
			JUDGE_CONTINUE(day >= iter->first);
			JUDGE_CONTINUE(max_day < iter->first);
			max_day = iter->first;
		}
		return day_item_[max_day];
	}
	else
	{
		return day_item_[day];
	}
}

ChangeItemMap& SpecialBox::fetch_special_box_change_item_by_day(int day)
{
	if(day <= 0)
		day = 1;

	if (day_change_item_.count(day) <= 0)
	{
		int max_day = 0;
		for (DayChangeItemMap::iterator iter = day_change_item_.begin();
				iter != day_change_item_.end(); ++iter)
		{
			JUDGE_CONTINUE(day >= iter->first);
			JUDGE_CONTINUE(max_day < iter->first);
			max_day = iter->first;
		}
		return day_change_item_[max_day];
	}
	else
	{
		return day_change_item_[day];
	}
}

void SpecialBox::test_print_refresh_list()
{
	for(IntMap::iterator iter = refresh_times_.begin(); iter != refresh_times_.end(); ++iter)
	{
		MSG_DEBUG("slot_id:%d, refresh_times:%d", iter->first, iter->second);
	}
}
