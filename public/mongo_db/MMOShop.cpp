/*
 * MMOShop.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: peizhibi
 */


#include "MMOShop.h"
#include "GameField.h"
#include "ShopMonitor.h"

#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>


MMOShop::MMOShop()
{
	// TODO Auto-generated constructor stub

}

MMOShop::~MMOShop()
{
	// TODO Auto-generated destructor stub
}

void MMOShop::load_shop(DBShopMode* shop_mode)
{
//    auto_ptr<DBClientCursor> cursor = this->conection().query(DBShopItem::COLLECTION);
//    while (cursor->more())
//    {
//        BSONObj res = cursor->next();
//
//        ShopItem* shop_item = SHOP_MONITOR->pop_shop_item();
//        JUDGE_CONTINUE(shop_item != NULL);
//
//        shop_item->shop_type_ = res[DBShopItem::SHOP_TYPE].numberInt();
//        shop_item->item_pos_ = res[DBShopItem::ITEM_POS].numberInt();
//        shop_item->item_id_ = res[DBShopItem::ITEM_ID].numberInt();
//        shop_item->item_bind_ = res[DBShopItem::ITEM_BIND].numberInt();
//        shop_item->money_type_ = res[DBShopItem::MONEY_TYPE].numberInt();
//        shop_item->src_price_ = res[DBShopItem::SRC_PRICE].numberInt();
//        shop_item->cur_price_ = res[DBShopItem::CUR_PRICE].numberInt();
//        shop_item->start_tick_ = res[DBShopItem::START_TICK].numberLong();
//        shop_item->end_tick_ = res[DBShopItem::END_TICK].numberLong();
//        shop_item->content_ = res[DBShopItem::CONTENT].str();
//
//        if(res.hasField(DBShopItem::ITEM_TYPE.c_str()))
//        {
//        	BSONObj obj = res.getObjectField(DBShopItem::ITEM_TYPE.c_str());
//        	BSONObjIterator it(obj);
//        	while(it.more())
//        	{
//        		int item_type = it.next().numberInt();
//        		shop_item->item_type_.push_back(item_type);
//        	}
//        }
//
//        shop_mode->output_argv_.void_vec_.push_back(shop_item);
//    }
}

void MMOShop::load_shop_by_config(DBShopMode* shop_mode)
{
	string version = CONFIG_INSTANCE->fetch_shop_verison();
	JUDGE_RETURN(shop_mode->input_argv_.type_string_ != version, ;);

	MMOShop::load_shop_by_config(shop_mode->output_argv_.void_vec_);
	shop_mode->output_argv_.type_int_ = 1;
	shop_mode->output_argv_.type_string_ = version;
}

void MMOShop::load_shop_by_config(VoidVec& void_vec)
{
	const Json::Value& all_shop = CONFIG_INSTANCE->shop()["shop"];
	for (uint i = 0; i < all_shop.size(); ++i)
	{
	    ShopItem* shop_item = SHOP_MONITOR->pop_shop_item();
	    JUDGE_CONTINUE(shop_item != NULL);

	    shop_item->shop_type_ = all_shop[i][DBShopItem::SHOP_TYPE].asInt();
	    shop_item->item_pos_ = all_shop[i][DBShopItem::ITEM_POS].asInt();
	    shop_item->item_id_ = all_shop[i][DBShopItem::ITEM_ID].asInt();
	    shop_item->money_type_ = all_shop[i][DBShopItem::MONEY_TYPE].asInt();
	    shop_item->src_price_ = all_shop[i][DBShopItem::SRC_PRICE].asInt();
	    shop_item->cur_price_ = all_shop[i][DBShopItem::CUR_PRICE].asInt();
        shop_item->content_ = all_shop[i][DBShopItem::CONTENT].asString();
        shop_item->max_item_ = all_shop[i][DBShopItem::MAX_ITEM].asInt();
        shop_item->max_total_ = all_shop[i][DBShopItem::MAX_TOTAL].asInt();
        shop_item->item_bind_ = all_shop[i][DBShopItem::ITEM_BIND].asInt();

	    if (all_shop[i][DBShopItem::ITEM_TYPE].isArray())
	    {
	    	GameCommon::json_to_t_vec(shop_item->item_type_, all_shop[i][DBShopItem::ITEM_TYPE]);
	    }

	    void_vec.push_back(shop_item);
	}
}

void MMOShop::load_limited_item(ShopMonitorDetail* monitor_detail)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBLWTicker::COLLECTION,
			BSON(DBLWTicker::ID << int(2)));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	GameCommon::bson_to_map(monitor_detail->limited_map_, res.getObjectField(
			DBLWTicker::Shop::LIMITED_SET.c_str()));
}

void MMOShop::load_limited_total_item(ShopMonitorDetail* monitor_detail)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBLWTicker::COLLECTION,
			BSON(DBLWTicker::ID << int(2)));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	GameCommon::bson_to_map(monitor_detail->limited_total_map_, res.getObjectField(
			DBLWTicker::Shop::LIMITED_TOTAL_SET.c_str()));
}

void MMOShop::save_limited_item(ShopMonitorDetail* monitor_detail)
{
	JUDGE_RETURN(monitor_detail->limited_map_.empty() == false, ;);

	BSONVec prop_bson_set;
	GameCommon::map_to_bson(prop_bson_set, monitor_detail->limited_map_);

	BSONObjBuilder builder;
	builder << DBLWTicker::Shop::LIMITED_SET << prop_bson_set;

	GameCommon::request_save_mmo_begin(DBLWTicker::COLLECTION,
			BSON(DBLWTicker::ID << int(2)), BSON("$set" << builder.obj()));
}


void MMOShop::save_limited_total_item(ShopMonitorDetail* monitor_detail)
{
	JUDGE_RETURN(monitor_detail->limited_total_map_.empty() == false, ;);

	BSONVec prop_bson_set;
	GameCommon::map_to_bson(prop_bson_set, monitor_detail->limited_total_map_);

	BSONObjBuilder builder;
	builder << DBLWTicker::Shop::LIMITED_TOTAL_SET << prop_bson_set;

	GameCommon::request_save_mmo_begin(DBLWTicker::COLLECTION,
			BSON(DBLWTicker::ID << int(2)), BSON("$set" << builder.obj()));
}

void MMOShop::ensure_all_index()
{
	this->conection().ensureIndex(DBShopItem::COLLECTION,
			BSON(DBShopItem::ITEM_ID << 1 << DBShopItem::SHOP_TYPE << 1), true);
}

