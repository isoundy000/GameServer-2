/*
 * MMOShop.h
 *
 *  Created on: Nov 11, 2013
 *      Author: peizhibi
 */

#ifndef MMOSHOP_H_
#define MMOSHOP_H_

#include "MongoTable.h"
#include "ShopMonitor.h"

class DBShopMode;
class MMOShop : public MongoTable
{
public:
	MMOShop();
	virtual ~MMOShop();

	void load_shop(DBShopMode* shop_mode);
	void load_shop_by_config(DBShopMode* shop_mode);

	static void load_shop_by_config(VoidVec& void_vec);
	static void load_limited_item(ShopMonitorDetail* monitor_detail);
	static void save_limited_item(ShopMonitorDetail* monitor_detail);
	static void load_limited_total_item(ShopMonitorDetail* monitor_detail);
	static void save_limited_total_item(ShopMonitorDetail* monitor_detail);

protected:
	virtual void ensure_all_index();
};

#endif /* MMOSHOP_H_ */
