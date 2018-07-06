/*
 * AIManager.cpp
 *
 *  Created on: Jul 19, 2013
 *      Author: peizhibi
 */

#include "AIManager.h"
#include "AIBTCName.h"
#include "FloatAI.h"
#include "Scene.h"

AIManager::AIManager()
{
	this->float_ai_package_ = new PoolPackage<FloatAI, long>();
	this->init_ai_flag();
}

AIManager::~AIManager()
{
	SAFE_DELETE(this->float_ai_package_);
}

void AIManager::init_ai_flag()
{
	this->ai_flag_map_[AIBTCName::NO_MOVE] = GameEnum::AI_CF_NO_MOVE;
	this->ai_flag_map_[AIBTCName::NO_ATTACK] = GameEnum::AI_CF_NO_ATTACK;
	this->ai_flag_map_[AIBTCName::NO_BE_ATTACKED] = GameEnum::AI_CF_NO_BE_ATTACKED;
	this->ai_flag_map_[AIBTCName::NO_AIM] = GameEnum::AI_CF_NO_AIM;
	this->ai_flag_map_[AIBTCName::RECYCLE_ON_ATTACKED] = GameEnum::AI_CF_RECYCLE_ON_ATTACKED;
}

PoolPackage<GameAI, long>* AIManager::ai_package()
{
	return &this->ai_package_;
}

PoolPackage<AIDropPack, long>* AIManager::aidrop_package()
{
	return &this->aidrop_package_;
}

PoolPackage<EffectAI, long> *AIManager::effect_ai_package(void)
{
    return &(this->effect_ai_package_);
}

PoolPackage<FloatAI, long> *AIManager::float_ai_package(void)
{
	return this->float_ai_package_;
}

int AIManager::fetch_ai_flag(const std::string& flag_name)
{
	JUDGE_RETURN(this->ai_flag_map_.count(flag_name) > 0, 0);
	return this->ai_flag_map_[flag_name];
}

AIAreaIndexInfo* AIManager::check_add_ai_area(int monster_sort, int scene_id, int area_index)
{
	AIAreaIndexInfo* area_info = this->fetch_ai_area(monster_sort);
	JUDGE_RETURN(area_info == NULL, area_info);

	area_info = &this->ai_area_map_[monster_sort];
	area_info->ai_sort_ = monster_sort;

	CONFIG_INSTANCE->fetch_coord_by_mpt_value(area_info->move_set_,
			scene_id, area_index);
	for (CoordVec::iterator iter = area_info->move_set_.begin();
			iter != area_info->move_set_.end(); ++iter)
	{
		Int64 coord_index = iter->coord_index();
		area_info->move_map_[coord_index] = true;
	}

	MSG_USER("%d, %d, %d, %d", monster_sort, scene_id, area_index, area_info->move_map_.size());
	return area_info;
}

AIAreaIndexInfo* AIManager::fetch_ai_area(int monster_sort)
{
	JUDGE_RETURN(this->ai_area_map_.count(monster_sort) > 0, NULL);
	return &this->ai_area_map_[monster_sort];
}

MoverCoord AIManager::fetch_ai_area_coord(GameAI* game_ai, int offset)
{
	MoverCoord rand_location = game_ai->location();

	AIAreaIndexInfo* area_info = this->fetch_ai_area(game_ai->ai_sort());
	JUDGE_RETURN(area_info != NULL, rand_location);

	Scene *scene = game_ai->fetch_scene();
	JUDGE_RETURN(scene != NULL, rand_location);

	int rand_times = 0;
	while (rand_times  < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
		rand_times += 1;
		rand_location = scene->rand_dynamic_coord(game_ai->birth_coord(), offset);

		JUDGE_CONTINUE(area_info->move_map_.count(rand_location.coord_index()) > 0);
		break;
	}

	return rand_location;
}

Int64 AIManager::generate_ai(int layout_index, Scene* scene)
{
	const Json::Value& layout_item = scene->layout_index(layout_index);
	JUDGE_RETURN(layout_item.isMember("monster_tree") == true, 0);

	string born_type = layout_item["born_type"].asString();
	if (born_type == GameName::CENTER_BORN)
	{
		return this->generate_ai_by_center(layout_index, scene,
				layout_item["born_count"].asInt());
	}
	else if (born_type == GameName::AREA_BORN)
	{
		return this->generate_ai_by_area(layout_index, scene,
				layout_item["born_count"].asInt());
	}
	else if (born_type == GameName::FIXED_BORN)
	{
		return this->generate_ai_by_fixed(layout_index, scene);
	}
	else
	{
		MSG_USER("Error AI Born Type Name: %s", born_type.c_str());
		return 0;
	}
}

