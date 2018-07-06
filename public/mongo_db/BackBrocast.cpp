/*
 * BackBrocast.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: louis
 */

#include "BackField.h"
#include "GameField.h"
#include "BackBrocast.h"
#include "LogicStruct.h"
#include "BackstageBrocastControl.h"

#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "LogicPlayer.h"
#include "MongoDataMap.h"
#include "LogicMonitor.h"
#include "BackActivityTick.h"
#include "MongoData.h"

#include "MongoConnector.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
#include "GameCommon.h"
#include "DBCommon.h"


BackRecharge::BackRecharge() {
	// TODO Auto-generated constructor stub

}

BackRecharge::~BackRecharge() {
	// TODO Auto-generated destructor stub
}

int BackRecharge::update_data(MongoDataMap* data_map, const int order_id)
{
	data_map->push_update(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::ID << order_id),
			BSON(DBBackRecharge::FLAG << 1), true);

	return 0;
}

int BackRecharge::recharge_test(MongoDataMap* data_map, int64_t role_id, const string& account, int money)
{
	BSONObjBuilder builder;
	builder << DBBackRecharge::RECHARGE_MONEY << money
			<< DBBackRecharge::RECHANGE_GOLD << (money * 10)
			<< DBBackRecharge::ACCOUNT << account
			<< DBBackRecharge::FLAG << 0
			<< DBBackRecharge::RECHARGE_TICK << (Int64)::time(0)
			<< DBBackRecharge::ROLE_ID << (Int64) role_id
			<< DBBackRecharge::ORDER_NUM << string("TEST !!")
			<< DBBackRecharge::RECHARGE_CHANNEL << string("4399");

	data_map->push_update(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::ID << (Int64)::rand()), builder.obj(), true);
	return 0;
}

void BackRecharge::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::ID << 1), true);
	this->conection().ensureIndex(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::ROLE_ID << -1 << DBBackRecharge::FLAG << 1));
	this->conection().ensureIndex(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::ACCOUNT << 1));
	this->conection().ensureIndex(DBBackRecharge::COLLECTION, BSON(DBBackRecharge::RECHARGE_RANK_TAG << 1 << DBBackRecharge::RECHARGE_TICK << -1));
END_CATCH
}

BackBrocast::BackBrocast() {
	// TODO Auto-generated constructor stub

}

BackBrocast::~BackBrocast() {
	// TODO Auto-generated destructor stub
}

int BackBrocast::updata_data(BackstageBrocastRecord* record, MongoDataMap* data_map)
{
	BSONObjBuilder build;
	build << (DBBackBroDetail::BRO_RECORD + "." + DBBackBroDetail::Bro_record::BRO_TICK) << record->__brocast_tick
			<< (DBBackBroDetail::BRO_RECORD + "." + DBBackBroDetail::Bro_record::BRO_TIMES) << record->__brocast_times;
	data_map->push_update(DBBackBroDetail::COLLECTION, BSON(DBBackBroDetail::ID << record->__index),
			build.obj(), true);
	return 0;
}

int BackBrocast::request_load_data(void)
{
	BackstageBrocastControl* control = BBC_INSTANCE;
	BackstageBrocastControl::BrocastRecordMap& record_map = control->brocast_record_map();

	auto_ptr<DBClientCursor> cursor = this->conection().query(DBBackBroDetail::COLLECTION);
	while(cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		int data_change = res[DBBackBroDetail::DATA_CHANGE].numberInt();
		JUDGE_CONTINUE(data_change != 0);

		int id = res[DBBackBroDetail::ID].numberInt();
		int op_type = res[DBBackBroDetail::DB_OP_TYPE].numberInt();
		if (op_type == 1) //remove
		{
			control->remove_set().insert(id);
			continue;
		}

		BackstageBrocastRecord* record = NULL;
		if (record_map.count(id) <= 0)
		{
			record = control->pop_record();
			JUDGE_CONTINUE(record != NULL);

			record->__index = id;
			BBRecord_PACKAGE->bind_object(id, record);

			control->modify_set().insert(id);
		}
		else
		{
			record = BBRecord_PACKAGE->find_object(id);
			JUDGE_CONTINUE(record != NULL);
		}

		BSONObj obj = res.getObjectField(DBBackBroDetail::BRO_RECORD.c_str());
		record->__brocast_type = obj[DBBackBroDetail::Bro_record::BRO_TYPE].numberInt();
		record->__brocast_times = obj[DBBackBroDetail::Bro_record::BRO_TIMES].numberInt();
		record->__brocast_tick = obj[DBBackBroDetail::Bro_record::BRO_TICK].numberLong();
		record->__max_repeat_times = obj[DBBackBroDetail::Bro_record::REPEAT_TIMES].numberInt();
		record->__interval_sec = obj[DBBackBroDetail::Bro_record::INTERVAL_SEC].numberInt();
		record->__content = obj[DBBackBroDetail::Bro_record::CONTENT].str();

		this->conection().update(DBBackBroDetail::COLLECTION, BSON(DBBackBroDetail::ID << id),
			BSON("$set" << BSON(DBBackBroDetail::DATA_CHANGE << 0)), true);

	}
	return 0;
}

