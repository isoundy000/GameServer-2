/*
 * MMOMediaGift.h
 *
 *  Created on: Aug 13, 2014
 *      Author: root
 */

#ifndef MMOMEDIAGIFT_H_
#define MMOMEDIAGIFT_H_

#include "MongoTable.h"
#include "MapLogicStruct.h"

class MapLogicPlayer;
class MongoDataMap;

class MMOMediaGift : public MongoTable
{
public:
	MMOMediaGift();
	virtual ~MMOMediaGift();

	int load_player_media_gift(MapLogicPlayer *player);
	static int update_data(MapLogicPlayer *player, MongoDataMap* mongo_data);

protected:
	void ensure_all_index(void);
};


#endif /* MMOMEDIAGIFT_H_ */
