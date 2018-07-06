/*
 * ScriptScene.cpp
 *
 * Created on: 2013-12-30 10:43
 *     Author: lyz
 */

#include "ScriptScene.h"
#include "AIStruct.h"
#include "MapMonitor.h"
#include "ScriptAI.h"
#include "BaseScript.h"
#include "MapPlayerScript.h"
#include "MapPlayerEx.h"

ScriptScene::AIManagerTimer::AIManagerTimer(void) : scene_(0)
{ /*NULL*/ }

ScriptScene::AIManagerTimer::~AIManagerTimer(void)
{ /*NULL*/ }

void ScriptScene::AIManagerTimer::set_script_scene(ScriptScene *scene)
{
    this->scene_ = scene;
}

ScriptScene *ScriptScene::AIManagerTimer::scene(void)
{
    return this->scene_;
}

int ScriptScene::AIManagerTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int ScriptScene::AIManagerTimer::handle_timeout(const Time_Value &tv)
{
    JUDGE_RETURN(this->scene() != 0, 0);

    this->scene()->run_scene_record(tv);

    return 0;
}

ScriptScene::ScriptScene(void):
		s_is_inited_scene_(false),
		is_running_scene_(false),
		script_sort_(0),
        protect_npc_sort_(0),
        protect_npc_id_(0),
		generate_check_tick_(Time_Value::zero),
        tick_record_times_(0),
        chase_index_(0)
{
    this->ai_manager_timer_.set_script_scene(this);
}

ScriptScene::~ScriptScene(void)
{ /*NULL*/ }

void ScriptScene::reset(void)
{
    this->s_is_inited_scene_ = false;
    this->is_running_scene_ = false;
    this->script_sort_ = 0;
    this->protect_npc_sort_ = 0;
    this->protect_npc_id_ = 0;

    this->generate_check_tick_ = Time_Value::zero;
    this->ai_manager_timer_.cancel_timer();
    this->chase_index_ = 0;
    this->script_detail_.reset();

	this->tick_record_times_ = 0;
    for (SceneAIRecordVec::iterator iter = this->record_vec_.begin();
            iter != this->record_vec_.end(); ++iter)
    {
    	SceneAIRecord *record = *iter;
    	for (ScriptAIMap::iterator ai_iter = record->__script_ai_map.begin(); ai_iter != record->__script_ai_map.end(); ++ai_iter)
    	{
    		ScriptAI *ai = ai_iter->second;

    		MSG_USER("recycle script ai in reset:  %d %d %d %ld %d %d %d", ai->space_id(), ai->script_sort(), ai->ai_sort(),
    				ai->ai_id(), ai->is_enter_scene(), ai->scene_config_index(), ai->record_index());

    		this->push_script_ai(ai);
    	}
    	record->__script_ai_map.unbind_all();
        this->monitor()->scene_ai_record_pool()->push(*iter);
    }
    this->record_vec_.clear();
    this->script_ai_map_.unbind_all();
    this->tick_record_queue_.clear();
    this->wave_record_set_.clear();

    Scene::reset();
}

bool ScriptScene::is_running(void)
{
    return this->is_running_scene_;
}

void ScriptScene::set_script_sort(const int script_sort)
{
    this->script_sort_ = script_sort;
}

int ScriptScene::script_sort(void)
{
    return this->script_sort_;
}

