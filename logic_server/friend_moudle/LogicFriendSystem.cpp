/*
 * LogicFriendSystem.cpp
 *
 *  Created on: 2016年10月19日
 *      Author: lyw
 */

#include "LogicFriendSystem.h"
//#include "LogicStruct.h"
#include "MMOSocialer.h"

LogicFriendSystem::LogicFriendSystem() {
	// TODO Auto-generated constructor stub
}

LogicFriendSystem::~LogicFriendSystem() {
	// TODO Auto-generated destructor stub
}

void LogicFriendSystem::start()
{
	MMOSocialer::load_friend_pair(this);

	MSG_USER("friend system start!");
}

void LogicFriendSystem::stop()
{
	MMOSocialer::save_friend_pair(this, FRIEND_ADD, true);
	MMOSocialer::save_friend_pair(this, FRIEND_DELETE, true);

	MSG_USER("friend system stop!");
}

void LogicFriendSystem::insert_offline_friend_pair(Int64 player_one, Int64 player_two, int type)
{
	if (type == FRIEND_ADD)
	{
		int pair_size = this->friend_map_.size() + 1;
		LongPair& friend_pair = this->friend_map_[pair_size];
		friend_pair.first = player_one;
		friend_pair.second = player_two;
	}
	else
	{
		int pair_size = this->delete_map_.size() + 1;
		LongPair& friend_pair = this->delete_map_[pair_size];
		friend_pair.first = player_one;
		friend_pair.second = player_two;
	}

#ifdef LOCAL_DEBUG
	if (type == FRIEND_ADD)
	{
		MMOSocialer::save_friend_pair(this, type);
	}
	else
	{
		MMOSocialer::save_friend_pair(this, type);
	}
#endif
}

void LogicFriendSystem::remove_offline_friend_pair(Int64 player_one, Int64 player_two, int type)
{
	int find_flag = false;
	FriendPair new_pair;
	FriendPair *clone_pair = NULL;

	if (type == FRIEND_ADD)
		clone_pair = &(this->friend_map_);
	else
		clone_pair = &(this->delete_map_);

	JUDGE_RETURN(clone_pair != NULL, ;);

	int number = 1;
	for (FriendPair::iterator iter = clone_pair->begin();
			iter != clone_pair->end(); ++iter)
	{
		LongPair &pair_info = iter->second;
		if ((pair_info.first == player_one && pair_info.second == player_two)
				|| (pair_info.first == player_two && pair_info.second == player_one))
		{
			find_flag = true;
		}
		else
		{
			LongPair &new_info = new_pair[number];
			new_info.first = player_one;
			new_info.second = player_two;
			++number;
		}
	}
	JUDGE_RETURN(find_flag == true, ;);

	if (type == FRIEND_ADD)
	{
		this->friend_map_.clear();
		this->friend_map_ = new_pair;
	}
	else
	{
		this->delete_map_.clear();
		this->delete_map_ = new_pair;
	}
}

LongVec LogicFriendSystem::get_friend_pair_set(Int64 role_id, int type)
{
	LongVec friend_set;
	LongMap remove_pair;
	FriendPair *clone_pair = NULL;
	if (type == FRIEND_ADD)
		clone_pair = &(this->friend_map_);
	else
		clone_pair = &(this->delete_map_);

	JUDGE_RETURN(clone_pair != NULL, friend_set);

	for (FriendPair::iterator iter = clone_pair->begin();
			iter != clone_pair->end(); ++iter)
	{
		LongPair &pair_info = iter->second;
		if (role_id == pair_info.first)
		{
			friend_set.push_back(pair_info.second);
			remove_pair[pair_info.first] = pair_info.second;
		}
		else if(role_id == pair_info.second)
		{
			friend_set.push_back(pair_info.first);
			remove_pair[pair_info.second] = pair_info.first;
		}
	}

	for (LongMap::iterator iter = remove_pair.begin(); iter != remove_pair.end(); ++iter)
	{
		this->remove_offline_friend_pair(iter->first, iter->second, type);
	}

	return friend_set;
}







