/*
 * MMOActivityTipsSystem.h
 *
 *  Created on: Apr 18, 2014
 *      Author: lijin
 */

#ifndef MMOACTIVITYTIPSSYSTEM_H_
#define MMOACTIVITYTIPSSYSTEM_H_

#include "MongoTable.h"

class MMOActivityTips : public MongoTable
{
public:
	MMOActivityTips();
	~MMOActivityTips();

	int load_player_activity_tips(LogicPlayer *player);
	static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);

protected:
    virtual void ensure_all_index(void);
};


#endif /* MMOACTIVITYTIPSSYSTEM_H_ */