int ScriptScene::init_scene(const int space_id, const int scene_id)
{
    int ret = Scene::init_scene(space_id, scene_id);
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

// 启动副本场景
void ScriptScene::run_scene(void)
{
    if (this->s_is_inited_scene_ == false)
    {
        this->init_scene_record();
        this->s_is_inited_scene_ = true;
    }

    if (this->start_scene_ == false)
    {
        this->ai_manager_timer_.schedule_timer(Time_Value(1));
        this->start_scene_ = true;
        this->run_scene_record(Time_Value::gettimeofday());
    }
}

// 挂起副本场景
void ScriptScene::holdup_scene(void)
{
    if (this->start_scene_ == true)
    {
        MSG_USER("holdup scene %d %d", this->scene_id(), this->space_id());

        this->holdup_scene_record();
        this->ai_manager_timer_.cancel_timer();
        this->start_scene_ = false;
    }
}

int ScriptScene::fetch_all_around_player(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center, int radius, const int max_amount)
{
    for (Scene::MoverMap::iterator player_iter = this->player_map_.begin();
            player_iter != this->player_map_.end(); ++player_iter)
    {
        JUDGE_CONTINUE(this->is_validate_around_mover(player_iter->second, center, radius) == true);
        GameFighter *fighter = dynamic_cast<GameFighter *>(player_iter->second);
        JUDGE_CONTINUE(fighter != 0 && fighter->is_death() == false);
        player_map[player_iter->first] = player_iter->second;
        if (int(player_map.size()) >= max_amount)
            break;
    }
    if (player_map.size() <= 0)
        return -1;

    return 0;
}
//
//int ScriptScene::fetch_all_around_fighter(Scene::MoverMap &fighter_map, const MoverCoord &center, int radius)
//{
//    int center_block_index = -1;
//    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
//    JUDGE_RETURN(ret == 0, ret);
//
//    BlockIndexSet around_set;
//    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
//    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
//    {
//        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
//        for (SceneBlock::MoverMap::iterator mover_iter = scene_block.__mover_map.begin();
//                mover_iter != scene_block.__mover_map.end(); ++mover_iter)
//        {
//            JUDGE_CONTINUE(this->is_validate_around_mover(mover_iter->second, center, radius) == true);
//            fighter_map[mover_iter->first] = mover_iter->second;
//        }
//    }
//    return 0;
//}

void ScriptScene::recycle_all_monster(void)
{
	this->tick_record_times_ = 0;
    for (SceneAIRecordVec::iterator iter = this->record_vec_.begin();
            iter != this->record_vec_.end(); ++iter)
    {
    	SceneAIRecord *record = *iter;
    	std::vector<ScriptAI *> ai_vc;
    	for (ScriptAIMap::iterator ai_iter = record->__script_ai_map.begin(); ai_iter != record->__script_ai_map.end(); ++ai_iter)
    	{
    		ai_vc.push_back(ai_iter->second);
    	}
    	for (std::vector<ScriptAI *>::iterator vc_iter = ai_vc.begin(); vc_iter != ai_vc.end(); ++vc_iter)
    	{
    		ScriptAI *ai = *vc_iter;

    		MSG_USER("recycle script all monster %d %d %d %ld %d %d %d", ai->space_id(), ai->script_sort(), ai->ai_sort(),
    				ai->ai_id(), ai->is_enter_scene(), ai->scene_config_index(), ai->record_index());

    		ai->exit_scene();
    		ai->sign_out();
    	}
    	record->__script_ai_map.unbind_all();
        this->monitor()->scene_ai_record_pool()->push(*iter);
    }
    this->record_vec_.clear();
    this->script_ai_map_.unbind_all();
    this->tick_record_queue_.clear();
    this->wave_record_set_.clear();
}

int ScriptScene::recycle_monster(ScriptAI *script_ai)
{
    int config_index = script_ai->scene_config_index(),
    		record_index = script_ai->record_index();

    if (record_index < 0 || int(this->record_vec_.size()) <= record_index)
        return 0;

    SceneAIRecord *record = this->record_vec_[record_index];
    if (script_ai->level_index() >= 0)
    {
        SceneAIRecord::BirthRecord birth_record;
        birth_record.__birth_coord = script_ai->birth_coord();
        birth_record.__level_index = script_ai->level_index() + 1;
        birth_record.__chase_index = script_ai->chase_index();
        record->__birth_record_list.push_back(birth_record);
    }
    record->__script_ai_map.unbind(script_ai->ai_id());

    MSG_USER("script scene recycle monster %d %d %d %ld %d %d", this->space_id(), this->scene_id(),
    		script_ai->ai_sort(), script_ai->ai_id(), config_index, record_index);

    return 0;
}

ScriptAI *ScriptScene::pop_script_ai(const int scene_id, const int monster_sort)
{
    return this->monitor()->script_ai_pool()->pop();
}

int ScriptScene::push_script_ai(ScriptAI *script_ai)
{
    return this->monitor()->script_ai_pool()->push(script_ai);
}

int ScriptScene::init_scene_record(void)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    const Json::Value &layout_json = scene_json["layout"];
    SceneAIRecord::BirthRecord birth_record;
    Time_Value nowtime = Time_Value::gettimeofday();

    int piece = 0, chapter = 0, floor = 0;
    BaseScript *script = this->monitor()->find_script(this->space_id());
    if (script != 0)
    {
    	floor = piece = script->piece();
        chapter = script->chapter();
    }

    for (uint i = 0; i < layout_json.size(); ++i)
    {
    	//问鼎江湖/论剑武林特殊处理
    	if (this->scene_id() == GameEnum::LEGEND_TOP_SCENE
    			|| this->scene_id() == GameEnum::SWORD_TOP_SCENE)
    	{
    		int cur_floor = i + 1;
    		JUDGE_CONTINUE(floor == cur_floor);
    	}

    	if (script->is_wave_script() == true)
    	{
    		int cur_piece = i + 1;
    		JUDGE_CONTINUE(piece <= cur_piece);
    	}

    	if (layout_json[i].isMember("chapter"))
    	{
    		int layout_chapter = layout_json[i]["chapter"].asInt();
    		JUDGE_CONTINUE(layout_chapter == chapter);
    	}

        SceneAIRecord *record = this->monitor()->scene_ai_record_pool()->pop();
        record->__config_index = i;
        record->__record_index = this->record_vec_.size();
        record->__wave_id = layout_json[i]["wave"].asInt();
        MSG_USER("record->__wave_id: %d", record->__wave_id);

        const Json::Value &point_json = layout_json[i]["point_coordxy"];
        for (uint j = 0; j < point_json.size(); ++j)
        {
        	birth_record.reset();
        	birth_record.__birth_coord.set_pixel(point_json[j][0u].asInt(), point_json[j][1u].asInt());
        	birth_record.__level_index = 0;
        	birth_record.__chase_index = this->chase_index_;
        	this->chase_index_ = (this->chase_index_ + 1) % this->max_chase_point_size_;
        	record->__birth_record_list.push_back(birth_record);
        	record->__left_fresh_sec = layout_json[i]["first_wait"].asInt();
        }
        this->record_vec_.push_back(record);
    }

    return 0;
}