int BackBrocast::update_flag(const int record_id, BSONObj& res)
{
	BSONObjBuilder build;
	build << DBBackBroDetail::DATA_CHANGE << 0
			<< DBBackBroDetail::DB_OP_TYPE << 0
	    	<< DBBackBroDetail::BRO_RECORD << res;
	this->conection().update(DBBackBroDetail::COLLECTION, BSON(DBBackBroDetail::ID << record_id),
					BSON("$set" << build.obj()), true);
	return 0;
}

void BackBrocast::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(DBBackBroDetail::COLLECTION, BSON(DBBackBroDetail::ID << 1), true);
END_CATCH
}


int BackDraw::load_activity_tick(MongoDataMap *data_map)
{
    MongoData *data = 0;
    if (data_map->find_data(DBBackDraw::COLLECTION, data) != 0)
        return 0;

    data->set_multithread_cursor(this->conection().query(DBBackDraw::COLLECTION,
                QUERY(DBBackDraw::FLAG << 1)));
    this->conection().update(DBBackDraw::COLLECTION, QUERY(DBBackDraw::FLAG << 1),
    		BSON("$set" << BSON(DBBackDraw::FLAG << 0)), false, true);

    return 0;
}

int BackDraw::load_activity_tick_at_init(BackActivityTick *back_act_tick)
{
BEGIN_CATCH
	BackActivityTick::ActivityTickMap &tick_map = back_act_tick->next_activity_tick_map();
    tick_map.clear();

	BackActivityTick::ActivityTickInfo info;
    auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBBackDraw::COLLECTION, mongo::Query());
    while (cursor->more())
    {
        BSONObj obj = cursor->next();
        info.reset();
        info.__begin_tick.sec(obj[DBBackDraw::S_TICK].numberInt());
        info.__end_tick.sec(obj[DBBackDraw::E_TICK].numberInt());
        int activity_id = obj[DBBackDraw::ACTIVITY_ID].numberInt();
        tick_map[activity_id] = info;

        MSG_USER("start activity tick %d begin[%d] end[%d]", activity_id, info.__begin_tick.sec(), info.__end_tick.sec());
    }
    back_act_tick->update_version();
    return 0;
END_CACHE_CATCH
    return -1;
}

void BackDraw::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBBackDraw::COLLECTION, BSON(DBBackDraw::ACTIVITY_ID << 1), true);
    this->conection().ensureIndex(DBBackDraw::COLLECTION, BSON(DBBackDraw::FLAG << 1), false);
END_CATCH
}

BackSerial::~BackSerial(void)
{ /*NULL*/ }

int BackSerial::load_map_serial(MapPlayerEx *player)
{
    BSONObj fields = BSON(DBBackSerial::EXP_SERIAL << 1);
    BSONObj res = this->conection().findOne(DBBackSerial::COLLECTION,
    		QUERY(DBBackSerial::ID << player->role_id()), &fields);

    if (res.isEmpty() || res.hasField(DBBackSerial::EXP_SERIAL.c_str()) == false)
        return 0;

    MapSerial &serial = player->serial_record();
    BSONObjIterator iter(res.getObjectField(DBBackSerial::EXP_SERIAL.c_str()));
    while (iter.more())
    {
        BSONObj obj = iter.next().embeddedObject();

        int type = obj[DBBackSerial::ExpSerial::SERIAL].numberInt();
        SerialInfo &serial_info = serial.exp_serial(type);
        serial_info.__fresh_tick = obj[DBBackSerial::ExpSerial::FRESH_TICK].numberLong();
        serial_info.__value = obj[DBBackSerial::ExpSerial::VALUE].numberLong();
    }
    return 0;
}

