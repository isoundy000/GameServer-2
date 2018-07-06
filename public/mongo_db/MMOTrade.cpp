/*
 * MMOTrade.cpp
 *
 *  Created on: Jul 8, 2013
 *      Author: peizhibi
 */

#include "GameField.h"
#include "MMOTrade.h"
#include "LogicPlayer.h"
#include "MapLogicPlayer.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOTrade::MMOTrade()
{
	// TODO Auto-generated constructor stub

}

MMOTrade::~MMOTrade()
{
	// TODO Auto-generated destructor stub
}

void MMOTrade::ensure_all_index(void)
{
	this->conection().ensureIndex(DBTrade::COLLECTION, BSON(DBTrade::ID << 1), true);
}

void MMOTrade::remove_mmo_trade(Int64 role_id)
{
    this->conection().remove(DBTrade::COLLECTION, QUERY(DBTrade::ID << role_id));
}

void MMOTrade::save_mmo_trade_begin(LogicPlayer* player)
{
}

void MMOTrade::remove_mmo_trade_begin(MapLogicPlayer *player)
{
    BSONObjBuilder builder;
    builder << DBTrade::DBSTATE << int(true);
    player->request_save_mmo_begin(DBTrade::COLLECTION, BSON(DBTrade::ID
    		<< player->role_id()), BSON("$set" << builder.obj()));
}

bool MMOTrade::validate_mmo_trade(BSONObj& res, Int64 role_id)
{
    JUDGE_RETURN(res.isEmpty() == false, false);
    JUDGE_RETURN(res[DBTrade::DBSTATE].numberInt() == true, true);

	this->remove_mmo_trade(role_id);
	return false;
}

void MMOTrade::load_mmo_trade(MapLogicPlayer *player)
{
//    BSONObj res = this->conection().findOne(DBTrade::COLLECTION,
//    		QUERY(DBTrade::ID << player->role_id()));
//    JUDGE_RETURN(this->validate_mmo_trade(res, player->role_id()) == true, ;);
//
//    Money money;
//    GameCommon::bson_to_money(money, res);
//    player->pack_money_add(money, ADD_MONEY_TRADE_FINISH);
//
//    PackageItem *item = GamePackage::pop_item();
//	BSONObjIterator iter(res.getObjectField(DBTrade::GOODS.c_str()));
//	while (iter.more())
//	{
//		GameCommon::bson_to_item(item, iter.next().embeddedObject());
//
//		ItemObj item_obj(item->__id, item->__amount, item->__bind);
//		player->pack_insert(ITEM_TRADE_RETURN, item_obj);
//	}
//
//	GamePackage::push_item(item);
//    this->remove_mmo_trade(player->role_id());
}
