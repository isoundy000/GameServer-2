/*
 * NormalScene.cpp
 *
 *  Created on: Aug 25, 2013
 *      Author: peizhibi
 */

#include "NormalScene.h"
#include "AIManager.h"
#include "MapMonitor.h"
#include "ProtoInner007.pb.h"

int NormalScene::BornAITimer::type(void)
{
    return GTT_MAP_PLAYER;
}

int NormalScene::BornAITimer::handle_timeout(const Time_Value &tv)
{
	this->scene_->check_and_run_scene_monster();
	this->scene_->dump_info(tv.sec());
	return 0;
}

NormalScene::NormalScene()
{
	this->born_timer_.scene_ = this;
	this->next_time_ = 0;
}

void NormalScene::reset()
{
	this->monster_map_.clear();
	this->born_timer_.cancel_timer();
	Scene::reset();
}

void NormalScene::start_scene()
{
	Scene::start_scene();
	this->born_timer_.schedule_timer(0.1);
}

void NormalScene::dump_info(Int64 cur_time)
{
	JUDGE_RETURN(this->next_time_ < cur_time, ;);

	this->next_time_ = cur_time + Time_Value::MINUTE;
	for (IntPairMap::iterator iter = this->monster_map_.begin();
			iter != this->monster_map_.end(); ++iter)
	{
		IntPair& pair = iter->second;
		MSG_USER("scene id %d, index %d, span %d count %d", this->scene_id(),
				iter->first, pair.first, pair.second);

		JUDGE_CONTINUE(pair.second == 0);
		this->run_scene_monster(iter->first);
	}
}

void NormalScene::check_and_run_scene_monster()
{
	for (IntPairMap::iterator iter = this->monster_map_.begin();
			iter != this->monster_map_.end(); ++iter)
	{
		IntPair& pair = iter->second;
		JUDGE_CONTINUE(pair.second > 0);

		pair.first -= 1;
		JUDGE_CONTINUE(pair.first <= 0);

		this->run_center_scene_monster(iter->first, pair.second);
		pair.second = 0;
	}
}

void NormalScene::run_center_scene_monster(int layout_index, int differ_count)
{
	const Json::Value &layout_item = this->layout_index(layout_index);
	JUDGE_RETURN(layout_item["born_type"].asString() == GameName::CENTER_BORN, ;);

	AIMANAGER->generate_ai_by_center(layout_index, this, differ_count, true);
}

void NormalScene::run_area_scene_monster(int layout_index, int differ_count)
{
	const Json::Value &layout_item = this->layout_index(layout_index);
	JUDGE_RETURN(layout_item["born_type"].asString() == GameName::AREA_BORN, ;);

	AIMANAGER->generate_ai_by_area(layout_index, this, differ_count);
}

int NormalScene::handle_reborn_info(GameAI* game_ai)
{
	int layout_index = game_ai->layout_index();
	JUDGE_RETURN(layout_index >= 0, -1);

	int born_span = game_ai->ai_detail().__born_span / 100;
	if (born_span > 0)
	{
		IntPair& pair = this->monster_map_[layout_index];
		pair.second += 1;

		JUDGE_RETURN(pair.first <= 0, -1);
		pair.first = born_span;
	}
	else
	{
		this->run_center_scene_monster(layout_index, 1);
	}

	return 0;
}

int NormalScene::handle_festival_boss(GameAI* game_ai)
{
	return 0;
}

int NormalScene::run_scene_monster(int layout_index)
{
	const Json::Value &layout_item = this->layout_index(layout_index);
	JUDGE_RETURN(layout_item.empty() == false, -1);
	JUDGE_RETURN(layout_item["born_times"].asString() == GameName::UNLIMITED_BORN, -1);

	int monster_sort = layout_item["monster_sort"].asInt();
	int differ_count = layout_item["born_count"].asInt()
			- this->scene_detail_.ai_sort_map_[monster_sort].size();

#ifndef LOCAL_DEBUG
	MSG_USER("NormalScene %d, %d, %d", layout_index, monster_sort, differ_count);
#endif
	JUDGE_RETURN(differ_count > 0, -1);

	this->run_center_scene_monster(layout_index, differ_count);
	this->run_area_scene_monster(layout_index, differ_count);
	return 0;
}

int NormalScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	JUDGE_RETURN(game_ai->is_boss() == false, -1);
	return this->handle_reborn_info(game_ai);
}

int NormalScene::handle_boss_die_action(GameAI* game_ai)
{
	JUDGE_RETURN(game_ai->is_boss() == true, -1);
	return this->handle_reborn_info(game_ai);
}
