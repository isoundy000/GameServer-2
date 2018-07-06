/*
 * MarketSystem.h
 *
 *  Created on: 2013-7-19
 *      Author: root
 */

#ifndef MARKETSYSTEM_H_
#define MARKETSYSTEM_H_

#include "MarketStruct.h"

class MarketSystem
{
	friend class MMOMarket;

public:
	MarketSystem();
	~MarketSystem();

	int init(void);
    int fina(void);

    int start(void);
    int stop(void);

    void save_market(int direct_save = true);

    int market_handle_time_out(Int64 now_tick);

	MarketerInfo* find_marketer(Int64 role_id, int flag = true);
	MarketItem* find_market_item(int market_index);

	MarketItemType* find_market(int market_type);
	MarketItemType* find_market_by_item(int item_id);

	int fetch_main_type(int item_id);
	int fetch_sub_type(int item_id);

	MarketItem* pop_market_item();
	int push_market_item(MarketItem* market_item);

	int set_low_price(int item_id, int price);
	int fetch_low_price(int item_id);
	int arrive_max_onsell(Int64 role_index);

	int add_market_item(MarketItem* market_item,bool save = true);
	int remove_market_item(MarketItem* market_item);
	int update_market_item(MarketItem* market_item);

	void undo_all_buy();
	PoolPackage<MarketItemType>* market_type_package();
	PoolPackage<MarketerInfo, Int64>* marketer_package();

private:
	static int handle_market_item(ArrTimeItem* arr_item);

private:
	IntMap low_price_map_;

	ArrTimeManager* market_item_timer_;

	PoolPackage<MarketItem> market_item_package_;
	PoolPackage<MarketItemType> market_type_package_;
	PoolPackage<MarketerInfo, Int64> marketer_package_;
};

typedef Singleton<MarketSystem> 	MarketSystemSingle;
#define MARKET_SYS   				MarketSystemSingle::instance()

#endif /* MARKETSYSTEM_H_ */
