/*
 * MMOVip.h
 *
 *  Created on: 2013-12-27
 *      Author: root
 */

#ifndef MMOVIP_H_
#define MMOVIP_H_

#include "MongoTable.h"

class BaseVipDetail;

class MMOVip : public MongoTable
{
public:
	MMOVip();

    int load_player_vip(MapLogicPlayer *player);
    int load_player_vip(MapPlayerEx* player);
    int load_player_vip(LogicPlayer* player);

    static int load_base_vip(BaseVipDetail& base_vip, const BSONObj& res);
    static int update_data(MapLogicPlayer* player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

class MMOExpRestore : public MongoTable
{
public:
	MMOExpRestore();
    int load_player_exp_restore(MapLogicPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};

#endif /* MMOVIP_H_ */
