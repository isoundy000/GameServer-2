/*
 * BaseScript.cpp
 *
 * Created on: 2013-12-13 20:03
 *     Author: lyz
 */

#include "Transaction.h"
#include "MongoDataMap.h"
#include "BaseScript.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "SceneFactory.h"
#include "GameConfig.h"
#include "ScriptScene.h"
#include "ScriptAI.h"
#include "MMOScriptProgress.h"
#include "GlobalScriptHistory.h"
#include "MapPlayerEx.h"
#include "ScriptSystem.h"

int BaseScript::check_enter_script(const int gate_sid, const int64_t role_id, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400901*, request, -1);

    MapMonitor *monitor = MAP_MONITOR;

    const Json::Value &script_json = CONFIG_INSTANCE->script(request->script_sort());
    if (script_json.empty() == true)
    {
        return monitor->dispatch_to_client_from_gate(gate_sid, role_id,
        		RETURN_REQUEST_ENTER_SCRIPT, ERROR_CONFIG_NOT_EXIST);
    }

    int src_scene = request->scene_id();
    ScriptPlayerRel *player_rel = BaseScript::init_script_player_rel(msg);
    if (player_rel == 0)
    {
    	return monitor->dispatch_to_client_from_gate(gate_sid, role_id,
    			RETURN_REQUEST_ENTER_SCRIPT, ERROR_SERVER_INNER);
    }

    player_rel->__gate_sid = gate_sid;

    bool is_finish_current_script = false;
    BaseScript *script = 0;
    ScriptPlayerRel *cur_player_rel = monitor->find_script_player_rel(player_rel->__script_sort, player_rel->__role_id);
    if (cur_player_rel == 0 || (script = monitor->find_script(cur_player_rel->__script_id)) == 0)
    {
    	// 当前进程没有进入过的副本对象
    	is_finish_current_script = true;
    	if (player_rel->__progress_id > 0)
        {
    		if (BaseScript::load_data_for_check_script_progress(player_rel) != 0)
                monitor->script_player_rel_pool()->push(player_rel);
            return 0;
        }
    }
    else
    {
    	if (script->is_finish_script() == true || script->is_failure_script() == true ||
    			(script->is_single_script() == false && script->team_id() != player_rel->__team_id) ||
    			(script->is_single_script() == true && script->owner_id() != player_rel->__role_id))
    	{
    		// 没有未完成的副本对象
    		is_finish_current_script = true;
    		if (player_rel->__progress_id > 0)
            {
    			if (BaseScript::load_data_for_check_script_progress(player_rel) != 0)
                    monitor->script_player_rel_pool()->push(player_rel);
                return 0;
            }
    	}
    }

    if (cur_player_rel != 0)
    {
    	if (is_finish_current_script == true)
    	{
    		*cur_player_rel = *player_rel;
    	}
		monitor->script_player_rel_pool()->push(player_rel);
		player_rel = cur_player_rel;
    }
    else
    {
        if (monitor->bind_script_player_rel(player_rel->__script_sort, player_rel->__role_id, player_rel) != 0)
        {
            monitor->script_player_rel_pool()->push(player_rel);
            return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ERROR_SERVER_INNER);
        }
    }

    if (is_finish_current_script == true)
    {
		// if has ScriptPlayerRel, than no check config;
		int ret = BaseScript::validate_script_config(player_rel);
		if (ret != 0)
		{
			return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ret);
		}

		player_rel->__script_id = 0;
		if (script_json["prev_condition"]["is_single"].asInt() == 1)
		{
			// if single script, fetch script direct;
			 BaseScript::fetch_single_script(player_rel, script);
		}
		else
		{
			// if team script, check all team script object;
			// init script_id, progress_id;
			ret = BaseScript::fetch_team_script(player_rel, script);
			if (ret != 0)
			{
				monitor->unbind_script_player_rel(player_rel->__script_sort, player_rel->__role_id);
				monitor->script_player_rel_pool()->push(player_rel);
				return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ret);
			}

		}
    }

    player_rel->__scene_id = src_scene;
    if (script != 0)
    {
    	player_rel->__script_id = script->script_id();
        script->insert_history_role(player_rel->__role_id, player_rel->__fresh_tick);

        if (script->is_single_script() == false)
        {
            ScriptTeamDetail *team_rel = monitor->find_script_team_rel(player_rel->__team_id);
            if (team_rel != 0)
                team_rel->__script_list.insert(script->script_id());
        }

        // load player copy
        BaseScript::request_load_player_copy(script, player_rel);

        int scene_id = 0;
        MoverCoord enter_coord;
        script->fetch_enter_scene_coord(role_id, scene_id, enter_coord);

        Proto30400902 inner_req;
        inner_req.set_script_sort(player_rel->__script_sort);
        inner_req.set_script_id(script->script_id());
        inner_req.set_scene_id(scene_id);
        inner_req.set_pixel_x(enter_coord.pixel_x());
        inner_req.set_pixel_y(enter_coord.pixel_y());
        inner_req.set_used_times(player_rel->__used_times);
        inner_req.set_progress_id(player_rel->__progress_id);
        monitor->dispatch_to_scene(gate_sid, role_id, src_scene, &inner_req);
    }

    return 0;
}

ScriptPlayerRel *BaseScript::init_script_player_rel(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400901 *, request, NULL);

    MapMonitor *monitor = MAP_MONITOR;
    ScriptPlayerRel *player_rel =  monitor->script_player_rel_pool()->pop();

    player_rel->__role_id = request->role_id();
    player_rel->__script_sort = request->script_sort();
    player_rel->__progress_id = request->progress_id();
    player_rel->__scene_id = request->scene_id();
    player_rel->__team_id = request->team_id();
    player_rel->__level = request->level();
    player_rel->__used_times = request->used_times();
    player_rel->__buy_times = request->buy_times();
    player_rel->__fresh_tick.sec(request->used_times_tick());
    player_rel->__pass_piece = request->pass_piece();
    player_rel->__pass_chapter = request->pass_chapter();
    player_rel->__piece = request->piece();
    player_rel->__chapter = request->chapter();
    player_rel->__vip_type = request->vip_type();
    player_rel->__day_online_sec = request->online_sec();
    player_rel->__cheer_num = request->cheer_num();
    player_rel->__encourage_num = request->encourage_num();

    for (int i = 0; i < request->teamer_set_size(); ++i)
    {
    	Int64 role_id = request->teamer_set(i);
    	player_rel->teamer_set_[role_id] = role_id;
    }

    for (int i = 0; i < request->replacements_set_size(); ++i)
    {
    	Int64 role_id = request->replacements_set(i);
    	player_rel->replacement_set_[role_id] = role_id;
    }

    return player_rel;
}

int BaseScript::load_data_for_check_script_progress(ScriptPlayerRel *player_rel)
{
    if (TRANSACTION_MONITOR->request_mongo_transaction(player_rel->__role_id, TRANS_LOAD_SCRIPT_PROGRESS, 
                DB_SCRIPT_PLAYER_REL, player_rel, MAP_MONITOR->script_player_rel_pool(),
                MAP_MONITOR->map_unit()) != 0)
    {
        return -1;
    }
    return 0;
}

int BaseScript::after_load_script_progress(Transaction *transaction)
{
    JUDGE_RETURN(transaction != 0, ERROR_CLIENT_OPERATE);

    TransactionData *trans_data = transaction->fetch_data(DB_SCRIPT_PLAYER_REL);
    JUDGE_RETURN(trans_data != 0, ERROR_CLIENT_OPERATE);

    ScriptPlayerRel *player_rel = trans_data->__data.__script_player_rel;
    JUDGE_RETURN(player_rel != 0, ERROR_CLIENT_OPERATE);

    MapMonitor *monitor = MAP_MONITOR;
    int gate_sid = player_rel->__gate_sid;
    Int64 role_id = player_rel->__role_id;
    
    if (transaction->is_failure())
    {
        monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, transaction->detail().__error);
        transaction->rollback();
        return 0;
    }

    trans_data->reset();
    transaction->summit();
    MMOScriptProgress::fetch_and_check_script_progress(player_rel);

    bool is_finish_current_script = false;
    BaseScript *script = 0;
    ScriptPlayerRel *cur_player_rel = monitor->find_script_player_rel(player_rel->__script_sort, player_rel->__role_id);
    if (cur_player_rel == 0 || (script = monitor->find_script(cur_player_rel->__script_id)) == 0)
    {
        // 当前进程没有进入过的副本对象
        is_finish_current_script = true;
    }
    else
    {
        if (script->is_finish_script() == true || script->is_failure_script() == true ||
                (script->is_single_script() == false && script->team_id() != player_rel->__team_id) ||
                (script->is_single_script() == true && script->owner_id() != player_rel->__role_id))
        {
            // 没有未完成的副本对象
            is_finish_current_script = true;
        }
    }

    if (cur_player_rel != 0)
    {
    	// 有副本对象,但是已经完成了,回收对象 (将已经绑定的赋值, 回收未绑定那个)
    	if (is_finish_current_script == true)
    	{
    		*cur_player_rel = *player_rel;
    	}
        monitor->script_player_rel_pool()->push(player_rel);
        player_rel = cur_player_rel;
    }
    else
    {
        if (monitor->bind_script_player_rel(player_rel->__script_sort, player_rel->__role_id, player_rel) != 0)
        {
            monitor->script_player_rel_pool()->push(player_rel);
            return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ERROR_SERVER_INNER);
        }
    }

    if (is_finish_current_script == true) // 未进入过, 生成副本对象
    {
        int ret = BaseScript::validate_script_config(player_rel);
        if (ret != 0)
        {
            return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ret);
        }

        const Json::Value &script_json = CONFIG_INSTANCE->script(player_rel->__script_sort);
        player_rel->__script_id = 0;
        if (script_json["prev_condition"]["is_single"].asInt() == 1)
        {
        	BaseScript::fetch_single_script(player_rel, script);
        }
        else
        {
            ret = BaseScript::fetch_team_script(player_rel, script);
            if (ret != 0)
            {
                monitor->unbind_script_player_rel(player_rel->__script_sort, player_rel->__role_id);
                monitor->script_player_rel_pool()->push(player_rel);
                return monitor->dispatch_to_client_from_gate(gate_sid, role_id, RETURN_REQUEST_ENTER_SCRIPT, ret);
            }
        }
    }

    if (script != 0)
    {
    	player_rel->__script_id = script->script_id();
        script->insert_history_role(player_rel->__role_id, player_rel->__fresh_tick);
        if (script->is_single_script() == false)
        {
            ScriptTeamDetail *team_rel = monitor->find_script_team_rel(player_rel->__team_id);
            if (team_rel != 0)
                team_rel->__script_list.insert(script->script_id());
        }

        // load player copy
        BaseScript::request_load_player_copy(script, player_rel);

        int scene_id = 0;
        MoverCoord enter_coord;
        script->fetch_enter_scene_coord(role_id, scene_id, enter_coord);

        Proto30400902 inner_req;
        inner_req.set_script_sort(player_rel->__script_sort);
        inner_req.set_script_id(script->script_id());
        inner_req.set_scene_id(scene_id);
        inner_req.set_pixel_x(enter_coord.pixel_x());
        inner_req.set_pixel_y(enter_coord.pixel_y());
        inner_req.set_used_times(player_rel->__used_times);
        inner_req.set_progress_id(player_rel->__progress_id);
        monitor->dispatch_to_scene(gate_sid, role_id, player_rel->__scene_id, &inner_req);
    }

    return 0;
}

int BaseScript::validate_script_config(ScriptPlayerRel *player_rel)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(player_rel->__script_sort);
    JUDGE_RETURN(script_json.empty() == false, ERROR_CONFIG_NOT_EXIST);

//    if (player_rel->__script_sort / 1000 == 29)
//    {
//    	JUDGE_RETURN(BaseScript::validate_seven_script_time() == 0, ERROR_SEVEN_SCRIPT_OPEN_TIME);
//    }

//    if(GameCommon::is_monster_tower_script(player_rel->__script_sort) == 1)
//    {
//    	JUDGE_RETURN(GameCommon::validate_monster_tower_script_time() == 0, ERROR_MONSTER_TOWER_SCRIPT_OPEN_TIME);
//    }

    const Json::Value &prev_json = script_json["prev_condition"];
    JUDGE_RETURN(player_rel->__level >= prev_json["level"].asInt(), ERROR_SELF_LEVEL_LIMIT);

    int vip_type = player_rel->__vip_type;

    int finish_times = prev_json["finish_times"].asInt();
    finish_times += GameCommon::script_vip_extra_use_times(vip_type, player_rel->__script_sort);

    JUDGE_RETURN(player_rel->__used_times < (finish_times + player_rel->__buy_times), ERROR_SCRIPT_FINISH_TIMES);

    if (prev_json.isMember("online_tick"))
    {
        JUDGE_RETURN(player_rel->__day_online_sec >= prev_json["online_tick"].asInt(), ERROR_PLAYER_ONLINE_LIMIT);
    }

    if (prev_json.isMember("piece") == true)
    {
    	int piece = player_rel->__piece, pass_piece = player_rel->__pass_piece,
    			chapter = player_rel->__chapter, pass_chapter = player_rel->__pass_chapter;
        JUDGE_RETURN(piece > 0 && piece <= pass_piece + 1 &&
        		piece <= int(prev_json["piece"].size()), ERROR_CLIENT_OPERATE);

        if (piece > pass_piece)
        {
        	// 新的篇，从第一章开始
        	JUDGE_RETURN(chapter == 1, ERROR_NOPASS_LASS_CHAPTER);
        }
        else if (piece == pass_piece)
        {
        	// 正在进行中的篇
        	if (chapter > pass_chapter)
        	{
        		// 进行该篇的新章节
				int max_chapter = prev_json["piece"][pass_piece - 1].asInt();
				JUDGE_RETURN(0 < chapter && chapter <= (pass_chapter + 1) && chapter <= max_chapter, ERROR_NOPASS_LASS_CHAPTER);
        	}
        }
        else
        {
        	// 进行旧的篇
        	int max_chapter = prev_json["piece"][piece - 1].asInt();
        	JUDGE_RETURN(0 < chapter && chapter <= max_chapter, ERROR_NOPASS_LASS_CHAPTER);
        }

        bool is_chapter_level = false;
        const Json::Value &chapter_scene_json = prev_json["chapter_scene"];
        for (uint i = 0; i < chapter_scene_json.size(); ++i)
        {
            if (chapter_scene_json[i][0u].asInt() != piece)
                continue;
            if (chapter < chapter_scene_json[i][1u].asInt() || chapter_scene_json[i][2u].asInt() < chapter)
                continue;
            
            JUDGE_RETURN(player_rel->__level >= chapter_scene_json[i][4u].asInt(), ERROR_PLAYER_LEVEL_LIMIT);
            
            is_chapter_level = true;
            break;
        }
        JUDGE_RETURN(is_chapter_level == true, ERROR_CONFIG_NOT_EXIST);
    }

    if (prev_json["is_single"].asInt() == 0)
    {
        JUDGE_RETURN(player_rel->__team_id > 0, ERROR_NO_TEAM);
    }

    return 0;
}

