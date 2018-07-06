/*
 * LogicShoper.cpp
 *
 *  Created on: Nov 1, 2013
 *      Author: peizhibi
 */

#include "LogicShoper.h"
#include "LogicPlayer.h"
#include "ShopMonitor.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "GameFont.h"
#include "LogicGameSwitcherSys.h"

LogicShoper::LogicShoper()
{
	// TODO Auto-generated constructor stub

}

LogicShoper::~LogicShoper()
{
	// TODO Auto-generated destructor stub
}

void LogicShoper::reset()
{

}

int LogicShoper::request_mall_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100021*, request, RETURN_MALL_FETCH_INFO);

	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::shop) == true, RETURN_MALL_BUY_GOODS, ERROR_MODEL_CLOSED);

	GameShop* game_shop = SHOP_MONITOR->find_shop(request->mall_type());
	CONDITION_NOTIFY_RETURN(game_shop != NULL, RETURN_MALL_FETCH_INFO,
			ERROR_CONFIG_NOT_EXIST);

	Proto31400027 inner;
    inner.set_shop_type(game_shop->shop_type_);

	for(GameShop::ShopItemMap::iterator iter = game_shop->shop_item_map_.begin();
			iter != game_shop->shop_item_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second != NULL);

		ShopItem* shop_item = iter->second;
		inner.add_item_id(shop_item->item_id_);
	}

	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int LogicShoper::after_request_mall_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30100021*, request, RETURN_MALL_FETCH_INFO);

	GameShop* game_shop = SHOP_MONITOR->find_shop(request->shop_type());
	CONDITION_NOTIFY_RETURN(game_shop != NULL, RETURN_MALL_FETCH_INFO,
			ERROR_CONFIG_NOT_EXIST);

	Proto50100021 respond;
	respond.set_vip_lvl(this->vip_type());

	ProtoMallList* proto_mall = respond.add_mall_list();
	proto_mall->set_shop_type(request->shop_type());

	for(IntVec::iterator iter = game_shop->item_vec_.begin();
			iter != game_shop->item_vec_.end(); ++iter)
	{
		ShopItem* shop_item = game_shop->find_item(*iter);
		JUDGE_CONTINUE(this->judge_mall_item_sex(shop_item) == true);

		ProtoMallItem* proto_item = proto_mall->add_item_list();
		proto_item->set_item_pos(shop_item->item_pos_);
		proto_item->set_item_id(shop_item->item_id_);
		proto_item->set_item_bind(shop_item->item_bind_);
		proto_item->set_money_type(shop_item->money_type_);
		proto_item->set_prime_price(shop_item->src_price_);
		proto_item->set_cur_price(shop_item->cur_price_);

		if (SHOP_MONITOR->is_item_limited(shop_item) == true)
		{
			//每天每人限购
			proto_item->set_limit_condt_type(2);
			proto_item->set_max_item(shop_item->max_item_);
			proto_item->set_left_item(this->fetch_left_item(shop_item));
		}

		if (SHOP_MONITOR->is_total_limited(shop_item) == true)
		{
			//永久每人限购
			proto_item->set_limit_condt_type(2);
			proto_item->set_max_item(shop_item->max_total_);
			proto_item->set_left_item(this->fetch_left_total_item(shop_item));
		}

		for (IntVec::iterator it = shop_item->item_type_.begin();
				it != shop_item->item_type_.end(); ++it)
		{
			proto_item->add_item_type_list(*it);
		}
	}

	if (SHOP_MONITOR->mall_activity().is_in_mall_activity_period())
	{
		respond.set_activity_active(1);
		respond.set_left_time(SHOP_MONITOR->mall_activity().calc_mall_activity_left_time());
	}

	FINER_PROCESS_RETURN(RETURN_MALL_FETCH_INFO, &respond);
}

