/*
 * MMOBeast.h
 *
 *  Created on: Nov 21, 2013
 *      Author: peizhibi
 */

#ifndef MMOBEAST_H_
#define MMOBEAST_H_

#include "MongoTable.h"
#include "GameHeader.h"

class MMOBeast : public MongoTable
{
public:
	MMOBeast();
	virtual ~MMOBeast();

	void load_master(MapLogicPlayer* player);
	void load_map_master(MapPlayerEx *player);

	static void save_master(MapLogicPlayer* player, MongoDataMap* data_map);

protected:
	virtual void ensure_all_index();

};

#endif /* MMOBEAST_H_ */