int BackSerial::load_map_logic_serial(MapLogicPlayer *player)
{
    BSONObj fields = BSON(DBBackSerial::ITEM_SERIAL << 1 << DBBackSerial::MONEY_SERIAL << 1);
    BSONObj res = this->conection().findOne(DBBackSerial::COLLECTION,
    		QUERY(DBBackSerial::ID << player->role_id()), &fields);
    JUDGE_RETURN(res.isEmpty() == false, 0);

    MapLogicSerial &serial = player->serial_record();
    if (res.hasField(DBBackSerial::ITEM_SERIAL.c_str()) == true)
    {
        BSONObjIterator iter(res.getObjectField(DBBackSerial::ITEM_SERIAL.c_str()));
        while (iter.more())
        {
            BSONObj obj = iter.next().embeddedObject();

            int type = obj[DBBackSerial::ItemSerial::SERIAL].numberInt(),
                item_id = obj[DBBackSerial::ItemSerial::ITEM_ID].numberInt();
            SerialInfo &serial_info = serial.item_serial(type, item_id, 0);
            serial_info.__fresh_tick = obj[DBBackSerial::ItemSerial::FRESH_TICK].numberLong();
            serial_info.__value = obj[DBBackSerial::ItemSerial::VALUE].numberLong();
        }
    }

    if (res.hasField(DBBackSerial::MONEY_SERIAL.c_str()) == true)
    {
        BSONObjIterator iter(res.getObjectField(DBBackSerial::MONEY_SERIAL.c_str()));
        while (iter.more())
        {
            BSONObj obj = iter.next().embeddedObject();

            int type = obj[DBBackSerial::MoneySerial::SERIAL].numberInt();
            SerialInfo &serial_info = serial.money_serial(type);
            serial_info.__fresh_tick = obj[DBBackSerial::MoneySerial::FRESH_TICK].numberLong();
            serial_info.__gold = obj[DBBackSerial::MoneySerial::GOLD].numberLong();
            serial_info.__copper = obj[DBBackSerial::MoneySerial::COPPER].numberLong();
            serial_info.__bind_gold = obj[DBBackSerial::MoneySerial::BIND_GOLD].numberLong();
            serial_info.__bind_copper = obj[DBBackSerial::MoneySerial::BIND_COPPER].numberLong();
        }
    }
    return 0;
}

int BackSerial::update_data(MapPlayerEx *player, MongoDataMap *data_map)
{
    MapSerial::SerialMap &exp_serial_map = player->serial_record().exp_serial_map();
    std::vector<BSONObj> exp_vc;
    {
        for (MapSerial::SerialMap::iterator iter = exp_serial_map.begin();
                iter != exp_serial_map.end(); ++iter)
        {
            SerialInfo &info = iter->second;
            exp_vc.push_back(BSON(DBBackSerial::ExpSerial::SERIAL << iter->first
                        << DBBackSerial::ExpSerial::FRESH_TICK << (long long int)info.__fresh_tick
                        << DBBackSerial::ExpSerial::VALUE << (long long int)info.__value));
        }
    }

    BSONObjBuilder builder;
    builder << DBBackSerial::EXP_SERIAL << exp_vc;

    data_map->push_update(DBBackSerial::COLLECTION,	BSON(DBBackSerial::ID << player->role_id()),
    		builder.obj(), true);

    return 0;
}

int BackSerial::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
    MapLogicSerial::ItemSerialMap &item_serial_map = player->serial_record().item_serial_map();
    MapLogicSerial::SerialMap &money_serial_map = player->serial_record().money_serial_map();
    std::vector<BSONObj> item_vc, money_vc;
    {
        for (MapLogicSerial::ItemSerialMap::iterator iter = item_serial_map.begin();
                iter != item_serial_map.end(); ++iter)
        {
            int type = iter->first;
            MapLogicSerial::SerialMap &serial_map = iter->second;
            for (MapLogicSerial::SerialMap::iterator serial_iter = serial_map.begin();
                    serial_iter != serial_map.end(); ++serial_iter)
            {
                int item_id = serial_iter->first;
                SerialInfo &info = serial_iter->second;
                item_vc.push_back(BSON(DBBackSerial::ItemSerial::SERIAL << type
                            << DBBackSerial::ItemSerial::ITEM_ID << item_id
                            << DBBackSerial::ItemSerial::FRESH_TICK << (long long int)info.__fresh_tick
                            << DBBackSerial::ItemSerial::VALUE << (long long int)info.__value));
            }
        }
    }
    {
        for (MapLogicSerial::SerialMap::iterator iter = money_serial_map.begin();
                iter != money_serial_map.end(); ++iter)
        {
            int type = iter->first;
            SerialInfo &info = iter->second;
            money_vc.push_back(BSON(DBBackSerial::MoneySerial::SERIAL << type
                        << DBBackSerial::MoneySerial::FRESH_TICK << (long long int)info.__fresh_tick
                        << DBBackSerial::MoneySerial::GOLD << (long long int)info.__gold
                        << DBBackSerial::MoneySerial::COPPER << (long long int)info.__copper
                        << DBBackSerial::MoneySerial::BIND_GOLD << (long long int)info.__bind_gold
                        << DBBackSerial::MoneySerial::BIND_COPPER << (long long int)info.__bind_copper));
        }
    }

    BSONObjBuilder builder;
    builder << DBBackSerial::ITEM_SERIAL << item_vc
        << DBBackSerial::MONEY_SERIAL << money_vc;

    data_map->push_update(DBBackSerial::COLLECTION, BSON(DBBackSerial::ID << player->role_id()), builder.obj(), true);

    return 0;
}

