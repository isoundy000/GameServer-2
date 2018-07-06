/*
 * BackFlowControl.cpp
 *
 *  Created on: May 22, 2014
 *      Author: louis
 */

#include "DaemonServer.h"
#include "BackField.h"
#include "FlowControl.h"
#include "BackFlowControl.h"
#include "GameCommon.h"

#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>

BackFlowControl::BackFlowControl() {
	// TODO Auto-generated constructor stub

}

BackFlowControl::~BackFlowControl() {
	// TODO Auto-generated destructor stub
}

int BackFlowControl::request_load_flow_detail()
{
BEGIN_CATCH
	BSONObj res = this->conection().findOne(DBFlowControl::COLLECTION,
			QUERY(DBFlowControl::ID << 1 << DBFlowControl::NEED_LOAD_DATA << 1));
	if(res.isEmpty())
		return -1;

	MSG_DEBUG();

	IntSet server_set, readed_server_set;
	int current_server_index = DAEMON_SERVER->server_index();
#ifndef LOCAL_DEBUG
	{
    	int i = 0;
    	const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
    	for (GameConfig::ServerList::const_iterator iter = server_list.begin();
    			iter != server_list.end(); ++iter)
    	{
    		const ServerDetail &detail = *iter;
    		if (detail.__server_type == SERVER_GATE ||
    				detail.__server_type == SERVER_LOGIC ||
    				detail.__server_type == SERVER_MAP)
    		{
    			server_set.insert(i);
    		}
    		++i;
    	}

        BSONObjIterator iter(res.getObjectField(DBFlowControl::SERVER_INDEX_SET.c_str()));
        while (iter.more())
        {
            int index = iter.next().numberInt();
            readed_server_set.insert(index);
            server_set.erase(index);
        }

        if (readed_server_set.find(current_server_index) != readed_server_set.end())
        	return -1;
	}
#endif

	FlowControl::FlowControlDetail &detail = FLOW_INSTANCE->revert_flow_detail();
	detail.reset();
	/*
	 * 只在流水总开关有设置的情况下，才读其他流水字段开关
	 * 防止默认同步成了关闭状态
	 * */
	if (res.hasField(DBFlowControl::SERIAL_RECORD.c_str()))
	{
		detail.__serial_record = res[DBFlowControl::SERIAL_RECORD].numberInt();

		if (res.hasField(DBFlowControl::MONEY_SERIAL_RECORD.c_str()))
			detail.__money_serial_record = res[DBFlowControl::MONEY_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::ITEM_SERIAL_RECORD.c_str()))
			detail.__item_serial_record = res[DBFlowControl::ITEM_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::EQUIP_SERIAL_RECORD.c_str()))
			detail.__equip_serial_record = res[DBFlowControl::EQUIP_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MOUNT_SERIAL_RECORD.c_str()))
			detail.__mount_serial_record = res[DBFlowControl::MOUNT_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::PET_SERIAL_RECORD.c_str()))
			detail.__pet_serial_record = res[DBFlowControl::PET_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::SKILL_SERIAL_RECORD.c_str()))
			detail.__skill_serial_record = res[DBFlowControl::SKILL_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MAIL_SERIAL_RECORD.c_str()))
			detail.__mail_serial_record = res[DBFlowControl::MAIL_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MARKET_SERIAL_RECORD.c_str()))
			detail.__market_serial_record = res[DBFlowControl::MARKET_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::ACHIEVE_SERIAL_RECORD.c_str()))
			detail.__achieve_serial_record = res[DBFlowControl::ACHIEVE_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::PLAYER_LEVEL_SERIAL.c_str()))
			detail.__player_level_record = res[DBFlowControl::PLAYER_LEVEL_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::OTHER_SERIAL.c_str()))
			detail.__other_record = res[DBFlowControl::OTHER_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::ONLINE_USER_SERIAL.c_str()))
			detail.__online_user_record = res[DBFlowControl::ONLINE_USER_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::LOGIN_SERIAL.c_str()))
			detail.__login_record = res[DBFlowControl::LOGIN_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::TASK_SERIAL.c_str()))
			detail.__task_record = res[DBFlowControl::TASK_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::RANK_SERIAL.c_str()))
			detail.__rank_record = res[DBFlowControl::RANK_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::CHAT_SERIAL.c_str()))
			detail.__chat_record = res[DBFlowControl::CHAT_SERIAL].numberInt();
	}

	detail.__is_forbit_login = res[DBFlowControl::IS_FORBIT_LOGIN].numberInt();
	if (res.hasField(DBFlowControl::FORBIT_CHANNEL.c_str()))
	{
		BSONObjIterator iter(res.getObjectField(DBFlowControl::FORBIT_CHANNEL.c_str()));
		while (iter.more())
		{
			detail.__forbit_channel_set.insert(iter.next().str());
		}
	}

	if(res.hasField(DBFlowControl::FORCE_REFRSH_RANK_TPYE.c_str()))
	{
		BSONObj rank_obj = res.getObjectField(DBFlowControl::FORCE_REFRSH_RANK_TPYE.c_str());
		BSONObjIterator it(rank_obj);
		while(it.more())
		{
			detail.__force_refresh_rank_type_set.push_back(it.next().numberInt());
		}
	}

	FLOW_INSTANCE->update_version();

	server_set.erase(current_server_index);
	this->update_flow_detail_load_flag(&detail, server_set, current_server_index);

	return 0;
END_CATCH
	return -1;
}

