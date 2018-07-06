/*
 * MMOLuckyWheel.h
 *
 *  Created on: 2016年12月14日
 *      Author: lyw
 */

#ifndef MMOLUCKYWHEEL_H_
#define MMOLUCKYWHEEL_H_

#include "MongoTable.h"

class LuckyWheelSys;
class DailyActSys;

class MMOLuckyWheel : public MongoTable
{
public:
	MMOLuckyWheel();
	virtual ~MMOLuckyWheel();

	int load_lucky_wheel_activity(LogicPlayer *player);
	static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);

	static void load_lucky_wheel_system(LuckyWheelSys* sys);
	static void save_lucky_wheel_system(LuckyWheelSys* sys);

	static void load_daily_act_system(DailyActSys* sys);
	static void save_daily_act_system(DailyActSys* sys);

	int load_back_activity_info(DBShopMode* shop_mode);	//后台控制数据

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOLUCKYWHEEL_H_ */
