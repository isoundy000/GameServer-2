/*
 * ShopMonitor.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#include "ShopMonitor.h"
#include "LogicMonitor.h"

#include "MongoDataMap.h"
#include "MongoData.h"
#include "MMOShop.h"
#include <mongo/client/dbclient.h>
#include "GameField.h"
#include "ProtoDefine.h"
#include "GameHeader.h"
#include "Transaction.h"

int ShopMonitor::ShopMonitorTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int ShopMonitor::ShopMonitorTimer::handle_timeout(const Time_Value &tv)
{
	this->cancel_timer();

	JUDGE_RETURN(NULL != shop_monitor_, -1);
	return shop_monitor_->sync_shop_item_to_scene();
}

void ShopMonitor::ShopMonitorTimer::set_parent(ShopMonitor* shop_monitor)
{
	this->shop_monitor_ = shop_monitor;
}

ShopMonitor::ShopMonitor()
{
	// TODO Auto-generated constructor stub
	this->shop_monitor_timer_.set_parent(this);

}

ShopMonitor::~ShopMonitor()
{
	// TODO Auto-generated destructor stub
}

void ShopMonitor::start()
{
	VoidVec void_vec;
	MMOShop::load_shop_by_config(void_vec);

	this->shop_version_ = CONFIG_INSTANCE->fetch_shop_verison();
	this->init_game_shop(void_vec);

	MMOShop::load_limited_item(&this->monitor_detail_);
	MMOShop::load_limited_total_item(&this->monitor_detail_);

	MSG_USER("ShopMonitor start %d...", void_vec.size());
}

void ShopMonitor::stop()
{
	MMOShop::save_limited_item(&this->monitor_detail_);
	MMOShop::save_limited_total_item(&this->monitor_detail_);
}

void ShopMonitor::reset_everyday()
{
	this->monitor_detail_.limited_map_.clear();
}

int ShopMonitor::is_item_limited(const ShopItem* item)
{
	if (item->max_item_ > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ShopMonitor::is_total_limited(const ShopItem* item)
{
	if (item->max_total_ > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ShopMonitor::fetch_left_item(ShopItem* item)
{
	JUDGE_RETURN(item->max_item_ > 0, 0);

	if (this->monitor_detail_.limited_map_.count(item->item_id_) > 0)
	{
		int use_item = this->monitor_detail_.limited_map_[item->item_id_];
		return std::max<int>(0, item->max_item_ - use_item);
	}
	else
	{
		return item->max_item_;
	}
}

int ShopMonitor::fetch_left_total_item(ShopItem* item)
{
	JUDGE_RETURN(item->max_total_ > 0, 0);

	if (this->monitor_detail_.limited_total_map_.count(item->item_id_) > 0)
	{
		int use_item = this->monitor_detail_.limited_total_map_[item->item_id_];
		return std::max<int>(0, item->max_total_ - use_item);
	}
	else
	{
		return item->max_total_;
	}
}
int ShopMonitor::set_item_buy_limited(ShopItem* item, int buy_amount)
{
	this->monitor_detail_.limited_map_[item->item_id_] += buy_amount;
	return 0;
}

int ShopMonitor::set_item_buy_total_limited(ShopItem* item, int buy_amount)
{
	this->monitor_detail_.limited_total_map_[item->item_id_] += buy_amount;
	return 0;
}

MallActivity& ShopMonitor::mall_activity(void)
{
	return this->mall_activity_;
}

int ShopMonitor::request_load_mall_activity(void)
{
//	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//	JUDGE_RETURN(data_map != NULL, -1);
//
//	return TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_MALL_ACTIVITY,
//			DB_MONGO_DATA_MAP, data_map, POOL_MONITOR->mongo_data_map_pool(), LOGIC_MONITOR->logic_unit());
	return 0;
}

int ShopMonitor::request_save_mall_activity(void)
{
//	JUDGE_RETURN(this->mall_activity().mall_activity_detail().__data_change == true, 0);
//	JUDGE_RETURN(this->mall_activity().mall_activity_detail().__last_save_tick <=
//			Time_Value::gettimeofday().sec(), 0 );
//
//	MallActivityDetail& detail = this->mall_activity().mall_activity_detail();
//	detail.__last_save_tick += 600;
//
//	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//	JUDGE_RETURN(data_map != NULL, -1);
//	MMOMallActivity::update_data(data_map);
//
//	TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_MALL_ACTIVITY, data_map);

	return 0;
}

int ShopMonitor::after_load_mall_activity(Transaction* transaction)
{
//	JUDGE_RETURN(transaction != NULL, -1);
//    if (transaction->detail().__error != 0)
//    {
//        transaction->rollback();
//        return transaction->detail().__error;
//    }
//
//    TransactionData *trans_data = transaction->fetch_data(DB_MONGO_DATA_MAP);
//    if (trans_data == 0)
//    {
//    	transaction->rollback();
//        return -1;
//    }
//
//    MongoDataMap* data_map = trans_data->__data.__mongo_data_map;
//
//    MongoData* mongo_data = NULL;
//    if(data_map->find_data(MallActivityInfo::COLLECTION, mongo_data) == 0)
//    {
//    	BSONObj res = mongo_data->data_bson();
//    	this->bson2mall_activity(res);
//    }
//
//    transaction->summit();

	return 0;
}

int ShopMonitor::reqeust_load_game_shop()
{
	DBShopMode* shop_mode = POOL_MONITOR->shop_mode_pool()->pop();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_LOAD_SHOP_INFO;
	shop_mode->input_argv_.type_string_ = this->shop_version_;

	return LOGIC_MONITOR->db_load_mode_begin(shop_mode);
}

int ShopMonitor::after_load_game_shop(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL, -1);
	JUDGE_RETURN(shop_mode->output_argv_.type_int_ == 1, -1);

	this->shop_version_ = shop_mode->output_argv_.type_string_;
	return this->init_game_shop(shop_mode->output_argv_.void_vec_);
}

int ShopMonitor::init_game_shop(VoidVec& shop_item_list)
{
	GameShopMap shop_map;
	for (VoidVec::iterator iter = shop_item_list.begin(); iter != shop_item_list.end(); ++iter)
	{
		ShopItem* shop_item = (ShopItem*)(*iter);
		this->add_shop_item(shop_map, shop_item);
	}

	for (GameShopMap::iterator iter = shop_map.begin(); iter != shop_map.end(); ++iter)
	{
		if (this->shop_map_.count(iter->first) > 0)
		{
			GameShop* game_shop = this->shop_map_[iter->first];
			this->recycle_game_shop(game_shop);
		}

		iter->second->sort();
		this->shop_map_[iter->first] = iter->second;
	}

	return 0;
}

int ShopMonitor::sync_hot_sale_price(void)
{
//	JUDGE_RETURN(this->shop_map_.count(GameEnum::MALL_TYPE_VIP_PLAYER) > 0, NULL);
//
//	GameShop* game_shop = this->shop_map_[GameEnum::MALL_TYPE_VIP_PLAYER];
//	JUDGE_RETURN(NULL != game_shop, NULL);
//
//	GameShop::ShopItemMap::iterator it = game_shop->shop_item_map_.begin();
//	for(; it != game_shop->shop_item_map_.end(); ++it)
//	{
//		JUDGE_CONTINUE(it->second != NULL);
//
//		this->sync_hot_sale_price_to_all(it->first);
//	}

	return 0;
}

int ShopMonitor::sync_hot_sale_price_to_all(const int item_id)
{
//	ShopItem* src_item = this->fetch_shop_item(GameEnum::MALL_TYPE_VIP_PLAYER, item_id);
//	JUDGE_RETURN(NULL != src_item, -1);
//
//	GameShopMap& shop_map = this->shop_map_;
//	for (GameShopMap::iterator iter = shop_map.begin(); iter != shop_map.end();
//			++iter)
//	{
//		JUDGE_CONTINUE(iter->first != GameEnum::MALL_TYPE_VIP_PLAYER);
//
//		GameShop* game_shop = iter->second;
//		JUDGE_CONTINUE(NULL != game_shop);
//
//		JUDGE_CONTINUE(game_shop->shop_item_map_.count(item_id) > 0);
//
//		ShopItem* aim_item = game_shop->shop_item_map_[item_id];
//		JUDGE_CONTINUE(NULL != aim_item);
//
//		aim_item->cur_price_ = src_item->cur_price_;
//	}

	return 0;
}

int ShopMonitor::bson2mall_activity(mongo::BSONObj& res)
{
//	MallActivityDetail& detail = this->mall_activity_.mall_activity_detail();
//
//	if (res.isEmpty())
//		return 0;
//	detail.__activity_id = res[MallActivityInfo::ID].numberInt();
//	detail.__activity_name = res[MallActivityInfo::NAME].String();
//	detail.__activity_memo = res[MallActivityInfo::MEMO].String();
//	detail.__activity_ctrl.__start_tick
//			= res[MallActivityInfo::ACTIVITY_START_TICK].numberLong();
//	detail.__activity_ctrl.__end_tick
//			= res[MallActivityInfo::ACTIVITY_END_TICK].numberLong();
//	detail.__visible_ctrl.__start_tick
//			= res[MallActivityInfo::VISIBLE_START_TICK].numberLong();
//	detail.__visible_ctrl.__end_tick
//			= res[MallActivityInfo::VISIBLE_END_TICK].numberLong();
//	detail.__open_activity = res[MallActivityInfo::OPEN].numberInt();
//	detail.__limit_type = res[MallActivityInfo::LIMIT_TYPE].numberInt();
//	detail.__server_limit_amount
//			= res[MallActivityInfo::TOTAL_LIMIT_AMOUNT].numberInt();
//	detail.__single_limit_amount
//			= res[MallActivityInfo::SINGLE_LIMIT_AMOUNT].numberInt();
//
//	if (res.hasField(MallActivityInfo::TOTAL_BUY_RECORD.c_str()))
//	{
//		BSONObj record = res.getObjectField(
//				MallActivityInfo::TOTAL_BUY_RECORD.c_str());
//		BSONObjIterator it(record);
//		while (it.more())
//		{
//			BSONObj obj = it.next().embeddedObject();
//			int goods_id = obj[MallActivityInfo::GOODS_ID].numberInt();
//			int goods_amount = obj[MallActivityInfo::GOODS_AMOUNT].numberInt();
//			detail.__server_buy_map.insert(
//					std::pair<int, int>(goods_id, goods_amount));
//		}
//	}
//
//	if (res.hasField(MallActivityInfo::PLAYER_RECORD.c_str()))
//	{
//		BSONObj single = res.getObjectField(
//				MallActivityInfo::PLAYER_RECORD.c_str());
//		BSONObjIterator it(single);
//		while (it.more()) {
//			BSONObj obj = it.next().embeddedObject();
//			Int64 role_id = obj[MallActivityInfo::ID].numberLong();
//
//			IntMap player_buy_map;
//			BSONObjIterator iter(
//					obj.getObjectField(
//							MallActivityInfo::SINGLE_BUY_RECORD.c_str()));
//			while (iter.more()) {
//				BSONObj info = iter.next().embeddedObject();
//				int goods_id = info[MallActivityInfo::GOODS_ID].numberInt();
//				int goods_amount =
//						info[MallActivityInfo::GOODS_AMOUNT].numberInt();
//				player_buy_map[goods_id] = goods_amount;
//			}
//			detail.__player_record.insert(
//					std::pair<Int64, IntMap>(role_id,
//							player_buy_map));
//		}
//	}
//
//	detail.__daily_refresh_type
//			= res[MallActivityInfo::REFRESH_TYPE].numberInt();
//	switch (detail.__daily_refresh_type)
//	{
//	{
//		case GameEnum::MALL_ACTIIVTY_NON_REFRESH:
//		detail.__refresh_tick = detail.__activity_ctrl.__end_tick;
//		break;
//	}
//	{
//		case GameEnum::MALL_ACTIIVTY_DAILY_REFRESH:
//		detail.__refresh_tick = next_day(0, 0).sec();
//		break;
//	}
//
//	default:
//		break;
//	}
	return 0;
}

int ShopMonitor::push_game_shop_info(GameShop* game_shop, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401101*, sync_shop_info, 0);

	for(GameShop::ShopItemMap::iterator iter = game_shop->shop_item_map_.begin();
			iter != game_shop->shop_item_map_.end(); ++iter)
	{
		ShopItem* shop_item = iter->second;

		ProtoOnSaleItem *proto_shop_item = sync_shop_info->add_shop_item_list();
		proto_shop_item->set_item_id(shop_item->item_id_);
		proto_shop_item->set_item_bind(shop_item->item_bind_);

		proto_shop_item->set_shop_type(shop_item->shop_type_);
		proto_shop_item->set_money_type(shop_item->money_type_);
		proto_shop_item->set_src_price(shop_item->src_price_);
		proto_shop_item->set_cur_price(shop_item->cur_price_);

		proto_shop_item->set_start_tick(shop_item->start_tick_);
		proto_shop_item->set_end_tick(shop_item->end_tick_);

		ProtoPairObj *proto_item = proto_shop_item->mutable_need_item();
		proto_item->set_obj_id(shop_item->need_item_.first);
		proto_item->set_obj_value(shop_item->need_item_.second);
	}

	return 0;
}

int ShopMonitor::sync_shop_item_to_scene(int scene_id/*=0*/)
{
//	GameShopMap& shop_map = this->shop_map_;
//	Proto31401101 sync_shop_info;
//	for (GameShopMap::iterator iter = shop_map.begin();
//			iter != shop_map.end(); ++iter)
//	{
//		GameShop* game_shop = iter->second;
//		JUDGE_CONTINUE(NULL != game_shop);
//
//		this->push_game_shop_info(game_shop, &sync_shop_info);
//	}
//
//	if(0 == scene_id)
//		return LOGIC_MONITOR->dispatch_to_all_map(&sync_shop_info);
//	else
//		return LOGIC_MONITOR->dispatch_to_scene(scene_id, &sync_shop_info);
	return 0;
}

int ShopMonitor::respond_sync_shop_item(Message *msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30101201*, request, -1);
//
//	int scene_id = request->scene_id();
//	return this->sync_shop_item_to_scene(scene_id);
	return 0;
}
