/*
 * MMOEscort.h
 *
 *  Created on: 2016年10月25日
 *      Author: lzy
 */

#ifndef MMOESCORT_H_
#define MMOESCORT_H_

#include "MongoTable.h"

class MapPlayerEx;
class MapLogicPlayer;
class MongoDataMap;

class MMOEscort : public MongoTable
{
public:
    virtual ~MMOEscort(void);

    int load_player_escort(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};


#endif /* MMOESCORT_H_ */
