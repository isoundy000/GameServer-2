/*
 * LogicMarketer.cpp
 *
 *  Created on: Nov 4, 2013
 *      Author: peizhibi
 */

#include "LogicMarketer.h"
#include "MarketSystem.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "LogicPlayer.h"
#include "LogicGameSwitcherSys.h"

LogicMarketer::LogicMarketer()
{
	// TODO Auto-generated constructor stub
}

LogicMarketer::~LogicMarketer()
{
	// TODO Auto-generated destructor stub
}

int LogicMarketer::fetch_market_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100501*, request, RETURN_MARKET_INFO);

	// 后台开关检查
	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::market) == true, RETURN_MARKET_INFO, ERROR_MODEL_CLOSED);

	static int ALL_TYPE	= CONFIG_INSTANCE->market_const("all").asInt();
	static uint MARKET_PAGE_SIZE = CONFIG_INSTANCE->market_const("page_size").asInt();

	int sort_type = request->type();
	int market_type = request->main_type();

	ThreeObjVec obj_vec;
	if (market_type > 0)
	{
		MarketItemType* market = MARKET_SYS->find_market(market_type);
		CONDITION_NOTIFY_RETURN(market != NULL, RETURN_MARKET_INFO,
				ERROR_CLIENT_OPERATE);

		market->fetch_item_sort(this->role_id(), obj_vec,
				request->sub_type(), sort_type);
	}
	else
	{
		for (int market_type_i = 1; market_type_i <= ALL_TYPE; ++market_type_i)
		{
			MarketItemType* market = MARKET_SYS->find_market(market_type_i);
			JUDGE_CONTINUE(market != NULL);
			market->fetch_item_sort(this->role_id(), obj_vec, 0, sort_type);
		}
	}

	if (request->sort() == GameEnum::SORT_DESC)
	{
		std::stable_sort(obj_vec.begin(), obj_vec.end(),
				GameCommon::three_comp_by_desc_b);
	}
	else
	{
		std::stable_sort(obj_vec.begin(), obj_vec.end(),
				GameCommon::three_comp_by_asc);
	}

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			obj_vec.size(), MARKET_PAGE_SIZE);

	Proto50100501 market_info;
	market_info.set_type(request->type());
	market_info.set_main_type(request->main_type());
	market_info.set_sub_type(request->sub_type());
	market_info.set_page_index(page_info.cur_page_);
	market_info.set_total_page(page_info.total_page_);
	market_info.set_buy_times(this->role_detail().today_market_buy_times_);

	for (int i = page_info.start_index_; i < page_info.total_count_; ++i)
	{
		int item_index = obj_vec[i].id_;

		MarketItem* market_item = MARKET_SYS->find_market_item(item_index);
		JUDGE_CONTINUE(market_item != NULL);

		ProtoMarketItem* proto_item = market_info.add_market_set();
		JUDGE_CONTINUE(proto_item != NULL);

		proto_item->set_market_index(market_item->market_index_);
		proto_item->set_item_id(market_item->item_obj_.__id);
		proto_item->set_item_amount(market_item->item_obj_.__amount);
		proto_item->set_money_type(market_item->money_type_);
		proto_item->set_price(market_item->price_);
//		market_item->item_obj_.__equipment.serialize(proto_item->mutable_equip());

		page_info.add_count_ += 1;
		JUDGE_CONTINUE(page_info.add_count_ >= MARKET_PAGE_SIZE);

		break;
	}

	FINER_PROCESS_RETURN(RETURN_MARKET_INFO, &market_info);
}

int LogicMarketer::fetch_sell_log(Message* msg)
{
	Proto50100509 log_info;

	MarketerInfo* marketer = MARKET_SYS->find_marketer(this->role_id());
	if (marketer == NULL)
	{
		FINER_PROCESS_RETURN(RETURN_MARKET_SELL_LOG, &log_info);
	}

	for (ThreeObjList::iterator iter = marketer->sell_log_.begin();
			iter != marketer->sell_log_.end(); ++iter)
	{
		ProtoThreeObj* obj = log_info.add_sell_log();
		obj->set_id(iter->id_);
		obj->set_value(iter->value_);
		obj->set_tick(iter->tick_);
	}

	FINER_PROCESS_RETURN(RETURN_MARKET_SELL_LOG, &log_info);
}

