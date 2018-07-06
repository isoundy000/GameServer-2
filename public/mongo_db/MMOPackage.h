/*
 * MMOPackage.h
 *
 * Created on: 2013-05-04 10:18
 *     Author: lyz
 */

#ifndef _MMOPACKAGE_H_
#define _MMOPACKAGE_H_

#include "MongoTable.h"

class RechargeOrder;

class MMOPackage : public MongoTable
{
public:
    virtual ~MMOPackage(void);

    int load_player_package(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
    virtual void ensure_all_index(void);
};

#endif //_MMOPACKAGE_H_
