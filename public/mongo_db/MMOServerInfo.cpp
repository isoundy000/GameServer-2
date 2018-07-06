/*
 * MMOServerInfo.cpp
 *
 * Created on: 2016-02-03 20:53
 *     Author: lyz
 */

#include "MMOServerInfo.h"
#include "BackField.h"
#include "MongoConnector.h"
#include "MongoDataMap.h"
#include "PoolMonitor.h"

#include "GameCommon.h"
#include "DBCommon.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>


int MMOServerInfo::update_server_info(ServerInfo *server_info, int index)
{
	return MMOServerInfo::update_server_info(server_info, index, CACHED_CONNECTION);
}

int MMOServerInfo::update_server_info(ServerInfo *server_info, int index,
    		mongo::DBClientConnection& conn)
{
    if (index == -1)
    {
    	index = server_info->__server_id;
    }

    BSONObjBuilder builder;
	builder << DBServerInfo::INDEX << index
		<< DBServerInfo::SERVER_ID << server_info->__server_id
		<< DBServerInfo::IS_IN_USE << server_info->__is_in_use
		<< DBServerInfo::SERVER_FLAG << server_info->__server_flag
		<< DBServerInfo::SERVER_PREV << server_info->__server_prev
		<< DBServerInfo::SERVER_NAME << server_info->__back_server_name
		<< DBServerInfo::FRONT_SERVER_NAME << server_info->__server_name
		<< DBServerInfo::OPEN_SERVER << Int64(server_info->__open_server.sec())
		<< DBServerInfo::COMBINE_SERVER_SET << server_info->__combine_set
		<< DBServerInfo::COMBINE_TO_SERVER_ID  << server_info->__combine_to_server_id;

	conn.update(DBServerInfo::COLLECTION, QUERY(DBServerInfo::INDEX << index),
            BSON("$set" << builder.obj()), true);

    return 0;
}

int MMOServerInfo::load_server_info(ServerInfo *server_info)
{
	//index == 0 表示本服的服务器信息
	BSONObj res = CACHED_CONNECTION.findOne(DBServerInfo::COLLECTION,
			QUERY(DBServerInfo::INDEX << 0));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	DBCommon::bson_to_int_vec(server_info->__combine_set,
			res.getObjectField(DBServerInfo::COMBINE_SERVER_SET.c_str()));

	return 0;
}

int MMOServerInfo::remove_server_info(int server_id)
{
    CACHED_CONNECTION.remove(DBServerInfo::COLLECTION, QUERY(DBServerInfo::SERVER_ID << server_id));
    return 0;
}

void MMOServerInfo::ensure_all_index(void)
{
    this->conection().ensureIndex(DBServerInfo::COLLECTION, BSON(DBServerInfo::INDEX << 1), true);
	this->conection().ensureIndex(DBServerInfo::COLLECTION, BSON(DBServerInfo::SERVER_ID << 1), false);
}


int MMOCombineServer::load_combine_server(DBShopMode* shop_mode)
{
	BSONObj res = this->conection().findOne(DBCombineServer::COLLECTION,
			QUERY(DBCombineServer::ID << 0));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	Int64 update_tick = res[DBCombineServer::UPDATE_TICK].numberLong();
	JUDGE_RETURN(shop_mode->input_argv_.type_int64_ != update_tick, -1);

	shop_mode->sub_value_ = 1;
	shop_mode->output_argv_.type_int64_ = update_tick;

	shop_mode->output_argv_.type_int_ = res[DBCombineServer::PORT].numberInt();
	shop_mode->output_argv_.str_vec_.push_back(res[DBCombineServer::SERVER_FLAG].str());
	shop_mode->output_argv_.str_vec_.push_back(res[DBCombineServer::IP].str());
	shop_mode->output_argv_.str_vec_.push_back(res[DBCombineServer::SPECIAL_FILE].str());
	return 0;
}

void MMOCombineServer::ensure_all_index(void)
{
	this->conection().ensureIndex(DBCombineServer::COLLECTION, BSON(DBCombineServer::ID << 1), true);
}