int ScriptScene::run_scene_record(const Time_Value &nowtime)
{
    BaseScript *script = this->monitor()->find_script(this->space_id());
    if (script == 0 || script->is_failure_script() == true || script->is_finish_script() == true ||
    		script->is_finish_script_scene() == true || script->is_failure_script_scene() == true)
    	return 0;

    if (this->scene_id() != script->current_script_scene())
    	return 0;

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    const Json::Value &layout_json = scene_json["layout"];

    if (this->is_running() == false)
    {
        this->tick_record_queue_.clear();
        this->wave_record_set_.clear();
        int total_wave = scene_json["exec"]["wave"].asInt();
        this->wave_record_set_.resize(total_wave);

    	//　挂起的副本重新启动；初始化时间戳
        double sec = 0, usec = 0;
        for (SceneAIRecordVec::iterator iter = this->record_vec_.begin();
                iter != this->record_vec_.end(); ++iter)
        {
            SceneAIRecord *record = *iter;
            record->__fresh_tick = Time_Value::zero;
            usec = modf(record->__left_fresh_sec, &sec) * 1000000;
            record->__fresh_tick = nowtime + Time_Value(long(sec), long(usec));
            record->__left_fresh_sec = 0;

            if (record->__wave_id <= 0)
            {
                this->tick_record_queue_.push(record);
            }
            else
            {
                if (record->__wave_id > int(this->wave_record_set_.size()))
                    continue;

                this->wave_record_set_[record->__wave_id - 1].push(record);
            }
        }

        this->is_running_scene_ = true;
    }

    this->process_tick_record(layout_json);
    this->process_wave_record(layout_json, script);

    return 0;
}

