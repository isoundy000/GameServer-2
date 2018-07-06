/*
 * MMOOpenActivity.h
 *
 *  Created on: Sep 4, 2014
 *      Author: jinxing
 */

#ifndef MMOOPENACTIVITY_H_
#define MMOOPENACTIVITY_H_

#include "MongoTable.h"

class OpenActivitySys;
class MayActivitySys;

class MMOOpenActivity: public MongoTable
{
public:
	MMOOpenActivity();
	virtual ~MMOOpenActivity();

	//open activity
	int load_open_activity(LogicPlayer *player);
	static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);

	static void load_open_activity_sys(OpenActivitySys* sys);
	static void save_open_activity_sys(OpenActivitySys* sys);

	//festival activity
	static void load_fetst_act_time();
	void load_fest_act_time(DBShopMode* shop_mode);

	//may activity
	static void load_may_activity_sys(MayActivitySys* sys);
	static void save_may_activity_sys(MayActivitySys* sys);

	int load_may_activityer(LogicPlayer *player);
	static int save_may_activity(LogicPlayer *player, MongoDataMap *mongo_data);

	int load_back_may_activity(DBShopMode* shop_mode);		//后台控制数据

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOOPENACTIVITY_H_ */
