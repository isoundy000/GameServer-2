/*
 * MMOCollectChests.h
 *
 *  Created on: 2016年8月8日
 *      Author: lzy0927
 */

#ifndef MMOCOLLECTCHESTS_H_
#define MMOCOLLECTCHESTS_H_



#include "MongoTable.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOCollectChests : public MongoTable
{
public:
	MMOCollectChests();
	virtual ~MMOCollectChests();

	virtual int load_player_collect_chests(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);


protected:
	virtual void ensure_all_index(void);
};


#endif /* MMOCOLLECTCHESTS_H_ */