int ScriptScene::holdup_scene_record(void)
{
    JUDGE_RETURN(this->is_running() == true, 0);

    Time_Value nowtime = Time_Value::gettimeofday();
    for (SceneAIRecordVec::iterator iter = this->record_vec_.begin();
            iter != this->record_vec_.end(); ++iter)
    {
        SceneAIRecord *record = *iter;
        Time_Value left_tick = record->__fresh_tick - nowtime;
        if (left_tick < Time_Value::zero)
        	left_tick = Time_Value::zero;
        record->__left_fresh_sec = double(left_tick.sec()) + double(left_tick.usec()) / 1000000.0;
        record->__fresh_tick = Time_Value::zero;
    }
    this->tick_record_queue_.clear();
    this->wave_record_set_.clear();

    this->is_running_scene_ = false;
    return 0;
}

int ScriptScene::generage_scene_monster(SceneAIRecord *record, const Json::Value &layout_monster_json)
{
    int once_num = layout_monster_json["once"].asInt(),
        max_num = layout_monster_json["max_exist"].asInt(),
        monster_sort = layout_monster_json["monster_sort"].asInt(),
        max_count = layout_monster_json["max_count"].asInt(), generated_count = 0;
    if (max_count == 0)
    	max_count = 99999999;
    if (max_num == 0)
    	max_num = max_count;

	const Json::Value &monster_json = CONFIG_INSTANCE->monster(monster_sort);
	if (monster_json == Json::Value::null)
	{
		MSG_USER("ERROR script monster sort error %d %d %d", this->script_sort(), this->scene_id(), monster_sort);
		return -1;
	}

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    int npc_sort = scene_json["exec"]["protect_npc"].asInt();

    BaseScript *script = this->monitor()->find_script(this->space_id());
    if (script != 0)
    {
        ScriptDetail::Monster &monster_info = script->script_detail().__monster;
        generated_count = monster_info.__config_monster_map[record->__config_index][monster_sort];
    }

    const Json::Value &point_json = layout_monster_json["point_coordxy"];
    int current_num = 0;
    BLongSet *sort_set = this->ai_sort_set(monster_sort);
    if (sort_set != NULL && sort_set->size() == record->__script_ai_map.size())
    {
    	// 此处可能欠妥，只是为了减少下面的循环次数
    	current_num = int(record->__script_ai_map.size());
    }
    else
    {
		for (ScriptAIMap::iterator ai_iter = record->__script_ai_map.begin();
				ai_iter != record->__script_ai_map.end(); ++ai_iter)
		{
			if (ai_iter->second != NULL && ai_iter->second->ai_sort() == monster_sort)
				++current_num;
		}
    }

    for (int i = 0; i < once_num && (i + current_num) < max_num && (i + generated_count) < max_count; ++i)
    {
        ScriptAI *script_ai = this->pop_script_ai(this->scene_id(), monster_sort);
        
        MoverCoord mover_coord;
        int level_index = 0, chase_index = this->chase_index_;
        if (record->__birth_record_list.size() <= 0)
        {
            int index = rand() % point_json.size();
            mover_coord.set_pixel(point_json[index][0u].asInt(), point_json[index][1u].asInt());
        }
        else
        {
        	SceneAIRecord::BirthRecord &birth_record = record->__birth_record_list.front();
            mover_coord = birth_record.__birth_coord;
            level_index = birth_record.__level_index;
            chase_index = birth_record.__chase_index;
            record->__birth_record_list.pop_front();
        }
        if (level_index < this->tick_record_times_)
        	level_index = this->tick_record_times_;
        if (script != 0 && script->monster_level_index() >= 0)
        {
            level_index = script->monster_level_index();
        }

        if (monster_sort == npc_sort)
        {
            script_ai->set_camp_id(GameEnum::MONSTER_CAMP_NPC);
            this->set_protect_npc(npc_sort, script_ai->ai_id());
        }
        else
        {
            script_ai->set_camp_id(GameEnum::MONSTER_CAMP_BASE);
        }

        // 把当前场景的玩家作为初始伤害列表
        if (script != 0)
        {
        	for (BLongSet::iterator iter = script->player_set().begin();
        			iter != script->player_set().end(); ++iter)
        	{
        		script_ai->hurt_map()[*iter] = 0;
        	}
        }

        script_ai->set_script_sort(this->script_sort());
        script_ai->set_scene_config_index(record->__config_index);
        script_ai->set_record_index(record->__record_index);
        script_ai->set_level_index(level_index);
        script_ai->set_chase_index(chase_index);
        script_ai->set_scene_mode(SCENE_MODE_SCRIPT);
        this->chase_index_ = (this->chase_index_ + 1) % this->max_chase_point_size_;

        if (record->__wave_id > 0)
            script_ai->set_wave_id(record->__wave_id);

        script_ai->set_layout_index(record->__config_index);
        if (script_ai->sign_in_with_scene(monster_sort, mover_coord, this) != 0)
        {
            this->push_script_ai(script_ai);
            continue;
        }
        if (script_ai->enter_scene() != 0)
        {
            script_ai->exit_scene();
            script_ai->sign_out();
            continue;
        }

        if (monster_sort == npc_sort && script != 0)
        	script->notify_npc_blood(script_ai->ai_id(), script_ai->fight_detail().__blood);

        if (record->__script_ai_map.bind(script_ai->ai_id(), script_ai) != 0)
        {
            script_ai->exit_scene();
            script_ai->sign_out();
            continue;
        }
         
        if (script != 0)
            script->sync_increase_monster(script_ai);

    	MSG_USER("generate script monster %ld %d %d %d(%d,%d) %d", script_ai->mover_id(), script_ai->ai_sort(),
    			script_ai->space_id(), script_ai->scene_id(), script_ai->location().pixel_x(),
    			script_ai->location().pixel_y(), script_ai->fight_detail().__blood);
    }

    return 0;
}

