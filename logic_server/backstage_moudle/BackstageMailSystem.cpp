/*
 * BackstageMailSystem.cpp
 *
 *  Created on: Jun 30, 2014
 *      Author: louis
 */

#include "BackstageMailSystem.h"
#include "BackField.h"
#include "LogicMonitor.h"
#include "PoolMonitor.h"
#include "ProtoDefine.h"
#include "GameFont.h"
#include "GameCommon.h"
#include "Transaction.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "TQueryCursor.h"
#include "BackMailRequest.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

BackstageMailSystem::BackstageMailSystem() {
	// TODO Auto-generated constructor stub

}

BackstageMailSystem::~BackstageMailSystem() {
	// TODO Auto-generated destructor stub
}

int BackstageMailSystem::request_load_back_mail_request(void)
{
	return LOGIC_MONITOR->process_inner_back_thread(TRANS_LOAD_BACK_MAIL_REQUEST);
//	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
//	JUDGE_RETURN(NULL != data_map, -1);
//
//	BackMailRequest::load_mail_request(data_map);
//	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_BACK_MAIL_REQUEST,
//			data_map, LOGIC_MONITOR->logic_unit()) != 0)
//	{
//		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
//		return -1;
//	}
//	return 0;
}

// 这接口已不用，转为BackUnit处理process_back_mail_request；
int BackstageMailSystem::after_load_back_mail_request(Transaction* trans)
{
	JUDGE_RETURN(NULL != trans, -1);
	if(trans->detail().__error != 0)
	{
		trans->rollback();
		return trans->detail().__error;
	}

	MongoDataMap* data_map = trans->fetch_mongo_data_map();
	if (NULL == data_map)
	{
		trans->rollback();
		return 0;
	}

	MongoData* mongo_data = NULL;
	if(data_map->find_data(DBBackMail::COLLECTION, mongo_data) == 0)
	{
		auto_ptr<TQueryCursor> cursor = mongo_data->multithread_cursor();
		while(cursor->more())
		{
			BSONObj res = cursor->next();
			this->handle_back_mail_request(res);

			int request_id = res[DBBackMail::ID].numberInt();
			this->request_update_back_mail_request_flag(request_id);
		}
	}

	trans->summit();
	return 0;
}

int BackstageMailSystem::handle_back_mail_request(mongo::BSONObj& res)
{
	JUDGE_RETURN(res.isEmpty() == false, -1);

	//receiver id set
	LongVec receiver_set;
	if(res.hasField(DBBackMail::RECEIVER_SET.c_str()))
	{
		BSONObj obj = res.getObjectField(DBBackMail::RECEIVER_SET.c_str());
		BSONObjIterator it(obj);
		while(it.more())
		{
			Int64 player_id = it.next().numberLong();
			receiver_set.push_back(player_id);
		}
	}

	JUDGE_RETURN(receiver_set.empty() == false, -1);

	FontPair pair = CONFIG_INSTANCE->font(FONT_SYSTEM_MAIL);
	std::string sender_name = pair.first;

	std::string title   = res[DBBackMail::TITLE].String();
	std::string content = res[DBBackMail::CONTENT].String();

	//money
	int gold = 0, bind_gold = 0, copper = 0, bind_copper = 0;
	if(res.hasField(DBBackMail::MONEY.c_str()))
	{
		BSONObj obj = res.getObjectField(DBBackMail::MONEY.c_str());
		gold = obj[DBBackMail::Money::GOLD].numberInt();
		bind_gold = obj[DBBackMail::Money::BIND_GOLD].numberInt();
		copper = obj[DBBackMail::Money::COPPER].numberInt();
		bind_copper = obj[DBBackMail::Money::BIND_COPPER].numberInt();
	}
	Money attach_money(gold, bind_gold, copper, bind_copper);

	//make up mail info
	LongVec::iterator iter = receiver_set.begin();
	for(; iter != receiver_set.end(); ++iter)
	{
		Int64 receiver_id = *iter;

		MailInformation* mail_info = GameCommon::create_sys_mail(title, content, FONT_SYSTEM_MAIL);
		mail_info->sender_name_ = sender_name;
//		mail_info->receiver_id_ = receiver_id;
		mail_info->money_       = attach_money;

		if(res.hasField(DBBackMail::ITEM.c_str()))
		{
			BSONObj obj = res.getObjectField(DBBackMail::ITEM.c_str());
			BSONObjIterator it(obj);
			while(it.more())
			{
				BSONObj item = it.next().embeddedObject();
				int item_id = item[DBBackMail::Item::ITEM_ID].numberInt();
				int item_amount = item[DBBackMail::Item::ITEM_AMOUNT].numberInt();
				int item_bind = item[DBBackMail::Item::ITEM_BIND].numberInt();

				mail_info->add_goods(item_id, item_amount, item_bind);
			}
		}

		GameCommon::request_save_mail(receiver_id, mail_info);
		MSG_USER(BACK MAIL SEND TO PLAYER : %ld, receiver_id);

	}


	return 0;
}

int BackstageMailSystem::request_update_back_mail_request_flag(int request_id)
{
	JUDGE_RETURN(request_id > 0, -1);

	MongoDataMap* data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
	JUDGE_RETURN(NULL != data_map, -1);

	BackMailRequest::update_data(data_map, request_id);
	if(TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_UPDATE_BACK_MAIL_REQUEST_FLAG,
			data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}
	return 0;
}