Int64 AIManager::generate_ai_by_center(int layout_index, Scene* scene, int born_count, bool is_rand_center/*=false*/)
{
	const Json::Value& layout_item = scene->layout_index(layout_index);
	JUDGE_RETURN(born_count > 0, 0);

	int born_range = layout_item["born_range"].asInt();
	IntPair monster_index(layout_index, layout_item["monster_sort"].asInt());

	GameAI* last_ai = NULL;
	int center_size = layout_item["center_coordxy"].size() / 2;
	if (is_rand_center == false)
	{
		for (int center_index = 0; center_index < center_size; center_index++)
		{
			MoverCoord center_coord;
			center_coord.set_pixel(layout_item["center_coordxy"][center_index * 2].asInt(),
						layout_item["center_coordxy"][center_index * 2+ 1].asInt());
			JUDGE_CONTINUE(GameCommon::is_movable_coord(scene->scene_id(), center_coord) == true);

			for (int i = 0; i < born_count; ++i)
			{
				MoverCoord coordxy = scene->rand_dynamic_coord(center_coord, born_range);
				last_ai = this->generate_monster_by_scene(monster_index, coordxy, scene);
			}
		}
	}
	else
	{
		for (int i = 0; i < born_count; ++i)
		{
			int center_index = ::rand() % center_size;

			MoverCoord center_coord;
			center_coord.set_pixel(layout_item["center_coordxy"][center_index * 2].asInt(),
					layout_item["center_coordxy"][center_index * 2 + 1].asInt());
			JUDGE_CONTINUE(GameCommon::is_movable_coord(scene->scene_id(), center_coord) == true);

			MoverCoord coordxy = scene->rand_dynamic_coord(center_coord, born_range);
			last_ai = this->generate_monster_by_scene(monster_index, coordxy, scene);
		}
	}

	return (last_ai != NULL) ? last_ai->ai_id() : 0;
}

Int64 AIManager::generate_ai_by_area(int layout_index, Scene* scene, int born_count)
{
	const Json::Value& layout_item = scene->layout_index(layout_index);
	JUDGE_RETURN(born_count > 0, 0);

	int area_index = layout_item["area_index"].asInt();
	int monster_sort = layout_item["monster_sort"].asInt();

	AIAreaIndexInfo* area_info = this->check_add_ai_area(monster_sort,
			scene->scene_id(), area_index);
	JUDGE_RETURN(area_info != NULL, 0);

	GameAI* last_ai = NULL;
	IntPair monster_index(layout_index, monster_sort);

	int total_size = area_info->move_set_.size();
	for (int i = 0; i < born_count; ++i)
	{
		MoverCoord coordxy = area_info->move_set_[std::rand() % total_size];
		last_ai = this->generate_monster_by_scene(monster_index, coordxy, scene);
	}

	return (last_ai != NULL) ? last_ai->ai_id() : 0;
}

Int64 AIManager::generate_ai_by_fixed(int layout_index, Scene* scene)
{
	const Json::Value& layout_item = scene->layout_index(layout_index);

	GameAI* last_ai = NULL;
	IntPair monster_index(layout_index, layout_item["monster_sort"].asInt());

	int born_count = layout_item["born_coordxy"].size() / 2;
	for (int i = 0; i < born_count; i++)
	{
		MoverCoord coordxy;
		coordxy.set_pixel(layout_item["born_coordxy"][i * 2].asInt(),
				layout_item["born_coordxy"][i * 2 + 1].asInt());
		last_ai = this->generate_monster_by_scene(monster_index, coordxy, scene);
	}

	return (last_ai != NULL) ? last_ai->ai_id() : 0;
}

GameAI* AIManager::generate_monster_by_scene(const IntPair& monster_index, const MoverCoord& coordxy,
		Scene* scene)
{
	const Json::Value &monster_json = CONFIG_INSTANCE->monster(monster_index.second);
	JUDGE_RETURN(monster_json != Json::Value::null, NULL);

	GameAI* game_ai = this->ai_package()->pop_object();
	JUDGE_RETURN(game_ai != NULL, NULL);

	game_ai->set_layout_index(monster_index.first);
	game_ai->sign_in_with_scene(monster_index.second, coordxy, scene);
	this->ai_package()->bind_object(game_ai->ai_id(), game_ai);

	MSG_USER("generate monster %ld %d %d %d %d(%d,%d)", game_ai->mover_id(), game_ai->ai_sort(),
			monster_index.first, game_ai->space_id(), game_ai->scene_id(), game_ai->location().pixel_x(),
			game_ai->location().pixel_y());

	game_ai->enter_scene();
	return game_ai;
}