int ScriptScene::register_monster(GameMover *mover)
{
	int ret = Scene::register_monster(mover);
	JUDGE_RETURN(ret == 0, ret);

    ScriptAI *script_ai = dynamic_cast<ScriptAI *>(mover);
    if (script_ai == NULL || this->script_ai_map_.bind(script_ai->ai_id(), script_ai) != 0)
        return -1;
    return 0;

}

int ScriptScene::unregister_monster(GameMover *mover)
{
	Scene::unregister_monster(mover);
    return this->script_ai_map_.unbind(mover->mover_id());
}

int ScriptScene::update_generate_check_tick(SceneAIRecord *record, const int num)
{
    if (num == 1)
        this->generate_check_tick_ = record->__fresh_tick;
    else if (this->generate_check_tick_ > record->__fresh_tick)
        this->generate_check_tick_ = record->__fresh_tick;
    return 0;
}

int ScriptScene::summon_monster(GameFighter *caller, const Json::Value &effect_json)
{
    if (caller->is_player())
        return Scene::summon_monster(caller, effect_json);

    //超过某个上限不召唤怪物
    if(caller->is_monster() && effect_json.isMember("summon_num") && caller->alive_summon_ai_size() >= effect_json["summon_num"].asInt())
    {
    	return -1;
    }

    ScriptAI *caller_ai = dynamic_cast<ScriptAI *>(caller);
    if (caller_ai == 0)
        return -1;

    int range = effect_json["range"].asInt();
    if (range <= 0)
        range = 5;

    if (effect_json["sort"].isArray())
    {
        for (uint i = 0; i < effect_json["sort"].size(); ++i)
        {
            int monster_sort = effect_json["sort"][i].asInt();

            if (effect_json.isMember("max_summon_amount"))
            {
            	int max_summon_amount = effect_json["max_summon_amount"].asInt();
            	if (caller->alive_summon_ai_size() >= max_summon_amount)
            		return -1;
            }

            MoverCoord birth_coord = this->rand_dynamic_coord(caller_ai->location(), range);

            ScriptAI *script_ai = this->pop_script_ai(caller->scene_id(), monster_sort);
            script_ai->set_caller(caller->fighter_id());
            script_ai->set_camp_id(caller->camp_id());
            script_ai->set_self_owner(caller->fighter_id());
            script_ai->set_scene_mode(SCENE_MODE_SCRIPT);
            script_ai->set_script_sort(this->script_sort());
            script_ai->set_scene_config_index(caller_ai->scene_config_index());
            script_ai->set_record_index(caller_ai->record_index());
            script_ai->set_level_index(-1);
            script_ai->set_chase_index(caller_ai->chase_index());
            script_ai->set_wave_id(caller_ai->wave_id());
            script_ai->set_group_id(caller_ai->group_id());
            if(effect_json["no_self_attack"].asInt() != 1)
            {
            	script_ai->set_aim_object(caller_ai->aim_object_id());
            }
            else
            {
            	script_ai->set_aim_object(0);
            }

            caller->insert_summon_ai(script_ai->ai_id());

            if (script_ai->sign_in_with_sort(monster_sort, birth_coord, this) != 0)
            {
                this->push_script_ai(script_ai);
                continue;
            }

            if (effect_json["copy_blood"].asInt() == 1)
            {
            	script_ai->fight_detail().__blood = caller->fight_detail().__blood;
            }

            if (script_ai->enter_scene() != 0)
            {
                script_ai->exit_scene();
                script_ai->sign_out();
                continue;
            }

            if (script_ai->record_index() < 0 || int(this->record_vec_.size()) <= script_ai->record_index())
            {
                script_ai->exit_scene();
                script_ai->sign_out();
                continue;
            }

            SceneAIRecord *record = this->record_vec_[script_ai->record_index()];
            if (record == 0 || record->__script_ai_map.bind(script_ai->ai_id(), script_ai) != 0)
            {
                script_ai->exit_scene();
                script_ai->sign_out();
                continue;
            }

            BaseScript *script = this->monitor()->find_script(this->space_id());
            if (script != 0)
            {
            	if(effect_json.isMember("inherit_player_attr_percent"))
            	{
            		script->sync_increase_monster(script_ai, effect_json);
            	}
            	else
            	{
            		script->sync_increase_monster(script_ai);
            	}
            }
            
            MSG_USER("summon script monster %ld %d %d %d(%d,%d)", script_ai->mover_id(), script_ai->ai_sort(),
                    script_ai->space_id(), script_ai->scene_id(), script_ai->location().pixel_x(),
                    script_ai->location().pixel_y());
        }
    }
    return 0;
}


