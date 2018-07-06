/*
 * BackSceneLine.h
 *
 * Created on: 2014-05-23 15:20
 *     Author: lyz
 */

#ifndef _BACKSCENELINE_H_
#define _BACKSCENELINE_H_

#include "MongoTable.h"
#include "SceneLineManager.h"

class BackSceneLine : public MongoTable
{
public:
    virtual ~BackSceneLine(void);

    int load_scene_line_config(MongoDataMap *data_map);
    static int generate_find_data(MongoDataMap *data_map);

    static int read_data(SceneLineManager::SceneLineMap &scene_line_map, MongoDataMap *data_map);

protected:
    virtual void ensure_all_index(void);
};

#endif //_BACKSCENELINE_H_