int BaseScript::fetch_single_script(ScriptPlayerRel *player_rel, BaseScript *&script)
{
    script = MAP_MONITOR->script_factory()->pop_script(player_rel->__script_sort);
    script->init_for_single(player_rel);
    return 0;
}

int BaseScript::fetch_team_script(ScriptPlayerRel *player_rel, BaseScript *&script)
{
    script = 0;
    MapMonitor *monitor = MAP_MONITOR;

    ScriptTeamDetail* team_rel = monitor->find_script_team_rel(player_rel->__team_id);
    if (team_rel == 0)
    {
        team_rel = monitor->script_team_detail_pool()->pop();
        team_rel->__team_id = player_rel->__team_id;
        if (monitor->bind_script_team_rel(team_rel->__team_id, team_rel) != 0)
        {
            monitor->script_team_detail_pool()->push(team_rel);
            return 0;
        }

//        for (LongMap::iterator iter = player_rel->teamer_set_.begin();
//        		iter != player_rel->teamer_set_.end(); ++iter)
//        {
//        	team_rel->__teamer_list.push_back(iter->first);
//        }
    }

    const Json::Value &script_json = CONFIG_INSTANCE->script(player_rel->__script_sort);
    const Json::Value &prev_json = script_json["prev_condition"];

    for (LongList::iterator iter = team_rel->__teamer_list.begin();
            iter != team_rel->__teamer_list.end(); ++iter)
    {
        Int64 teamer_id = *iter;
        ScriptPlayerRel *tmp_player_rel = monitor->find_script_player_rel(player_rel->__script_sort, teamer_id);
        JUDGE_CONTINUE(tmp_player_rel != NULL);

        BaseScript *tmp_script = monitor->find_script(tmp_player_rel->__script_id);
        if (tmp_script != 0 && tmp_script->is_finish_script() == false && 
                tmp_script->is_failure_script() == false &&
                tmp_script->player_set().size() < GameEnum::MAX_TEAMER_COUNT)
        {
        	if (prev_json.isMember("max_teamer") && int(tmp_script->player_set().size()) >= prev_json["max_teamer"].asInt())
        		continue;
            
            script = tmp_script;
            break;
        }
    }
    if (script == 0)
    {
        script = monitor->script_factory()->pop_script(player_rel->__script_sort);

        int ret = script->init_for_team(player_rel);
        if (ret != 0)
        {
            script->recycle_self();
            return ret;
        }
    }
    
    player_rel->__script_id = script->script_id();
    team_rel->__script_list.insert(script->script_id());
    team_rel->__teamer_list.push_back(player_rel->__role_id);
    return 0;
}

int BaseScript::request_load_player_copy(BaseScript* script, ScriptPlayerRel* player_rel)
{
	JUDGE_RETURN(script->is_single_script() == false, -1);

	int scene_id = 0;
	MoverCoord location;
	script->fetch_enter_scene_coord(0, scene_id, location);

	Scene* scene = NULL;
	JUDGE_RETURN(MAP_MONITOR->find_scene(script->script_id(), scene_id, scene) == 0, -1);

	ScriptDetail& script_detail = script->script_detail();
	script_detail.teamer_set_ = player_rel->teamer_set_;

	for (LongMap::iterator iter = player_rel->replacement_set_.begin();
			iter != player_rel->replacement_set_.end(); ++iter)
	{
		Int64 role_id = iter->first;
		JUDGE_CONTINUE(script_detail.src_replacements_set_.count(role_id) == 0);

		DBShopMode* shop_mode = GameCommon::pop_shop_mode();
		JUDGE_CONTINUE(shop_mode != NULL);

		MapPlayerEx* copy_player = MAP_MONITOR->player_pool()->pop();
		JUDGE_CONTINUE(copy_player != NULL);

		MoverDetail& mover_detail = copy_player->mover_detail();
		mover_detail.__location = location;

		copy_player->set_scene_mode(SCENE_MODE_SCRIPT);
		copy_player->init_mover_scene(scene);

		shop_mode->recogn_ = TRANS_LOAD_COPY_PLAYER;
		shop_mode->input_argv_.type_int64_ = role_id;
		shop_mode->output_argv_.type_void_ = copy_player;

		MAP_MONITOR->db_map_load_mode_begin(shop_mode);
		script_detail.src_replacements_set_[role_id] = true;
	}

	return 0;
}

int BaseScript::start_copy_player(DBShopMode* shop_mode)
{
	MapPlayerEx* player = (MapPlayerEx*) shop_mode->output_argv_.type_void_;

	if (player->start_offline_copy(*shop_mode->output_argv_.bson_obj_) != 0)
	{
		player->sign_out(false);
	}
	else if (shop_mode->output_argv_.bson_vec_->empty() == false)
	{
		player->start_beast_offline(*(shop_mode->output_argv_.bson_vec_->begin()));
	}

	return 0;
}

BaseScript::ScriptTimer::ScriptTimer(void) : script_(0)
{ /*NULL*/ }

BaseScript::ScriptTimer::~ScriptTimer(void)
{ /*NULL*/ }

int BaseScript::ScriptTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int BaseScript::ScriptTimer::handle_timeout(const Time_Value &nowtime)
{
    return this->script_->process_script_timeout(nowtime);
}

int BaseScript::global_script_id_ = 0;
BaseScript::BaseScript(void)
{
    this->script_id_ = (++BaseScript::global_script_id_);
    this->monitor_ = MAP_MONITOR;
    this->script_timer_.script_ = this;
    BaseScript::reset();
}

BaseScript::~BaseScript(void)
{ /*NULL*/ }

void BaseScript::reset(void)
{
    this->script_type_ = 0;
    this->timer_stage_ = 0;
    this->save_enter_teamers_ = 0;
    this->couple_flag_ = 0;

    this->script_timer_.cancel_timer();
    this->failure_check_tick_ = Time_Value::zero;
    this->couple_check_tick_  = Time_Value::zero;

    this->first_role_.clear();
    this->history_role_.clear();
    this->script_detail_.reset();

    for (BaseScript::SceneMap::iterator iter = this->scene_map_.begin();
            iter != this->scene_map_.end(); ++iter)
    {
    	ScriptScene *scene = iter->second;
    	MSG_USER("recycle scene %d %d %d %x", scene->space_id(), scene->scene_id(), this->script_id(), scene);

    	scene->holdup_scene();
    	this->monitor()->unbind_scene(scene->space_id(), scene->scene_id());
        this->monitor()->push_scene(iter->second);
    }
    this->scene_map_.unbind_all();
}

MapMonitor *BaseScript::monitor(void)
{
    return this->monitor_;
}

MapPlayerEx* BaseScript::find_player(Int64 role_id)
{
	return this->monitor()->find_map_player(role_id);
}

const Json::Value& BaseScript::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->current_script_scene());
}

const Json::Value& BaseScript::script_conf()
{
	return CONFIG_INSTANCE->script(this->script_sort());
}

int BaseScript::fetch_pass_total_reward(ThreeObjVec& reward_vec, int flag)
{
	const Json::Value& finish_json = this->script_conf()["finish_condition"];

	//日常奖励
	if (this->is_wave_script() == false)
	{
		int normal_reward = this->fetch_normal_reward();
		reward_vec.push_back(ThreeObj(normal_reward));
	}

	//跨服奖励
	if (finish_json.isMember("travel_award_item") == true)
	{
		int normal_reward = 0;
		int total = finish_json["travel_award_item"].size();

		for (int i = 0; i < total; ++i)
		{
			JUDGE_CONTINUE(flag <= finish_json["travel_award_item"][i][0u].asInt());
			normal_reward = finish_json["travel_award_item"][i][1u].asInt();
			break;
		}

		reward_vec.push_back(ThreeObj(normal_reward));
	}

	//掉落奖励
	return this->fetch_drop_reward(reward_vec);
}

int BaseScript::fetch_drop_reward(ThreeObjVec& reward_vec)
{
    const Json::Value &script_json = this->script_conf();
    JUDGE_RETURN(script_json["finish_condition"].isMember("drop_item") == true, -1);

    const Json::Value &drop_item = script_json["finish_condition"]["drop_item"];
	for (uint i = 0; i < drop_item.size(); ++i)
	{
		ThreeObj obj(drop_item[i][1u].asInt());
		reward_vec.push_back(obj);
	}

	return 0;
}

int BaseScript::fetch_reward_index()
{
	return 0;
}

int BaseScript::fetch_first_reward(ThreeObjVec& reward_vec)
{
	const Json::Value &script_json = this->script_conf();
    JUDGE_RETURN(script_json.empty() == false, -1);

    int reward_id = script_json["finish_condition"]["first_pass_award"].asInt();
    reward_vec.push_back(ThreeObj(reward_id, true));

    return 0;
}

int BaseScript::fetch_normal_reward()
{
	const Json::Value &script_json = this->script_conf();
    JUDGE_RETURN(script_json.empty() == false, -1);

    return script_json["finish_condition"]["award_item"].asInt();
}

int BaseScript::fetch_wave_reward(ThreeObjVec& reward_vec)
{
	return 0;
}

int BaseScript::script_pass_chapter()
{
	return 0;
}

int BaseScript::sync_restore_pass(MapPlayerScript* player)
{
	return 0;
}

ScriptScene *BaseScript::fetch_scene(const int scene_id)
{
    Scene *scene = 0;
    if (this->monitor()->find_scene(this->script_id(), scene_id, scene) == 0)
        return dynamic_cast<ScriptScene *>(scene);
    return 0;
}

ScriptScene *BaseScript::fetch_current_scene(void)
{
    Scene *scene = 0;
    if (this->monitor()->find_scene(this->script_id(), this->current_script_scene(), scene) == 0)
        return dynamic_cast<ScriptScene *>(scene);
    return 0;
}

int BaseScript::script_type(void)
{
    return this->script_type_;
}

int BaseScript::set_script_type(const int type)
{
    this->script_type_ = type;
    return type;
}

bool BaseScript::is_exp_script(void)
{
    return this->script_type() == GameEnum::SCRIPT_T_EXP;
}
bool BaseScript::is_rama_script(void)
{
    return this->script_type() == GameEnum::SCRIPT_T_RAMA;
}

bool BaseScript::is_league_fb_script(void)
{
	return this->script_type() == GameEnum::SCRIPT_T_LEAGUE_FB;
}

bool BaseScript::is_climb_tower_script(void)
{
    return this->script_type() == GameEnum::SCRIPT_T_CLIMB_TOWER;
}

bool BaseScript::is_top_script(void)
{
	return this->script_type() == GameEnum::SCRIPT_T_LEGEND_TOP
			|| this->script_type() == GameEnum::SCRIPT_T_SWORD_TOP;
}

bool BaseScript::is_upclass_script(void)
{
    return this->script_type() == GameEnum::SCRIPT_T_UPCLASS;
}

bool BaseScript::is_tower_defense_script(void)
{
    return this->script_type() == GameEnum::SCRIPT_T_TOWER_DEFENSE;
}

bool BaseScript::is_monster_tower_script(void)
{
	return (this->script_type() == GameEnum::SCRIPT_T_MONSTER_TOWER);
}

bool BaseScript::is_couble_script(void)
{
	return this->script_type() == GameEnum::SCRIPT_T_COUPLES;
}

ScriptDetail &BaseScript::script_detail(void)
{
    return this->script_detail_;
}

int BaseScript::script_id(void)
{
    return this->script_id_;
}

int BaseScript::script_sort(void)
{
    return this->script_detail_.__script_sort;
}

Int64 BaseScript::progress_id(void)
{
    return this->script_detail_.__progress_id;
}

int BaseScript::monster_level_index(void)
{
    return this->script_detail_.__scene.__monster_level_index;
}

Int64 BaseScript::owner_id(void)
{
	return this->script_detail_.__owner_id;
}

void BaseScript::set_owner_id(const Int64 id)
{
	this->script_detail_.__owner_id = id;
}

void BaseScript::set_single_script(const bool flag)
{
    this->script_detail_.__is_single = flag;
}

// 是否单人副本
bool BaseScript::is_single_script(void)
{
    return this->script_detail_.__is_single;
}

int BaseScript::team_id(void)
{
    return this->script_detail_.__team.__team_id;
}

void BaseScript::reset_all_scene_flag(void)
{
    this->script_detail_.__scene.__scene_flag_set.reset();
}

void BaseScript::set_scene_flag(const string& flag_str)
{
    if (flag_str == "kill_all")
    {
        this->set_scene_flag(GameEnum::SCRIPT_SF_KILL_ALL);
    }

    if (flag_str == "alive")
    {
    	this->set_scene_flag(GameEnum::SCRIPT_SF_ALL_RELIVE);
    }

    if (flag_str == "couple")
    {
    	this->set_scene_flag(GameEnum::SCRIPT_SF_COUPLES);
    }
}

void BaseScript::set_scene_flag(const int flag)
{
    JUDGE_RETURN(flag >= 0 && flag < GameEnum::SCRIPT_SF_END, ;);
    this->script_detail_.__scene.__scene_flag_set.set(flag);
}

void BaseScript::reset_scene_flag(const int flag)
{
	JUDGE_RETURN(flag >= 0 && flag < GameEnum::SCRIPT_SF_END, ;);
    this->script_detail_.__scene.__scene_flag_set.reset(flag);
}