int LogicMarketer::market_onsell(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100502*, request, RETURN_MARKET_ONSELL);

	// 后台开关检查
	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::market) == true, RETURN_MARKET_ONSELL, ERROR_MODEL_CLOSED);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_MARKET_ONSELL, ERROR_USE_SCENE_LIMIT);

	CONDITION_NOTIFY_RETURN(MARKET_SYS->arrive_max_onsell(this->role_id()) == false,
			RETURN_MARKET_ONSELL, ERROR_MAX_ONSELL);

	static int MAX_SELL_AMOUNT = CONFIG_INSTANCE->market_const("max_sell_amount").asInt();
	CONDITION_NOTIFY_RETURN(request->amount() > 0 && request->amount() <= MAX_SELL_AMOUNT,
			RETURN_MARKET_ONSELL, ERROR_SET_ITEM_AMOUNT);

	static int MAX_SELL_PRICE = 1000;
	CONDITION_NOTIFY_RETURN(request->price() > 0 && request->price() <= MAX_SELL_PRICE,
			RETURN_MARKET_ONSELL, ERROR_SET_MONEY);

	CONDITION_NOTIFY_RETURN(request->money_type() == 0 || request->money_type() == 1,
			RETURN_MARKET_ONSELL, ERROR_CLIENT_OPERATE);

	Proto31400221 onsell_info;
	onsell_info.set_money_type(GameEnum::ITEM_MONEY_UNBIND_GOLD);
	onsell_info.set_price(request->price());
	onsell_info.set_index(request->index());
	onsell_info.set_amount(request->amount());
	return LOGIC_MONITOR->dispatch_to_scene(this, &onsell_info);
}

int LogicMarketer::map_market_onsell(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400221*, request, RETURN_MARKET_ONSELL);

	MarketItem* market_item = MARKET_SYS->pop_market_item();
	JUDGE_RETURN(market_item != NULL, -1);

	market_item->item_obj_.unserialize(request->item_obj());
	market_item->set_item_id(market_item->item_obj_.__id);

	market_item->role_id_ = this->role_id();
	market_item->price_ = request->price();
	market_item->money_type_ = request->money_type();

	market_item->onsale_tick_ = ::time(NULL);
	market_item->arr_tick_ = market_item->onsale_tick_
			+ this->market_onsell_time();

	MARKET_SYS->add_market_item(market_item);
	FINER_PROCESS_NOTIFY(RETURN_MARKET_ONSELL);
}

int LogicMarketer::fetch_market_low_price(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100507*, request, RETURN_MARKET_LOW_PRICE);
	CONDITION_NOTIFY_RETURN(GameCommon::validate_item_id(request->item_id())
			== true, RETURN_MARKET_LOW_PRICE, ERROR_CLIENT_OPERATE);

	int low_price = MARKET_SYS->fetch_low_price(request->item_id());

	Proto50100507 price_info;
	price_info.set_low_price(low_price);
	FINER_PROCESS_RETURN(RETURN_MARKET_LOW_PRICE, &price_info);
}

