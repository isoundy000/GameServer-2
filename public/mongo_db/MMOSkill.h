/*
 * MMOSkill.h
 *
 * Created on: 2013-06-18 09:59
 *     Author: lyz
 */

#ifndef _MMOSKILL_H_
#define _MMOSKILL_H_

#include "MongoTable.h"

class MapPlayerEx;
class MapLogicPlayer;
class MongoDataMap;

class MMOSkill : public MongoTable
{
public:
    virtual ~MMOSkill(void);

    int load_player_skill(MapPlayerEx *player);
    static int update_data(MapPlayerEx *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOSKILL_H_
