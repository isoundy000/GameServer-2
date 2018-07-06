/*
 * LuckyWheelSys.h
 *
 *  Created on: 2016年12月15日
 *      Author: lyw
 */

#ifndef LUCKYWHEELSYS_H_
#define LUCKYWHEELSYS_H_

#include "LogicStruct.h"
#include "ProtoDefine.h"

class LuckyWheelSys : public LuckyWheelActivity
{
public:
	enum
	{
		STATE_NOT_START	= 0,
		STATE_BROCAST	= 1,
		STATE_FLICKER	= 2,
		STATE_END		= 3,

		STATE_FINISH
	};

	class BackActivityTimer : public GameTimer
	{
	public:
		virtual int type();
	    virtual int handle_timeout(const Time_Value &tv);
	};

	class ShopRefreshTimer : public GameTimer
	{
	public:
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		void set_act_type(int act_type);

		int act_type_;
	};
	typedef std::map<int, ShopRefreshTimer> ShopTimerMap;

	class BrocastTimer : public GameTimer
	{
	public:
		BrocastTimer();
		virtual int type();
		virtual int handle_timeout(const Time_Value &tv);

		void set_state(int state);
		int get_state();

	private:
		int state_;
	};

public:
	LuckyWheelSys();
	virtual ~LuckyWheelSys();
	void reset();

	int start(void);
	int stop(void);

	int midnight_handle_timeout();
	int reset_every_day();

	int init_shop_timer(int activity_id);

	int handle_brocast_timeout(); //暂时只用于限时秒杀

	int request_load_activity();
	int after_load_activity_done(DBShopMode* shop_mode);
	int check_update_activity();
	int check_delete_activity(IntMap &type_map);

	int start_limit_buy_brocast();
	int handle_limit_buy_shout();
	int handle_limit_buy_flicker();
	int handle_flicker_end();

	int is_need_cost(ActivityDetail* activity_detail);
	int add_pool_money(ActivityDetail* activity_detail, SlotInfo* slot_info);
	int draw_refresh_activity_info(ActivityDetail* act_detail, SlotInfo* slot_info,
			Int64 role_id, ItemObj& obj, int reward_mult = 0);
	int save_server_record(ActivityDetail* act_detail, SlotInfo* slot_info, ServerItemInfo &item_info);

	int check_combine_reset_act();	//合服后判断是否重置活动
	int update_activity_list();
	IntMap& fetch_all_activity();

	void slot_info_serialize(ProtoSlotInfo* slot, SlotInfo* slot_info,
			WheelPlayerInfo::PlayerDetail* player_detail, int id = 0);

	void test_set_activity(int activity_id, int date_type, int first_date, int last_date);

	ShopRefreshTimer& get_refresh_timer(int act_type);
	int refresh_cabinet_info(int activity_id);
	int check_is_cabinet_activity(int activity_id);

public:
	void save_activity();
	int update_couple_rank_info(Message* msg);
	int trvl_wedding_reward_info(Message* msg);
	int trvl_recharge_rank_mail(Message* msg);

	//跨服世界boss发送邮件
	int trvl_wboss_send_mail(Message* msg);

protected:
	void activity_brocast(int shout_id, ServerItemInfo &item_info);

private:
	IntMap act_map_; //当前的活动
	BackActivityTimer act_timer_;

	ShopTimerMap shop_timer_;
	BrocastTimer brocast_timer_;

};

typedef Singleton<LuckyWheelSys> LuckyWheelSysSingle;
#define LUCKY_WHEEL_SYSTEM	LuckyWheelSysSingle::instance()

#endif /* LUCKYWHEELSYS_H_ */