bool BaseScript::test_scene_flag(const int flag)
{
    JUDGE_RETURN(flag >= 0 && flag < GameEnum::SCRIPT_SF_END, false);
    return this->script_detail_.__scene.__scene_flag_set.test(flag);
}

BLongSet &BaseScript::player_set(void)
{
    return this->script_detail_.__player_set;
}

int BaseScript::current_script_scene(void)
{
    return this->script_detail_.__scene.__cur_scene;
}

int BaseScript::current_script_floor(void)
{
    return this->script_detail_.__scene.__cur_floor;
}

int BaseScript::last_script_scene(void)
{
    return this->script_detail_.__scene.__last_scene;
}

int BaseScript::last_script_floor(void)
{
    return this->script_detail_.__scene.__last_floor;
}

int BaseScript::next_script_scene(void)
{
    return this->script_detail_.__scene.__next_scene;
}

int BaseScript::next_script_floor(void)
{
    return this->script_detail_.__scene.__next_floor;
}

int BaseScript::passed_script_scene(void)
{
    return this->script_detail_.__scene.__passed_scene;
}

int BaseScript::passed_script_floor(void)
{
    return this->script_detail_.__scene.__passed_floor;
}

int BaseScript::piece(void)
{
    return this->script_detail_.__piece.__piece;
}

int BaseScript::chapter(void)
{
    return this->script_detail_.__piece.__chapter;
}

int BaseScript::chapter_key(void)
{
    return this->piece() * 1000 + this->chapter();
}

int BaseScript::floor_id(void)
{
	return this->script_detail_.__piece.__piece;
}

int BaseScript::finish_tick(void)
{
	return this->script_detail_.__scene.__notify_finish_tick;
}

const Time_Value &BaseScript::monster_fail_tick(void)
{
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    return scene_info.__monster_failure_tick;
}

const Time_Value &BaseScript::monster_amount_notify_tick(void)
{
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    return scene_info.__monster_fail_notify_tick;
}

bool BaseScript::is_holdup(void)
{
    return (this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == false);
}

bool BaseScript::is_finish_script_scene(void)
{
    return this->test_scene_flag(GameEnum::SCRIPT_SF_FINISH_SCENE);
}

bool BaseScript::is_failure_script_scene(void)
{
    return this->test_scene_flag(GameEnum::SCRIPT_SF_FAILURE_SCENE);
}

bool BaseScript::is_finish_script(void)
{
    return this->test_scene_flag(GameEnum::SCRIPT_SF_FINISH_SCRIPT);
}

bool BaseScript::is_failure_script(void)
{
    return this->test_scene_flag(GameEnum::SCRIPT_SF_FAILURE_SCRIPT);
}

bool BaseScript::is_self_script(MapPlayerScript* player)
{
	JUDGE_RETURN(player != NULL, false);

	if (this->is_single_script() == false)
	{
		return player->team_id() == this->team_id();
	}
	else
	{
		return player->role_id() == this->owner_id();
	}
}

Time_Value &BaseScript::recycle_tick(void)
{
    return this->script_detail_.__recycle_tick;
}

int BaseScript::left_scene_ready_tick(void)
{
    ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
    return GameCommon::left_time(tick_info.__ready_tick);
}

int BaseScript::total_scene_tick(void)
{
    ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
    return tick_info.__total_sec + tick_info.__inc_sec;
}

int BaseScript::left_scene_tick(void)
{
	return this->total_scene_tick() - this->used_scene_tick();
}

int BaseScript::used_scene_tick(void)
{
    ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
    return ::time(NULL) - tick_info.__begin_tick;
}

int BaseScript::total_used_script_tick(void)
{
    return this->script_detail_.__total_used_tick;
}

int BaseScript::reset_left_tick(void)
{
	ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
	tick_info.__begin_tick = ::time(NULL);
	return 0;
}

int BaseScript::current_wave(void)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    return wave_info.__cur_wave;
}

int BaseScript::total_wave(void)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    return wave_info.__total_wave;
}

int BaseScript::pass_wave(void)
{
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	return wave_info.__pass_wave;
}

int BaseScript::finish_wave(void)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    return wave_info.__finish_wave;
}

int BaseScript::begin_wave(void)
{
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	return wave_info.__begin_wave;
}

int BaseScript::script_status(void)
{
    if (this->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE))
    {
    	return 3;	//准备状态
    }

    if (this->is_holdup() == true)
    {
    	return 1;	//挂起
    }

    return 2;	//正在进行
}

int BaseScript::script_finish_flag()
{
	return this->is_finish_script() == true ? BaseScript::WIN : BaseScript::LOSE;
}

int BaseScript::fetch_task_wave()
{
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	return wave_info.__finish_wave;
}

int BaseScript::left_relive_amount(const Int64 role_id)
{
    return this->script_detail_.__relive.__left_relive;
}

int BaseScript::total_relive_amount(const Int64 role_id)
{
    return this->script_detail_.__relive.__total_relive;
}

int BaseScript::used_relive_amount(const Int64 role_id)
{
    return this->script_detail_.__relive.__used_relive;
}

int BaseScript::used_relive_amout_by_item(const Int64 role_id)
{
	return this->script_detail_.__relive.__used_item_relive;
}

int BaseScript::reduce_relive_times(const Int64 role_id, const int relive_mode)
{
    if (this->script_detail_.__relive.__left_relive > 0)
        --this->script_detail_.__relive.__left_relive;

    ++this->script_detail_.__relive.__used_relive;
    if (relive_mode == GameEnum::RELIVE_LOCATE)
    	++this->script_detail_.__relive.__used_item_relive;

    this->notify_update_relive_rec();
    return 0;
}

int BaseScript::left_monster_amount(void)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    return monster_info.__left_monster;
}

int BaseScript::total_monster_amount(void)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    return monster_info.__total_monster;
}

int BaseScript::kill_monster_amount(void)
{
	ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
	return std::max<int>(monster_info.__total_monster - monster_info.__left_monster, 0);
}

int BaseScript::top_evencut(void)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    return monster_info.__max_evencut;
}

ScriptAI *BaseScript::fetch_protect_npc(void)
{
	Scene *scene = this->fetch_scene(this->current_script_scene());
	ScriptAI *game_ai = 0;
	GameFighter *fighter = 0;
	if (scene->find_fighter(scene->protect_npc_id(), fighter) == 0 &&
			(game_ai = dynamic_cast<ScriptAI *>(fighter)) != 0)
		return game_ai;
	return 0;
}

void BaseScript::set_script_recycle_time(int delay)
{
	this->script_detail_.__recycle_tick = GameCommon::fetch_add_time_value(delay);
}

int BaseScript::fetch_monster_coord(MoverCoord &coord)
{
	const Json::Value &scene_json = this->scene_conf();
	const Json::Value &script_json = this->script_conf();
	int wave = script_json["scene"][0u]["exec"]["wave"].asInt();

	int finish_wave = this->finish_wave();
	if (finish_wave >= wave)
	{
		const Json::Value &transfer = script_json["finish_condition"]["transfer"];
		coord.set_pixel(transfer["posX"].asInt(), transfer["posY"].asInt());
	}
	else
	{
		const Json::Value &layout_json = scene_json["layout"][finish_wave];
		const Json::Value &point_coordxy = layout_json["point_coordxy"];
		coord.set_pixel(point_coordxy[0u][0u].asInt(), point_coordxy[0u][1u].asInt());
	}

	return 0;
}
void BaseScript::check_recycle_team_info()
{
    JUDGE_RETURN(this->is_single_script() == false, ;);

	ScriptTeamDetail* team_rel = this->monitor()->find_script_team_rel(this->team_id());
	JUDGE_RETURN(team_rel != NULL, ;);

	team_rel->__script_list.erase(this->script_id());
	this->monitor()->unbind_script_team_rel(this->team_id());
	this->monitor()->script_team_detail_pool()->push(team_rel);
}

void BaseScript::recycle_self(void)
{
    for (BaseScript::SceneMap::iterator iter = this->scene_map_.begin();
            iter != this->scene_map_.end(); ++iter)
    {
    	ScriptScene*scene = iter->second;
    	JUDGE_CONTINUE(scene != NULL);
    	scene->recycle_all_monster();
    }

	this->check_recycle_team_info();
    this->monitor()->unbind_script(this->script_id());

    ScriptDetail::Team &team_info = this->script_detail_.__team;
    for (BLongSet::iterator iter = team_info.__history_role.begin();
            iter != team_info.__history_role.end(); ++iter)
    {
        ScriptPlayerRel* player_rel = this->monitor()->find_script_player_rel(this->script_sort(), *iter);
        JUDGE_CONTINUE(player_rel != NULL && player_rel->__script_id == this->script_id());

        this->monitor()->unbind_script_player_rel(this->script_sort(), *iter);
        this->monitor()->script_player_rel_pool()->push(player_rel);
    }

    this->recycle_copy_player();
    this->recycle_self_to_pool();
}

int BaseScript::init_script_base(ScriptPlayerRel *player_rel)
{
    int script_sort = player_rel->__script_sort;
    Int64 progress_id = player_rel->__progress_id;

    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    this->set_script_type(script_json["type"].asInt());
    this->script_detail_.__script_sort = script_sort;
    this->script_detail_.__progress_id = progress_id;
    this->script_detail_.__cheer_num = player_rel->__cheer_num;
    this->script_detail_.__encourage_num = player_rel->__encourage_num;
    this->set_owner_id(player_rel->__role_id);

    if (script_json["prev_condition"].isMember("stop_times"))
    {
    	this->script_detail_.__stop_times = script_json["prev_condition"]["stop_times"].asInt();
    }
    else
    {
    	this->script_detail_.__stop_times = CONFIG_INSTANCE->script_stop_times();
    }

    this->script_detail_.__max_monster_num = script_json["prev_condition"]["monster_max_fail"].asInt();
    if (this->script_detail_.__max_monster_num <= 0)
        this->script_detail_.__max_monster_num = CONFIG_INSTANCE->script_monster_max();

    ScriptDetail::Relive &relive_info = this->script_detail_.__relive;
    if (script_json["relive"].isMember("relive_times"))
    {
        // 初始化复活次数
        relive_info.__left_relive = script_json["relive"]["relive_times"].asInt();
        relive_info.__total_relive = script_json["relive"]["relive_times"].asInt();
    }
    else
    {
        // 无限次复活
        relive_info.__left_relive = -1;
        relive_info.__total_relive = -1;
    }
    // 不需要复活
    if (script_json["relive"]["no_relive"].asInt() == 1)
    {
        relive_info.__left_relive = 0;
        relive_info.__total_relive = 0;
    }

    ScriptDetail::Piece &piece_info = this->script_detail_.__piece;
    piece_info.__piece = player_rel->__piece;
    piece_info.__chapter = player_rel->__chapter;

    Int64 progress = this->monitor()->script_progress_id(this->script_sort());
    this->script_detail_.__progress_id = this->script_sort() * 1000000L + progress % 999999L + 1;
    this->monitor()->update_script_progress_id(this->script_sort(), this->progress_id());

    this->request_save_script_progress();
    this->set_script_recycle_time(Time_Value::HOUR);
    this->failure_check_tick_ = GameCommon::fetch_add_time_value(Time_Value::MINUTE);
    this->couple_check_tick_  = GameCommon::fetch_add_time_value(10);
    this->script_timer_.schedule_timer(1);

    return 0;
}

// bind script to monitor;
int BaseScript::init_for_single(ScriptPlayerRel *player_rel)
{
    this->set_single_script(true);
    this->init_script_base(player_rel);
    this->init_script_scene();

    //启动副本定时器
    this->monitor()->bind_script(this->script_id(), this);

    player_rel->__script_id = this->script_id();
    return 0;
}

// bind script to monitor;
int BaseScript::init_for_team(ScriptPlayerRel *player_rel)
{
	int team_id = player_rel->__team_id;

    if (this->monitor()->bind_script(this->script_id(), this) != 0)
    {
        return ERROR_SERVER_INNER;
    }

    this->script_detail_.__team.__team_id = team_id;
    this->init_script_base(player_rel);

    int ret = this->init_script_scene();
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int BaseScript::insert_history_role(const int64_t role_id, const Time_Value &used_times_tick)
{
    ScriptDetail::Team &team_info = this->script_detail_.__team;
    team_info.__history_role.insert(role_id);
    if (team_info.__used_times_tick_map.find(role_id) == 
            team_info.__used_times_tick_map.end())
        team_info.__used_times_tick_map[role_id] = used_times_tick.sec();

    return 0;
}


int BaseScript::fetch_enter_scene_coord(const int64_t role_id, int &scene_id, MoverCoord &enter_coord)
{
    scene_id = this->current_script_scene();
    return this->relive_coord_from_config_point(role_id, enter_coord);
}

int BaseScript::relive_coord_from_config_point(const int64_t role_id, MoverCoord &relive_coord)
{
	const Json::Value &scene_json = this->scene_conf();

	uint finish_wave = this->finish_wave();
	const Json::Value &layout_json = scene_json["layout"][finish_wave];
	if (layout_json.isMember("born_point"))
	{
		relive_coord.set_pixel(layout_json["born_point"][0u].asInt(),
					layout_json["born_point"][1u].asInt());
		return 0;
	}

	const Json::Value* p_relive_json = NULL;
	if (scene_json.isMember("relive") == false)
	{
		p_relive_json = &this->script_conf()["relive"];
	}
	else
	{
		p_relive_json = &scene_json["relive"];
	}

	const Json::Value& relive_json = *p_relive_json;
	if (relive_json.isMember("posX"))
	{
		relive_coord.set_pixel(relive_json["posX"].asInt(),
				relive_json["posY"].asInt());
	}
	else
	{
		relive_coord.set_pixel(relive_json["fixed_pos"][0u].asInt(),
				relive_json["fixed_pos"][1u].asInt());
	}

    return 0;
}

int BaseScript::init_script_scene(void)
{
    this->init_script_all_scenes();
    this->init_script_first_scene();

    const Json::Value &prev_json = this->script_conf()["prev_condition"];
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->current_script_scene());

    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    scene_info.__monster_failure_tick = Time_Value::zero;

    //问鼎江湖/论剑武林不需要在这里设置floor
//    if (this->script_sort() != GameEnum::SCRIPT_SORT_LEGEND_TOP)
    if (prev_json["is_tower"] == false)
    {
    	scene_info.__cur_floor = scene_json["floor"].asInt();
    }

    this->init_scene_exec_condition(scene_json);
    return 0;
}

