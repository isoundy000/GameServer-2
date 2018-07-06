/*
 * ShopMonitor.h
 *
 *  Created on: Oct 29, 2013
 *      Author: peizhibi
 */

#ifndef SHOPMONITOR_H_
#define SHOPMONITOR_H_

#include "GameShop.h"
#include "BaseShop.h"
#include "MallActivity.h"

class ShopMonitor : public BaseShop
{
private:
	class ShopMonitorTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
		void set_parent(ShopMonitor* shop_monitor);

	private:
		ShopMonitor* shop_monitor_;
	};

public:
	ShopMonitor();
	~ShopMonitor();
	/*
	 * server start
	 * */
	void start();
	void stop();
	void reset_everyday();

	int is_item_limited(const ShopItem* item);
	int is_total_limited(const ShopItem* item);
	int fetch_left_item(ShopItem* item);
	int fetch_left_total_item(ShopItem* item);
	int set_item_buy_limited(ShopItem* item, int buy_amount);
	int set_item_buy_total_limited(ShopItem* item, int buy_amount);

	/*
	 * mall activity
	 * */
	MallActivity& mall_activity(void);
	int request_load_mall_activity(void);
	int request_save_mall_activity(void);
	int after_load_mall_activity(Transaction* trans);

	int reqeust_load_game_shop();
	int after_load_game_shop(DBShopMode* shop_mode);
	int init_game_shop(VoidVec& shop_item_list);

	/*将热销里面的道具价格同步到商城其他标签页下*/
	int sync_hot_sale_price(void);
	int sync_hot_sale_price_to_all(const int item_id);
	int sync_shop_item_to_scene(int scene_id=0);

	int respond_sync_shop_item(Message *msg);

protected:
	int bson2mall_activity(mongo::BSONObj& res);
	int push_game_shop_info(GameShop* game_shop, Message *msg);

protected:
	string shop_version_;
	ShopMonitorDetail monitor_detail_;

	//mall activity
	MallActivity mall_activity_;
	ShopMonitorTimer shop_monitor_timer_;
};

typedef Singleton<ShopMonitor> 	ShopMonitorSingle;
#define SHOP_MONITOR     		ShopMonitorSingle::instance()

#endif /* SHOPMONITOR_H_ */
