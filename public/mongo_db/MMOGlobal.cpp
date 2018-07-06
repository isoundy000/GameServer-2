/*
 * MMOGlobal.cpp
 *
 * Created on: 2013-03-21 17:27
 *     Author: lyz
 */

#include "GameDefine.h"
#include "GameField.h"
#include "HashMap.h"
#include "MMOGlobal.h"
#include "MongoConnector.h"
#include "GameConfig.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
#include "MongoDataMap.h"
#include "GameCommon.h"
using namespace mongo;

MMOGlobal::~MMOGlobal(void)
{ /*NULL*/ }

int MMOGlobal::load_global_key(HashMap<std::string, int64_t, NULL_MUTEX> *global_key_map)
{
BEGIN_CATCH
	global_key_map->unbind_all();

	std::vector<std::string> key_list;
	key_list.push_back(Global::ROLE);
	key_list.push_back(Global::MAIL);
	key_list.push_back(Global::FRIENDSHIP);
	key_list.push_back(Global::LEAGUE);
	key_list.push_back(Global::CUSTOMER_SERVICE);
	key_list.push_back(Global::WEDDING);
	key_list.push_back(Global::LSTORE_APPLY);
	key_list.push_back(Global::LSTORE_ITEM_ID);
	key_list.push_back(Global::BROTHER);
	key_list.push_back(Global::TRAVEL_TEAM);

	for (std::vector<std::string>::iterator iter = key_list.begin(); iter != key_list.end(); ++iter)
	{
		BSONObj res = this->conection().findOne(Global::COLLECTION, QUERY(Global::KEY << *iter));
		int64_t id = (((int64_t)CONFIG_INSTANCE->server_id())) << 32;
		if (res.isEmpty() == false)
			id += res[Global::GID].numberInt();
		global_key_map->rebind(*iter, id);
	}
	return 0;
END_CATCH
	return -1;
}

void MMOGlobal::ensure_all_index(void)
{
    this->conection().ensureIndex(Global::COLLECTION, BSON(Global::KEY << 1), true);
    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::ROLE)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::ROLE << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::MAIL)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::MAIL << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::FRIENDSHIP)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::FRIENDSHIP << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::LEAGUE)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::LEAGUE << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION,	BSON(Global::KEY << Global::BEAST)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::BEAST << Global::GID << BASE_OFFSET_BEAST));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::VOICE_ID)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::VOICE_ID << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::SCRIPT_PROGRESS)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::SCRIPT_PROGRESS << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::CUSTOMER_SERVICE)) == 0)
    {
    	this->conection().insert(Global::COLLECTION, BSON(Global::KEY
    			<< Global::CUSTOMER_SERVICE << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::WEDDING)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::WEDDING << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::LSTORE_APPLY)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::LSTORE_APPLY << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::LSTORE_ITEM_ID)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::LSTORE_ITEM_ID << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::BROTHER)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::BROTHER << Global::GID << 0));
    }

    if (this->conection().count(Global::COLLECTION, BSON(Global::KEY << Global::TRAVEL_TEAM)) == 0)
    {
        this->conection().insert(Global::COLLECTION, BSON(Global::KEY
        		<< Global::TRAVEL_TEAM << Global::GID << 0));
    }
}

int64_t MMOGlobal::update_global_key(const std::string &key)
{
BEGIN_CATCH
    char cmd_format[128] = "{findandmodify:'global', query: {key:'%s'}, update:{$inc:{id:1}}}";
    char sz_cmd[512];
    ::sprintf(sz_cmd, cmd_format, key.c_str());
    BSONObj cmd = fromjson(sz_cmd), res;

    if (this->conection().runCommand("mmo", cmd, res) == false)
    {
        MSG_USER("increase global key failed %s", key.c_str());
        return -1;
    }
    return (((int64_t)CONFIG_INSTANCE->server_id()) << 32)
    		+ (res.getFieldDotted("value.id").numberInt() + 1);
END_CATCH
    return -1;
}