int BaseScript::init_script_all_scenes()
{
    const Json::Value &scenes_json = this->script_conf()["scene"];
    for (uint i = 0; i < scenes_json.size(); ++i)
    {
        const Json::Value &scene_json = scenes_json[i];
        int scene_id = scene_json["scene_id"].asInt();
        ScriptScene *scene = dynamic_cast<ScriptScene *>(this->monitor()->pop_scene(scene_id));
        if (scene == 0)
        {
            MSG_USER("ERROR no scene %d", scene_id);
            return ERROR_SERVER_INNER;
        }

        scene->set_script_sort(this->script_sort());
        scene->init_scene(this->script_id(), scene_id);

        if (this->monitor()->bind_scene(this->script_id(), scene_id, scene) != 0)
        {
            this->monitor()->push_scene(scene);
            MSG_USER("ERROR bind scene %d %d", this->script_id(), scene_id);
            return ERROR_SERVER_INNER;
        }

        this->scene_map_.bind(scene_id, scene);
    }
	return 0;
}

int BaseScript::init_script_first_scene(void)
{
    const Json::Value &script_json = this->script_conf();

    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    scene_info.__cur_scene = script_json["scene"][0u]["scene_id"].asInt();

    return 0;
}

int BaseScript::init_scene_exec_condition(const Json::Value &scene_json)
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;

    this->reset_all_scene_flag();
    monster_info.reset_everytime();

    const Json::Value &exec_json = scene_json["exec"];
    if (exec_json.isMember("last_tick") || exec_json.isMember("ready_tick"))
    {
        this->set_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE);

        ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
        tick_info.reset();
        tick_info.__total_sec = exec_json["last_tick"].asDouble();
        tick_info.__ready_sec = exec_json["ready_tick"].asDouble();
    }
    if (exec_json.isMember("flag"))
    {
        const Json::Value &flag_json = exec_json["flag"];
        for (uint i = 0; i < flag_json.size(); ++i)
        {
            string flag_str = flag_json[i].asString();
            this->set_scene_flag(flag_str);
        }
    }
    if (exec_json.isMember("evencut"))
    {
        this->set_scene_flag(GameEnum::SCRIPT_SF_EVENCUT);
    }
    if (exec_json.isMember("protect_npc"))
    {
        this->set_scene_flag(GameEnum::SCRIPT_SF_PROTECT_NPC);
        this->script_detail_.__protect_npc_sort = exec_json["protect_npc"].asInt();
    }
    if (exec_json.isMember("puppet"))
    {
        wave_info.__active_puppet_flag.clear();
        for (uint i = 0; i < exec_json["puppet"].size(); ++i)
        {
            wave_info.__active_puppet_flag.push_back(0);
        }
    }

    bool is_has_flag = false;
    if (exec_json.isMember("monster"))
    {
    	is_has_flag = true;
        this->set_scene_flag(GameEnum::SCRIPT_SF_KILL_MONSTER);

        const Json::Value &monster_json = exec_json["monster"];
        for (uint i = 0; i < monster_json.size(); ++i)
        {
            int sort = monster_json[i][0u].asInt();
            int num = monster_json[i][1u].asInt();
            monster_info.__monster_total_map[sort] = num;
        }
    }
    if (exec_json.isMember("wave"))
    {
    	is_has_flag = true;
        this->set_scene_flag(GameEnum::SCRIPT_SF_WAVE);

        wave_info.reset_everytime();
        wave_info.__total_wave = exec_json["wave"].asInt();

        if (this->is_wave_script() == true)
        {
        	wave_info.__cur_wave 	= this->script_detail_.__piece.__piece;
        	wave_info.__finish_wave = this->script_detail_.__piece.__piece;
        	wave_info.__begin_wave  = this->script_detail_.__piece.__piece;
        }
    }
    if (exec_json.isMember("chapter_monster"))
    {
        is_has_flag = true;
        this->set_scene_flag(GameEnum::SCRIPT_SF_CHAPTER_MONSTER);

        const Json::Value &monster_json = exec_json["chapter_monster"];
        for (uint i = 0; i < monster_json.size(); ++i)
        {
            if (this->piece() != monster_json[i][0u].asInt())
                continue;
            if (this->chapter() < monster_json[i][1u].asInt() || monster_json[i][2u].asInt() < this->chapter())
                continue;

            for (uint j = 3; j < monster_json[i].size(); ++j)
            {
                int sort = monster_json[i][j][0u].asInt(),
                    num = monster_json[i][j][1u].asInt();
                monster_info.__monster_total_map[sort] = num;
            }
        }
    }
    if (exec_json.isMember("poem_text") && exec_json["poem_text"].size() > 0)
    {
        is_has_flag = true;
        int index = rand() % exec_json["poem_text"].size();
        if (exec_json["poem_text"][index].size() > 1)
        {
        	const Json::Value &poem_json = exec_json["poem_text"][index];
			monster_info.__text_size = poem_json[0u].asInt();
			index = rand() % (poem_json.size() - 1);
			monster_info.__poem_text = poem_json[index + 1].asString();

			if (exec_json.isMember("special_text_monster") && exec_json["special_text_monster"].size() >= 4)
			{
				monster_info.__special_sort_set.insert(exec_json["special_text_monster"][3u].asInt());
			}

			this->set_scene_flag(GameEnum::SCRIPT_SF_COLLECT_TEXT);
        }
    }

    if (is_has_flag == false)
    {
    	this->set_scene_flag(GameEnum::SCRIPT_SF_KILL_ALL);
    }

    return 0;
}

int BaseScript::player_enter_script_scene(MapPlayerScript *player, const int scene_id)
{
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    ScriptDetail::Team &team_info = this->script_detail_.__team;

	// 更新玩家副本的进度号
	ScriptPlayerDetail::ScriptRecord *record = player->refresh_script_record(this->script_sort());
	record->__progress_id = this->progress_id();

    // 插入队伍列表
    if (this->player_set().size() <= 0 || this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == false)
    {
    	MSG_DEBUG("script check start %d %d scene(%d %d) %d", this->script_sort(),
    			(scene_info.__finish_scene.find(scene_id) == scene_info.__finish_scene.end()),
    			this->current_script_scene(), scene_id, this->script_detail_.__exit_script_state);
        // 玩家进入副本时判断是否需要启动副本AI
        if (scene_info.__finish_scene.find(scene_id) == scene_info.__finish_scene.end() &&
                this->current_script_scene() == scene_id &&
                this->script_detail_.__exit_script_state != GameEnum::SCRIPT_EXIT_STOPED)
            this->start_script_timer();
    }

    // 判断是否第一次进入此副本，是则副本次数加1
    if (this->history_role_.find(player->role_id()) == this->history_role_.end())
    {
        this->history_role_.insert(player->role_id());

        if (this->is_climb_tower_script())
        {
        	ScriptPlayerDetail::PieceRecord &piece_rec = player->script_detail().__piece_record;
        	if (this->piece() < piece_rec.__pass_piece || (this->piece() == piece_rec.__pass_piece && this->chapter() < piece_rec.__pass_chapter))
        		this->first_role_.insert(player->role_id());
        }
        else
        {
        	ScriptPlayerDetail::ScriptRecord *script_rec = player->refresh_script_record(this->script_sort());
        	if (script_rec->__is_first_pass == 0)
        		this->first_role_.insert(player->role_id());
        }

        Int64 used_times_tick = 0;
        if (team_info.__used_times_tick_map.find(player->role_id()) != team_info.__used_times_tick_map.end())
            used_times_tick = team_info.__used_times_tick_map[player->role_id()];

        const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
        if (script_json["prev_condition"]["fresh_times_type"].asInt() == GameEnum::SCRIPT_FTT_ENTER_FRESH)
        {
            player->update_script_used_times_in_enter(this, Time_Value(used_times_tick));
        }
        else
        {
            // 进入时不扣次数
        }
    }

    if (this->script_detail_.__player_set.find(player->role_id()) == this->script_detail_.__player_set.end())
    {
        this->script_detail_.__player_set.insert(player->role_id());
        team_info.__teamer_map[player->role_id()] = player->role_name();
    }

    this->failure_check_tick_ = Time_Value::zero;
    this->restore_script_state(player);

    return 0;
}

int BaseScript::start_script_timer(Message *msg)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == false, ERROR_SCRIPT_STARTED);

    this->set_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE);
    this->keepon_special_script_tick();

	ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
	if (tick_info.__ready_sec > 0)
	{
		//有准备时间
		tick_info.__ready_tick = ::time(NULL) + tick_info.__ready_sec;
		this->set_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE);
		this->set_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE);
	}
	else if (tick_info.__total_sec > 0)
	{
		//没有准备时间
		this->process_ready_tick_timeout();
	}

    return 0;
}

int BaseScript::stop_script_timer(void)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == true, ERROR_SCRIPT_STOPED);
    JUDGE_RETURN(this->is_finish_script() == false && this->is_failure_script() == false, ERROR_CLIENT_OPERATE);
//    JUDGE_RETURN(this->script_detail_.__stop_times == -1 || this->script_detail_.__stop_times > 0, ERROR_SCRIPT_NO_STOP_TIMES);

    --this->script_detail_.__stop_times;

//    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
//    if (scene_info.__monster_failure_tick != Time_Value::zero)
//    {
//        Time_Value nowtime = Time_Value::gettimeofday();
//        Time_Value diff = scene_info.__monster_failure_tick - nowtime;
//        scene_info.__monster_failure_tick = diff;
//    }


    if (this->test_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE) ||
    		this->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE))
    {
    	ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
    	int left_ready_sec = 0, left_scene_sec = 0, used_scene_sec = 0;
    	if (this->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE))
    	{
    		left_ready_sec = this->left_scene_ready_tick();
            tick_info.__ready_sec = left_ready_sec;
            tick_info.__ready_tick = 0;
//            if (left_ready_sec == 0)
//            {
//            	this->process_ready_tick_timeout();
//            }
    	}
    	if (this->test_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE))
    	{
    		left_scene_sec = this->left_scene_tick();
            used_scene_sec = this->used_scene_tick();
            tick_info.__inc_sec = 0;
            tick_info.__used_sec = used_scene_sec;
            tick_info.__begin_tick = 0;
            tick_info.__end_tick = 0;
    	}

        tick_info.__ready_tick = 0;
        tick_info.__begin_tick = 0;
        tick_info.__end_tick = 0;
    }
    
    ScriptScene *scene = this->fetch_current_scene();
    if (scene != 0)
    {
    	scene->holdup_scene();
    }

    MSG_USER("Player Exit Hold up %d", this->script_sort());
    if (this->is_single_script() == true)
    {
    	this->set_script_recycle_time(Time_Value::SECOND);
    }
    else
    {
    	this->set_script_recycle_time(Time_Value::MINUTE);
    }

    this->holdup_special_script_tick();
    this->reset_scene_flag(GameEnum::SCRIPT_SF_STARTED_GEN_MONSTER);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE);
    return 0;
}

int BaseScript::player_transfer_script_scene(MapPlayerScript *player, const int scene_from, const int scene_to)
{
    // 退出副本的切换场景移到接口上一层调用
    if (GameCommon::is_script_scene(scene_to) == false)
    {
        int ret = this->validate_exit_script(player);
        JUDGE_RETURN(ret == 0, ret);

        MSG_USER("tranfer exit script %s %ld %d %d %d", 
                player->name(), player->role_id(), player->scene_id(),
                scene_from, scene_to);

        return player->request_exit_script();
    }

    int ret = player->validate_fighter_movable();
    JUDGE_RETURN(ret == 0, ret);

    // 测试场景是否相邻, 是否在传送点附近
    ret = this->validate_legal_scene(player, scene_from, scene_to);
    JUDGE_RETURN(ret == 0, ret);

    ret = this->validate_exit_scene(player, scene_to);
    JUDGE_RETURN(ret == 0, ret);

    return this->process_change_scene(player, scene_from, scene_to);
}

int BaseScript::player_exit_script(MapPlayerScript *player, bool transfer)
{
    this->process_team_player_exit(player);
    this->script_detail_.__player_set.erase(player->role_id());

    if (transfer == true)
    {
    	player->process_exit_script();
    }

    JUDGE_RETURN(this->script_detail_.__player_set.empty() == true, 0);

    this->force_kill_all_monster();
    this->set_script_recycle_time(1);

    return 0;
}

int BaseScript::process_team_player_exit(MapPlayerScript *player)
{
	JUDGE_RETURN(this->is_single_script() == false, -1);

	ScriptTeamDetail *team_rel = this->monitor()->find_script_team_rel(this->team_id());
	JUDGE_RETURN(team_rel != NULL, -1);

	for (LongList::iterator iter = team_rel->__teamer_list.begin();
			iter != team_rel->__teamer_list.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter == player->role_id());
		team_rel->__teamer_list.erase(iter);
		break;
	}

	return 0;
}

int BaseScript::sync_increase_monster(ScriptAI *script_ai)
{
    int sort = script_ai->ai_sort(), num = 1;

    {
        ScriptScene *scene = this->fetch_current_scene();
        if (scene != 0)
            scene->increase_monster_amount(sort, num);
    }

    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    monster_info.__monster_left_map[sort] += num;
    monster_info.__left_monster += num;
    monster_info.__total_monster += num;
    ++(monster_info.__config_monster_map[script_ai->scene_config_index()][sort]);

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE))
    {
        ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
        ++wave_info.__wave_monster_map[script_ai->wave_id()];
        ++(wave_info.__wave_total_map[script_ai->wave_id()][sort]);

        MSG_USER("add script ai: %d %d %d %d %ld (%d %d %d)", this->script_id(), this->script_sort(),
        		script_ai->wave_id(), script_ai->ai_sort(), script_ai->mover_id(),
        		wave_info.__wave_monster_map[script_ai->wave_id()],
        		this->finish_wave(), this->current_wave());
    }

    Time_Value nowtime = Time_Value::gettimeofday();
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    if (monster_info.__left_monster == this->script_detail_.__max_monster_num)
    {
        // 怪物数量达到指定失败的数量
        scene_info.__monster_failure_tick = nowtime + Time_Value(SCRIPT_MONSTER_MAX_FAIL_TICK);
    }
    // 通知怪物数量快达到上限
    if ((this->script_detail_.__max_monster_num - 10) <= monster_info.__left_monster &&
            monster_info.__left_monster < this->script_detail_.__max_monster_num)
    {
        if (scene_info.__monster_fail_notify_tick == Time_Value::zero)
        {
            this->notify_monster_amount_ready_to_max();
            scene_info.__monster_fail_notify_tick = nowtime + Time_Value(SCRIPT_MONSTER_MAX_NOTIFY_INTERVAL);
        }
    }
    else
    {
        scene_info.__monster_fail_notify_tick = Time_Value::zero;
    }

    this->notify_update_monster_rec();

    return 0;
}

