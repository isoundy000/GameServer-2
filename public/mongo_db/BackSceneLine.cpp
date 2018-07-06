/*
 * BackSceneLine.cpp
 *
 * Created on: 2014-05-23 15:23
 *     Author: lyz
 */

#include "GameConfig.h"
#include "GameCommon.h"
#include "BackField.h"
#include "BackSceneLine.h"
#include "DaemonServer.h"
#include "MongoConnector.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "MongoException.h"

#include <mongo/client/dbclient.h>
using namespace mongo;

BackSceneLine::~BackSceneLine(void)
{ /*NULL*/ }

int BackSceneLine::load_scene_line_config(MongoDataMap *data_map)
{
BEGIN_CATCH
    MongoData *data = 0;
    if (data_map->find_data(DBSceneLine::COLLECTION, data) != 0)
        return -1;

    BSONObj res = this->conection().findOne(data->table(), mongo::Query(data->condition()));
    if (res.isEmpty())
        return -1;

    BSONObj arg_obj = data->data_bson();
    IntSet server_set, readed_server_set;
    int server_index = arg_obj[DBSceneLine::SERVER_INDEX].numberInt();
    {
        BSONObjIterator iter(arg_obj.getObjectField(DBSceneLine::SERVER_INDEX_SET.c_str()));
        while (iter.more())
        {
            server_set.insert(iter.next().numberInt());
        }
    }
    {
        BSONObjIterator iter(res.getObjectField(DBSceneLine::SERVER_INDEX_SET.c_str()));
        while (iter.more())
        {
            int index = iter.next().numberInt();
            readed_server_set.insert(index);
            server_set.erase(index);
        }
    }

    if (readed_server_set.find(server_index) != readed_server_set.end())
        return -1;

    MSG_USER("load scene line config %d", server_index);

    server_set.erase(server_index);

    data->set_data_bson(res);

    if (server_set.size() <= 0)
    {
        this->conection().update(DBSceneLine::COLLECTION, QUERY(DBSceneLine::FLAG << 1), 
                BSON("$set" << BSON(DBSceneLine::FLAG << 0 << DBSceneLine::SERVER_INDEX_SET << (std::vector<int>()))), false);
    }
    else
    {
        this->conection().update(DBSceneLine::COLLECTION, QUERY(DBSceneLine::FLAG << 1), 
                BSON("$push" << BSON(DBSceneLine::SERVER_INDEX_SET << server_index)), false);
    }

    return 0;
END_CATCH
	return -1;
}

int BackSceneLine::generate_find_data(MongoDataMap *data_map)
{
    data_map->push_find(DBSceneLine::COLLECTION, BSON(DBSceneLine::FLAG << 1));

#ifndef LOCAL_DEBUG
    MongoData *mongo_data = 0;
    if (data_map->find_data(DBSceneLine::COLLECTION, mongo_data) == 0)
    {
    	IntVec idx_svc;
    	int i = 0;
    	const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
    	for (GameConfig::ServerList::const_iterator iter = server_list.begin();
    			iter != server_list.end(); ++iter)
    	{
    		const ServerDetail &detail = *iter;
    		for (BIntSet::const_iterator scene_iter = detail.__scene_list.begin();
    				scene_iter != detail.__scene_list.end(); ++scene_iter)
    		{
    			if (GameCommon::is_normal_scene(*scene_iter) == true)
    			{
    				idx_svc.push_back(i);
    			}
    		}
    		++i;
    	}

    	mongo_data->set_data_bson(BSON(DBSceneLine::SERVER_INDEX << DAEMON_SERVER->server_index()
    			<< DBSceneLine::SERVER_INDEX_SET << idx_svc));
    }
#endif

    return 0;
}

int BackSceneLine::read_data(SceneLineManager::SceneLineMap &scene_line_map, MongoDataMap *data_map)
{
    MongoData *data = 0;
    if (data_map->find_data(DBSceneLine::COLLECTION, data) != 0)
        return -1;

    int ret = -1;
    BSONObj res = data->data_bson();
    BSONObjIterator iter(res.getObjectField(DBSceneLine::SCENE.c_str()));
    while (iter.more())
    {
        BSONObj obj = iter.next().embeddedObject();
        int scene_id = obj[DBSceneLine::Scene::SCENE_ID].numberInt();
        if (scene_id <= 0)
        	continue;
        SceneLineManager::SceneLineInfo &line_info = scene_line_map[scene_id];
        line_info.__scene_id = scene_id;
        line_info.__type = obj[DBSceneLine::Scene::TYPE].numberInt();
        line_info.__max_line = obj[DBSceneLine::Scene::MAX_LINE].numberInt();
        line_info.__per_player = obj[DBSceneLine::Scene::PER_PLAYER].numberInt();
        ret = 0;
    }
    return ret;
}

void BackSceneLine::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(DBSceneLine::COLLECTION, BSON(DBSceneLine::FLAG << 1), false);
END_CATCH
}

