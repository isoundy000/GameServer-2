/*
 * MMOOnlineRewards.h
 *
 *  Created on: 2014-1-16
 *      Author: root
 */

#ifndef MMOONLINEREWARDS_H_
#define MMOONLINEREWARDS_H_

#include "MongoTable.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOOnlineRewards : public MongoTable
{
public:
	MMOOnlineRewards();
	virtual ~MMOOnlineRewards();

	virtual int load_player_online_rewards(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

public:
    static int mmo_list_dump_to_bson(MapLogicPlayer *player, BSONObj& res_obj);
    static int mmo_list_load_from_bson(BSONObj& bson_obj, MapLogicPlayer *player);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOONLINEREWARDS_H_ */
