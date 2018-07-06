/*
 * MarketStruct.h
 *
 *  Created on: Nov 4, 2013
 *      Author: peizhibi
 */

#ifndef MARKETSTRUCT_H_
#define MARKETSTRUCT_H_

#include "LogicStruct.h"

struct MarketItem : public ArrTimeItem
{
	MarketItem(void);
	void reset(void);

	int set_item_id(int item_id);
	int left_tick(Int64 now_tick = ::time(NULL));

	int market_index_;

	Int64 role_id_;
	Int64 onsale_tick_;

	PackageItem item_obj_;

	int main_type_;
	int sub_type_;

	int money_type_;
	int price_;

	int undo_amount_;
	bool item_back_;
};

struct MarketItemType
{
	typedef std::vector<MarketItem*> MarketItemSet;

	MarketItemType(void);

	void reset(void);
	void fetch_item_sort(Int64 req_role, ThreeObjVec& obj_vec, int sub_type, int sort_type);

	MarketItem* find_market_item(int market_index);
	bool erase_market_item(MarketItem* market_item);

	int market_type_;
	MarketItemSet item_set_;
};

struct MarketerInfo
{
	typedef std::map<int, MarketItem*> MarketItemMap;

	MarketerInfo(void);

	void reset(void);
	void add_sell_log(int item_id, int money, Int64 tick = ::time(NULL));

	Int64 role_index_;
	ThreeObjList sell_log_;
	MarketItemMap onsell_map_;
};


#endif /* MARKETSTRUCT_H_ */
