/*
 * MMOOfflineRewards.h
 *
 *  Created on: 2016年8月19日
 *      Author: lzy
 */

#ifndef MMOOFFLINEREWARDS_H_
#define MMOOFFLINEREWARDS_H_

#include "MongoTable.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOOfflineRewards : public MongoTable
{
public:
	MMOOfflineRewards();
	virtual ~MMOOfflineRewards();

	virtual int load_player_offline_rewards(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOOFFLINEREWARDS_H_ */
