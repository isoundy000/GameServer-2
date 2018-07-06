/*
 * MMOMagicWeapon.h
 *
 *  Created on: 2015-12-15
 *      Author: xu
 */

#ifndef MMOMAGICWEAPON_H_
#define MMOMAGICWEAPON_H_

#include "MongoTable.h"

class MapPlayer;

class MMOMagicWeapon : public MongoTable
{
public:
	MMOMagicWeapon();
    virtual ~MMOMagicWeapon(void);

    int load_player_magic_weapon(MapLogicPlayer *player);
    int load_player_magic_weapon(MapPlayer *player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);

};

#endif /* MMOMAGICWEAPON_H_ */
