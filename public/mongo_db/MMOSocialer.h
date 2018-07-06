/*
 * MMOSocialer.h
 *
 *  Created on: 2013-7-9
 *      Author: louis
 */

#ifndef MMOSOCIALER_H_
#define MMOSOCIALER_H_

#include "MongoTable.h"

class MongoDataMap;
class LogicFriendSystem;

class MMOSocialer : public MongoTable
{
public:
	virtual ~MMOSocialer();

	int load_player_socialer(LogicPlayer* player);
    static int update_data(LogicPlayer *player, MongoDataMap *mongo_data);

    static void load_friend_pair(LogicFriendSystem* sys);
    static void save_friend_pair(LogicFriendSystem* sys,
    		int pair_type, bool direct_save = false);

protected:
	virtual void ensure_all_index(void);
};

#endif /* MMOSOCIALER_H_ */
