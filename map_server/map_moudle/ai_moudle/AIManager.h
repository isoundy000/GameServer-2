/*
 * AIManager.h
 *
 *  Created on: Jul 19, 2013
 *      Author: peizhibi
 */

#ifndef AIMANAGER_H_
#define AIMANAGER_H_

#include "GameAI.h"
#include "EffectAI.h"
#include "AIDropPack.h"

class FloatAI;

class AIManager
{
public:
	AIManager();
	~AIManager();

	PoolPackage<GameAI, long>* ai_package();
	PoolPackage<AIDropPack, long>* aidrop_package();
    PoolPackage<EffectAI, long> *effect_ai_package(void);
    PoolPackage<FloatAI, long> *float_ai_package();

	int fetch_ai_flag(const std::string& flag_name);

	AIAreaIndexInfo* check_add_ai_area(int monster_sort, int scene_id, int area_index);
	AIAreaIndexInfo* fetch_ai_area(int monster_sort);

	MoverCoord fetch_ai_area_coord(GameAI* game_ai, int offset);

	Int64 generate_ai(int layout_index, Scene* scene);
	Int64 generate_ai_by_center(int layout_index, Scene* scene, int born_count, bool is_rand_center = false);
	Int64 generate_ai_by_area(int layout_index, Scene* scene, int born_count);
	Int64 generate_ai_by_fixed(int layout_index, Scene* scene);

	GameAI* generate_monster_by_scene(const IntPair& monster_index, const MoverCoord& coordxy, Scene* scene);
    Int64 generate_monster_by_sort(int monster_sort, const MoverCoord &coordxy,
    		Scene *scene, int enter_flag = true);
    Int64 generate_ai_effect(int monster_sort, const MoverCoord &location, GameFighter *fighter);
    Int64 generate_float_ai(const int monster_sort, const MoverCoord &location, Scene *scene);

    AIDropPack *generate_drop_item(const ItemObj &item_obj, const MoverCoord &location, Scene *scene,
    		const int drop_type = GameEnum::DROP_TYPE_ALL, const LongMap &player_map = LongMap());

	void init_ai_flag();
    void report_pool_info(std::ostringstream &stream);

private:
	std::map<std::string, int> ai_flag_map_;
	std::map<int, AIAreaIndexInfo> ai_area_map_;

	PoolPackage<GameAI, long> ai_package_;
	PoolPackage<AIDropPack, long> aidrop_package_;
    PoolPackage<EffectAI, long> effect_ai_package_;
    PoolPackage<FloatAI, long> *float_ai_package_;
};

typedef Singleton<AIManager> AIManagerSingle;
#define AIMANAGER				AIManagerSingle::instance()
#define AI_PACKAGE				AIMANAGER->ai_package()
#define AIDROP_PACKAGE			AIMANAGER->aidrop_package()

#endif /* AIMANAGER_H_ */
