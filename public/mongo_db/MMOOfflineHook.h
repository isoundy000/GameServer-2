/*
 * MMOOfflineHook.h
 *
 *  Created on: Mar 21, 2017
 *      Author: root
 */

#ifndef MMOOFFLINEHOOK_H_
#define MMOOFFLINEHOOK_H_

#include "MongoTable.h"

class MapLogicPlayer;

class MMOOfflineHook: public MongoTable
{
public:
	virtual ~MMOOfflineHook(void);

	int load_player_offlinehook(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap *mongo_data);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOOFFLINEHOOK_H_ */
