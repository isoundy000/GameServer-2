/*
 * MMOSocialerInfo.cpp
 *
 *  Created on: 2013-7-9
 *      Author: louis
 */

#include "GameField.h"
//#include "MMORole.h"
#include "MMOSocialer.h"
#include "LogicPlayer.h"
#include "MongoConnector.h"
#include "MongoException.h"
#include "MongoDataMap.h"
#include "LogicStruct.h"
//#include "MapPlayerEx.h"
#include "DBCommon.h"
#include "LogicFriendSystem.h"

#include <mongo/client/dbclient.h>
using namespace mongo;

MMOSocialer::~MMOSocialer() {
	// TODO Auto-generated destructor stub
}

int MMOSocialer::load_player_socialer(LogicPlayer* player)
{
	BSONObj res = this->conection().findOne(SocialerInfo::COLLECTION,
			QUERY(SocialerInfo::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	LogicSocialerDetail& socialer = player->socialer_detail();
	socialer.open_ = res[SocialerInfo::OPEN].numberInt();

	BSONObjIterator iter = res.getObjectField(SocialerInfo::APPLY_LIST.c_str());
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		Int64 friend_id = obj[SocialerInfo::ApplyInfo::FRIEND_ID].numberLong();
		LogicSocialerDetail::ApplyInfo &apply_info = socialer.__apply_map[friend_id];
		apply_info.friend_id_ = friend_id;
		apply_info.friend_name_ = obj[SocialerInfo::ApplyInfo::FRIEND_NAME].str();
		apply_info.league_id_ = obj[SocialerInfo::ApplyInfo::LEAGUE_ID].numberLong();
		apply_info.league_name_ = obj[SocialerInfo::ApplyInfo::LEAGUE_NAME].str();
		apply_info.level_ = obj[SocialerInfo::ApplyInfo::LEVEL].numberInt();
		apply_info.sex_ = obj[SocialerInfo::ApplyInfo::SEX].numberInt();
		apply_info.tick_ = obj[SocialerInfo::ApplyInfo::TICK].numberLong();
	}

	DBCommon::bson_to_long_map(socialer.__friend_list, res.getObjectField(SocialerInfo::FRIEND_LIST.c_str()));
	DBCommon::bson_to_long_map(socialer.__stranger_list, res.getObjectField(SocialerInfo::STRANGER_LIST.c_str()));
	DBCommon::bson_to_long_map(socialer.__black_list, res.getObjectField(SocialerInfo::BLACK_LIST.c_str()));
	DBCommon::bson_to_long_map(socialer.__enemy_list, res.getObjectField(SocialerInfo::ENEMY_LIST.c_str()));

//	JUDGE_CONTINUE(this->conector_->mmo_role()->check_role(0) == true);

//	if(res.hasField(SocialerInfo::STRENGTH.c_str()))
//	{
//		StrengthDetail& strength_detail = player->strength_detail();
//		strength_detail.give_times = res[SocialerInfo::GIVE_TIMES].numberInt();
//		strength_detail.strength = res[SocialerInfo::STRENGTH].numberInt();
//		strength_detail.vip_buys = res[SocialerInfo::VIP_BUY].numberInt();
//		{
//			BSONObj strength_list = res.getObjectField(SocialerInfo::STRENGTH_GIVE_LIST.c_str());
//			BSONObjIterator it(strength_list);
//			while(it.more())
//			{
//				strength_detail.give_set.insert(it.next().numberLong());
//			}
//		}
//		if(strength_detail.strength < 0)
//			strength_detail.strength = 0;
//	}
//	else
//	{
//		player->strength_detail().strength = CONFIG_INSTANCE->strength("default")[player->get_vip_index()].asInt();
//	}
	return 0;
}

int MMOSocialer::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
	LogicSocialerDetail &socailer = player->socialer_detail();

	BSONVec friend_vec, stranger_vec, black_vec, enemy_vec, apply_vec;
	DBCommon::long_map_to_bson(friend_vec, socailer.__friend_list);
	DBCommon::long_map_to_bson(stranger_vec, socailer.__stranger_list);
	DBCommon::long_map_to_bson(black_vec, socailer.__black_list);
	DBCommon::long_map_to_bson(enemy_vec, socailer.__enemy_list);

	for (LogicSocialerDetail::ApplyInfoMap::iterator iter = socailer.__apply_map.begin();
			iter != socailer.__apply_map.end(); ++iter)
	{
		LogicSocialerDetail::ApplyInfo &apply_info = iter->second;

		apply_vec.push_back(BSON(SocialerInfo::ApplyInfo::FRIEND_ID << apply_info.friend_id_
				<< SocialerInfo::ApplyInfo::FRIEND_NAME << apply_info.friend_name_
				<< SocialerInfo::ApplyInfo::LEAGUE_ID << apply_info.league_id_
				<< SocialerInfo::ApplyInfo::LEAGUE_NAME << apply_info.league_name_
				<< SocialerInfo::ApplyInfo::LEVEL << apply_info.level_
				<< SocialerInfo::ApplyInfo::SEX << apply_info.sex_
				<< SocialerInfo::ApplyInfo::TICK << apply_info.tick_));
	}

//	// 玩家sign_in时没有reset，导致重复数据太多
//	LongVec give_strength_set;
//	StrengthDetail& strength_detail = player->strength_detail();
//	for(BLongSet::iterator it = strength_detail.give_set.begin();
//				it != strength_detail.give_set.end(); ++it)
//	{
//		give_strength_set.push_back(*it);
//	}
//	if(strength_detail.strength < 0)
//		strength_detail.strength = 0;

	BSONObjBuilder builder;
	builder << SocialerInfo::ID << player->role_id()
			<< SocialerInfo::OPEN << socailer.open_
			<< SocialerInfo::APPLY_LIST << apply_vec
			<< SocialerInfo::FRIEND_LIST << friend_vec
			<< SocialerInfo::STRANGER_LIST << stranger_vec
			<< SocialerInfo::BLACK_LIST << black_vec
			<< SocialerInfo::ENEMY_LIST << enemy_vec;

    mongo_data->push_update(SocialerInfo::COLLECTION,
            BSON(SocialerInfo::ID << player->role_id()),
            builder.obj());
    return 0;
}