int LogicMarketer::market_buy_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100503*, request, RETURN_MARKET_BUY);

	CONDITION_NOTIFY_RETURN(this->role_detail().is_validate_permi(
			GameEnum::PERT_NORMAL_PLAYER) == true,
			RETURN_MARKET_BUY, ERROR_INNER_PLAYER_BUY);

	// 后台开关检查
	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::market) == true, RETURN_MARKET_BUY, ERROR_MODEL_CLOSED);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_MARKET_ONSELL, ERROR_USE_SCENE_LIMIT);

	MarketItem* market_item = MARKET_SYS->find_market_item(request->market_index());
	CONDITION_NOTIFY_RETURN(market_item != NULL && market_item->item_back_ == false,
			RETURN_MARKET_BUY, ERROR_GOODS_SELLOUT);

	CONDITION_NOTIFY_RETURN(market_item->role_id_ != this->role_id(), RETURN_MARKET_BUY,
			ERROR_BUY_SELF_GOODS);

	CONDITION_NOTIFY_RETURN(market_item->left_tick() > Time_Value::MINUTE,
			RETURN_MARKET_BUY, ERROR_GOODS_SELLOUT);

	int day_buy_times = CONFIG_INSTANCE->market_const("day_buy_times").asInt();
	CONDITION_NOTIFY_RETURN(this->role_detail().today_market_buy_times_ < day_buy_times,
			RETURN_MARKET_BUY, ERROR_TODAY_BUY_TIMES);

	MarketerInfo* marketer = MARKET_SYS->find_marketer(market_item->role_id_);
	CONDITION_NOTIFY_RETURN(marketer != NULL,  RETURN_MARKET_BUY, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(market_item->item_obj_.__amount >= request->buy_amount()
			&& request->buy_amount() > 0, RETURN_MARKET_BUY, ERROR_SET_ITEM_AMOUNT);

	market_item->item_obj_.__amount -= request->buy_amount();
	market_item->undo_amount_ += request->buy_amount();

	Proto31400222 buy_info;
	buy_info.set_operate_result(ERROR_CLIENT_OPERATE);
	buy_info.set_role_id(market_item->role_id_);
	buy_info.set_market_index(request->market_index());
	buy_info.set_money_type(market_item->money_type_);
	buy_info.set_price(market_item->price_);
	buy_info.set_amount(request->buy_amount());
	market_item->item_obj_.serialize(buy_info.mutable_item_obj());
	return LOGIC_MONITOR->dispatch_to_scene_with_back(this, &buy_info);
}

int LogicMarketer::map_market_buy_goods(Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400222*, request, RETURN_MARKET_BUY);

	MarketerInfo* marketer = MARKET_SYS->find_marketer(request->role_id());
	JUDGE_RETURN(marketer != NULL,-1);
	JUDGE_RETURN(marketer->onsell_map_.count(request->market_index()) > 0,-1);

	MarketItem* market_item = marketer->onsell_map_[request->market_index()];
	market_item->undo_amount_ -= request->amount();
	if (request->operate_result() != 0)
	{
		market_item->item_obj_.__amount += request->amount();
	}
	else
	{
		marketer->add_sell_log(market_item->item_obj_.__id,
				request->amount() * request->price());

		LogicPlayer* player = BaseLogicPlayer::find_player(role_id);
		if (player != NULL)
		{
			MARKET_SYS->update_market_item(market_item);
			player->role_detail().today_market_buy_times_ += 1;
			player->respond_to_client(RETURN_MARKET_BUY);
		}

		// sell finish
		if (market_item->item_obj_.__amount <= 0)
		{
			MARKET_SYS->remove_market_item(market_item);
			MARKET_SYS->push_market_item(market_item);
		}
	}

	return 0;
}

int LogicMarketer::fetch_self_market_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100504*, request, RETURN_MARKET_INFO);

	MarketerInfo* marketer = MARKET_SYS->find_marketer(this->role_id());
	CONDITION_NOTIFY_RETURN(marketer != NULL,  RETURN_MARKET_SELF,
			ERROR_CLIENT_OPERATE);

	typedef std::vector<MarketItem*> MarketItemSet;
	MarketItemSet item_set;

	for(MarketerInfo::MarketItemMap::iterator iter = marketer->onsell_map_.begin();
			iter != marketer->onsell_map_.end(); ++iter)
	{
		item_set.push_back(iter->second);
	}

	int self_page_size = std::max<int>(CONFIG_INSTANCE->market_const("self_page_size").asInt(), 5);

	PageInfo page_info;
	GameCommon::game_page_info(page_info, request->page_index(),
			item_set.size(), self_page_size);

	int large_index = std::min<int>(page_info.total_count_,
			(page_info.start_index_ + self_page_size));
	Int64 now_tick = ::time(NULL);

	Proto50100504 market_info;
	market_info.set_page_index(page_info.cur_page_);
	market_info.set_total_page(page_info.total_page_);

	for (int i = page_info.start_index_; i < large_index; ++i)
	{
		MarketItem* market_item = item_set[i];
		JUDGE_CONTINUE(market_item != NULL);

		ProtoMarketItem* proto_item = market_info.add_onsell_set();
		JUDGE_CONTINUE(proto_item != NULL);

		proto_item->set_market_index(market_item->market_index_);
		proto_item->set_item_id(market_item->item_obj_.__id);
		proto_item->set_item_amount(market_item->item_obj_.__amount);
		proto_item->set_money_type(market_item->money_type_);
		proto_item->set_price(market_item->price_);
		proto_item->set_left_time(market_item->left_tick(now_tick));
//		market_item->item_obj_.__equipment.serialize(proto_item->mutable_equip());
	}

	FINER_PROCESS_RETURN(RETURN_MARKET_SELF, &market_info);
}