int ScriptScene::call_puppet(const int puppet_sort, MapPlayerScript *player, MoverCoord &coord, const bool is_rand/*= true*/)
{
	MoverCoord birth_coord = coord;
	if (is_rand)
	{
		int range = 5;
		birth_coord = this->rand_dynamic_coord(coord, range);
	}

    ScriptAI *script_ai = this->pop_script_ai(this->scene_id(), puppet_sort);
    script_ai->set_caller(player->fighter_id());
    script_ai->set_self_owner(player->fighter_id());
    script_ai->set_scene_mode(SCENE_MODE_SCRIPT);
    script_ai->set_script_sort(this->script_sort());
    script_ai->set_scene_config_index(0);
    script_ai->set_record_index(0);
    script_ai->set_level_index(-1);
    script_ai->set_camp_id(GameEnum::MONSTER_CAMP_NPC);

    player->insert_summon_ai(script_ai->ai_id());
    script_ai->sign_in_with_sort(puppet_sort, birth_coord, this);
    script_ai->enter_scene();

    SceneAIRecord *record = this->record_vec_[script_ai->record_index()];
    if (record != 0)
        record->__script_ai_map.bind(script_ai->ai_id(), script_ai);

    MSG_USER("script call puppet %ld %s %d %d %d %ld", player->mover_id(), player->name(),
            this->script_sort(), this->scene_id(), puppet_sort, script_ai->ai_id());

    return 0;
}

