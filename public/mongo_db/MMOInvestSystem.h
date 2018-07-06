/*
 * MMOInvestSystem.h
 *
 *  Created on: 2016年11月15日
 *      Author: lzy
 */

#ifndef MMOINVESTSYSTEM_H_
#define MMOINVESTSYSTEM_H_

#include "MongoTable.h"

class MongoDataMap;

class MMOInvestSystem : public MongoTable
{
public:
	MMOInvestSystem(void);
    virtual ~MMOInvestSystem(void);

    int load_player(MapLogicPlayer* player);
    int load_old_version(MapLogicPlayer* player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

    static int load_data();
    static int update_data(MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};




#endif /* MMOINVESTSYSTEM_H_ */
