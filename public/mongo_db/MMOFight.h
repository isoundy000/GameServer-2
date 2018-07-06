/*
 * MMOFight.h
 *
 * Created on: 2013-04-28 15:17
 *     Author: lyz
 */

#ifndef _MMOFIGHT_H_
#define _MMOFIGHT_H_

#include "MongoTable.h"

class MMOFight : public MongoTable
{
public:
    virtual ~MMOFight(void);

    int load_player_fight(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *mongo_data);

    int load_tiny_player(MapPlayerEx *player);
    static int update_tiny_data(MapPlayerEx *player, MongoDataMap *mongo_data);

    int load_tiny_player(MapLogicPlayer *player);
    static int update_tiny_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};


#endif //_MMOFIGHT_H_
