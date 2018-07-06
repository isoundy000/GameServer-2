///*
// * MMOFriendshipValue.cpp
// *
// *  Created on: 2013-7-25
// *      Author: louis
// */
//
//#include "GameField.h"
//#include "LogicStruct.h"
//#include "MMOFriendshipValue.h"
//#include "MongoConnector.h"
//#include "MongoException.h"
//#include "MongoDataMap.h"
//#include <mongo/client/dbclient.h>
//using namespace mongo;
//
//MMOFriendshipValue::MMOFriendshipValue() {
//	// TODO Auto-generated constructor stub
//
//}
//
//MMOFriendshipValue::~MMOFriendshipValue() {
//	// TODO Auto-generated destructor stub
//}
//
//int MMOFriendshipValue::load_friendship_value(std::map<int64_t, FriendValueDetail> &friendship_map)
//{
//BEGIN_CATCH
//
//	friendship_map.clear();
//
//    int64_t friendship_id = 0;
//    FriendValueDetail friend_datail;
//
//	auto_ptr<DBClientCursor> cursor = this->conection().query(FriendshipValue::COLLECTION, BSONObj());
//	while(cursor->more())
//	{
//		BSONObj res = cursor->next();
//
//		friendship_id = res[FriendshipValue::FRIEND_ID].numberLong();
//		friend_datail.reset();
//		friend_datail.__first_id = res[FriendshipValue::FriendValueDetail::FIRST_ID].numberLong();
//		friend_datail.__second_id = res[FriendshipValue::FriendValueDetail::SECOND_ID].numberLong();
//		friend_datail.__friend_value = res[FriendshipValue::FriendValueDetail::FRIEND_VALUE].numberInt();
//		friend_datail.__friendship_id = friendship_id;
//
//		friendship_map.insert(std::pair<int64_t, FriendValueDetail>(friendship_id, friend_datail));
//	}
//	return 0;
//END_CATCH
//	return -1;
//}
//
//int MMOFriendshipValue::update_data(FriendValueDetail &friend_datail, MongoDataMap *mongo_data)
//{
//	MSG_USER("friendship_id : %ld, value : %d", friend_datail.__friendship_id , friend_datail.__friend_value);
//
//	BSONObjBuilder builder;
//	builder << FriendshipValue::FRIEND_ID << (long long int)friend_datail.__friendship_id
//			<< FriendshipValue::FriendValueDetail::FIRST_ID << (long long int)friend_datail.__first_id
//			<< FriendshipValue::FriendValueDetail::SECOND_ID << (long long int)friend_datail.__second_id
//			<< FriendshipValue::FriendValueDetail::FRIEND_VALUE << friend_datail.__friend_value;
//	mongo_data->push_update(FriendshipValue::COLLECTION,
//			BSON(FriendshipValue::FRIEND_ID << (long long int)friend_datail.__friendship_id),
//	        builder.obj(), true);
//
//	return 0;
//}
//
//int MMOFriendshipValue::remove_date(const int64_t friendship_id, MongoDataMap *mongo_data)
//{
//	BSONObj condition = BSON(FriendshipValue::FRIEND_ID << (long long int)friendship_id);
//	mongo_data->push_remove(FriendshipValue::COLLECTION, condition);
//	return 0;
//}
//
//int MMOFriendshipValue::remove_friendship_single_record(const int64_t friendship_id)
//{
//	this->conection().remove(FriendshipValue::COLLECTION,
//			QUERY(FriendshipValue::FRIEND_ID << (long long int)friendship_id));
//	return 0;
//}
//
//void MMOFriendshipValue::ensure_all_index(void)
//{
//	this->conection().ensureIndex(FriendshipValue::COLLECTION, BSON(FriendshipValue::FRIEND_ID << 1), true);
//}
