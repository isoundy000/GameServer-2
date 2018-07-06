/*
 * SceneFactory.h
 *
 * Created on: 2013-03-01 10:06
 *     Author: glendy
 */

#ifndef _SCENEFACTORY_H_
#define _SCENEFACTORY_H_

#include "ObjectPoolEx.h"

class Scene;
class NormalScene;
class ScriptScene;

class SceneFactory
{
public:
    typedef ObjectPoolEx<NormalScene> NormalScenePool;
    typedef ObjectPoolEx<ScriptScene> ScriptScenePool;

public:
    SceneFactory(void);
    ~SceneFactory(void);

    Scene *pop_scene(const int scene_id);
    int push_scene(Scene *scene);

    bool is_normal_scene(const int scene_id);
    bool is_script_scene(const int scene_id);

    void report_pool_info(std::ostringstream &msg_stream);

protected:
    NormalScenePool *normal_scene_pool_;
    ScriptScenePool *script_scene_pool_;
};

#endif //_SCENEFACTORY_H_
