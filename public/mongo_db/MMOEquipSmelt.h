/*
 * MMOEquipSmelt.h
 *
 *  Created on: 2016年8月9日
 *      Author: lzy0927
 */

#ifndef MMOEQUIPSMELT_H_
#define MMOEQUIPSMELT_H_

#include "MongoTable.h"

class MMOEquipSmelt : public MongoTable
{
public:
	MMOEquipSmelt ();
	virtual ~MMOEquipSmelt ();

	virtual int load_player_equip_smelt(MapLogicPlayer * player);
    static int update_data(MapLogicPlayer *player, MongoDataMap *data_map);

protected:
	virtual void ensure_all_index(void);
};
#endif /* MMOEQUIPSMELT_H_ */