int LogicShoper::fetch_mall_good_detail(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100024*, request, RETURN_FETCH_MALL_ITEM_INFO);

	bool found = false;
	int item_id = request->item_id();

	Proto50100024 respond;
	ShopMonitor::GameShopMap::iterator it = SHOP_MONITOR->shop_map()->begin();
	for(; it != SHOP_MONITOR->shop_map()->end(); ++it)
	{
		GameShop* game_shop = it->second;
		JUDGE_CONTINUE(game_shop != NULL);

		GameShop::ShopItemMap::iterator iter = game_shop->shop_item_map_.begin();
		for (; iter != game_shop->shop_item_map_.end(); ++iter)
		{
			ShopItem* shop_item = iter->second;
			JUDGE_CONTINUE(shop_item != NULL);

			JUDGE_CONTINUE(shop_item->item_id_ == item_id);

			respond.set_shop_type(it->first);

			ProtoMallItem* proto_shop = respond.mutable_shop_item();
			JUDGE_CONTINUE(proto_shop != NULL);

			proto_shop->set_item_pos(shop_item->item_pos_);
			proto_shop->set_item_id(shop_item->item_id_);
			proto_shop->set_item_bind(shop_item->item_bind_);
			proto_shop->set_money_type(shop_item->money_type_);
			proto_shop->set_cur_price(shop_item->cur_price_);
			proto_shop->set_context(shop_item->content_);
			found = true;
		}
	}

	if (false == found)
	{
		const Json::Value& cfg = CONFIG_INSTANCE->item(item_id);
		CONDITION_NOTIFY_RETURN(Json::Value::null != cfg,
				RETURN_FETCH_MALL_ITEM_INFO, ERROR_CONFIG_NOT_EXIST);

		respond.set_shop_type(0);
		ProtoMallItem* proto_shop = respond.mutable_shop_item();
		proto_shop->set_item_pos(0);
		proto_shop->set_item_id(item_id);
		proto_shop->set_money_type(GameEnum::MONEY_UNBIND_GOLD);
		proto_shop->set_cur_price(cfg["goldPrice"].asInt());
	}

	FINER_PROCESS_RETURN(RETURN_FETCH_MALL_ITEM_INFO, &respond);
}

int LogicShoper::mall_buy_goods_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100023*, request, RETURN_MALL_BUY_GOODS);
	CONDITION_NOTIFY_RETURN(request->item_id() > 0, RETURN_MALL_BUY_GOODS, ERROR_CLIENT_OPERATE);

	CONDITION_NOTIFY_RETURN(LOGIC_SWITCHER_SYS->logic_check_switcher(
			GameSwitcherName::shop) == true, RETURN_MALL_BUY_GOODS, ERROR_MODEL_CLOSED);

	CONDITION_NOTIFY_RETURN(request->buy_amount() > 0 && request->buy_amount() < 1000,
			RETURN_MALL_BUY_GOODS, ERROR_CLIENT_OPERATE);

	GameShop* game_shop = SHOP_MONITOR->find_shop(request->shop_type());
	CONDITION_NOTIFY_RETURN(game_shop != NULL, RETURN_MALL_BUY_GOODS,
			ERROR_CLIENT_OPERATE);

	ShopItem* shop_item = game_shop->find_item(request->item_id());
	CONDITION_NOTIFY_RETURN(shop_item != NULL, RETURN_MALL_BUY_GOODS,
			ERROR_CLIENT_OPERATE);

	int ret = this->judge_mall_buy_condition(shop_item);
	CONDITION_NOTIFY_RETURN(0 == ret, RETURN_MALL_BUY_GOODS, ret);

	/*判断是否每天限购*/
	if (SHOP_MONITOR->is_item_limited(shop_item) == true)
	{
		int left_amount = this->fetch_left_item(shop_item);
		CONDITION_NOTIFY_RETURN(request->buy_amount() <= left_amount,
				RETURN_MALL_BUY_GOODS, ERROR_MALL_ACTIVITY_TOTAL_BUY_LIMIT);

		this->set_item_buy_limited(shop_item, request->buy_amount());
	}

	/*判断是否总共限购*/
	if (SHOP_MONITOR->is_total_limited(shop_item) == true)
	{
		int left_amount = this->fetch_left_total_item(shop_item);
		CONDITION_NOTIFY_RETURN(request->buy_amount() <= left_amount,
				RETURN_MALL_BUY_GOODS, ERROR_MALL_ACTIVITY_BUY_TOTAL_LIMIT);

		this->set_item_buy_total_limited(shop_item, request->buy_amount());
	}

	Proto31400252 buy_info;
	buy_info.set_shop_type(request->shop_type());
	buy_info.set_buy_amount(request->buy_amount());

	buy_info.set_item_id(shop_item->item_id_);
	buy_info.set_item_bind(shop_item->item_bind_);
	buy_info.set_money_type(shop_item->money_type_);
	buy_info.set_money_amount(shop_item->cur_price_);

	return LOGIC_MONITOR->dispatch_to_scene(this, &buy_info);
}

