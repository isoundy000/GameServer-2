/*
 * SceneLineManager.cpp
 *
 * Created on: 2014-05-21 14:33
 *     Author: lyz
 */

#include "SceneLineManager.h"
#include "Transaction.h"
#include "MongoDataMap.h"
#include "MapMonitor.h"
#include "GameCommon.h"
#include "BackSceneLine.h"
#include "Scene.h"

SceneLineManager::SceneLineInfo::SceneLineInfo(void) :
    __scene_id(0), __type(0), __max_line(0), __per_player(10000)
{ /*NULL*/ }

void SceneLineManager::SceneLineInfo::reset(void)
{
    this->__scene_id = 0;
    this->__type = 0;
    this->__max_line = 0;
    this->__per_player = 10000;

    for (std::vector<Scene *>::iterator iter = this->__scene_vc.begin(); iter != this->__scene_vc.end(); ++iter)
    {
        MAP_MONITOR->push_scene(*iter);
    }
    this->__scene_vc.clear();
}

SceneLineManager::SceneLineManager(void)
{
    this->monitor_ = MAP_MONITOR;
    this->check_config_tick_ = Time_Value::zero;
}

SceneLineManager::~SceneLineManager(void)
{ /*NULL*/ }

MapMonitor *SceneLineManager::monitor(void)
{
	return this->monitor_;
}

void SceneLineManager::stop(void)
{
    for (SceneLineMap::iterator iter = this->scene_line_map_.begin(); iter != this->scene_line_map_.end(); ++iter)
    {
        SceneLineInfo &scene_info = iter->second;
        for (SceneVec::iterator scene_iter = scene_info.__scene_vc.begin(); scene_iter != scene_info.__scene_vc.end(); ++scene_iter)
        {
            this->monitor()->push_scene(*scene_iter);
        }
    }
    this->scene_line_map_.clear();
}

int SceneLineManager::init_line_scene(const int scene_id)
{
    JUDGE_RETURN(GameCommon::is_normal_scene(scene_id) == true, -1);

    const Json::Value &scene_line_json = CONFIG_INSTANCE->scene_line(scene_id);
    JUDGE_RETURN(scene_line_json != Json::Value::null, -1);

    SceneLineInfo &scene_info = this->scene_line_map_[scene_id];
    scene_info.__scene_id = scene_id;
    scene_info.__type = scene_line_json["type"].asInt();
    scene_info.__max_line = scene_line_json["max_line"].asInt();
    scene_info.__per_player = scene_line_json["per_player"].asInt();
    if (scene_info.__per_player <= 0)
        scene_info.__per_player = 10000;

    int line_type = scene_info.__type;
    if (line_type == SCENE_LINE_STATIC)
    {
        int scene_line = scene_info.__max_line;
        for (int i = int(scene_info.__scene_vc.size()); i < scene_line; ++i)
        {
            this->monitor()->init_scene(scene_id, i + 1);
            Scene *scene = 0;
            if (this->monitor()->find_scene(i + 1, scene_id, scene) == 0)
                scene_info.__scene_vc.push_back(scene);
        }
        return 0;
    }
    else if (line_type == SCENE_LINE_DYNAMIC)
    {
        JUDGE_RETURN(scene_info.__scene_vc.size() <= 0, 0);

        this->monitor()->init_scene(scene_id, 1);
        Scene *scene = 0;
        if (this->monitor()->find_scene(1, scene_id, scene) == 0)
            scene_info.__scene_vc.push_back(scene);

        return 0;
    }
    return -1;
}

int SceneLineManager::fetch_scene_line(const int scene_id)
{
    SceneLineMap::iterator iter = this->scene_line_map_.find(scene_id);
    if (iter == this->scene_line_map_.end())
        return 0;

    SceneLineInfo &scene_info = iter->second;
    if (scene_info.__type == SCENE_LINE_STATIC)
    {
        Scene *select_scene = 0;
        int i = 0;
        for (SceneVec::iterator scene_iter = scene_info.__scene_vc.begin(); scene_iter != scene_info.__scene_vc.end() && i < scene_info.__max_line; ++scene_iter, ++i)
        {
            Scene *scene = *scene_iter;
            if (select_scene == 0)
            {
                select_scene = scene;
                continue;
            }
            if (select_scene->player_amount() > scene->player_amount())
            {
                select_scene = scene;
                continue;
            }
        }
        if (select_scene == 0)
            return 0;
        return select_scene->space_id();
    }
    
    if (scene_info.__type == SCENE_LINE_DYNAMIC)
    {
        for (SceneVec::iterator scene_iter = scene_info.__scene_vc.begin(); scene_iter != scene_info.__scene_vc.end(); ++scene_iter)
        {
            Scene *scene = *scene_iter;
            if (scene->player_amount() < scene_info.__per_player)
                return scene->space_id();
        }

        this->monitor()->init_scene(scene_id, scene_info.__scene_vc.size() + 1);
        Scene *scene = 0;
        if (this->monitor()->find_scene(scene_info.__scene_vc.size() + 1, scene_id, scene) == 0)
            scene_info.__scene_vc.push_back(scene);

        return scene->space_id();
    }
    return 0;
}

int SceneLineManager::check_scene_line_config(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->check_config_tick_ <= nowtime, 0);
	this->check_config_tick_ = nowtime + Time_Value(10, 0);

    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    BackSceneLine::generate_find_data(data_map);

    if (TRANSACTION_MONITOR->request_mongo_transaction(1, TRANS_LOAD_SCENE_LINE_CONFIG, data_map, this->monitor()->map_unit()) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return -1;
    }
    return 0;
}

int SceneLineManager::sync_scene_line_config(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);

	if (transaction->is_failure() == true)
	{
		return 0;
	}

    MongoDataMap *data_map = transaction->fetch_mongo_data_map();
    if (data_map == 0)
        return 0;

    if (BackSceneLine::read_data(this->scene_line_map_, data_map) != 0)
    	return 0;

    for (SceneLineMap::iterator iter = this->scene_line_map_.begin(); iter != this->scene_line_map_.end(); ++iter)
    {
        SceneLineInfo &line_info = iter->second;
        if (line_info.__type != SCENE_LINE_STATIC)
            continue;
        if (line_info.__scene_id <= 0)
            continue;
       
        int i = 0;
        while (line_info.__max_line > int(line_info.__scene_vc.size()))
        {
            if (++i > 10000)
                break;
            int space_id = int(line_info.__scene_vc.size()) + 1, scene_id = line_info.__scene_id;
            this->monitor()->init_scene(scene_id, space_id);
            Scene *scene = 0;
            if (this->monitor()->find_scene(space_id, scene_id, scene) == 0)
                line_info.__scene_vc.push_back(scene);
        }
    }
    return 0;
}

int SceneLineManager::report_line_info(std::ostringstream &msg_stream)
{
	msg_stream << "scene line info:" << std::endl;
	for (SceneLineMap::iterator iter = this->scene_line_map_.begin(); iter != this->scene_line_map_.end(); ++iter)
	{
		SceneLineInfo &scene_info = iter->second;

		msg_stream << "scene: " << scene_info.__scene_id << "," << scene_info.__scene_vc.size()
				<< "," << scene_info.__type
				<< "," << scene_info.__max_line
				<< "," << scene_info.__per_player << std::endl;
	}
	return 0;
}
