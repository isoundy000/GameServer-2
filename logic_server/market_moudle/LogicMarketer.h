/*
 * LogicMarketer.h
 *
 *  Created on: Nov 4, 2013
 *      Author: peizhibi
 */

#ifndef LOGICMARKETER_H_
#define LOGICMARKETER_H_

#include "BaseLogicPlayer.h"
#include "MarketStruct.h"

class LogicMarketer : virtual public BaseLogicPlayer
{
public:
	LogicMarketer();
	virtual ~LogicMarketer();

	int fetch_market_info(Message* msg);
	int fetch_sell_log(Message* msg);

	int market_onsell(Message* msg);
	int map_market_onsell(Message* msg);

	int market_buy_goods(Message* msg);
	static int map_market_buy_goods(Int64 role_id, Message* msg);

	int fetch_market_low_price(Message* msg);
	int fetch_self_market_info(Message* msg);

	int market_get_back(Message* msg);
	int shout_market_item(Message* msg);

	int market_continue_sell(Message* msg);
	int market_onsell_time();
};

#endif /* LOGICMARKETER_H_ */
