///*
// * MMOFriendshipValue.h
// *
// *  Created on: 2013-7-25
// *      Author: louis
// */
//
//#ifndef MMOFRIENDSHIPVALUE_H_
//#define MMOFRIENDSHIPVALUE_H_
//
//#include "MongoTable.h"
//
//class LogicPlayer;
//class MongoDataMap;
//
//class MMOFriendshipValue : public MongoTable
//{
//public:
//	MMOFriendshipValue();
//	virtual ~MMOFriendshipValue();
//
//	int load_friendship_value(std::map<int64_t, FriendValueDetail> &friendship_map);
//	static int update_data(FriendValueDetail &friend_datail, MongoDataMap *mongo_data);
//	static int remove_date(const int64_t friendship_id, MongoDataMap *mongo_data);
//
//	int remove_friendship_single_record(const int64_t friendship_id);
//
//protected:
//	virtual void ensure_all_index(void);
//};
//
//#endif /* MMOFRIENDSHIPVALUE_H_ */