int LogicMarketer::market_get_back(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100505*, request, RETURN_MARKET_GETBACK);

	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
			RETURN_MARKET_ONSELL, ERROR_USE_SCENE_LIMIT);

	MarketerInfo* marketer = MARKET_SYS->find_marketer(this->role_id());
	CONDITION_NOTIFY_RETURN(marketer != NULL,  RETURN_MARKET_GETBACK,
			ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(marketer->onsell_map_.count(request->market_index()) > 0,
			RETURN_MARKET_GETBACK, ERROR_GOODS_SELLOUT);

	MarketItem* market_item = marketer->onsell_map_[request->market_index()];
	if (market_item->undo_amount_ > 0)
	{
		market_item->arr_tick_ = ::time(NULL) + Time_Value::MINUTE;
		return this->respond_to_client_error(RETURN_MARKET_GETBACK, ERROR_WAIT_ITEM_UNDO);
	}

	static int back_mail_id = CONFIG_INSTANCE->market_const("back_mail").asInt();
	if (market_item->item_obj_.__amount > 0)
	{
		MailInformation* back_mail = GameCommon::create_sys_mail(back_mail_id);
		back_mail->add_goods(market_item->item_obj_.__id, market_item->item_obj_.__amount);
		GameCommon::request_save_mail(this->role_id(), back_mail);
	}

	MARKET_SYS->remove_market_item(market_item);
	MARKET_SYS->push_market_item(market_item);

	FINER_PROCESS_NOTIFY(RETURN_MARKET_GETBACK);
}

int LogicMarketer::shout_market_item(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100510*, request, RETURN_MARKET_SHOURT);

	//传闻
	BrocastParaVec para_vec;
	GameCommon::push_brocast_para_string(para_vec, request->item());
	GameCommon::push_brocast_para_int(para_vec, request->amount());
	GameCommon::push_brocast_para_int(para_vec, request->total());

	static int shout_id = CONFIG_INSTANCE->market_const("shout_id").asInt();
	this->announce(shout_id, para_vec);

	FINER_PROCESS_NOTIFY(RETURN_MARKET_SHOURT);
}

int LogicMarketer::market_continue_sell(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100506*, request, RETURN_MARKET_CONSELL);

	// 后台开关检查
	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::market) == true, RETURN_MARKET_CONSELL, ERROR_MODEL_CLOSED);

	MarketerInfo* marketer = MARKET_SYS->find_marketer(this->role_id());
	CONDITION_NOTIFY_RETURN(marketer != NULL,  RETURN_MARKET_CONSELL,
			ERROR_CLIENT_OPERATE);

	int market_index = request->market_index();
	CONDITION_NOTIFY_RETURN(marketer->onsell_map_.count(market_index) > 0,
			RETURN_MARKET_CONSELL, ERROR_CLIENT_OPERATE);

	MarketItem* market_item = marketer->onsell_map_[market_index];
	CONDITION_NOTIFY_RETURN(market_item->item_obj_.__amount > 0,
			RETURN_MARKET_CONSELL, ERROR_CLIENT_OPERATE);
	CONDITION_NOTIFY_RETURN(market_item->left_tick() < Time_Value::MINUTE,
			RETURN_MARKET_CONSELL, ERROR_CLIENT_OPERATE);

	market_item->onsale_tick_ = ::time(NULL);
	market_item->arr_tick_ = market_item->onsale_tick_
			+ this->market_onsell_time();

	MARKET_SYS->remove_market_item(market_item);
	MARKET_SYS->add_market_item(market_item);

	Proto10100504 req;
	this->fetch_self_market_info(&req);

	FINER_PROCESS_NOTIFY(RETURN_MARKET_CONSELL);
}

int LogicMarketer::market_onsell_time()
{
	static int ONSELL_TIME = CONFIG_INSTANCE->market_const("onsell_time").asInt();
	return ONSELL_TIME;
}