int BaseScript::sync_increase_monster(ScriptAI *script_ai, const Json::Value &json)
{
	this->sync_increase_monster(script_ai);
	this->summon_ai_inherit_player_attr(script_ai, json);		//让召唤出来的怪物继承人物的相关属性
	return 0;
}

int BaseScript::sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id)
{
	//保护的NPC被杀，副本失败
	int fail_flag = this->check_and_handle_kill_script_npc(script_ai);
	JUDGE_RETURN(fail_flag == false, -1);

	//怪物减少记录
	this->descrease_monster_amount(script_ai);
	//处理生成下一波怪
	this->check_and_handle_next_wave(script_ai);
    //阶段完成处理，如一波完成
    this->process_script_stage_finish();
	//通知怪物数量
    this->notify_update_monster_rec();

    //检测是否完成完成副本某个场景
    int success_flag = this->check_script_scene_finish();
    JUDGE_RETURN(success_flag == true, -1);

    //副本完成某个场景处理
    this->script_scene_finish_operate();

    //检测副本是否全部完成
    int finish_flag = this->check_script_finish();
    JUDGE_RETURN(finish_flag == true, -1);

    //副本全部完成处理
    return this->process_finish_script();
}

int BaseScript::sync_kill_npc(ScriptAI *script_ai)
{
    this->process_scene_tick_failure();
    return 0;
}

int BaseScript::check_add_player_buff()
{
	return 0;
}

bool BaseScript::check_couple_fb_finish()
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_COUPLES) == true, false);

	LongMap &couple_sel = this->script_detail_.couple_sel_;
	JUDGE_RETURN(couple_sel.size() >= 2, false);

	this->process_finish_script();
	couple_sel.clear();

	return true;
}

bool BaseScript::check_couple_fb_failure(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_COUPLES) == true, false);
	JUDGE_RETURN(this->couple_check_tick_ < nowtime, false);
	JUDGE_RETURN(this->couple_flag_ == false, false);

	if (this->script_detail_.__player_set.size() < 2)
	{
		this->update_script_tick_info();
		this->force_set_script_failure();
		this->set_script_recycle_time(1);
		this->couple_flag_ = true;
	}

	return 0;
}

//保护的NPC被杀，副本失败
bool BaseScript::check_and_handle_kill_script_npc(ScriptAI *script_ai)
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_PROTECT_NPC) == true, false);
	JUDGE_RETURN(this->script_detail_.__protect_npc_sort == script_ai->ai_sort(), false);

	this->sync_kill_npc(script_ai);
    return true;
}

int BaseScript::process_script_timeout(const Time_Value &nowtime)
{
	//检测玩家是否在
	this->check_and_process_no_enter_timeout(nowtime);

	//给玩家增加buff
	this->check_add_player_buff();

	//夫妻副本有人退出直接失败处理
	this->check_couple_fb_failure(nowtime);

	//检测夫妻副本是否完成
	this->check_couple_fb_finish();

	//回收处理
    int recycle_flag = this->check_and_process_recycle_timeout(nowtime);
    JUDGE_RETURN(recycle_flag == false, 0);

    //是否启动副本
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == true, 0);

    //开始前准备
    int pre_flag = this->check_and_process_pre_timeout(nowtime);
    JUDGE_RETURN(pre_flag == false, 0);

    //使用时间超时
    int use_flag = this->check_and_process_use_timeout(nowtime);
    JUDGE_RETURN(use_flag == false, 0);

    this->notify_fight_hurt_detail();	//DPS排行榜
    this->other_script_timeout(nowtime);

    return 0;
}

int BaseScript::other_script_timeout(const Time_Value &nowtime)
{
    return 0;
}

bool BaseScript::check_and_process_no_enter_timeout(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->failure_check_tick_ != Time_Value::zero, false);
	JUDGE_RETURN(this->failure_check_tick_ < nowtime, false);

	MSG_USER("no enter %d", this->script_sort());
	if (this->script_detail_.__player_set.empty() == true)
	{
		this->set_script_recycle_time(1);
	}
	else
	{
		this->failure_check_tick_ = Time_Value::zero;
	}

	return true;
}

//回收处理
bool BaseScript::check_and_process_recycle_timeout(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->recycle_tick() < nowtime, false);

	if (this->player_set().empty() == true)
	{
		this->recycle_self();
	}
	else
	{
		this->kickout_all_player();
	}

	return true;
}

//副本开始前准备
bool BaseScript::check_and_process_pre_timeout(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE) == true, false);
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE) == true, false);
	JUDGE_RETURN(this->left_scene_ready_tick() <= 0, false);

	this->process_ready_tick_timeout();
	this->notify_script_progress_detail();
    return true;
}

//副本使用时间超时
bool BaseScript::check_and_process_use_timeout(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE) == true, false);
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE) == true, false);

	JUDGE_RETURN(this->left_scene_tick() <= 0, false);
	this->process_scene_tick_failure();

	return true;
}

int BaseScript::run_current_scene(void)
{
    ScriptScene* scene = this->fetch_current_scene();
    JUDGE_RETURN(scene->is_running() == false, -1);

    scene->run_scene();
    return 0;
}

int BaseScript::validate_exit_script(MapPlayerScript *player)
{
    if (this->is_failure_script() || this->is_finish_script())
        return 0;

    JUDGE_RETURN(player->is_fight_state() == false, ERROR_BEING_FIGHT);

    return 0;
}

int BaseScript::validate_legal_scene(MapPlayerScript *player, const int scene_from, const int scene_to)
{
    MoverCoord coord = player->location();

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(scene_from);
    JUDGE_RETURN(scene_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

    MoverCoord exits_coord;
    const Json::Value &exits_json = scene_json["exits"];
    if (exits_json.size() <= 0)
        return 0;

    bool is_legal = false;
    for (uint i = 0; i < exits_json.size(); ++i)
    {
    	int scene_id = ::atoi(exits_json[i]["id"].asCString());
        if (scene_id != scene_to)
            continue;

        if (exits_json[i].isMember("pos"))
        {
            exits_coord.set_pos(exits_json[i]["pos"][0u].asInt(), exits_json[i]["pos"][1u].asInt());
        }
        else
        {
            exits_coord.set_pixel(exits_json[i]["posX"].asInt(),
                    exits_json[i]["posY"].asInt());
        }

        if (coord_offset_grid(coord, exits_coord) < 5)
        {
            is_legal = true;
            break;
        }
    }

    JUDGE_RETURN(is_legal == true, ERROR_SCENE_NO_ADJACENT);
    return 0;
}

int BaseScript::validate_exit_scene(MapPlayerScript *player, const int scene_to)
{
    JUDGE_RETURN(player->scene_id() < this->current_script_scene() ||
    		(player->scene_id() == this->current_script_scene() && this->is_finish_script_scene()),
    		ERROR_SCRIPT_SCENE_NO_FINISH);

    return 0;
}

int BaseScript::process_change_scene(MapPlayerScript *player, const int scene_from, const int scene_to)
{
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    if (scene_info.__finish_scene.find(scene_to) == scene_info.__finish_scene.end() &&
            this->current_script_scene() != scene_to)
    {
        scene_info.__last_scene = scene_info.__cur_scene;
        scene_info.__last_floor = scene_info.__cur_floor;

        scene_info.__cur_scene = scene_to;
        const Json::Value &scene_json = CONFIG_INSTANCE->scene(scene_to);
        scene_info.__cur_floor = scene_json["floor"].asInt();
        scene_info.__monster_failure_tick = Time_Value::zero;

        this->init_scene_exec_condition(scene_json);
    }

    int target_scene = 0;
    MoverCoord target_coord;
    this->fetch_enter_scene_coord(player->role_id(), target_scene, target_coord);

    this->copy_player_change_scene();
    player->transfer_dispatcher(target_scene, target_coord, SCENE_MODE_SCRIPT, this->script_id());

    return 0;
}

int BaseScript::process_ready_tick_timeout(void)
{
	this->reset_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE);
    this->set_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE);

    ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
    tick_info.__begin_tick = ::time(NULL) - tick_info.__used_sec;	//当前时间 - 已使用时间
    tick_info.__ready_sec = 0;
    tick_info.__ready_tick = 0;

    this->process_generate_monster();
    return 0;
}

//场景时间超时失败
int BaseScript::process_scene_tick_failure(void)
{
	MSG_USER("script timeout failure %d %d %d %d", this->script_id(), this->script_sort(),
			this->is_finish_script(), this->is_failure_script());

	JUDGE_RETURN(this->is_finish_script() == false, -1);
	JUDGE_RETURN(this->is_failure_script() == false, -1);

    this->update_script_tick_info();
    this->force_set_script_failure();
    this->set_script_recycle_time();

    return 0;
}

int BaseScript::process_generate_monster(void)
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_GEN_MONSTER) == false, -1);

	this->set_scene_flag(GameEnum::SCRIPT_SF_STARTED_GEN_MONSTER);
    this->run_current_scene();

	return 0;
}

int BaseScript::process_script_failure(void)
{
    this->request_save_script_progress();
    return 0;
}

int BaseScript::process_finish_script(void)
{
    //副本成功完成
    this->set_scene_flag(GameEnum::SCRIPT_SF_FINISH_SCRIPT);
    //保存进度
    this->request_save_script_progress();
    //计算星级
	this->script_star_level();
	//玩家奖励
	this->process_script_player_finish();
	//播报
    this->broad_script_pass();
    //队伍
    this->sync_logic_team_script_end();

//    this->request_update_intimacy_by_finish();
//    this->request_update_emotion_by_finish();
    return 0;
}

int BaseScript::update_chapter_history(void)
{
    int total_use_tick = this->total_used_script_tick();
    if (this->is_top_star_level() == false)
    	return 0;

    Int64 role_id = 0, team_state = 0;
    std::string role_name;
    if (this->player_set().size() > 0)
        role_id = *(this->player_set().begin());
    MapPlayerEx *player = 0;
    if (this->monitor()->find_player(role_id, player) == 0)
    {
        role_name = player->role_name();
        team_state = player->teamer_state();
    }

    HistoryChapterRecord *org_chapter_rec = GLOBAL_SCRIPT_HISTORY->chapter_rec(this->chapter_key());
    if (org_chapter_rec == 0)
    {
        HistoryChapterRecord chapter_rec;
        chapter_rec.reset();
        chapter_rec.__chapter_key = this->chapter_key();
        chapter_rec.__best_use_tick = total_use_tick;
        chapter_rec.__first_top_level_player = role_id;
        chapter_rec.__first_top_level_role_name = role_name;
        GLOBAL_SCRIPT_HISTORY->bind_chapter_rec(this->chapter_key(), chapter_rec);

        GLOBAL_SCRIPT_HISTORY->request_save_history_chapter_rec();

//        {
//            BrocastParaVec para_vec;
//            GameCommon::push_brocast_para_role_detail(para_vec, role_id, role_name, team_state);
//            GameCommon::push_brocast_para_int(para_vec, this->piece());
//            GameCommon::push_brocast_para_int(para_vec, this->chapter());
//            this->monitor()->announce_world(SHOUT_ALL_FIRST_PASS_ZYFML, para_vec);
//        }
    }
    return 0;
}

int BaseScript::check_script_scene_failure(void)
{
	if (this->test_scene_flag(GameEnum::SCRIPT_SF_ALL_RELIVE))
	{
		bool is_all_death = true;
		for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
				iter != this->script_detail_.__player_set.end(); ++iter)
		{
			MapPlayerEx* player = this->find_player(*iter);

			JUDGE_CONTINUE(player != NULL);
			JUDGE_CONTINUE(player->is_death() == false || this->left_relive_amount(*iter) != 0);

			is_all_death = false;
			break;
		}

		if (this->is_finish_script() == true)
		{
			this->process_finish_script();
			return -1;
		}

		JUDGE_RETURN(is_all_death == false, 0);	// 全部死亡则失败
	}

	return -1;
}

bool BaseScript::check_script_scene_finish(void)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE) == true, false);

    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    JUDGE_RETURN(scene_info.__finish_scene.count(this->current_script_scene()) == 0, false);

    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    if (this->test_scene_flag(GameEnum::SCRIPT_SF_KILL_ALL))
    {
        JUDGE_RETURN(monster_info.__left_monster <= 0, false);
    }

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_KILL_MONSTER)
    		|| this->test_scene_flag(GameEnum::SCRIPT_SF_CHAPTER_MONSTER))
    {
        for (MonsterMap::iterator iter = monster_info.__monster_total_map.begin();
                iter != monster_info.__monster_total_map.end(); ++iter)
        {
            int killed_num = 0;
        	int num = iter->second;
            JUDGE_RETURN(monster_info.__monster_killed_map.find(iter->first, killed_num) == 0, false);
            JUDGE_RETURN(num <= killed_num, false);
        }
    }

    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    if (this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE))
    {
        JUDGE_RETURN(wave_info.__finish_wave >= wave_info.__total_wave, false);
    }

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_COLLECT_TEXT))
    {
        JUDGE_RETURN(monster_info.__appear_text_max >= monster_info.__text_size, false);
    }

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_COUPLES))
    {
    	if (monster_info.__left_monster <= 0)
    	{
    		for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
    		    	iter != this->script_detail_.__player_set.end(); ++iter)
    		{
    		    MapPlayerEx *player = this->find_player(*iter);
    		    JUDGE_CONTINUE(player != NULL);

    		    player->respond_to_client(ACTIVE_OPEN_KEY_SELECT_INTERFACE);
    		}
    	}

    	LongMap couple_sel = this->script_detail_.couple_sel_;
    	JUDGE_RETURN(couple_sel.size() >= 2, false);
    }

    return true;
}

