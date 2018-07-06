/*
 * SceneFactory.cpp
 *
 * Created on: 2013-03-01 10:20
 *     Author: glendy
 */

#include "SceneFactory.h"
#include "GameHeader.h"
#include "NormalScene.h"
#include "ScriptScene.h"

SceneFactory::SceneFactory(void)
{
	this->normal_scene_pool_ = new NormalScenePool();
	this->script_scene_pool_ = new ScriptScenePool();
}

SceneFactory::~SceneFactory(void)
{
    this->normal_scene_pool_->clear();
    this->script_scene_pool_->clear();

    SAFE_DELETE(this->normal_scene_pool_);
    SAFE_DELETE(this->script_scene_pool_);
}

Scene *SceneFactory::pop_scene(const int scene_id)
{
    if (this->is_normal_scene(scene_id) == true)
    {
        return this->normal_scene_pool_->pop();
    }

    if (this->is_script_scene(scene_id) == true)
    {
    	return this->script_scene_pool_->pop();
    }

    return 0;
}

int SceneFactory::push_scene(Scene *scene)
{
    JUDGE_RETURN(scene != NULL, -1);

	if (dynamic_cast<NormalScene *>(scene) != 0)
	{
		return this->normal_scene_pool_->push(dynamic_cast<NormalScene *>(scene));
	}

	if (dynamic_cast<ScriptScene *>(scene) != 0)
	{
		return this->script_scene_pool_->push(dynamic_cast<ScriptScene *>(scene));
	}

    return -1;
}

bool SceneFactory::is_normal_scene(const int scene_id)
{
    return GameCommon::is_normal_scene(scene_id);
}

bool SceneFactory::is_script_scene(const int scene_id)
{
	return GameCommon::is_script_scene(scene_id);
}

void SceneFactory::report_pool_info(std::ostringstream &msg_stream)
{
    if (this->normal_scene_pool_ != 0)
    {
        msg_stream << "NormalScene Pool" << std::endl;
        this->normal_scene_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->script_scene_pool_ != 0)
    {
        msg_stream << "ScriptScene Pool" << std::endl;
        this->script_scene_pool_->dump_info_to_stream(msg_stream);
    }
}