int LogicShoper::mall_buy_goods_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400252*, request, RETURN_MALL_BUY_GOODS);

	GameShop* game_shop = SHOP_MONITOR->find_shop(request->shop_type());
	JUDGE_RETURN(game_shop != NULL, -1);

	ShopItem* shop_item = game_shop->find_item(request->item_id());
	JUDGE_RETURN(shop_item != NULL, -1);

	if (request->ret() == 0)
	{
		this->respond_to_client(RETURN_MALL_BUY_GOODS);

		if (request->item_id() == CONFIG_INSTANCE->wedding_base()["wedding_ring"].asInt())
		{
			this->logic_player()->refresh_ring_info();
		}
	}
	else
	{
		if (SHOP_MONITOR->is_item_limited(shop_item) == true)
		{
			int &amount = this->role_detail().buy_map_[shop_item->item_id_];
			amount -= request->buy_amount();
			if (amount < 0)
				amount = 0;
		}
		else if (SHOP_MONITOR->is_total_limited(shop_item) == true)
		{
			int &buy_amount = this->role_detail().buy_map_[shop_item->item_id_];
			int &buy_total_amount = this->role_detail().buy_total_map_[shop_item->item_id_];
			buy_amount -= request->buy_amount();
			buy_total_amount -= request->buy_amount();
			if (buy_amount < 0)
				buy_amount = 0;
			if (buy_total_amount < 0)
				buy_total_amount = 0;
		}
		this->respond_to_client_error(RETURN_MALL_BUY_GOODS, request->ret());
	}
	return 0;
}

int LogicShoper::mall_donate_goods_begin(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto10100022*, request, RETURN_MALL_DONATE_GOODS);

	CONDITION_NOTIFY_RETURN(request->item_id() > 0 && request->item_anount() > 0, RETURN_MALL_DONATE_GOODS, ERROR_CLIENT_OPERATE);

	// 后台开关检查
	CONDITION_NOTIFY_RETURN(
			LOGIC_SWITCHER_SYS->logic_check_switcher(GameSwitcherName::mall_gift),
			RETURN_MALL_DONATE_GOODS, ERROR_MODEL_CLOSED);

	std::string receiver_name = request->player_name();

	GameShop* game_shop = SHOP_MONITOR->find_shop(request->shop_type());
	CONDITION_NOTIFY_RETURN(game_shop != NULL, RETURN_MALL_BUY_GOODS,
			ERROR_CLIENT_OPERATE);

	ShopItem* shop_item = game_shop->find_item(request->item_id());
	CONDITION_NOTIFY_RETURN(shop_item != NULL, RETURN_MALL_BUY_GOODS,
			ERROR_CLIENT_OPERATE);

	int max_amount = INT_MAX / shop_item->cur_price_;
	CONDITION_NOTIFY_RETURN(request->item_anount() <= max_amount, RETURN_MALL_BUY_GOODS, ERROR_CLIENT_OPERATE);

	//check money_type  != bind_gold
	CONDITION_NOTIFY_RETURN(GameEnum::MONEY_BIND_GOLD != shop_item->money_type_, RETURN_MALL_BUY_GOODS,
			ERROR_MALL_DONATE_GOOD_PAY_WAY);

	if(GameCommon::item_can_overlap(request->item_id()) == false)
	{
		CONDITION_NOTIFY_RETURN(request->item_anount() <= GameEnum::MAX_MAIL_GOODS_COUNT,
				RETURN_MALL_BUY_GOODS, ERROR_MAIL_ATTACH_SIZE);
	}

	DBShopMode* db_shop_mode = POOL_MONITOR->shop_mode_pool()->pop();
	CONDITION_NOTIFY_RETURN(db_shop_mode != NULL, RETURN_MALL_BUY_GOODS,
			ERROR_CLIENT_OPERATE);
	db_shop_mode->input_argv_.type_void_ = (void*)(shop_item);
	db_shop_mode->input_argv_.type_int_ = request->item_anount();
	db_shop_mode->input_argv_.type_string_ = receiver_name;
	db_shop_mode->recogn_ = TRANS_LOAD_PLAYER_FOR_MALL_DONATION;

	return GameCommon::db_load_mode_begin(db_shop_mode, LOGIC_MONITOR->logic_unit(), this->role_id());
}