int BaseScript::process_script_stage_finish()
{
    return 0;
}

//副本完成某个场景处理
int BaseScript::script_scene_finish_operate(void)
{
    ScriptDetail::Scene &scene_info = this->script_detail_.__scene;
    scene_info.__passed_scene = this->current_script_scene();
    scene_info.__passed_floor = this->current_script_floor();
    scene_info.__finish_scene.insert(this->current_script_scene());

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE) &&
            this->test_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE))
    {
        ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
        tick_info.__used_sec = this->used_scene_tick();
        this->script_detail_.__total_used_tick += tick_info.__used_sec;
    }

    this->force_kill_all_monster();
    this->reset_scene_flag(GameEnum::SCRIPT_SF_STARTED_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE);
    this->set_scene_flag(GameEnum::SCRIPT_SF_FINISH_SCENE);
    this->exp_restore_sync_finish_scene(this->current_script_floor());

    return 0;
}

//检测副本是否全部完成
bool BaseScript::check_script_finish(void)
{
	ScriptDetail::Scene &scene_info = this->script_detail_.__scene;

    const Json::Value &script_json = this->script_conf();
    const Json::Value &finish_json = script_json["finish_condition"];

    if (finish_json.isMember("scene"))
    {
    	IntVec scene_vec;
    	GameCommon::json_to_t_vec(scene_vec, finish_json["scene"]);

    	for (IntVec::iterator iter = scene_vec.begin(); iter != scene_vec.end(); ++iter)
    	{
    		JUDGE_CONTINUE(scene_info.__finish_scene.count(*iter) == 0);
    		return false;	//有场景未完成
    	}
    }
    else if (finish_json.isMember("scene_size"))
    {
        JUDGE_RETURN(int(scene_info.__finish_scene.size()) >= finish_json["scene_size"].asInt(), false);
    }

    return true;
}

int BaseScript::check_area_matrix(void)
{
//    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
//    JUDGE_RETURN(wave_info.__matrix_level_map.size() > 0, -1);
//
//    ScriptScene *scene = this->fetch_current_scene();
//    JUDGE_RETURN(scene != 0, -1);
//
//    MapPlayerEx *leader = 0;
//    if (this->is_single_script())
//    {
//        if (this->monitor()->find_player(this->owner_id(), leader) != 0)
//            return -1;
//    }
//    else
//    {
//    	MapPlayerEx *teamer = 0;
//    	ScriptDetail::Team &team_info = this->script_detail_.__team;
//    	for (LongList::iterator iter = team_info.__teamer_list.begin(); iter != team_info.__teamer_list.end(); ++iter)
//    	{
//    		if (this->monitor()->find_player(*iter, teamer) != 0)
//    			continue;
//    		if (teamer->team_id() != this->team_id())
//    			continue;
//    		if (teamer->team_info().leader_id_ != teamer->role_id())
//    		{
//    			if (this->monitor()->find_player(teamer->team_info().leader_id_, leader) != 0)
//    				return -1;
//    			break;
//    		}
//    		else
//    		{
//    			leader = teamer;
//    			break;
//    		}
//    	}
//    	if (leader == 0)
//    		return -1;
//    }
//
//    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->current_script_scene());
//    if (scene_json["exec"].isMember("matrix") == false)
//        return 0;
//
//    Scene::MoverMap fighter_map;
//    const Json::Value &matrix_json = scene_json["exec"]["matrix"];
//    for (uint i = 0; i < matrix_json.size(); ++i)
//    {
//        int matrix_id = matrix_json[i]["id"].asInt();
//        MatrixLevelMap::iterator iter = wave_info.__matrix_level_map.find(matrix_id);
//        if (iter == wave_info.__matrix_level_map.end())
//            continue;
//        int matrix_level = iter->second;
//        if (matrix_level <= 0)
//            continue;
//
//        int range = matrix_json[i]["range"][matrix_level - 1].asInt(),
//            value = matrix_json[i]["value"][matrix_level - 1].asInt(),
//            percent = matrix_json[i]["percent"][matrix_level - 1].asInt();
//
//        fighter_map.unbind_all();
//        const std::string &target_str = matrix_json[i]["target"].asString();
//        if (target_str.length() > GameEnum::SKILL_TARGET_ENEMY && target_str[GameEnum::SKILL_TARGET_ENEMY] == '1')
//        {
//            scene->fetch_all_around_fighter(leader, fighter_map, leader->location(), range, 30);
//            for (Scene::MoverMap::iterator mover_iter = fighter_map.begin();
//                    mover_iter != fighter_map.end(); ++mover_iter)
//            {
//                GameFighter *fighter = dynamic_cast<GameFighter *>(mover_iter->second);
//                if (fighter == 0 || fighter->is_death())
//                    continue;
//                if (fighter->is_monster() == false)
//                    continue;
//                leader->insert_defender_status(fighter, matrix_id, 0, 3, 0,
//                        value, percent, range);
//            }
//        }
//        else if (target_str.length() > GameEnum::SKILL_TARGET_ALLY && target_str[GameEnum::SKILL_TARGET_ALLY] == '1')
//        {
//            scene->fetch_all_around_player(leader, fighter_map, leader->location(), range, 10);
//            for (Scene::MoverMap::iterator mover_iter = fighter_map.begin();
//                    mover_iter != fighter_map.end(); ++mover_iter)
//            {
//                GameFighter *fighter = dynamic_cast<GameFighter *>(mover_iter->second);
//                if (fighter == 0 || fighter->is_death())
//                    continue;
//                leader->insert_defender_status(fighter, matrix_id, 0, 3, 0,
//                        value, percent, range);
//            }
//        }
//    }
    return 0;
}

int BaseScript::notify_failure_script()
{
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		//经验，罗摩副本关数奖励
		ThreeObjVec noraml_reward_vec;
		this->fetch_wave_reward(noraml_reward_vec);

		//展示客户端的奖励
		RewardInfo noraml_reward_info(false);
		GameCommon::make_up_reward_items(noraml_reward_info, noraml_reward_vec);

		int mult = SCRIPT_SYSTEM->fetch_script_mult(this->script_type());

		if (this->script_type() != GameEnum::SCRIPT_T_TRVL)
		{
			Proto80400929 req;
			this->make_up_script_finish_detail(player, NULL, &req);

			//奖励列表
			for (ItemObjVec::iterator iter = noraml_reward_info.item_vec_.begin();
					iter != noraml_reward_info.item_vec_.end(); ++iter)
			{
				ItemObj &item_obj = *iter;
				item_obj.amount_ *= mult;

				ProtoItem* proto = req.add_item();
				item_obj.serialize(proto);
			}
			player->respond_to_client(ACTIVE_RETURN_FINISH_SCRIPT, &req);
		}
		else
		{
			Proto80400906 req;
			this->make_up_script_finish_detail(player, &req, NULL);

			//奖励列表
			for (ItemObjVec::iterator iter = noraml_reward_info.item_vec_.begin();
					iter != noraml_reward_info.item_vec_.end(); ++iter)
			{
				ItemObj &item_obj = *iter;
				item_obj.amount_ *= mult;

				ProtoItem* proto = req.add_item();
				item_obj.serialize(proto);
			}
			player->respond_to_client(ACTIVE_FINISH_SCRIPT, &req);
		}

        player->update_finish_script_rec(this, false);
        player->request_script_type_info(this->script_sort());
    }
    return 0;
}

//副本失败，玩家处理
int BaseScript::process_script_player_failure(void)
{
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
    		iter != this->script_detail_.__player_set.end(); ++iter)
    {
    	MapPlayerEx *player = this->find_player(*iter);
    	JUDGE_CONTINUE(player != NULL);

        player->process_script_failure(this);
    }

    return 0;
}

//副本成功，玩家处理
int BaseScript::process_script_player_finish(void)
{
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
    		iter != this->script_detail_.__player_set.end(); ++iter)
    {
    	MapPlayerEx *player = this->find_player(*iter);
    	JUDGE_CONTINUE(player != NULL);

    	//更新副本记录
    	player->update_finish_script_rec(this);
    	//处理奖励
    	if (this->script_type() != GameEnum::SCRIPT_T_EXP &&
    			this->script_type() != GameEnum::SCRIPT_T_LEAGUE_FB)
    	{
    		//跨服组队副本用回旧消息号
    		if (this->script_type() != GameEnum::SCRIPT_T_TRVL)
    			player->process_script_pass_award(this);
    		else
    			player->process_script_travel(this);

    		player->check_pa_event_script_type_finish(this->script_type());
    	}
    	//副本次数
    	player->process_script_finish(this);
    	//任务检测
    	player->check_script_wave_task(this);
    	//通知副本信息
    	player->request_script_type_info(this->script_sort());
    	//其他处理
    	int ret = this->process_script_player_finish(player);
    	JUDGE_CONTINUE(ret != 0);

    	player->process_script_pass_award(this);
    }

    return 0;
}

int BaseScript::process_script_player_finish(MapPlayerEx* player)
{
	return 0;
}

int BaseScript::process_kill_wave(const int wave_id)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->current_script_scene());

    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    if (scene_json["exec"].isMember("spirit"))
    {
        int award_spirit = 0;
        if (int(scene_json["exec"]["spirit"].size()) >= wave_id)
            award_spirit = scene_json["exec"]["spirit"][wave_id - 1].asInt();
        wave_info.__spirit_value += award_spirit;

        this->notify_update_spirit();
    }

    if (scene_json["exec"].isMember("puppet"))
    {
        const Json::Value &puppet_json = scene_json["exec"]["puppet"];
        for (uint i = 0; i < puppet_json.size(); ++i)
        {
            if (puppet_json[i][0u].asInt() != wave_id)
                continue;

            int puppet_sort = puppet_json[i][1u].asInt();
            wave_info.__active_puppet[puppet_sort] = 0;

            if (int(wave_info.__active_puppet_flag.size()) > 0 && i < wave_info.__active_puppet_flag.size())
                wave_info.__active_puppet_flag[i] = 1;

            this->notify_update_spirit();
            break;
        }
    }

    return 0;
}

int BaseScript::broad_script_pass(void)
{
    return 0;
}

int BaseScript::notify_all_player(Message *msg)
{
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
        MapPlayerEx* player = this->find_player(*iter);
        JUDGE_CONTINUE(player != NULL);
        this->monitor()->dispatch_to_client_from_gate(player, msg);
    }
    return 0;
}

int BaseScript::notify_all_player(int recogn, Message* msg)
{
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
        MapPlayerEx* player = this->find_player(*iter);
        JUDGE_CONTINUE(player != NULL);
        player->respond_to_client(recogn, msg);
    }
    return 0;
}

int BaseScript::notify_update_evencut_rec(void)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    const Json::Value &evencut_json = CONFIG_INSTANCE->scene(this->current_script_scene())["exec"]["evencut"];
    int each_evencut = evencut_json[1u].asInt();
    if (each_evencut == 0)
        each_evencut = 5;
    int prop_inc = (monster_info.__max_evencut / each_evencut) * evencut_json[2u].asInt();

    Proto80400901 res;
    res.set_even_cut(monster_info.__evencut);
    res.set_prop_rate(prop_inc);
    res.set_max_even_cut(monster_info.__max_evencut);

    return this->notify_all_player(&res);
}

int BaseScript::notify_update_monster_rec(void)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;

    Proto80400902 res;
    res.set_left_monster(monster_info.__left_monster);
    res.set_total_monster(monster_info.__total_monster);
    res.set_killed_monster(this->kill_monster_amount());

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE) == true)
    {
    	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    	int wave_id = this->finish_wave() + 1, killed_amount = 0;
    	if (wave_id > this->total_wave())
    	{
    		wave_id = this->total_wave();
    	}

    	IntMap &wave_total_map = wave_info.__wave_total_map[wave_id];
    	IntMap &wave_killed_map = wave_info.__wave_killed_map[wave_id];
    	for (IntMap::iterator iter = wave_total_map.begin(); iter != wave_total_map.end(); ++iter)
    	{
            ProtoMonster *monster = res.add_monster_rec();
            monster->set_sort(iter->first);
            killed_amount = wave_killed_map[iter->first];
            monster->set_left_amount(iter->second - killed_amount);
            monster->set_total_amount(iter->second);

            MoverCoord coord;
            this->fetch_monster_coord(coord);

            ProtoCoord *proto_coord = monster->mutable_coord();
            proto_coord->set_pixel_x(coord.pixel_x());
            proto_coord->set_pixel_y(coord.pixel_y());
    	}
    }
    else
    {
        MonsterMap::iterator begin_iter, end_iter;
        bool is_left_map = false;
        if (monster_info.__monster_left_map.size() < monster_info.__monster_total_map.size())
        {
            begin_iter = monster_info.__monster_total_map.begin();
            end_iter = monster_info.__monster_total_map.end();
        }
        else
        {
            begin_iter = monster_info.__monster_left_map.begin();
            end_iter = monster_info.__monster_left_map.end();
            is_left_map = true;
        }
        for (; begin_iter != end_iter; ++begin_iter)
        {
            int sort = begin_iter->first, left = 0, total = 0;
            if (is_left_map == true)
            {
                left = begin_iter->second;
                monster_info.__monster_total_map.find(sort, total);
            }
            else
            {
                total = begin_iter->second;
                monster_info.__monster_left_map.find(sort, left);
            }

            ProtoMonster *monster = res.add_monster_rec();
            monster->set_sort(sort);
            monster->set_left_amount(left);
            monster->set_total_amount(total);

            MoverCoord coord;
            this->fetch_monster_coord(coord);

            ProtoCoord *proto_coord = monster->mutable_coord();
            proto_coord->set_pixel_x(coord.pixel_x());
            proto_coord->set_pixel_y(coord.pixel_y());
        }
    }

    return this->notify_all_player(ACTIVE_UPDATE_MONSTER, &res);
}

