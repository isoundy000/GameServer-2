/*
 * MMOChatLeague.cpp
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#include "MongoConnector.h"
#include "MMOChatLeague.h"
#include "LeagueChannel.h"
#include "GameField.h"
#include "ChannelAgency.h"
#include "Block_Buffer.h"

#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMOChatLeague::MMOChatLeague()
{

}

MMOChatLeague::~MMOChatLeague()
{

}

void MMOChatLeague::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBChatLeague::COLLECTION,
			BSON(DBChatLeague::LEAGUE_ID<<1),true);
END_CATCH
}

int MMOChatLeague::load_league_record(ChannelAgency *agency)
{
BEGIN_CATCH
	auto_ptr<DBClientCursor> cursor= this->conection().query(DBChatLeague::COLLECTION);
	while(cursor->more())
	{
		BSONObj league = cursor->next();
		int64_t league_id = league[DBChatLeague::LEAGUE_ID].numberLong();
		LeagueChannel *channel = agency->create_league_channel(league_id);
		if (league.hasElement(DBChatLeague::CHAT_LIST.c_str()))
		{
			BSONObj list = league.getObjectField(DBChatLeague::CHAT_LIST.c_str());
			BSONObjIterator iter(list);
			while (iter.more())
			{
				BSONObj record = iter.next().embeddedObject();
				ChatRecord *r = channel->pop_record();
				r->__src_role_id = record[DBChatRecord::SRC_ROLE_ID].numberLong();
				r->__dst_role_id = record[DBChatRecord::DST_ROLE_ID].numberLong();
				r->__time = record[DBChatRecord::TIME].numberLong();
				r->__type = record[DBChatRecord::TYPE].numberInt();
				r->__voice_id = record[DBChatRecord::VOICE_ID].numberLong();
				r->__voice_len = record[DBChatRecord::VOICE_LEN].numberInt();

				int len = 0;
				const char *bin_data = record[DBChatRecord::CONTENT].binData(len);
				r->__buffer->copy(bin_data, len);
				channel->push_history(r);
			}
		}
		channel->start();
	}
	return 0;
END_CATCH
    MSG_USER("ERROR load league ChannelAgency record");
    return -1;
}

int MMOChatLeague::load_league_record(LeagueChannel *channel)
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBChatLeague::COLLECTION,
			QUERY(DBChatLeague::LEAGUE_ID << (long long int)channel->channel_id()));
	if(res.isEmpty())
	{
		return 0;
	}
	if(res.hasElement(DBChatLeague::CHAT_LIST.c_str()))
	{
		BSONObj list = res.getObjectField(DBChatLeague::CHAT_LIST.c_str());
		BSONObjIterator iter(list);
		while(iter.more())
		{
			BSONObj record = iter.next().embeddedObject();
			ChatRecord *r = channel->pop_record();
			r->__src_role_id = record[DBChatRecord::SRC_ROLE_ID].numberLong();
			r->__dst_role_id = record[DBChatRecord::DST_ROLE_ID].numberLong();
			r->__time = record[DBChatRecord::TIME].numberLong();
			r->__type = record[DBChatRecord::TYPE].numberInt();
			r->__voice_id = record[DBChatRecord::VOICE_ID].numberLong();
			r->__voice_len = record[DBChatRecord::VOICE_LEN].numberInt();

			int len = 0;
			const char *bin_data = record[DBChatRecord::CONTENT].binData(len);
			std::string str = std::string(bin_data);
			//(*r->__buffer) << bin_data;
			r->__buffer->copy(bin_data,len);
			channel->push_history(r);
		}
	}
	return 0;
END_CATCH
    MSG_USER("ERROR load league LeagueChannel record");
    return -1;
}

int MMOChatLeague::save_league_record(LeagueChannel *channel)
{
BEGIN_CATCH
	std::vector<BSONObj> record_list;
	while(channel->history_for_save_.size()>0)
	{
		ChatRecord *tmp = channel->history_for_save_.back();

//		record_list.push_back(BSON(DBChatRecord::SRC_ROLE_ID<< (long long int)tmp->__src_role_id
//				<< DBChatRecord::DST_ROLE_ID<<(long long int)tmp->__dst_role_id
//				<< DBChatRecord::TIME << tmp->__time
//				<< DBChatRecord::CONTENT << tmp->__buffer));

		BSONObjBuilder builder;
		builder << DBChatRecord::SRC_ROLE_ID << (long long int)tmp->__src_role_id
				<< DBChatRecord::DST_ROLE_ID <<(long long int)tmp->__dst_role_id
				<< DBChatRecord::TIME << tmp->__time
				<< DBChatRecord::TYPE << tmp->__type
				<< DBChatRecord::VOICE_ID << (long long int)(tmp->__voice_id + BASE_OFFSET_HISTORY_VOID_ID)
				<< DBChatRecord::VOICE_LEN << tmp->__voice_len;
		builder.appendBinData(DBChatRecord::CONTENT,tmp->__buffer->readable_bytes(),
				mongo::BinDataGeneral,tmp->__buffer->get_read_ptr());

		record_list.push_back(builder.obj());

		channel->push_record(tmp);
		channel->history_for_save_.pop_back();
	}

	BSONObjBuilder builder;
	builder << DBChatLeague::LEAGUE_ID << (long long int)channel->channel_id()
			<< DBChatLeague::CHAT_LIST << record_list;
	this->conection().update(DBChatLeague::COLLECTION,
			QUERY(DBChatLeague::LEAGUE_ID<< (long long int)channel->channel_id()),
			BSON("$set"<< builder.obj()),true);
	return 0;
END_CATCH
    MSG_USER("ERROR save league LeagueChannel record");
    return -1;
}
