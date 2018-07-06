/*
 * MapShoper.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#include "MapShoper.h"
#include "MapEquipment.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "ShopMonitor.h"
#include "GameFont.h"
#include "MapLogicPlayer.h"
#include "TipsEnum.h"

MapShoper::MapShoper()
{
	// TODO Auto-generated constructor stub

}

MapShoper::~MapShoper()
{
	// TODO Auto-generated destructor stub
}

void MapShoper::reset()
{
	this->sellout_detial_.reset();
}

int MapShoper::get_own_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400027*, request, RETURN_MALL_BUY_GOODS);

	Proto30100021 inner;
	for(int i = 0;i < request->item_id_size(); i++)
	{
		int id = request->item_id(i);
		int count = this->pack_count(id,GameEnum::INDEX_PACKAGE);
		count += this->pack_count(id,GameEnum::INDEX_EQUIP);
		count += this->pack_count(id,GameEnum::INDEX_STORE);
		count += this->pack_count(id,GameEnum::INDEX_BOX_STORE);
		count += this->pack_count(id,GameEnum::INDEX_MOUNT);
		ProtoItem* item = inner.add_item();
		item->set_id(id);
		item->set_amount(count);
	}
    inner.set_shop_type(request->shop_type());
	return MAP_MONITOR->dispatch_to_logic(this, &inner);

}

int MapShoper::buy_mall_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400252*, request, RETURN_MALL_BUY_GOODS);

	if (GameCommon::is_money_item(request->money_type()) == true)
	{
		//金钱
		return this->buy_mall_goods_use_money(msg);
	}
	else if (GameCommon::is_resource_item(request->money_type()) == true)
	{
		//特殊资源
		return this->buy_mall_goods_use_resource(msg);
	}
	else
	{
		//物品兑换
		return this->buy_mall_goods_use_goods(msg);
	}
}

int MapShoper::buy_mall_goods_use_money(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400252*, request, RETURN_MALL_BUY_GOODS);

	Money need_money = this->fetch_buy_money(request->money_type(),
				request->money_amount(), request->buy_amount());

	if (this->validate_money(need_money) == false)
	{
		request->set_ret(ERROR_PACKAGE_MONEY_AMOUNT);
		return MAP_MONITOR->dispatch_to_logic(this, request);
	}

	int ret = this->insert_package(SerialObj(ADD_FROM_ITEM_MALL_BUY, request->money_type()),
			request->item_id(), request->buy_amount(), request->item_bind());

	if (ret != 0)
	{
		request->set_ret(ret);
		return MAP_MONITOR->dispatch_to_logic(this, request);
	}
	else
	{
		//扣钱
		this->pack_money_sub(need_money, SerialObj(SUB_MONEY_MALL_BUY, request->item_id()));

		return MAP_MONITOR->dispatch_to_logic(this, request);
	}
}

int MapShoper::buy_mall_goods_use_resource(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400252*, request, RETURN_MALL_BUY_GOODS);

	int total_need = request->money_amount() * request->buy_amount();
	int ret = this->validate_game_resource(request->money_type(), total_need);
	if (ret < 0)
	{
		request->set_ret(ret);
		return MAP_MONITOR->dispatch_to_logic(this, request);
	}

	ret = this->insert_package(SerialObj(ADD_FROM_ITEM_MALL_BUY, request->money_type()),
			request->item_id(), request->buy_amount(), request->item_bind());

	if (ret != 0)
	{
		request->set_ret(ret);
		return MAP_MONITOR->dispatch_to_logic(this, request);
	}
	else
	{
		//扣资源
		this->sub_game_resource(request->money_type(), total_need,
				SerialObj(RESOURCE_CHANGE_SERIAL, request->item_id()));

		return MAP_MONITOR->dispatch_to_logic(this, request);
	}
}

int MapShoper::buy_mall_goods_use_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400252*, request, RETURN_MALL_BUY_GOODS);
	return 0;
}

int MapShoper::map_process_donate_mall_goods(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto31400251*, request, RETURN_MALL_DONATE_GOODS);
//
//	//process sub money
//	Money need_money = this->fetch_buy_money(request->money_type(),
//			request->money_amount(), request->buy_amount());
//	GameCommon::adjust_money(need_money, this->own_money());
//	CONDITION_NOTIFY_RETURN(this->validate_money(need_money) == true,
//			RETURN_MALL_DONATE_GOODS, ERROR_PACKAGE_MONEY_AMOUNT);
//
//	int ret = this->pack_money_sub(need_money, SUB_MONEY_MALL_DONATE);
//	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MALL_DONATE_GOODS, ret);
//
////	GameCommon::request_send_system_mail(request->receiver_id(), FONT_MALL_DONATE_GOODS,
////			this->role_name(), request->item_id(), request->buy_amount(), request->item_bind());
//
//	{
//		FontPair mail_font = CONFIG_INSTANCE->font(FONT_MALL_DONATE_GOODS);
//		char mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1] = {0};
//		::snprintf(mail_content, GameEnum::MAX_MAIL_CONTENT_LENGTH, mail_font.second.c_str(), this->role_name().c_str());
//		mail_content[GameEnum::MAX_MAIL_CONTENT_LENGTH] = '\0';
//
//		MailInformation* mail_info = GameCommon::create_sys_mail(mail_font.first, mail_content,FONT_MALL_DONATE_GOODS);
//		mail_info->receiver_id_ = request->receiver_id();
//		mail_info->sender_name_ = this->role_name();
//
//		int goods_id = request->item_id();
//		int goods_amount = request->buy_amount();
//		int bind = request->item_bind();
//		if(goods_id != 0 && goods_amount > 0)
//			GameCommon::make_up_mail_attach_map(goods_id, goods_amount,
//					bind, mail_info->goods_map_);
//
//		GameCommon::request_save_mail(mail_info);
//	}
//
//
//	return MAP_MONITOR->dispatch_to_logic(this, request);
	return 0;
}

int MapShoper::sell_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400110*, request, RETURN_REQUEST_SELL_GOODS);

	PackageItem* pack_item = this->pack_find(request->index());
	CONDITION_NOTIFY_RETURN(pack_item != NULL, RETURN_REQUEST_SELL_GOODS,
			ERROR_PACKAGE_INDEX);

	CONDITION_NOTIFY_RETURN(pack_item->__amount >= request->amount()
			&& request->amount() > 0, RETURN_REQUEST_SELL_GOODS, ERROR_CLIENT_OPERATE);

	const Json::Value& item_conf = CONFIG_INSTANCE->item(pack_item->__id);
	CONDITION_NOTIFY_RETURN(item_conf != Json::Value::null, RETURN_REQUEST_SELL_GOODS,
			ERROR_SERVER_INNER);

	int sys_price = GameCommon::json_value(item_conf, "sys_price");
	CONDITION_NOTIFY_RETURN(sys_price > 0, RETURN_REQUEST_SELL_GOODS,
			ERROR_ITEM_FORBID_TO_SELL);

	Money money = GameCommon::make_up_money(sys_price, item_conf);
	GameCommon::make_up_money_times(money, request->amount());

	int ret = this->pack_money_add(money, SerialObj(ADD_MONEY_FROM_SELL,
			pack_item->__id, request->amount()));
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REQUEST_SELL_GOODS, ret);

	this->sellout_detial_.add_sell_out(pack_item, request->amount());
	this->pack_remove(ITEM_SELL_REMOVE, pack_item, request->amount());

	FINER_PROCESS_NOTIFY(RETURN_REQUEST_SELL_GOODS);
}

int MapShoper::sell_out_info()
{
	int item_index = 1;
	Proto51400133 respond;

	for (ItemList::iterator iter = this->sellout_detial_.sell_out_.begin();
			iter != this->sellout_detial_.sell_out_.end(); ++iter)
	{
		PackageItem* pack_item = *iter;
		JUDGE_CONTINUE(pack_item != NULL);

		ProtoSellOut* sellout = respond.add_sellout_set();
		JUDGE_CONTINUE(sellout != NULL);

		sellout->set_index(item_index);
		sellout->set_item_id(pack_item->__id);
		sellout->set_item_bind(pack_item->__bind);
		sellout->set_item_amount(pack_item->__amount);

		item_index += 1;
	}

	FINER_PROCESS_RETURN(RETURN_SELLOUT_INFO, &respond);
}

int MapShoper::buy_goods_back(Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto11400132*, request, RETURN_SHOP_BUY_BACK);
//
//	int cur_index = 0;
//
//	ItemList::iterator iter = this->sellout_detial_.sell_out_.begin();
//	for (; iter != this->sellout_detial_.sell_out_.end(); ++iter)
//	{
//		cur_index += 1;
//		JUDGE_CONTINUE(cur_index == request->index());
//		break;
//	}
//
//	CONDITION_NOTIFY_RETURN(iter != this->sellout_detial_.sell_out_.end(),
//			RETURN_SHOP_BUY_BACK, ERROR_CLIENT_OPERATE);
//
//	PackageItem* pack_item = *iter;
//	CONDITION_NOTIFY_RETURN(pack_item->__amount > 0, RETURN_SHOP_BUY_BACK,
//			ERROR_CLIENT_OPERATE);
//
//	const Json::Value& item_conf = CONFIG_INSTANCE->item(pack_item->__id);
//	CONDITION_NOTIFY_RETURN(item_conf != Json::Value::null, RETURN_SHOP_BUY_BACK,
//			ERROR_SERVER_INNER);
//
//	int sys_price = GameCommon::json_value(item_conf, "sys_price");
//	CONDITION_NOTIFY_RETURN(sys_price > 0, RETURN_SHOP_BUY_BACK,
//			ERROR_CLIENT_OPERATE);
//
//	Money need_money = GameCommon::make_up_money(sys_price, item_conf);
//	GameCommon::make_up_money_times(need_money, pack_item->__amount);
//
//	PROCESS_BIND_COPPER_ADJUST_EXCHANGE(need_money, RETURN_SHOP_BUY_BACK);
//
//	int  ret = this->pack_insert(ITEM_SHOP_BUY_BACK, pack_item);
//	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SHOP_BUY_BACK, ret);
//
//	ret = this->pack_money_sub(need_money, SUB_MONEY_SHOP_BUY_BACK);
//	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SHOP_BUY_BACK, ret);
//
//	this->sellout_detial_.sell_out_.erase(iter);
//	FINER_PROCESS_NOTIFY(RETURN_SHOP_BUY_BACK);
	return 0;
}

Money MapShoper::fetch_buy_money(int money_item, int money_amount, int buy_amount)
{
	return GameCommon::money_item_to_money(money_item, money_amount, buy_amount);
}

int MapShoper::fetch_need_item(IntMap& need_map, const IntMap& item_map,
		int buy_count)
{
	for (IntMap::const_iterator iter = item_map.begin(); iter != item_map.end();
			++iter)
	{
		need_map[iter->first] = iter->second * buy_count;
		JUDGE_CONTINUE(this->pack_count(iter->first) < need_map[iter->first]);

		return false;
	}

	return true;
}

int MapShoper::batch_sell_goods(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400134*, request, RETURN_REQUEST_batch_SELL_GOODS);

	int cnt = 0;
	bool error_flag = false;
	for(int i = 0; i < request->goods_list_size(); ++i)
	{
		int sell_index  = request->goods_list(i).obj_id();
		int sell_amount = request->goods_list(i).obj_value();

		PackageItem* pack_item = this->pack_find(sell_index);
		JUDGE_CONTINUE(pack_item != NULL);

		JUDGE_CONTINUE(pack_item->__amount >= sell_amount);

		const Json::Value& item_conf = CONFIG_INSTANCE->item(pack_item->__id);
		JUDGE_CONTINUE(item_conf != Json::Value::null);

		int sys_price = GameCommon::json_value(item_conf, "sys_price");
		if(sys_price <= 0)
		{
			error_flag = true;
			continue;
		}

		Money money = GameCommon::make_up_money(sys_price, item_conf);
		GameCommon::make_up_money_times(money, sell_amount);

		int ret = this->pack_money_add(money, SerialObj(ADD_MONEY_FROM_SELL,
				pack_item->__id, sell_amount));
		JUDGE_CONTINUE(ret == 0);

		this->sellout_detial_.add_sell_out(pack_item, sell_amount);
		this->pack_remove(ITEM_SELL_REMOVE, pack_item, sell_amount);

		++cnt;
//		this->pack_remove_notify(pack_item);

	}

	if (0 == cnt && error_flag == true)
	{
		return this->respond_to_client_error(RETURN_REQUEST_batch_SELL_GOODS,
				ERROR_ITEM_FORBID_TO_SELL);
	}

	FINER_PROCESS_NOTIFY(RETURN_REQUEST_batch_SELL_GOODS);
}

int MapShoper::lrf_buy_hickty(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400057*, request, -1);

	Money money = this->fetch_buy_money(request->buy_type(), request->amount());

	int ret = this->pack_money_sub(money, SUB_MONEY_HICKTY_BUY);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_LRF_CHANGE_MODE, ret);

	return this->send_to_map_thread(*request);
}