int LogicShoper::after_load_for_mall_donate(DBShopMode* shop_mode)
{
	CONDITION_NOTIFY_RETURN(shop_mode != NULL, RETURN_MALL_BUY_GOODS, ERROR_SERVER_INNER);

	CONDITION_NOTIFY_RETURN(shop_mode->output_argv_.type_int64_ > 0,
			RETURN_MALL_BUY_GOODS, ERROR_ROLE_NOT_EXISTS);

	ShopItem* shop_item = (ShopItem*)shop_mode->input_argv_.type_void_;
	CONDITION_NOTIFY_RETURN(shop_item != NULL, RETURN_MALL_BUY_GOODS, ERROR_SERVER_INNER);

	Proto31400251 respond;
	respond.set_receiver_name(shop_mode->input_argv_.type_string_);
	respond.set_receiver_id(shop_mode->output_argv_.type_int64_);
	respond.set_shop_type(shop_item->shop_type_);
	respond.set_item_id(shop_item->item_id_);
	respond.set_item_bind(shop_item->item_bind_);
	respond.set_buy_amount(shop_mode->input_argv_.type_int_);
	respond.set_money_type(shop_item->money_type_);
	respond.set_money_amount(shop_item->cur_price_);

	return LOGIC_MONITOR->dispatch_to_scene(this, &respond);
}

int LogicShoper::mall_donate_goods_done(Message* msg)
{
	FINER_PROCESS_NOTIFY(RETURN_MALL_DONATE_GOODS);
}

int LogicShoper::fetch_mall_item_price(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto51400656*, request, -1);
//	int fashion_id = request->fashion_id();
//	int martial_id = request->convert_martial_id();
//
//	request->set_mall_gold_price(-1);
//
//	int fashion_price = 0, martial_price = 0;
//	ShopMonitor::GameShopMap::iterator it = SHOP_MONITOR->shop_map()->begin();
//	for(; it != SHOP_MONITOR->shop_map()->end(); ++it)
//	{
//		GameShop* game_shop = it->second;
//		JUDGE_CONTINUE(game_shop != NULL);
//
//		ShopItem* shop_item = game_shop->find_item(fashion_id);
//		if(NULL != shop_item)
//		{
//			fashion_price = shop_item->cur_price_;
//			request->set_mall_gold_price(shop_item->cur_price_);
//			request->set_fashion_mall_type(shop_item->shop_type_);
//		}
//
//		shop_item = game_shop->find_item(martial_id);
//		if(NULL != shop_item)
//		{
//			martial_price = shop_item->cur_price_;
//			request->set_convert_martial_price(shop_item->cur_price_);
//			request->set_martial_mall_type(shop_item->shop_type_);
//		}
//
//		if(martial_price != 0 && fashion_price != 0)
//			break;
//	}
////	MSG_DEBUG(%s, request->Utf8DebugString().c_str());
//	FINER_PROCESS_RETURN(RETURN_FETCH_FASHION_EXTRA_INFO, request);
	return 0;
}


int LogicShoper::judge_mall_item_sex(ShopItem* shop_item)
{
	int sex = this->role_detail().__sex;

	if (this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_MALE))
	{
		return sex == GameEnum::SEX_MALE;
	}

	if (this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_FEMALE))
	{
		return sex == GameEnum::SEX_FEMALE;
	}

	return true;
}