void BackSerial::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBBackSerial::COLLECTION, BSON(DBBackSerial::ID << 1), true);
END_CATCH
}


BackCustomerSVC::BackCustomerSVC() {
	// TODO Auto-generated constructor stub

}

BackCustomerSVC::~BackCustomerSVC() {
	// TODO Auto-generated destructor stub
}

int BackCustomerSVC::load_customer_way_when_init(LogicPlayer* player)
{
	BSONObj res = this->conection().findOne(DBBackContactWay::COLLECTION,
						QUERY(DBBackContactWay::MARKET_CODE << player->role_detail().__market_code ));
	if(res.isEmpty())
	{
	    res = this->conection().findOne(DBBackContactWay::COLLECTION,
	    				QUERY(DBBackContactWay::MARKET_CODE << -1));
	}
	if(res.isEmpty())
	{
		return -1;
	}
	if(res.hasField(DBBackContactWay::CONTACT_WAY.c_str()))
	{
		std::string content = res[DBBackContactWay::CONTACT_WAY].String();
		player->set_contact_way(content);
	}
	return 0;
}

int BackCustomerSVC::load_player_customer_service_detail_when_init(LogicPlayer* player)
{
	BSONObj res = this->conection().findOne(DBCustomerSVCDetail::COLLECTION,
			BSON(DBCustomerSVCDetail::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	CustomerServiceDetail& detail = player->customer_service_detial();
	detail.__last_summit_type = res[DBCustomerSVCDetail::LAST_SUMMIT_TYPE].numberInt();
	detail.__content = res[DBCustomerSVCDetail::CONTENT].String();
	detail.__title = res[DBCustomerSVCDetail::TITLE].String();

	typedef DBCustomerSVCDetail::RewardInfo  DB_reward;
	detail.__opinion_reward.clear();
	if(res.hasField(DBCustomerSVCDetail::OPINION_REWARD.c_str()))
	{
		BSONObj obj = res.getObjectField(DBCustomerSVCDetail::OPINION_REWARD.c_str());
		BSONObjIterator json_iter(obj);

		while(json_iter.more())
		{
			BSONObj bson = json_iter.next().Obj();
			int opinion_index = bson[DB_reward::OPINION_INDEX].numberInt();
			int reward_status = bson[DB_reward::REWARD_STATUS].numberInt();

			detail.__opinion_reward.insert(IntMap::value_type(opinion_index, reward_status));
		}
	}

	//load customer record
	auto_ptr<DBClientCursor> cursor = this->conection().query(BackCustomerSVCRecord::COLLECTION,
			QUERY(BackCustomerSVCRecord::SENDER_ID << player->role_id()));
	while(cursor->more())
	{
		BSONObj record_obj = cursor->next();
		int remove_flag = record_obj[BackCustomerSVCRecord::REMOVE_FLAG].numberInt();
		JUDGE_CONTINUE(remove_flag == 0);

		CustomerServiceRecord* record = POOL_MONITOR->customer_service_record_pool()->pop();
		JUDGE_CONTINUE(NULL != record);

		DBCommon::bson_to_customer_service_record(record, record_obj);

		detail.__customer_record_map.insert(CustomerRecordMap::value_type(record->__record_id, record));
		detail.__customer_record_vec.push_back(record);
	}

	return 0;
}

int BackCustomerSVC::save_global_customer_service_index(int64_t index)
{
	BSONObjBuilder build;
	build << Global::GID << (Int64)index;

	GameCommon::request_save_mmo_begin(Global::COLLECTION,
			BSON(Global::KEY << Global::CUSTOMER_SERVICE),
			BSON("$set" << build.obj()));
	return 0;
}

int BackCustomerSVC::request_load_customer_service_record(LogicPlayer* player, MongoDataMap* data_map)
{
	data_map->push_multithread_query(BackCustomerSVCRecord::COLLECTION,
			BSON(BackCustomerSVCRecord::SENDER_ID << player->role_id()
					<< BackCustomerSVCRecord::NEED_LOAD_DATA << 1));
	return 0;
}

int BackCustomerSVC::request_remove_customer_service_record(const int64_t record_id, MongoDataMap* data_map)
{
	BSONObjBuilder build;
	build << BackCustomerSVCRecord::REMOVE_FLAG << 1;
	data_map->push_update(BackCustomerSVCRecord::COLLECTION,
			BSON(BackCustomerSVCRecord::ID << (long long int)record_id),
			build.obj(), true);
	return 0;
}

int BackCustomerSVC::update_player_customer_service_detail(LogicPlayer* player, MongoDataMap* data_map)
{
	JUDGE_RETURN(NULL != player , -1);
	CustomerServiceDetail& detail = player->customer_service_detial();

	BSONVec bson_vec;
	typedef DBCustomerSVCDetail::RewardInfo  DB_reward;

	for(IntMap::iterator iter = detail.__opinion_reward.begin();
			iter != detail.__opinion_reward.end(); iter++)
	{
		int reward_status = iter->second;
		JUDGE_CONTINUE(reward_status > 0);

		int opinion_index = iter->first;
		bson_vec.push_back(BSON(DB_reward::OPINION_INDEX << opinion_index
				<<  DB_reward::REWARD_STATUS << reward_status));
	}

	BSONObjBuilder build;
	build << DBCustomerSVCDetail::LAST_SUMMIT_TYPE << detail.__last_summit_type
			<< DBCustomerSVCDetail::CONTENT << detail.__content
			<< DBCustomerSVCDetail::TITLE << detail.__title
			<< DBCustomerSVCDetail::OPINION_REWARD << bson_vec;

	data_map->push_update(DBCustomerSVCDetail::COLLECTION,
			BSON(DBCustomerSVCDetail::ID << player->role_id()),
			build.obj(), true);
	return 0;
}

int BackCustomerSVC::update_backstage_customer_service_record(CustomerServiceRecord* record, MongoDataMap* data_map)
{
	JUDGE_RETURN(NULL != record, -1);

	BSONObjBuilder build;
	build << BackCustomerSVCRecord::ID << record->__record_id
			<< BackCustomerSVCRecord::SENDER_ID << record->__sender_id
			<< BackCustomerSVCRecord::SEND_TICK << record->__send_tick
			<< BackCustomerSVCRecord::SENDER_NAME << record->__sender_name
			<< BackCustomerSVCRecord::CONTENT << record->__content
			<< BackCustomerSVCRecord::TITLE << record->__title
			<< BackCustomerSVCRecord::REPLAY_CONTENT << record->__replay_content
			<< BackCustomerSVCRecord::RECORD_TYPE << record->__record_type
			<< BackCustomerSVCRecord::HAS_REPLAY << record->__has_replay
			<< BackCustomerSVCRecord::HAS_READ << record->__has_read
			<< BackCustomerSVCRecord::SENDER_LEVEL << record->__sender_level
			<< BackCustomerSVCRecord::SERVER_CODE << record->__server_code
			<< BackCustomerSVCRecord::PLATFORM << record->__platform
			<< BackCustomerSVCRecord::AGENT << record->__agent
			<< BackCustomerSVCRecord::RECHARGE_GOLD << record->__recharge_gold
			<< BackCustomerSVCRecord::EVALUATE_LEVEL << record->__evaluate_level
			<< BackCustomerSVCRecord::EVALUATE_TICK << record->__evaluate_tick
			<< BackCustomerSVCRecord::EVALUATE_STAR << record->__evaluate_star
			<< BackCustomerSVCRecord::OPINION_INDEX << record->__opinion_index
		<<  BackCustomerSVCRecord::NEED_LOAD_DATA << 0;

	data_map->push_update(BackCustomerSVCRecord::COLLECTION,
			BSON(BackCustomerSVCRecord::ID << record->__record_id),
			build.obj(), true);
	return 0;
}

void BackCustomerSVC::ensure_all_index()
{
	this->conection().ensureIndex(DBCustomerSVCDetail::COLLECTION, BSON(DBCustomerSVCDetail::ID << 1), true);
	this->conection().ensureIndex(BackCustomerSVCRecord::COLLECTION, BSON(BackCustomerSVCRecord::ID << 1), true);
	this->conection().ensureIndex(BackCustomerSVCRecord::COLLECTION, BSON(BackCustomerSVCRecord::SENDER_NAME << 1));
	this->conection().ensureIndex(BackCustomerSVCRecord::COLLECTION,
			BSON(BackCustomerSVCRecord::SENDER_ID << 1 << BackCustomerSVCRecord::NEED_LOAD_DATA << 1));
}


