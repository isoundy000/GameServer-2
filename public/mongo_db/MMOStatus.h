/*
 * MMOStatus.h
 *
 * Created on: 2013-06-18 10:50
 *     Author: lyz
 */

#ifndef _MMOSTATUS_H_
#define _MMOSTATUS_H_

#include "MongoTable.h"

class MapPlayerEx;
class MongoDataMap;

class MMOStatus : public MongoTable
{
public:
    virtual ~MMOStatus(void);

    int load_player_status(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSTATUS_H_
