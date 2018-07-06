/*
 * MMOTrade.h
 *
 *  Created on: Jul 8, 2013
 *      Author: peizhibi
 */

#ifndef MMOTRADE_H_
#define MMOTRADE_H_

#include "MongoTable.h"
#include "GameHeader.h"

class MMOTrade : public MongoTable
{
public:
	MMOTrade();
	virtual ~MMOTrade();

	static void save_mmo_trade_begin(LogicPlayer* player);
	static void remove_mmo_trade_begin(MapLogicPlayer *player);

	void load_mmo_trade(MapLogicPlayer *player);

protected:
	virtual void ensure_all_index();

	void remove_mmo_trade(Int64 role_id);
	bool validate_mmo_trade(BSONObj& obj, Int64 role_id);
};

#endif /* MMOTRADE_H_ */
