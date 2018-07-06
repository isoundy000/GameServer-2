/*
 * MMOTreasures.h
 *
 *  Created on: 2016年12月1日
 *      Author: lzy
 */

#ifndef MMOTREASURES_H_
#define MMOTREASURES_H_

#include "MongoTable.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOTreasures : public MongoTable
{
public:
	MMOTreasures();
	virtual ~MMOTreasures();

	virtual int load_player_treasures_info(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
	virtual void ensure_all_index(void);
};


#endif /* MMOTREASURES_H_ */