int BackFlowControl::load_flow_control_detail(FlowControl* flow_control)
{
	JUDGE_RETURN(flow_control != NULL, -1);

	BSONObj res = CACHED_CONNECTION.findOne(DBFlowControl::COLLECTION, QUERY(DBFlowControl::ID << 1));
	JUDGE_RETURN(res.isEmpty() == false, -1);

//	int need_load_data = res[DBFlowControl::NEED_LOAD_DATA].numberInt();
//	if(need_load_data == 0)
//		return -1;

	FlowControl::FlowControlDetail& detail = flow_control->revert_flow_detail();
	detail.reset();
	/*
	 * 只在流水总开关有设置的情况下，才读其他流水字段开关
	 * 防止默认同步成了关闭状态
	 * */
	if (res.hasField(DBFlowControl::SERIAL_RECORD.c_str()))
	{
		detail.__serial_record = res[DBFlowControl::SERIAL_RECORD].numberInt();

		if (res.hasField(DBFlowControl::MONEY_SERIAL_RECORD.c_str()))
			detail.__money_serial_record = res[DBFlowControl::MONEY_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::ITEM_SERIAL_RECORD.c_str()))
			detail.__item_serial_record = res[DBFlowControl::ITEM_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::EQUIP_SERIAL_RECORD.c_str()))
			detail.__equip_serial_record = res[DBFlowControl::EQUIP_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MOUNT_SERIAL_RECORD.c_str()))
			detail.__mount_serial_record = res[DBFlowControl::MOUNT_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::PET_SERIAL_RECORD.c_str()))
			detail.__pet_serial_record = res[DBFlowControl::PET_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::SKILL_SERIAL_RECORD.c_str()))
			detail.__skill_serial_record = res[DBFlowControl::SKILL_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MAIL_SERIAL_RECORD.c_str()))
			detail.__mail_serial_record = res[DBFlowControl::MAIL_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::MARKET_SERIAL_RECORD.c_str()))
			detail.__market_serial_record = res[DBFlowControl::MARKET_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::ACHIEVE_SERIAL_RECORD.c_str()))
			detail.__achieve_serial_record = res[DBFlowControl::ACHIEVE_SERIAL_RECORD].numberInt();
		if (res.hasField(DBFlowControl::PLAYER_LEVEL_SERIAL.c_str()))
			detail.__player_level_record = res[DBFlowControl::PLAYER_LEVEL_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::OTHER_SERIAL.c_str()))
			detail.__other_record = res[DBFlowControl::OTHER_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::ONLINE_USER_SERIAL.c_str()))
			detail.__online_user_record = res[DBFlowControl::ONLINE_USER_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::LOGIN_SERIAL.c_str()))
			detail.__login_record = res[DBFlowControl::LOGIN_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::TASK_SERIAL.c_str()))
			detail.__task_record = res[DBFlowControl::TASK_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::RANK_SERIAL.c_str()))
			detail.__rank_record = res[DBFlowControl::RANK_SERIAL].numberInt();
		if (res.hasField(DBFlowControl::CHAT_SERIAL.c_str()))
			detail.__chat_record = res[DBFlowControl::CHAT_SERIAL].numberInt();
	}

	detail.__is_forbit_login = res[DBFlowControl::IS_FORBIT_LOGIN].numberInt();
	if (res.hasField(DBFlowControl::FORBIT_CHANNEL.c_str()))
	{
		BSONObjIterator iter(res.getObjectField(DBFlowControl::FORBIT_CHANNEL.c_str()));
		while (iter.more())
		{
			detail.__forbit_channel_set.insert(iter.next().str());
		}
	}

	if(res.hasField(DBFlowControl::FORCE_REFRSH_RANK_TPYE.c_str()))
	{
		BSONObj rank_obj = res.getObjectField(DBFlowControl::FORCE_REFRSH_RANK_TPYE.c_str());
		BSONObjIterator it(rank_obj);
		while(it.more())
		{
			detail.__force_refresh_rank_type_set.push_back(it.next().numberInt());
		}
	}
	FLOW_INSTANCE->update_version();

	{
		//save db : update value of need_load_data

		BSONObjBuilder build;
		build << DBFlowControl::NEED_LOAD_DATA << 0
				<< DBFlowControl::FORCE_REFRSH_RANK_TPYE << BSONObj();

		CACHED_CONNECTION.update(DBFlowControl::COLLECTION,
				BSON(DBFlowControl::ID << 1),
				BSON("$set" << build.obj()), true);
	}

	return 0;
}

int BackFlowControl::update_flow_detail_load_flag(FlowControl::FlowControlDetail* detail, std::set<int> &server_set, const int current_index)
{
	if (detail == NULL)
		return -1;

	//save db : update value of need_load_data
	BSONObjBuilder build;

	if (server_set.size() > 0)
	{
		this->conection().update(DBFlowControl::COLLECTION,
				BSON(DBFlowControl::ID << 1 << DBFlowControl::NEED_LOAD_DATA << 1),
				BSON("$push" << BSON(DBFlowControl::SERVER_INDEX_SET << current_index)), false);
	}
	else
	{
		build << DBFlowControl::NEED_LOAD_DATA << 0
				<< DBFlowControl::FORCE_REFRSH_RANK_TPYE << BSONObj()
				<< DBFlowControl::SERVER_INDEX_SET << (std::vector<int>());
		this->conection().update(DBFlowControl::COLLECTION,
				BSON(DBFlowControl::ID << 1),
				BSON("$set" << build.obj()), true);
	}
	return 0;
}

void BackFlowControl::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBFlowControl::COLLECTION, BSON(DBFlowControl::ID << 1));
END_CATCH
}
