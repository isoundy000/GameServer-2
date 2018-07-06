/*
 * MMOMarket.cpp
 *
 *  Created on: 2013-7-26
 *      Author: root
 */

#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

#include "MMOMarket.h"
#include "GameField.h"
#include "GameCommon.h"
#include "MarketSystem.h"

MMOMarket::MMOMarket()
{

}

MMOMarket::~MMOMarket()
{

}

void MMOMarket::ensure_all_index(void)
{
	this->conection().ensureIndex(DBMarket::COLLECTION,
			BSON(DBMarket::ID <<1), true);
	this->conection().ensureIndex(DBMarketItem::COLLECTION,
			BSON(DBMarketItem::ID <<1), true);
}

void MMOMarket::load_market_system()
{
//	BSONObj res = CACHED_CONNECTION.findOne(DBMarket::COLLECTION,
//			QUERY(DBMarket::ID << 0));
//	JUDGE_RETURN(res.isEmpty() == false, ;);
//
//	GameCommon::bson_to_map(MARKET_SYS->last_gold_map_,
//			res.getObjectField(DBMarket::LAST_GOLD.c_str()));
//	GameCommon::bson_to_map(MARKET_SYS->last_copper_map_,
//			res.getObjectField(DBMarket::LAST_COPPER.c_str()));
}

void MMOMarket::save_market_system()
{
//	BSONVec gold_set;
//	GameCommon::map_to_bson(gold_set, MARKET_SYS->last_gold_map_, true);
//
//	BSONVec copper_set;
//	GameCommon::map_to_bson(copper_set, MARKET_SYS->last_copper_map_, true);
//
//    BSONObjBuilder builder;
//    builder << DBMarket::LAST_GOLD << gold_set
//    		<< DBMarket::LAST_COPPER << copper_set;
//    CACHED_CONNECTION.update(DBMarket::COLLECTION, BSON(DBMarket::ID << 0),
//    		BSON("$set" << builder.obj()), true);
}

void MMOMarket::remove_all_market_item()
{
	BSONObj empty;
	CACHED_CONNECTION.remove(DBMarketItem::COLLECTION, empty, false);
}

void MMOMarket::load_all_market_item()
{
    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(
    		DBMarketItem::COLLECTION);

    while (cursor->more())
    {
    	BSONObj res = cursor->next();

    	MarketItem* market_item = MARKET_SYS->pop_market_item();
    	JUDGE_CONTINUE(market_item != NULL);

    	if (res.hasField(DBMarketItem::ITEM_OBJ.c_str()) == false)
    	{
    		market_item->set_item_id(res[DBMarketItem::ITEM_ID].numberInt());
    		market_item->item_obj_.__amount = res[DBMarketItem::ITEM_AMOUNT].numberInt();
    	}
    	else
    	{
    		GameCommon::bson_to_item(&(market_item->item_obj_), res.getObjectField(DBMarketItem::ITEM_OBJ.c_str()));
    		market_item->set_item_id(market_item->item_obj_.__id);
    	}

    	market_item->role_id_ = res[DBMarketItem::ROLE_ID].numberLong();
    	market_item->onsale_tick_ = res[DBMarketItem::ON_TICK].numberLong();
    	market_item->arr_tick_ = res[DBMarketItem::OFF_TICK].numberLong();
    	market_item->money_type_ = res[DBMarketItem::MONEY_TYPE].numberInt();
    	market_item->price_ = res[DBMarketItem::PRICE].numberInt();
    	MARKET_SYS->add_market_item(market_item,false);
    }

    MMOMarket::remove_all_market_item();
    MARKET_SYS->save_market(true);
}

void MMOMarket::save_market_item(MarketItem* market_item, int direct_save)
{
	JUDGE_RETURN(market_item != NULL, ;);
	JUDGE_RETURN(market_item->item_obj_.__amount > 0, ;);

    BSONObjBuilder builder;
    builder << DBMarketItem::ROLE_ID << market_item->role_id_
    		<< DBMarketItem::ON_TICK << market_item->onsale_tick_
    		<< DBMarketItem::OFF_TICK << market_item->arr_tick_
    		<< DBMarketItem::ITEM_OBJ << GameCommon::item_to_bson(&(market_item->item_obj_))
    		<< DBMarketItem::MONEY_TYPE << market_item->money_type_
    		<< DBMarketItem::PRICE << market_item->price_;

    if (direct_save == true)
    {
		CACHED_CONNECTION.update(DBMarketItem::COLLECTION,
				BSON(DBMarketItem::ID << market_item->market_index_),
				BSON("$set" << builder.obj()), true);
    }
    else
    {
    	GameCommon::request_save_mmo_begin(DBMarketItem::COLLECTION,
    			BSON(DBMarketItem::ID << market_item->market_index_),
				BSON("$set" << builder.obj()));
    }
}
void MMOMarket::remove_market_item(MarketItem* market_item, int direct_remove /*=true*/)
{
	JUDGE_RETURN(market_item != NULL, ;);

	if (direct_remove == true)
	{
		CACHED_CONNECTION.remove(DBMarketItem::COLLECTION,
				BSON(DBMarketItem::ID << market_item->item_obj_.__index), true);
	}
	else
	{
		GameCommon::request_remove_mmo_begin(DBMarketItem::COLLECTION,
				BSON(DBMarketItem::ID << market_item->item_obj_.__index));
	}

}