void MMOSocialer::load_friend_pair(LogicFriendSystem* sys)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBFriendPair::COLLECTION);
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		int pair_type = res[DBFriendPair::ID].numberInt();
		BSONObjIterator iter = res.getObjectField(DBFriendPair::PAIR_SET.c_str());
		while (iter.more())
		{
			BSONObj iter_pair = iter.next().embeddedObject();
			JUDGE_CONTINUE(iter_pair.isEmpty() == false);

			LongPair pair_info;
			pair_info.first = iter_pair[DBFriendPair::PairSet::PLAYER_ONE].numberLong();
			pair_info.second = iter_pair[DBFriendPair::PairSet::PLAYER_TWO].numberLong();
			int number = iter_pair[DBFriendPair::PairSet::NUMBER].numberInt();

			if (pair_type == FriendPairInfo::FRIEND_ADD)
				sys->friend_map_.insert(FriendPair::value_type(number, pair_info));
			else if (pair_type == FriendPairInfo::FRIEND_DELETE)
				sys->delete_map_.insert(FriendPair::value_type(number, pair_info));
		}
	}
}

void MMOSocialer::save_friend_pair(LogicFriendSystem* sys, int pair_type, bool direct_save)
{
	FriendPair* friend_pair = NULL;
	if (pair_type == LogicFriendSystem::FRIEND_ADD)
	{
		friend_pair = &sys->friend_map_;
	}
	else
	{
		friend_pair = &sys->friend_map_;
	}

	BSONVec friend_set;
	for (FriendPair::iterator iter = friend_pair->begin();
			iter != friend_pair->end(); ++iter)
	{
		LongPair &pair_info = iter->second;
		friend_set.push_back(BSON(DBFriendPair::PairSet::PLAYER_ONE << pair_info.first
				<< DBFriendPair::PairSet::PLAYER_TWO << pair_info.second
				<< DBFriendPair::PairSet::NUMBER << iter->first));
	}

	BSONObjBuilder builder;
	builder << DBFriendPair::ID << pair_type
			<< DBFriendPair::PAIR_SET << friend_set;

	if (!direct_save)
	{
		GameCommon::request_save_mmo_begin(DBFriendPair::COLLECTION,
				BSON(DBFriendPair::ID << pair_type),
				BSON("$set" << builder.obj()));
	}
	else
	{
		CACHED_CONNECTION.update(DBFriendPair::COLLECTION,
				BSON(DBFriendPair::ID << pair_type),
				BSON("$set" << builder.obj()),true);
	}
}

void MMOSocialer::ensure_all_index(void)
{
	BEGIN_CATCH
	this->conection().ensureIndex(SocialerInfo::COLLECTION, BSON(SocialerInfo::ID << -1), true);
	this->conection().ensureIndex(DBFriendPair::COLLECTION, BSON(DBFriendPair::ID << 1), true);
	END_CATCH
}
