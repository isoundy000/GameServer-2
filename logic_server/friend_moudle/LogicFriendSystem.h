/*
 * LogicFriendSystem.h
 *
 *  Created on: 2016年10月19日
 *      Author: lyw
 */

#ifndef LOGICFRIENDSYSTEM_H_
#define LOGICFRIENDSYSTEM_H_

//#include "GameHeader.h"
#include "LogicStruct.h"

class LogicFriendSystem : public FriendPairInfo
{
public:
	LogicFriendSystem();
	virtual ~LogicFriendSystem();

	void start();
	void stop();

	void insert_offline_friend_pair(Int64 player_one, Int64 player_two, int type);
	void remove_offline_friend_pair(Int64 player_one, Int64 player_two, int type);

	LongVec get_friend_pair_set(Int64 role_id, int type);
};

typedef Singleton<LogicFriendSystem> LogicFriendSystemSingle;
#define LOGIC_FRIEND_SYSTEM	LogicFriendSystemSingle::instance()

#endif /* LOGICFRIENDSYSTEM_H_ */