Int64 AIManager::generate_monster_by_sort(int monster_sort, const MoverCoord &coordxy,
		Scene *scene, int enter_flag)
{
    JUDGE_RETURN(scene != NULL, -1);

	GameAI* game_ai = this->ai_package()->pop_object();
	JUDGE_RETURN(game_ai != NULL, -1);

	game_ai->sign_in_with_sort(monster_sort, coordxy, scene);
	this->ai_package()->bind_object(game_ai->ai_id(), game_ai);

	MSG_USER("generate monster %ld %d %d %d(%d,%d)", game_ai->mover_id(), game_ai->ai_sort(),
			game_ai->space_id(), game_ai->scene_id(), game_ai->location().pixel_x(),
			game_ai->location().pixel_y());

	scene->handle_init_ai(game_ai);
	if (enter_flag == true)
	{
		game_ai->enter_scene();
	}

	return game_ai->ai_id();
}

// 生成特效对象
Int64 AIManager::generate_ai_effect(int monster_sort, const MoverCoord &location, GameFighter *fighter)
{
	JUDGE_RETURN(fighter != NULL, -1);
    Scene *scene = fighter->fetch_scene();
    JUDGE_RETURN(scene != 0, -1);

	EffectAI* game_ai = this->effect_ai_package()->pop_object();
	JUDGE_RETURN(game_ai != NULL, -1);

    game_ai->set_room_scene_index(fighter->room_scene_index());

	game_ai->sign_in_with_sort(monster_sort, location, scene);
	this->effect_ai_package()->bind_object(game_ai->ai_id(), game_ai);
    game_ai->copy_fighter_property(fighter);
    game_ai->set_rect_skill_coord(location);
    
    const Json::Value &prop_monster = CONFIG_INSTANCE->prop_monster(monster_sort);
    if (prop_monster.isMember("effect_id"))
        game_ai->set_effect_sort(prop_monster["effect_id"].asInt());
    else
        game_ai->set_effect_sort(game_ai->ai_sort());

	MSG_USER("generate effect monster %ld %d %d %d(%d,%d) edge(%d,%d)", game_ai->mover_id(), game_ai->ai_sort(),
			game_ai->space_id(), game_ai->scene_id(), game_ai->location().pixel_x(),
			game_ai->location().pixel_y(), game_ai->rect_skill_coord().pixel_x(), game_ai->rect_skill_coord().pixel_y());

	return game_ai->ai_id();
}

Int64 AIManager::generate_float_ai(const int monster_sort, const MoverCoord &location, Scene *scene)
{
    JUDGE_RETURN(scene != NULL, -1);

    FloatAI *float_ai = this->float_ai_package()->pop_object();
    JUDGE_RETURN(float_ai != NULL, -1);

    float_ai->sign_in_with_sort(monster_sort, location, scene);

    this->float_ai_package()->bind_object(float_ai->ai_id(), float_ai);

    return float_ai->ai_id();
}

AIDropPack *AIManager::generate_drop_item(const ItemObj &item_obj, const MoverCoord &location,
		Scene *scene, const int drop_type, const LongMap &player_map)
{
    AIDropPack *drop_pack = this->aidrop_package()->pop_object();

    AIDropDetail &drop_detail = drop_pack->drop_detail();
    drop_detail.ai_sort_ = item_obj.id_;
    drop_detail.item_obj_.__id = item_obj.id_;
    drop_detail.item_obj_.__amount = item_obj.amount_;
    drop_detail.item_obj_.__bind = item_obj.bind_;
    drop_detail.player_map_ = player_map;
    drop_detail.drop_type_ = drop_type;

    drop_pack->calc_pickup_protect_tick(Json::Value::null);
    drop_pack->calc_recycle_tick(Json::Value::null, scene);
    drop_pack->sign_and_enter_scene(location, scene);
    return drop_pack;
}

void AIManager::report_pool_info(std::ostringstream &stream)
{
    stream << "AI Pool" << std::endl;
    this->ai_package_.report_pool_info(stream);

    stream << "AIdrop Pool" << std::endl;
    this->aidrop_package_.report_pool_info(stream);

    stream << "Effect AI Pool" << std::endl;
    this->effect_ai_package_.report_pool_info(stream);

    if (this->float_ai_package_ != NULL)
    {
        stream << "FloatAi Pool" << std::endl;
        this->float_ai_package_->report_pool_info(stream);
    }
}

