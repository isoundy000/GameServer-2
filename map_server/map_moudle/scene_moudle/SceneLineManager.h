/*
 * SceneLineManager.h
 *
 * Created on: 2014-05-21 14:19
 *     Author: lyz
 */

#ifndef _SCENELINEMANAGER_H_
#define _SCENELINEMANAGER_H_

#include "Singleton.h"
#include "Time_Value.h"
#include <vector>
#include <boost/unordered_map.hpp>

class MapMonitor;
class Scene;
class Transaction;

class SceneLineManager
{
public:
    enum {
        SCENE_LINE_STATIC = 1,      //按固定线分配玩家
        SCENE_LINE_DYNAMIC = 2     //按玩家数动态开线
    };
    struct SceneLineInfo
    {
        int __scene_id;
        int __type;
        int __max_line;
        int __per_player;
        std::vector<Scene *> __scene_vc;

        SceneLineInfo(void);
        void reset(void);
    };

    typedef std::vector<Scene *> SceneVec;
    typedef boost::unordered_map<int, SceneLineInfo> SceneLineMap;
public:
    SceneLineManager(void);
    ~SceneLineManager(void);

    MapMonitor *monitor(void);

    void stop(void);

    int init_line_scene(const int scene_id);

    int fetch_scene_line(const int scene_id);
    int check_scene_line_config(const Time_Value &nowtime);
    int sync_scene_line_config(Transaction *transaction);

    int report_line_info(std::ostringstream &msg_stream);

private:
    MapMonitor *monitor_;
    SceneLineMap scene_line_map_;
    Time_Value check_config_tick_;
};

#endif //_SCENELINEMANAGER_H_
