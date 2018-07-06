/*
 * MMOMarket.h
 *
 *  Created on: 2013-7-26
 *      Author: root
 */

#ifndef MMOMARKET_H_
#define MMOMARKET_H_

#include "MongoTable.h"

class MarketItem;

class MMOMarket: public MongoTable
{
public:
	MMOMarket();
	virtual ~MMOMarket();

	virtual void ensure_all_index(void);

	static void load_market_system();
	static void save_market_system();

	static void remove_all_market_item();
	static void load_all_market_item();
	static void save_market_item(MarketItem* market_item, int direct_save = true);
	static void remove_market_item(MarketItem* market_item, int direct_remove = true);
};

#endif /* MMOMARKET_H_ */