void ScriptScene::set_protect_npc(const int sort, const Int64 ai_id)
{
    this->protect_npc_sort_ = sort;
    this->protect_npc_id_ = ai_id;
}

int ScriptScene::protect_npc_sort(void)
{
    return this->protect_npc_sort_;
}

Int64 ScriptScene::protect_npc_id(void)
{
    return this->protect_npc_id_;
}

ScriptSceneDetail &ScriptScene::script_detail(void)
{
    return this->script_detail_;
}

int ScriptScene::increase_monster_amount(int sort, int amount)
{
	JUDGE_RETURN(amount > 0, -1);

    this->script_detail_.__left_monster += amount;
    this->script_detail_.__total_monster += amount;

    return 0;
}

int ScriptScene::descrease_monster_amount(int sort, int amount)
{
	int monster_num = std::abs(amount);

	this->script_detail_.__left_monster -= monster_num;
	this->script_detail_.__killed_monster += monster_num;
	JUDGE_RETURN(this->script_detail_.__left_monster <= 0, -1);

	SceneAIRecord* record = this->tick_record_queue_.pop();
	JUDGE_RETURN(record != NULL, -1);

	Time_Value nowtime = Time_Value::gettimeofday();
	if (record->__fresh_tick > nowtime)
	{
		record->__fresh_tick = nowtime;
	}

	return this->tick_record_queue_.push(record);
}

int ScriptScene::process_tick_record(const Json::Value &layout_json)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    int i = 0;
    SceneAIRecord *record = 0;
    while ((record = this->tick_record_queue_.top()) != 0)
    {
        if (i++ > 10)
            break;

        if (record->__fresh_tick > nowtime)
            break;

        this->tick_record_queue_.pop();
        if (record->__config_index < 0 || int(layout_json.size()) <= record->__config_index)
            continue;

        const Json::Value &monster_json = layout_json[record->__config_index];

        // 更新刷新间隔
        double fresh_sec = monster_json["fresh"].asDouble();
        record->__fresh_tick = GameCommon::fetch_add_time_value(fresh_sec);

        this->generage_scene_monster(record, monster_json);
        this->tick_record_queue_.push(record);
        ++this->tick_record_times_;
    }
    return 0;
}

int ScriptScene::process_wave_record(const Json::Value &layout_json, BaseScript *script)
{
    Time_Value nowtime = Time_Value::gettimeofday();

    int generate_index = script->current_wave();
    if (generate_index < 0 || int(this->wave_record_set_.size()) <= generate_index)
        return 0;

    AIRecordQueue &record_queue = this->wave_record_set_[generate_index];
    SceneAIRecord *record = record_queue.top();
    JUDGE_RETURN(record != NULL, -1);

    MSG_USER("record->__wave_id: %d, script->finish_wave(): %d", record->__wave_id, script->finish_wave());
    if (record->__fresh_tick > nowtime && record->__wave_id > (script->finish_wave() + 1))
    {
        return 0;
    }

    if ((script->is_exp_script() || script->is_league_fb_script()) && record->__wave_id > (script->finish_wave() + 1))
    {
    	return 0;
    }

    for (size_t i = 0; i < record_queue.size(); ++i)
    {
        record = record_queue.node(i);
        if (i == 0)
            script->generate_new_wave(this->scene_id(), record->__wave_id);

        if (record->__config_index < 0 || int(layout_json.size()) <= record->__config_index)
            continue;

        const Json::Value &monster_json = layout_json[record->__config_index];

        //// 更新刷新间隔 
        //double fresh_sec = monster_json["fresh"].asDouble();
        //double sec, usec;
        //usec = modf(fresh_sec, &sec) * 1000000;
        //record->__fresh_tick = nowtime + Time_Value(long(sec), long(usec));

        // 生成场景怪
        this->generage_scene_monster(record, monster_json);
    }

    if (int(this->wave_record_set_.size()) <= (generate_index + 1))
        return 0;

    this->notify_add_buff(script);

    // 更新下一波的生成时间
    AIRecordQueue &next_queue = this->wave_record_set_[generate_index + 1];
    std::vector<SceneAIRecord *> next_rec_set;
    for (size_t i = 0; i < next_queue.size(); ++i)
    {
        next_rec_set.push_back(next_queue.node(i));
    }

    for (std::vector<SceneAIRecord *>::iterator iter = next_rec_set.begin(); iter != next_rec_set.end(); ++iter)
    {
        record = *iter;
        next_queue.remove(record);
        JUDGE_CONTINUE(record->__config_index >= 0 && record->__config_index < int(layout_json.size()));

        // 更新刷新间隔
        double fresh_sec = layout_json[record->__config_index]["fresh"].asDouble();
        record->__fresh_tick = GameCommon::fetch_add_time_value(fresh_sec);

        next_queue.push(record);
    }

    return 0;
}