int BaseScript::notify_update_relive_rec(void)
{
    Proto80400903 res;
    MapPlayerEx *player = 0;
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
        if (this->monitor()->find_player(*iter, player) != 0)
            continue;

        res.set_left_relive(this->left_relive_amount(*iter));
        res.set_total_relive(this->total_relive_amount(*iter));

        this->monitor()->dispatch_to_client_from_gate(player, &res);
    }
    return 0;
}

int BaseScript::notify_kill_wave(void)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE) == true, 0);

    Proto80400904 res;
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;

	res.set_current_wave(wave_info.__cur_wave);
	res.set_finish_wave(wave_info.__finish_wave);
	res.set_total_wave(wave_info.__total_wave);
	return this->notify_all_player(ACTIVE_UPDATE_WAVE, &res);
}

int BaseScript::notify_fight_hurt_detail()
{
	return 0;
}

int BaseScript::notify_npc_blood(const Int64 npc_id, const int inc_blood)
{
	Proto80400909 respond;
	respond.set_blood(inc_blood);
	return this->notify_all_player(&respond);
}

int BaseScript::make_up_spirit_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto80400910*, respond, -1);

    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    respond->set_spirit_value(wave_info.__spirit_value);

    for  (IntMap::iterator iter = wave_info.__matrix_level_map.begin();
            iter != wave_info.__matrix_level_map.end(); ++iter)
    {
        ProtoSkill *proto_skill = respond->add_matrix_list();
        proto_skill->set_skill_id(iter->first);
        proto_skill->set_skill_level(iter->second);
    }

    for (uint i = 0; i < 3; ++i)
    {
        if (i < wave_info.__active_puppet_flag.size())
        	respond->add_puppet_list(wave_info.__active_puppet_flag[i]);
        else
        	respond->add_puppet_list(0);
    }
    return 0;
}

//怪物减少记录
void BaseScript::descrease_monster_amount(ScriptAI *script_ai)
{
	int num = 1;
    int sort = script_ai->ai_sort();

    ScriptDetail::Monster& monster_info = this->script_detail_.__monster;
    monster_info.__left_monster -= num;
    monster_info.__monster_left_map[sort] -= num;
    monster_info.__monster_killed_map[sort] += num;

	ScriptScene* scene = this->fetch_current_scene();
	JUDGE_RETURN(scene != NULL, ;);

	scene->descrease_monster_amount(sort, -num);
}

//处理下一波怪
void BaseScript::check_and_handle_next_wave(ScriptAI *script_ai)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE) == true, ;);

	ScriptScene* scene = this->fetch_current_scene();
	JUDGE_RETURN(scene != NULL, ;);

    int sort = script_ai->ai_sort();
    int wave_id = script_ai->wave_id();

	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	wave_info.__wave_monster_map[wave_id] -= 1;
	wave_info.__wave_killed_map[wave_id][sort] += 1;

	int left_num = wave_info.__wave_monster_map[wave_id];
	MSG_USER("kill script ai: %d %d %d %d %ld (%d %d %d)", this->script_id(),
			this->script_sort(), wave_id, sort, script_ai->mover_id(),
			left_num, this->finish_wave(), this->current_wave());
	JUDGE_RETURN(left_num <= 0, ;);

//	int total_wave_ai_size = scene->total_wave_ai_size();
	int max_wave_size = std::min<int>(wave_info.__total_wave, wave_info.__cur_wave);

	for (int i = wave_info.__finish_wave + 1; i <= max_wave_size; ++i)
	{
		JUDGE_BREAK(wave_info.__wave_monster_map.count(i) > 0)
		JUDGE_BREAK(wave_info.__wave_monster_map[i] <= 0);
		wave_info.__finish_wave = i;
	}

	this->process_kill_wave(script_ai->wave_id());
	this->notify_kill_wave();
}

void BaseScript::update_player_exp(MapPlayerEx *player, int inc_exp)
{
    ScriptDetail::PlayerAward &player_award = this->script_detail_.__player_award_map[player->role_id()];
    player_award.__exp += inc_exp;
}

int BaseScript::notify_update_spirit(void)
{
    Proto80400910 respond;
    this->make_up_spirit_info(&respond);
    
    return this->notify_all_player(&respond);
}

int BaseScript::refresh_script_evencut(void)
{
//    const Json::Value &evencut_json = CONFIG_INSTANCE->scene(this->current_script_scene())["exec"]["evencut"];
//
//    Time_Value nowtime = Time_Value::gettimeofday();
//    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
//
//    if (monster_info.__clear_evencut_tick < nowtime)
//        monster_info.__evencut = 0;
//
//    int interval = evencut_json[0u].asInt();
//    monster_info.__clear_evencut_tick = nowtime + Time_Value(interval);
    return 0;
}

int BaseScript::update_script_evencut(void)
{
//	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_EVENCUT) == true, 0);
//
//    this->refresh_script_evencut();
//
//    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
//    ++monster_info.__evencut;
//    if (monster_info.__evencut > monster_info.__max_evencut)
//    {
//        monster_info.__max_evencut = monster_info.__evencut;
//
//        const Json::Value &evencut_json = CONFIG_INSTANCE->scene(this->current_script_scene())["exec"]["evencut"];
//
//        int each_evencut = evencut_json[1u].asInt();
//        if (each_evencut == 0)
//            each_evencut = 5;
//        if (monster_info.__max_evencut % each_evencut == 0)
//        {
//            int prop_inc = (monster_info.__max_evencut / each_evencut) * evencut_json[2u].asInt();
//            if (evencut_json.size() >= 4 && prop_inc > evencut_json[3u].asInt())
//            {
//            	prop_inc = evencut_json[3u].asInt();
//            }
//
//            BasicStatus status(BasicStatus::EVENTCUT);
//            status.set_all(Time_Value(this->total_scene_tick() + 600),
//                    Time_Value::zero, 0, 0, 0,
//                    0.0, prop_inc);
//
//            MapPlayerEx *player = 0;
//            for (BLongSet::iterator iter = this->player_set().begin();
//                    iter != this->player_set().end(); ++iter)
//            {
//                if (this->monitor()->find_player(*iter, player) != 0)
//                    continue;
//
//                player->insert_status(status);
//            }
//        }
//    }
//    this->notify_update_evencut_rec();

    return 0;
}

int BaseScript::generate_new_wave(const int scene_id, const int wave_id)
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE), 0);

    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;

    if (wave_info.__cur_wave < wave_id)
        wave_info.__cur_wave = wave_id;

    this->notify_kill_wave();

    return 0;
}

int BaseScript::make_up_script_progress_detail(Proto50400903* respond)
{
    int left_tick = this->left_scene_tick();
    int total_tick = this->total_scene_tick();
    int used_tick = this->total_used_script_tick();

    int get_exp = this->script_pass_chapter();

    if (used_tick <= 0 || this->is_holdup() == false)
    {
    	used_tick += this->used_scene_tick();
    }

    Proto50400903 &res = *respond;
    res.set_left_sec(left_tick);
    res.set_used_sec(used_tick);
    res.set_total_sec(total_tick);
    res.set_left_monster(this->left_monster_amount());
    res.set_total_monster(this->total_monster_amount());
    res.set_killed_monster(this->kill_monster_amount());

    res.set_top_evencut(this->top_evencut());
    res.set_script_status(this->script_status());
    res.set_current_wave(this->current_wave());
    res.set_begin_wave(this->begin_wave());
    res.set_total_wave(this->total_wave());
    res.set_current_floor(this->current_script_floor());
    res.set_piece(this->piece());
    res.set_chapter(this->chapter());
    res.set_get_exp(get_exp);

//    {
//    	ScriptDetail::Monster &monster_info = this->script_detail().__monster;
//    	res.set_poem_size(monster_info.__text_size);
//    	res.set_poem_text(monster_info.__poem_text);
//    	for (BIntSet::iterator iter = monster_info.__appear_text_set.begin();
//    			iter != monster_info.__appear_text_set.end(); ++iter)
//    	{
//    		res.add_poem_appear_index(*iter);
//    	}
//    }

//    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
//    for (uint i = 0; i < 3; ++i)
//    {
//        if (i < wave_info.__active_puppet_flag.size())
//            res.add_puppet_list(wave_info.__active_puppet_flag[i]);
//        else
//            res.add_puppet_list(0);
//    }

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_PROTECT_NPC))
    {
    	GameAI *game_ai = this->fetch_protect_npc();
    	if (game_ai != 0)
    	{
    		res.set_protect_npc_blood(game_ai->cur_blood());
    		res.set_protect_npc_maxblood(game_ai->fight_detail().__blood_total_i(game_ai));
    	}
    	else
    	{
    		const Json::Value &monster_json = CONFIG_INSTANCE->monster(this->script_detail_.__protect_npc_sort);
    		if (monster_json != Json::Value::null)
    		{
    			res.set_protect_npc_maxblood(monster_json["hp"].asInt());
    		}
    	}
    }

    if (this->test_scene_flag(GameEnum::SCRIPT_SF_KILL_MONSTER) == true &&
        		this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE) == true)
    {
		ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
		for (MonsterMap::iterator iter = monster_info.__monster_total_map.begin();
				iter != monster_info.__monster_total_map.end(); ++iter)
		{
			int killed_num = 0;
			int sort = iter->first, total = iter->second;
			monster_info.__monster_killed_map.find(sort, killed_num);
			int left = total - killed_num;

			ProtoMonster *monster = res.add_monster_rec();
			monster->set_sort(sort);
			monster->set_left_amount(left);
			monster->set_total_amount(total);

			MoverCoord coord;
			this->fetch_monster_coord(coord);

			ProtoCoord *proto_coord = monster->mutable_coord();
			proto_coord->set_pixel_x(coord.pixel_x());
			proto_coord->set_pixel_y(coord.pixel_y());
		}
	}
    else if (this->test_scene_flag(GameEnum::SCRIPT_SF_WAVE) == true)
    {
    	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    	int wave_id = this->finish_wave() + 1, killed_amount = 0;
    	if (wave_id > this->total_wave())
    		wave_id = this->total_wave();
    	IntMap &wave_total_map = wave_info.__wave_total_map[wave_id];
    	IntMap &wave_killed_map = wave_info.__wave_killed_map[wave_id];
    	for (IntMap::iterator iter = wave_total_map.begin(); iter != wave_total_map.end(); ++iter)
    	{
            ProtoMonster *monster = res.add_monster_rec();
            monster->set_sort(iter->first);
            killed_amount = wave_killed_map[iter->first];
            monster->set_left_amount(iter->second - killed_amount);
            monster->set_total_amount(iter->second);

            MoverCoord coord;
            this->fetch_monster_coord(coord);

            ProtoCoord *proto_coord = monster->mutable_coord();
            proto_coord->set_pixel_x(coord.pixel_x());
            proto_coord->set_pixel_y(coord.pixel_y());
    	}
    }
    else
    {
        ScriptDetail::Monster &monster_info = this->script_detail().__monster;
        MonsterMap::iterator begin_iter, end_iter;
        bool is_left_map = false;
        if (monster_info.__monster_left_map.size() < monster_info.__monster_total_map.size())
        {
            begin_iter = monster_info.__monster_total_map.begin();
            end_iter = monster_info.__monster_total_map.end();
        }
        else
        {
            begin_iter = monster_info.__monster_left_map.begin();
            end_iter = monster_info.__monster_left_map.end();
            is_left_map = true;
        }
        for (; begin_iter != end_iter; ++begin_iter)
        {
            int sort = begin_iter->first, left = 0, total = 0;
            if (is_left_map == true)
            {
                left = begin_iter->second;
                if (monster_info.__monster_total_map.size() > 0)
                {
                	monster_info.__monster_total_map.find(sort, total);
                }
                else
                {
                	monster_info.__monster_left_map.find(sort, total);
                }
            }
            else
            {
                total = begin_iter->second;
                monster_info.__monster_left_map.find(sort, left);
            }

            ProtoMonster *monster = res.add_monster_rec();
            monster->set_sort(sort);
            monster->set_left_amount(left);
            monster->set_total_amount(total);

            MoverCoord coord;
            this->fetch_monster_coord(coord);

            ProtoCoord *proto_coord = monster->mutable_coord();
            proto_coord->set_pixel_x(coord.pixel_x());
            proto_coord->set_pixel_y(coord.pixel_y());
        }
    }

    this->make_up_special_script_progress_detail(respond);
    return 0;
}

int BaseScript::make_up_finish_scene_detail(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto80400908 *, respond, msg, -1);
	return 0;
}

int BaseScript::make_up_script_finish_detail(MapPlayerScript *player, Proto80400906* respond1, Proto80400929* respond2)
{
//    MSG_DYNAMIC_CAST_RETURN(Proto80400929 *, respond, -1);
    if (this->is_exp_script() || this->is_league_fb_script())
    {
    	ScriptPlayerDetail::TypeRecord *type_record = player->type_record(this->script_type());
    	type_record->__notify_wave = 0;
    	type_record->__notify_chapter = 0;
    }

    if (respond1 != NULL)
    {
		respond1->set_flag(this->script_finish_flag());
		respond1->set_scipt_sort(this->script_sort());
		respond1->set_used_tick(this->total_used_script_tick());
		respond1->set_cur_floor(this->floor_id());
    }

    if (respond2 != NULL)
    {
    	respond2->set_flag(this->script_finish_flag());
    	respond2->set_scipt_sort(this->script_sort());
    	respond2->set_used_tick(this->total_used_script_tick());
    	respond2->set_cur_floor(this->floor_id());
    }

    player->sword_script_set_skill(BaseScript::SKILL_DEL);

//    return this->make_up_special_script_finish_detail(player, respond);
    return 0;
}

int BaseScript::sync_script_inc_exp(const Int64 role_id, const int inc_exp)
{
    ScriptDetail::PlayerAward &player_award = this->script_detail_.__player_award_map[role_id];

    player_award.__additional_exp += inc_exp;
    player_award.__exp += inc_exp;
    return 0;
}

int BaseScript::sync_script_inc_money(const Int64 role_id, const Money &money)
{
    ScriptDetail::PlayerAward &player_award = this->script_detail_.__player_award_map[role_id];

    player_award.__copper += money.__copper;
    player_award.__copper += money.__bind_copper;

//    if (this->is_finish_script())
//    	this->notify_finish_script();
    return 0;

}

