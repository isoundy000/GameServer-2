/*
 * MMORechargeRankConfig.h
 *
 *  Created on: Jan 21, 2015
 *      Author: LiJin
 */

#ifndef MMORECHARGERANKCONFIG_H_
#define MMORECHARGERANKCONFIG_H_

#include "MongoTable.h"

class MLBackDailyRechargeSys;

class MMORechargeRewards : public MongoTable
{
public:
	MMORechargeRewards();
	virtual ~MMORechargeRewards();

	int load_player_recharge_rewards(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap* mongo_data);

	static void load_daily_recharge_mail_role(MLBackDailyRechargeSys *sys);
	static void save_daily_recharge_mail_role(MLBackDailyRechargeSys *sys);

protected:
	void ensure_all_index(void);
};

#endif /* MMORECHARGERANKCONFIG_H_ */