int LogicShoper::judge_mall_buy_condition(ShopItem* shop_item)
{
	JUDGE_RETURN(NULL != shop_item, ERROR_SERVER_INNER);
	JUDGE_RETURN(shop_item->item_type_.empty() == false, 0);

	int sex = this->role_detail().__sex;

	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_MALE))
		if(sex!= GameEnum::SEX_MALE) return ERROR_SEX_ERROR;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_FEMALE))
		if(sex!= GameEnum::SEX_FEMALE) return ERROR_SEX_ERROR;

	/*判断vip等级*/
	int vip_lvl = this->vip_type() ;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_1))
		if(vip_lvl < VIP_1) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_2))
		if(vip_lvl < VIP_2) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_3))
		if(vip_lvl < VIP_3) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_4))
		if(vip_lvl < VIP_4) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_5))
		if(vip_lvl < VIP_5) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_6))
		if(vip_lvl < VIP_6) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_7))
		if(vip_lvl < VIP_7) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_8))
		if(vip_lvl < VIP_8) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_9))
		if(vip_lvl < VIP_9) return ERROR_VIP_NOT_ENOUGH;
	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_VIP_BUY_10))
		if(vip_lvl < VIP_10) return ERROR_VIP_NOT_ENOUGH;

//	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_WARRIOR))
//		if(this->role_detail().__career != GameEnum::CAREER_WARRIOR ) return ERROR_CAREER_ERROR;
//	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_MASTER))
//		if(this->role_detail().__career != GameEnum::CAREER_MASTER ) return ERROR_CAREER_ERROR;
//	if(this->judge_mall_item_has_type(shop_item, GameEnum::MALL_ITEM_ONLY_ASSASSINT))
//		if(this->role_detail().__career != GameEnum:: CAREER_ASSASSIN ) return ERROR_CAREER_ERROR;

	return 0;
}

int LogicShoper::judge_mall_item_has_type(ShopItem* shop_item, const int type)
{
	return GameCommon::find_first(shop_item->item_type_, type);
}

/* shop_item.item_type 与 types 有交集 */
int LogicShoper::judge_mall_item_has_types_one(ShopItem* shop_item, const IntVec& types)
{
	for (IntVec::iterator type_it = shop_item->item_type_.begin();
			type_it != shop_item->item_type_.end(); type_it++)
	{
		IntVec::const_iterator find_it = std::find(types.begin(), types.end(), *type_it);
		if(find_it != types.end())
			return true;
	}
	return false;
}

ShopItem* LogicShoper::find_shop_item(int item_id, int shop_type)
{
	GameShop* game_shop = SHOP_MONITOR->find_shop(shop_type);
	JUDGE_RETURN(game_shop != NULL, NULL);
	return game_shop->find_item(item_id);
}

int LogicShoper::fetch_left_item(const ShopItem* item)
{
	JUDGE_RETURN(item->max_item_ > 0, 0);

	if (this->role_detail().buy_map_.count(item->item_id_) > 0)
	{
		int use_item = this->role_detail().buy_map_[item->item_id_];
		return std::max<int>(0, item->max_item_ - use_item);
	}
	else
	{
		return item->max_item_;
	}
}

int LogicShoper::fetch_left_total_item(const ShopItem* item)
{
	JUDGE_RETURN(item->max_total_ > 0, 0);

	if (this->role_detail().buy_total_map_.count(item->item_id_) > 0)
	{
		int use_item = this->role_detail().buy_total_map_[item->item_id_];
		return std::max<int>(0, item->max_total_ - use_item);
	}
	else
	{
		return item->max_total_;
	}
}

void LogicShoper::set_item_buy_limited(ShopItem* item, int buy_amount)
{
	this->role_detail().buy_map_[item->item_id_] += buy_amount;
}

void LogicShoper::set_item_buy_total_limited(ShopItem* item, int buy_amount)
{
	this->role_detail().buy_map_[item->item_id_] += buy_amount;
	this->role_detail().buy_total_map_[item->item_id_] += buy_amount;
}
