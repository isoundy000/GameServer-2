/*
 * BackMail.cpp
 *
 *  Created on: Jun 30, 2014
 *      Author: louis
 */

#include "PubStruct.h"
#include "BackField.h"
#include "GameField.h"
#include "BackMailRequest.h"
#include "GameCommon.h"
//#include "GameFont.h"
#include "DBCommon.h"

#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

BackMailRequest::BackMailRequest() {
	// TODO Auto-generated constructor stub

}

BackMailRequest::~BackMailRequest() {
	// TODO Auto-generated destructor stub
}

int BackMailRequest::update_data(MongoDataMap* data_map, const int mail_request_id)
{
	data_map->push_update(DBBackMail::COLLECTION, BSON(DBBackMail::ID << mail_request_id),
			BSON(DBBackMail::READ << 1), true);
	return 0;
}

int BackMailRequest::load_mail_request(MongoDataMap* data_map)
{
	data_map->push_multithread_query(DBBackMail::COLLECTION, BSON(DBBackMail::READ << 0));
	return 0;
}

int BackMailRequest::process_back_mail(void)
{
BEGIN_CATCH
    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackMail::COLLECTION,
    		QUERY(DBBackMail::READ << 0));

    int i = 0;
	BSONVec bson_set;
    while (cursor->more() && i < 100)
    {
        bson_set.push_back(cursor->next());
        ++i;
    }

    IntMap read_map;
    for (BSONVec::iterator vc_iter = bson_set.begin(); vc_iter != bson_set.end(); ++vc_iter)
    {
        BSONObj res = *vc_iter;
        JUDGE_CONTINUE(res.isEmpty() == false);

	    int request_id = res[DBBackMail::ID].numberInt();
	    read_map[request_id] = true;

        LongVec receiver_set;
        DBCommon::bson_to_long_vec(receiver_set, res.getObjectField(DBBackMail::RECEIVER_SET.c_str()));
        JUDGE_CONTINUE(receiver_set.empty() == false);

	    std::string title   = res[DBBackMail::TITLE].str();
	    std::string content = res[DBBackMail::CONTENT].str();

        MailInformation* mail_info = GameCommon::create_sys_mail(title, content, 0);
	    mail_info->sender_name_ = res[DBBackMail::SENDER].str();

        // item
	    if(res.hasField(DBBackMail::ITEM.c_str()))
	    {
	    	BSONObjIterator it(res.getObjectField(DBBackMail::ITEM.c_str()));
	    	while(it.more())
	    	{
	    		BSONObj item = it.next().embeddedObject();

	    		int item_id = item[DBBackMail::Item::ITEM_ID].numberInt();
	    		int item_amount = item[DBBackMail::Item::ITEM_AMOUNT].numberInt();
	    		int item_bind = item[DBBackMail::Item::ITEM_BIND].numberInt();
	    		JUDGE_CONTINUE(item_amount > 0);

	    		mail_info->add_goods(item_id, item_amount, item_bind);
	    	}
	    }

	    //make up mail info
	    for (LongVec::iterator iter = receiver_set.begin(); iter != receiver_set.end(); ++iter)
	    {
	    	Int64 receiver_id = *iter;
            mail_info->receiver_id_ = receiver_id;

            BSONObj mail_obj = GameCommon::mail_info_to_bson(mail_info);
            mail_info->mail_index_ = CACHED_INSTANCE->update_global_key(Global::MAIL);

            CACHED_CONNECTION.update(MailOffline::COLLECTION, QUERY(MailOffline::MAIL_ID << mail_info->mail_index_),
                BSON("$set" << mail_obj), true);
            MSG_USER("BACK MAIL SEND TO PLAYER : %ld", receiver_id);
        }

	    read_map.erase(request_id);
        CACHED_CONNECTION.update(DBBackMail::COLLECTION, QUERY(DBBackMail::ID << request_id),
        		BSON("$set" << BSON(DBBackMail::READ << 1)));
    }

    for (IntMap::iterator iter = read_map.begin(); iter != read_map.end(); ++iter)
    {
        CACHED_CONNECTION.update(DBBackMail::COLLECTION, QUERY(DBBackMail::ID << iter->first),
        		BSON("$set" << BSON(DBBackMail::READ << 1)));
    }

    if (bson_set.empty() == false)
    {
    	//两天前的邮件都不要
    	Int64 remove_tick = ::time(NULL) - Time_Value::DAY * 2;
    	CACHED_CONNECTION.remove(DBBackMail::COLLECTION,
    			QUERY(DBBackMail::READ << 1 << DBBackMail::TIME << BSON("$lt" << remove_tick)));
    	CACHED_CONNECTION.remove(DBBackMail::COLLECTION,
    			QUERY(DBBackMail::READ << 0 << DBBackMail::TIME << BSON("$lt" << remove_tick)));
    }

    return 0;