int MMOGlobal::update_global_key(const std::string &key, const int64_t value, MongoDataMap *data_map)
{
	BSONObjBuilder builder;
	builder << Global::GID << int(value);
	data_map->push_update(Global::COLLECTION,BSON(Global::KEY << key),builder.obj(),true);
	return 0;
}

int64_t MMOGlobal::load_global_voice_id(void)
{
    BSONObj res = this->conection().findOne(Global::COLLECTION, QUERY(Global::KEY << Global::VOICE_ID));

    int64_t id = (((int64_t)CONFIG_INSTANCE->server_id()) << 32);
    if (res.isEmpty() == false)
    {
       id += res[Global::GID].numberInt();
    }

//    int id = res[Global::GID].numberInt();
    id = (id <=0?0:id);
    return id;
}

int MMOGlobal::update_global_voice_id(int64_t id)
{
	BSONObjBuilder builder;
	builder << Global::GID << (long long int)id;
	this->conection().update(Global::COLLECTION,BSON(Global::KEY << Global::VOICE_ID),builder.obj(),true);
	return 0;
}

int MMOGlobal::load_global_script_progress(HashMap<int, Int64, NULL_MUTEX> *progress_map, const std::vector<int> &script_set)
{
BEGIN_CATCH
	progress_map->unbind_all();

    int script_sort = 0;
    Int64 progress_id = 0;
    char sz_script_sort[32];

	BSONObj res = this->conection().findOne(Global::COLLECTION, QUERY(Global::KEY << Global::SCRIPT_PROGRESS));
    for (std::vector<int>::const_iterator iter = script_set.begin(); iter != script_set.end(); ++iter)
	{
        script_sort = *iter;
        progress_id = 0;
        ::sprintf(sz_script_sort, "%d", script_sort);
	    if (res.hasField(sz_script_sort) == true)
	        progress_id = res[sz_script_sort].numberLong();
        if (progress_id == 0)
            progress_id = script_sort * 1000000L;
	    progress_map->rebind(script_sort, progress_id);
	}
	return 0;
END_CATCH
	return -1;
}


int MMOGlobal::update_global_script_progress(const int script_sort, const Int64 progress)
{
BEGIN_CATCH
    char cmd_format[128] = "{findandmodify:'global', query: {key:'%s'}, update:{$set:{'%d':NumberLong(%ld)}}}";
    char sz_cmd[512];
    ::sprintf(sz_cmd, cmd_format, Global::SCRIPT_PROGRESS.c_str(), script_sort, progress);
    BSONObj cmd = fromjson(sz_cmd), res;

    if (this->conection().runCommand("mmo", cmd, res) == false)
    {
        MSG_USER("increase global key failed %d", script_sort);
        return -1;
    }
    return 0;
END_CATCH
    return -1;
}

int MMOGlobal::load_global_key_value(const string& key, int& value)
{
	BSONObj res = CACHED_CONNECTION.findOne(Global::COLLECTION, QUERY(Global::KEY << key));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	value = res[Global::GID].numberInt();
	return 0;
}

int MMOGlobal::save_global_key_to_mongo_unit(const std::string &key, const int value)
{
    BSONObjBuilder builder;
    builder << Global::GID << value;

    GameCommon::request_save_mmo_begin(Global::COLLECTION,
            BSON(Global::KEY << key), BSON("$set" << builder.obj()));
    return 0;
}

int MMOGlobal::load_combine_first_value(int& value)
{
	BSONObj res = CACHED_CONNECTION.findOne(DBLWTicker::COLLECTION, QUERY(DBLWTicker::ID << 3));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	value = res[DBLWTicker::CombineFirst::COMB_FIRST].numberInt();
	return 0;
}

int MMOGlobal::save_comnbine_first_value(int value)
{
	BSONObjBuilder builder;
	builder << DBLWTicker::CombineFirst::COMB_FIRST << 0;

	CACHED_CONNECTION.update(DBLWTicker::COLLECTION, BSON(DBLWTicker::ID << int(3)),
			BSON("$set" << builder.obj()), true);
	return 0;
}