int BaseScript::sync_script_inc_item(const Int64 role_id, const ItemObj &item)
{
    ScriptDetail::PlayerAward &player_award = this->script_detail_.__player_award_map[role_id];

    ItemObj &award_item = player_award.__item_map[item.id_];
    award_item.id_ = item.id_;
    if (award_item.bind_ < item.bind_)
    	award_item.bind_ = item.bind_;
    award_item.amount_ += item.amount_;

//    if (this->is_finish_script())
//    	this->notify_finish_script();

    return 0;
}

int BaseScript::process_additional_exp(MapPlayerScript *player)
{
    ScriptDetail::PlayerAward &player_award = this->script_detail_.__player_award_map[player->role_id()];

    if (player_award.__additional_exp > 0)
    {
        player->modify_element_experience(player_award.__additional_exp, SerialObj(EXP_SCRIPT_ADDITION, this->script_sort(), player_award.__exp));
        player_award.__additional_exp = 0;
    }
    return 0;
}

int BaseScript::request_save_script_progress(void)
{
    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    MMOScriptProgress::update_data(this, data_map);
    if (TRANSACTION_MONITOR->request_mongo_transaction(this->script_sort(), TRANS_UPDATE_SCRIPT_PROGRESS, data_map) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return -1;
    }
    return 0;
}

int BaseScript::update_script_tick_info()
{
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE) == true, -1);
    JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE) == true, -1);

	ScriptDetail::Tick &tick_info = this->script_detail_.__tick;
	tick_info.__used_sec = this->used_scene_tick();

	this->script_detail_.__total_used_tick += tick_info.__used_sec;
    return 0;
}

int BaseScript::force_set_script_failure()
{
    this->reset_scene_flag(GameEnum::SCRIPT_SF_USE_TICK_SCENE);
    this->reset_scene_flag(GameEnum::SCRIPT_SF_ONGOING_SCENE);
    this->set_scene_flag(GameEnum::SCRIPT_SF_FAILURE_SCRIPT);

    this->process_script_failure();
    this->process_script_player_failure();
    this->notify_failure_script();
    this->sync_logic_team_script_end();

    return 0;
}

int BaseScript::notify_monster_amount_ready_to_max(void)
{
    Proto80400911 respond;
    respond.set_max_num(this->script_detail_.__max_monster_num);
    return this->notify_all_player(&respond);
}

int BaseScript::notify_script_progress_detail(void)
{ 
	Proto50400903 res;
	this->make_up_script_progress_detail(&res);

	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		MSG_USER("%ld %s script left tick: %d", player->role_id(), player->name(), res.left_sec());

		res.set_left_relive(this->left_relive_amount(*iter));
		res.set_total_relive(this->total_relive_amount(*iter));
		ScriptPlayerDetail::ScriptRecord * record = player->script_record(this->script_sort());
		if(record != NULL)
		{
			res.set_day_pass_time(record->__day_pass_times);
		}
		this->monitor()->dispatch_to_client_from_gate(player, &res);
	}

    return 0;
}

int BaseScript::sync_logic_team_script_end(void)
{
	JUDGE_RETURN(this->is_single_script() == false, -1);

    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
    	MapPlayerEx *player = this->find_player(*iter);
    	JUDGE_CONTINUE(player != NULL);
    	player->notify_quit_trvl_team();
    }

    return 0;
}

int BaseScript::recycle_copy_player()
{
	Scene* scene = this->fetch_current_scene();
	JUDGE_RETURN(scene != NULL, -1);

	scene->notify_all_player_exit(0, GameEnum::PLAYER_TYPE_OFFLINE);
	return 0;
}

int BaseScript::copy_player_change_scene(void)
{
    int target_scene = 0;
    MoverCoord target_coord;
    this->fetch_enter_scene_coord(0, target_scene, target_coord);

    ScriptScene *scene = this->fetch_current_scene();
    JUDGE_RETURN(scene != NULL, -1);

    for (LongMap::iterator iter = this->script_detail_.replacements_set_.begin();
    		iter != this->script_detail_.replacements_set_.end(); ++iter)
    {
    	MapPlayerEx* copy_player = NULL;
    	JUDGE_CONTINUE(MAP_MONITOR->find_player_with_offline(iter->first, copy_player) == 0);

    	JUDGE_CONTINUE(copy_player->scene_id() != target_scene);
    	copy_player->MapOfflineHook::exit_scene();
    	copy_player->master_exit_scene();

    	MoverDetail& mover_detial = copy_player->mover_detail();
    	mover_detial.__location = target_coord;

    	copy_player->init_mover_scene(scene);
    	copy_player->MapOfflineHook::enter_scene();

    	MSG_USER("scene_id %d, %d, role_name %s", scene->scene_id(), scene->space_id(),
    			copy_player->role_name().c_str());
    }

	return 0;
}

int BaseScript::exp_restore_sync_finish_scene(int stage)
{
	JUDGE_RETURN(this->is_climb_tower_script()== false, 0);

    Time_Value timev = Time_Value::gettimeofday();
    for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
            iter != this->script_detail_.__player_set.end(); ++iter)
    {
        MapPlayerEx *player = this->find_player(*iter);
        JUDGE_CONTINUE(player != NULL);
//        player->sync_storage_stage_info(this->script_sort(), stage, timev.sec());
    }

    return 0;
}

int BaseScript::first_start_script(void)
{
	JUDGE_RETURN(this->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE) == true, ERROR_SCRIPT_STARTED);
	return this->process_ready_tick_timeout();
}

int BaseScript::restore_script_state(MapPlayerScript *player)
{
//	ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
//
//	if (this->test_scene_flag(GameEnum::SCRIPT_SF_EVENCUT) == true)
//	{
//		const Json::Value &evencut_json = CONFIG_INSTANCE->scene(this->current_script_scene())["exec"]["evencut"];
//		int each_evencut = evencut_json[1u].asInt();
//		if (each_evencut == 0)
//			each_evencut = 5;
//		int prop_inc = (monster_info.__max_evencut / each_evencut) * evencut_json[2u].asInt();
//		if (evencut_json.size() >= 4 && prop_inc > evencut_json[3u].asInt())
//		{
//			prop_inc = evencut_json[3u].asInt();
//		}
//
//		BasicStatus status;
//		status.set(BasicStatus::EVENTCUT,
//				Time_Value(this->total_scene_tick() + 600),
//				Time_Value::zero, 0, 0, 0,
//				0.0, prop_inc);
//
//		player->insert_status(status);
//	}
//	if(GameCommon::is_drop_dragon_hole_scene(this->current_script_scene()))
//	{
//		DropDragonHoleScript *drop_script = dynamic_cast<DropDragonHoleScript *>(this);
//		if(drop_script == 0)
//			return 0;
//		Role_Set::iterator iter = drop_script->role_info().begin();
//		for(;iter != drop_script->role_info().end();++iter)
//		{
//			if(iter->role_id_ == player->role_id())
//			{
//				player->set_gather_sort(iter->gather_sort_);
//				player->insert_skill(iter->scirpt_skill_id_);
//				Proto80400236 spd;
//				spd.set_gather_skill_id(iter->scirpt_skill_id_);
//				player->respond_to_client(NOTIFY_CLIENT_DROP_DRAGON_SKILL,&spd);
//				player->set_drop_dragon_skill_buff_id(iter->skill_buff_id_);
//				//通知其他玩家显示
//				break;
//			}
//		}
//	}
	return 0;
}

int BaseScript::kickout_all_player(void)
{
    BLongSet player_set = this->script_detail_.__player_set;
    for (BLongSet::iterator iter = player_set.begin(); iter != player_set.end(); ++iter)
    {
    	MapPlayerEx *player = this->find_player(*iter);
    	JUDGE_CONTINUE(player != NULL);
//    	JUDGE_CONTINUE(player->script_id() == this->script_id());

		MSG_USER("kickout_all_player: name: %s, script_sort: %d",
				player->name(), this->script_sort());
		player->request_exit_script(true);
    }

    this->script_detail_.__player_set.clear();
    return 0;
}

int BaseScript::force_kill_all_monster(void)
{
    ScriptScene *scene = this->fetch_current_scene();
    JUDGE_RETURN(scene != 0, -1);
    return scene->force_kill_all_monster();
}

int BaseScript::record_player_hurt(Int64 role_id, int real_hurt)
{
	JUDGE_RETURN(this->script_detail_.__player_set.count(role_id) > 0, -1);
	this->script_detail_.__player_award_map[role_id].__dps += real_hurt;
	return 0;
}

int BaseScript::update_boss_hurt(const int sort, const double real_hurt)
{
    this->script_detail_.__monster.__boss_hurt += real_hurt;
    return 0;
}

int BaseScript::update_player_hurt(const Int64 role_id, const double real_hurt)
{
    this->script_detail_.__monster.__player_hurt += real_hurt;
    return 0;
}

bool BaseScript::is_top_star_level(const int value)
{
	JUDGE_RETURN(value > 0, false);
	const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
	const Json::Value &star_level_json = script_json["finish_condition"]["star_level"];
	JUDGE_RETURN(star_level_json.size() > 0, false);

	if (value <= star_level_json[(star_level_json.size() - 1)].asInt())
		return true;
	return false;
}

//计算副本星级
int BaseScript::script_star_level()
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	const Json::Value &star_level_json = script_json["finish_condition"]["star_level"];
	JUDGE_RETURN(star_level_json.size() > 0, -1);

	for (uint i = 0; i < star_level_json.size(); ++i)
	{
		int star_type = star_level_json[i][0u].asInt();
		int pass_num = star_level_json[i][1u].asInt();

		if (star_type == GameEnum::SCRIPT_KILL_MOSTER)
		{
			int kill_monster = this->script_detail_.__monster.__total_monster -
					this->script_detail_.__monster.__left_monster;
			JUDGE_CONTINUE(kill_monster >= pass_num);
		}
		else if (star_type == GameEnum::SCRIPT_PASS_TICK)
		{
			JUDGE_CONTINUE(this->script_detail_.__total_used_tick <= pass_num);
		}
		else if (star_type == GameEnum::SCRIPT_DIE_NUM)
		{
			JUDGE_CONTINUE(this->script_detail_.__relive.__used_relive <= pass_num);
		}

		this->script_detail_.__star_lvl.push_back(star_type);
	}

	return 0;
}

bool BaseScript::is_top_star_level(void)
{
	return this->is_top_star_level(this->total_used_script_tick());
}

int BaseScript::request_update_intimacy_by_finish(void)
{
    JUDGE_RETURN(this->is_single_script() == false, 0);

    const int TYPE_FINISH_SCRIPT = 1/*, TYPE_KILL_PLAYER = 2*/;

    Proto31101607 inner_req;
    inner_req.set_type(TYPE_FINISH_SCRIPT);
    inner_req.set_script_sort(this->script_sort());
    for (BLongSet::iterator iter = this->player_set().begin();
            iter != this->player_set().end(); ++iter)
    {
        inner_req.add_role_list(*iter);
    }

    return this->monitor()->dispatch_to_logic(&inner_req);
}

int BaseScript::calc_appear_text(ScriptAI *script_ai)
{
    ScriptDetail::Monster &monster_info = this->script_detail_.__monster;
    JUDGE_RETURN(monster_info.__text_size > 0, 0);

    int rand_value = 0;
    if (monster_info.__special_sort_set.find(script_ai->ai_sort()) != monster_info.__special_sort_set.end())
    {
        // 必定出现文字的怪物
        for (int i = monster_info.__appear_text_max + 1; i <= monster_info.__text_size; ++i)
        {
            if (monster_info.__appear_text_set.find(i) != monster_info.__appear_text_set.end())
                continue;

            rand_value = i;
            break;
        }
    }
    else
    {
        const Json::Value &exec_json = CONFIG_INSTANCE->scene(this->current_script_scene())["exec"];
        int rate = int(exec_json["appear_text_rate"].asDouble() * 100);

        rand_value = rand() % 10000;

        // 判断是否有出现文字
        JUDGE_RETURN(rand_value < rate, 0);

        rand_value = (rand() % monster_info.__text_size) + 1;
    }

    // 通知副本内所有玩家
    Proto80400918 respond;
    respond.set_poem_text_index(rand_value);
    respond.set_ai_id(script_ai->ai_id());
    this->notify_all_player(&respond);

    // 已出现过文字
    JUDGE_RETURN(monster_info.__appear_text_set.find(rand_value) == monster_info.__appear_text_set.end(), 0);

    monster_info.__appear_text_set.insert(rand_value);
    for (int i = monster_info.__appear_text_max + 1; i <= monster_info.__text_size; ++i)
    {
        if (monster_info.__appear_text_set.find(i) == monster_info.__appear_text_set.end())
        	break;

        monster_info.__appear_text_max = i;
    }

    return 0;
}

int BaseScript::poem_text_size(void)
{
    return this->script_detail_.__monster.__text_size;
}

double BaseScript::recalc_ai_modify_blood(const double inc_val)
{
    return inc_val;
}

int BaseScript::make_up_special_script_progress_detail(Message *msg)
{
    return 0;
}

int BaseScript::make_up_special_script_finish_detail(MapPlayerScript *player, Message *msg)
{
    return 0;
}

int BaseScript::keepon_special_script_tick(void)
{
    return 0;
}

int BaseScript::holdup_special_script_tick(void)
{
    return 0;
}

int BaseScript::summon_ai_inherit_player_attr(ScriptAI *script_ai, const Json::Value &json)
{
	return 0;
}

bool BaseScript::is_wave_script()
{
	if (this->script_type() != GameEnum::SCRIPT_T_RAMA
			&& this->script_type() != GameEnum::SCRIPT_T_EXP
			&& this->script_type() != GameEnum::SCRIPT_T_LEAGUE_FB)
		return false;
	return true;
}

int BaseScript::fetch_enter_teamers()
{
	return this->save_enter_teamers_;
}

int BaseScript::set_player_die_coords(MoverCoord die_coords)
{
	return 0;
}

int BaseScript::reset_player_die_coords()
{
	return 0;
}