END_CACHE_CATCH
    return -1;
}

int BackMailRequest::load_game_notice(DBShopMode* shop_mode)
{
	*shop_mode->output_argv_.bson_obj_ = this->conection().findOne(
			DBBackNotice::COLLECTION, QUERY(DBBackNotice::ID << 0)).copy();
	return 0;
}

int BackMailRequest::load_chat_limit(DBShopMode* shop_mode)
{
	auto_ptr<DBClientCursor> cursor = this->conection().query(DBChatLimit::COLLECTION, mongo::Query());
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		Int64 update_tick = res[DBChatLimit::UPDATE_TICK].numberLong();
		JUDGE_CONTINUE(update_tick != 0);

		shop_mode->output_argv_.bson_vec_->push_back(res.copy());
	}

	return 0;
}

int BackMailRequest::load_word_check(DBShopMode* shop_mode)
{
	auto_ptr<DBClientCursor> cursor = this->conection().query(DBWordCheck::COLLECTION,mongo::Query());

	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		Int64 update_tick = res[DBWordCheck::TIME].numberLong();
		JUDGE_CONTINUE(update_tick != 0);
		JUDGE_CONTINUE(shop_mode->input_argv_.type_int64_ != update_tick);

		shop_mode->output_argv_.bson_vec_->push_back(res.copy());
	}

	return 0;
}

int BackMailRequest::load_vip_chat_limit(DBShopMode* shop_mode)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBVipChat::COLLECTION,
			QUERY(DBVipChat::ID << 0));
	if (res.isEmpty() == true)
		return 0;

	Int64 update_tick = res[DBVipChat::UPDATE_TICK].numberLong();
	JUDGE_RETURN(update_tick != 0, 0);
	JUDGE_RETURN(shop_mode->input_argv_.type_int64_ != update_tick, 0);

	*shop_mode->output_argv_.bson_obj_ = res.copy();
	return 0;
}

int BackMailRequest::load_back_activity(DBShopMode* shop_mode)
{
//	int delay = 180;	//延迟时间（秒）
//	IntMap remove_map;
//	auto_ptr<DBClientCursor> cursor = this->conection().query(DBBackActivity::COLLECTION);
//
//	while (cursor->more())
//	{
//		BSONObj res = cursor->next();
//		JUDGE_CONTINUE(res.isEmpty() == false);
//
//		int act_index = res[DBBackActivity::ACT_INDEX].numberInt();
//		if (res[DBBackActivity::OPEN_FLAG].numberInt() == 0)
//		{
//			//close
//			remove_map[act_index] = true;
//			continue;
//		}
//
//		Int64 now_tick = ::time(NULL);
//		if (res[DBBackActivity::STOP_TICK].numberLong() <= now_tick)
//		{
//			// passed
//			remove_map[act_index] = true;
//			continue;
//		}
//		//if (res[DBBackActivity::START_TICK].numberLong() > now_tick  )
//		if (res[DBBackActivity::START_TICK].numberLong() - now_tick > BackSetActDetail::BACK_LOAD_AHEAD )	//提前进行加载
//		{
//			// future
//			continue;
//		}
//
//		shop_mode->output_argv_.bson_vec_->push_back(res.copy());
//	}
//
//	for (IntMap::iterator iter = remove_map.begin(); iter != remove_map.end(); ++iter)
//	{
//		this->conection().remove(DBBackActivity::COLLECTION,
//				BSON(DBBackActivity::ACT_INDEX << iter->first));
//	}

	return 0;
}

void BackMailRequest::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBBackMail::COLLECTION, BSON(DBBackMail::ID << 1), true);
	this->conection().ensureIndex(DBBackNotice::COLLECTION, BSON(DBBackNotice::ID << 1), true);
	this->conection().ensureIndex(DBWordCheck::COLLECTION, BSON(DBWordCheck::ID << 1), true);
	this->conection().ensureIndex(DBVipChat::COLLECTION, BSON(DBVipChat::ID << 1), true);
END_CATCH
}