void ScriptScene::notify_add_buff(BaseScript *script)
{
	if (script->script_type() == GameEnum::SCRIPT_T_RAMA)
	{
		MapPlayerEx *player = 0;
		for (MoverMap::iterator iter = this->player_map_.begin();
				iter != this->player_map_.end(); ++iter)
		{
			if (this->monitor()->find_player(iter->first, player) != 0)
				continue;

			ScriptPlayerDetail::TypeRecord *type_record =
					player->type_record(script->script_type());
			JUDGE_CONTINUE(type_record->__notify_chapter > type_record->__pass_chapter);

			const Json::Value &script_json = CONFIG_INSTANCE->script(script->script_sort());
			int buff_id = script_json["buff"].asInt();
			const Json::Value &buff_json = CONFIG_INSTANCE->buff(buff_id);
//			int buff_type = buff_json["buff_type"].asInt();
			int percent = buff_json["percent"].asInt();

			BasicStatus* c_status = NULL;
			if (player->find_status(buff_id, c_status) == 0 && percent != c_status->__value2)
			{
				player->remove_status(c_status);
				player->insert_defender_status(player, buff_id, 0, 86400, 0, 0, percent);
			}
			else if (player->find_status(buff_id, c_status) != 0)
			{
				player->insert_defender_status(player, buff_id, 0, 86400, 0, 0, percent);
			}
		}
	}
}

int ScriptScene::wave_ai_size(const int wave_id)
{
	int size = 0;
	for (ScriptAIMap::iterator iter = this->script_ai_map_.begin(); iter != this->script_ai_map_.end(); ++iter)
	{
		ScriptAI *script_ai = iter->second;
		if (script_ai->wave_id() == wave_id && script_ai->is_enter_scene() == true)
			++size;
	}
	return size;
}

int ScriptScene::total_wave_ai_size(void)
{
	int size = 0;
	for (ScriptAIMap::iterator iter = this->script_ai_map_.begin();
			iter != this->script_ai_map_.end(); ++iter)
	{
		ScriptAI *script_ai = iter->second;
		JUDGE_CONTINUE(script_ai->wave_id() > 0)
		JUDGE_CONTINUE(script_ai->is_enter_scene() == true);
		++size;
	}

	return size;
}

ScriptScene::ScriptAIMap &ScriptScene::script_ai_map(void)
{
    return this->script_ai_map_;
}

int ScriptScene::force_kill_all_monster(void)
{
    for (ScriptAIMap::iterator iter = this->script_ai_map_.begin();
            iter != this->script_ai_map_.end(); ++iter)
    {
        ScriptAI *script_ai = iter->second;
        if (script_ai->is_death() || script_ai->is_enter_scene() == false)
            continue;
        script_ai->modify_blood_by_fight(script_ai->fight_detail().__blood_total_i(script_ai));
    }
    return 0;
}

int ScriptScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	return Scene::handle_ai_die(game_ai, benefited_attackor);
}
