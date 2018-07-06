/*
 * MapPlayerScript.cpp
 *
 * Created on: 2013-12-25 16:48
 *     Author: lyz
 */

#include "MapPlayerEx.h"
#include "MapPlayerScript.h"
#include "ProtoDefine.h"
#include "BaseScript.h"
#include "MapMonitor.h"
#include "ScriptScene.h"
#include "TowerDefenseScript.h"
#include "RankStruct.h"
#include "GameAI.h"
#include "AIDropPack.h"
#include "AIManager.h"
#include "ScriptSystem.h"

MapPlayerScript::MapPlayerScript(void)
{ /*NULL*/ }

MapPlayerScript::~MapPlayerScript(void)
{ /*NULL*/ }

void MapPlayerScript::reset_script(void)
{
    this->script_detail_.reset();
    this->compact_type_ = 0;
    this->expired_tick_ = Time_Value::zero;
    this->drop_dragon_clean_tick_ = Time_Value::zero;
}

void MapPlayerScript::script_reset_everyday()
{
	this->script_detail_.__trvl_total_pass = 0;
}

int MapPlayerScript::logic_reset_script()
{
	this->reset_everyday();
	this->request_script_list_info(NULL);
	return 0;
}

int MapPlayerScript::request_enter_script(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400901*, request, ERROR_CLIENT_OPERATE);

    CONDITION_NOTIFY_RETURN(this->is_death() == false, RETURN_REQUEST_ENTER_SCRIPT,
    		ERROR_PLAYER_DEATH);

//    if(this->script_sort() != GameEnum::SCRIPT_SORT_LEGEND_TOP)
//    {
//    	CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == false, RETURN_REQUEST_ENTER_SCRIPT,
//    			ERROR_SCRIPT_SCENE_NO_FINISH);
//    }

    int script_sort = request->script_sort();
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    CONDITION_NOTIFY_RETURN(script_json.empty() == false, RETURN_REQUEST_ENTER_SCRIPT,
    		ERROR_CONFIG_NOT_EXIST);

//    if (this->is_in_script_mode() == true)
//    {
//        BaseScript *script = this->fetch_script();
//        if (script != 0)
//        {
//            if (script->is_finish_script() == false)
//                return this->respond_to_client_error(RETURN_REQUEST_ENTER_SCRIPT, ERROR_SCRIPT_SCENE_NO_FINISH);
//            else
//                script->player_exit_script(this, true);
//        }
//    }
//    else
//    {
//    	if(this->is_in_normal_mode() == false)
//    	{
//    		Proto30100108 sync_to_logic;
//			sync_to_logic.set_script_sort(script_sort);
//    		sync_to_logic.set_is_success(0); //如果为0,到逻辑进程里再判断队伍玩家是否都没进入副本(如都在熔恶之地的副本),然后重置队伍state，如果为1,则再扣减玩家体力
//    		MAP_MONITOR->dispatch_to_logic(this,&sync_to_logic);
//    		return this->respond_to_client_error(RETURN_REQUEST_ENTER_SCRIPT, ERROR_NO_NORMAL_SCENE);
//    	}
//    }

    Proto30400901 req;
    req.set_role_id(this->role_id());
    req.set_level(this->level());
    req.set_script_sort(script_sort);
    req.set_scene_id(this->scene_id());
    req.set_cheer_num(request->cheer_num());
    req.set_encourage_num(request->encourage_num());

    if (request->team_id() > 0)
    {
    	req.set_team_id(request->team_id());
    	for (int i = 0; i < request->team_set_size(); ++i)
    	{
    		Int64 role_id = request->team_set(i);
    		req.add_teamer_set(role_id);
    	}
    }
    else
    {
    	MapTeamInfo& team_info = this->team_info(GameEnum::NORMAL_TEAM);

    	req.set_team_id(team_info.team_index());
        for (LongMap::iterator iter = team_info.teamer_set_.begin();
        		iter != team_info.teamer_set_.end(); ++iter)
        {
        	req.add_teamer_set(iter->first);
        }
    }

    ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script_sort);
    req.set_used_times(script_record->__used_times);
    req.set_buy_times(script_record->__buy_times + script_record->__couple_buy_times);
    req.set_used_times_tick(script_record->__used_times_tick.sec());
    req.set_pass_piece(this->script_detail_.__piece_record.__pass_piece);
    req.set_pass_chapter(this->script_detail_.__piece_record.__pass_chapter);

    req.set_piece(request->piece());
    req.set_chapter(request->chapter());
    req.set_vip_type(this->vip_type());
    req.set_progress_id(script_record->__progress_id);
    req.set_online_sec(this->online().day_online_second());

    const Json::Value &pre_json = script_json["prev_condition"];
    if (pre_json.isMember("enter_cond"))
    {
    	int ret = this->check_enter_condition(script_sort, pre_json["enter_cond"]);
    	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_REQUEST_ENTER_SCRIPT, ret);
    }

    // 问鼎江湖,论剑武林
    if (pre_json["is_tower"].asInt() == true)
    {
    	ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(script_json["type"].asInt());
    	int total_floor = pre_json["total_floor"].asInt();
    	int fight_floor = top_info.__pass_floor + 1;

    	CONDITION_NOTIFY_RETURN((fight_floor <= 1) || (top_info.__is_sweep == false), RETURN_REQUEST_ENTER_SCRIPT, ERROR_MUST_SWEEP);
    	CONDITION_NOTIFY_RETURN(fight_floor <= total_floor, RETURN_REQUEST_ENTER_SCRIPT, ERROR_IS_HGIHEST_FLOOR);

    	req.set_piece(fight_floor);
    	req.set_used_times(0);
    }
    // 经验副本，罗摩副本，帮派副本
    else if(this->is_wave_script(script_json["type"].asInt()) == true)
    {
    	int chapter_id = script_json["prev_condition"]["chapter_id"].asInt();
    	int wave = script_json["scene"][0u]["exec"]["wave"].asInt();
    	CONDITION_NOTIFY_RETURN(chapter_id > 0 && wave > 0, RETURN_REQUEST_ENTER_SCRIPT, ERROR_CONFIG_NOT_EXIST);

    	int pass_piece = 0;
    	int pass_chapter = 0;
    	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_json["type"].asInt());
    	if (script_json["prev_condition"]["is_league"].asInt() > 0)
    	{
    		CONDITION_NOTIFY_RETURN(this->league_id() > 0, RETURN_REQUEST_ENTER_SCRIPT, ERROR_LEAGUE_NO_EXIST);

    		pass_piece = type_record->__start_wave;
    		pass_chapter = type_record->__start_chapter;
    	}
    	else
    	{
    		CONDITION_NOTIFY_RETURN(type_record->__is_sweep == false, RETURN_REQUEST_ENTER_SCRIPT, ERROR_MUST_SWEEP);

    		pass_piece = type_record->__pass_wave;
    		pass_chapter = type_record->__pass_chapter;
    	}
		CONDITION_NOTIFY_RETURN(chapter_id == (pass_chapter + 1) && pass_piece < wave, RETURN_REQUEST_ENTER_SCRIPT, ERROR_CHAPTER_ID);

		if (script_json["type"].asInt() == GameEnum::SCRIPT_T_EXP ||
				script_json["type"].asInt() == GameEnum::SCRIPT_T_LEAGUE_FB)
		{
			if (type_record->__notify_wave <= 0 && type_record->__notify_chapter <= 0)
			{
				type_record->__notify_wave = pass_piece;
				type_record->__notify_chapter = pass_chapter;
			}
		}
		req.set_piece(pass_piece);
		req.set_chapter(pass_chapter);
		req.set_used_times(0);
    }
    // vip副本
    else if(script_json["type"].asInt() == GameEnum::SCRIPT_T_VIP)
    {
    	int vip_level = script_json["prev_condition"]["vip_level"].asInt();
    	int my_vip = this->vip_detail().__vip_level;
    	CONDITION_NOTIFY_RETURN(vip_level <= my_vip, RETURN_REQUEST_ENTER_SCRIPT, ERROR_VIP_LEVEL);
    }

    int enter_scene = script_json["scene"][0u]["scene_id"].asInt();
    CONDITION_NOTIFY_RETURN(GameCommon::is_script_scene(enter_scene) == true,
    		RETURN_REQUEST_ENTER_SCRIPT, ERROR_SCENE_NO_EXISTS);

    req.set_script_scene(enter_scene);
//    MSG_DEBUG("0 request enter script %s %ld record[%d %d]", this->name(), this->role_id(), script_sort, script_record->__script_sort);

    if (script_json["prev_condition"]["pack_space"].asInt() > 0)
    {
        req.set_need_pack_space(script_json["prev_condition"]["pack_space"].asInt());
        return this->send_to_logic_thread(req);
    }
    else
    {
        return this->monitor()->dispatch_to_scene(this, enter_scene, &req);
    }
}

int MapPlayerScript::sync_enter_script(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto30400902 *, request, ERROR_CLIENT_OPERATE);

    CONDITION_NOTIFY_RETURN(this->is_death() == false, RETURN_REQUEST_ENTER_SCRIPT,
    		ERROR_PLAYER_DEATH);

	if (this->is_in_script_mode() == true)
	{
		BaseScript *script = this->fetch_script();
		const Json::Value &prev_json = CONFIG_INSTANCE->script(request->script_sort())["prev_condition"];
		JUDGE_RETURN(script == 0 || script->script_id() == request->script_id() || prev_json["second_enter"].asInt() == true, -1);

//		JUDGE_RETURN(script == 0 || script->script_id() == request->script_id()
//				|| request->script_sort() == GameEnum::SCRIPT_SORT_LEGEND_TOP
//				|| script->script_type() == GameEnum::SCRIPT_T_EXP
//				|| script->script_type() == GameEnum::SCRIPT_T_LEAGUE_FB, -1);

		Scene *scene = 0;
		if (this->monitor()->find_scene(request->script_id(), request->scene_id(), scene) == 0)
		{
			JUDGE_RETURN(scene->find_player(this->role_id()) == 0, -1);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		JUDGE_RETURN(this->is_in_normal_mode() == true, -1);
		this->script_detail_.__prev_scene = this->scene_id();
		this->script_detail_.__prev_coord = this->location();
		this->save_cur_scene_info(0);
	}

//    this->script_detail_.__first_pass_item.clear();
    this->script_detail_.__piece_record.__pass_chapter_item.clear();
    this->script_detail_.__script_id = request->script_id();
    this->script_detail_.__script_sort = request->script_sort();
    this->script_detail_.__prev_blood = this->fight_detail().__blood;
    this->script_detail_.__prev_magic = this->fight_detail().__magic;

    int used_times = 0, buy_times = 0;
    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(request->script_sort());
    if (record != 0)
    {
    	record->__used_times = request->used_times();
    	record->__progress_id = request->progress_id();

    	used_times = record->__used_times;
    	buy_times = record->__buy_times;
    }

    //更新资源找回
    this->script_sync_restore_info(msg);
    this->tiny_fb_recover_blood();

    Proto50400901 respond;
    this->respond_to_client(RETURN_REQUEST_ENTER_SCRIPT, &respond);

    MSG_USER("script enter %ld %s %d %d %d %d", this->role_id(), this->name(), request->script_id(),
    		request->script_sort(), request->pixel_x(), request->pixel_y());
	this->record_other_serial(SCRIPT_REL_OTHER_SERIAL, SUB_SCRIPT_ENTER,
			used_times, buy_times, request->script_sort());

    MoverCoord enter_coord;
    enter_coord.set_pixel(request->pixel_x(), request->pixel_y());
    return this->transfer_dispatcher(request->scene_id(), enter_coord,
            SCENE_MODE_SCRIPT, request->script_id());
}

int MapPlayerScript::request_exit_script(bool force)
{
    JUDGE_RETURN(this->is_in_script_mode() == true, 0);

    BaseScript* script = this->fetch_script();
    if (script == NULL)
    {
    	MSG_USER("ERROR exit script %ld, %s, %d", this->role_id(),
    			this->name(), this->scene_id());
    	return this->process_exit_script();
    }

    this->sword_script_set_skill(BaseScript::SKILL_DEL);

    if (script->is_finish_script() == true
    		|| script->is_failure_script() == true)
    {
    	//副本结束退出
    	return script->player_exit_script(this);
    }

    if (script->is_wave_script() == true)
    {
    	//需要处理副本失败
    	return script->process_scene_tick_failure();
    }
    else
    {
    	//直接退出
    	return script->player_exit_script(this);
    }
}

int MapPlayerScript::request_exit_special(void)
{
	return this->request_exit_script(true);
}

BaseScript *MapPlayerScript::fetch_script(void)
{
    int script_id = this->script_id();
    return this->monitor()->find_script(script_id);
}

int MapPlayerScript::process_exit_script()
{
    this->notify_exit_scene_cancel_info(EXIT_SCENE_TRANSFER);

    //清除信息
    this->script_detail_.__script_id = 0;

    //退出跨服组队
    MapTeamInfo& travel_team = this->team_info(GameEnum::TRAVEL_TEAM);
	if (travel_team.team_index() > 0)
	{
		travel_team.reset();
		this->notify_quit_trvl_team(true);
	}

	this->enter_recover_scene_info(false);
	return this->transfer_dispatcher(this->script_detail_.__prev_scene,
			this->script_detail_.__prev_coord);
}

int MapPlayerScript::process_script_failure(BaseScript *script)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script->script_sort());
    if (script_json["prev_condition"]["fresh_times_type"].asInt() == GameEnum::SCRIPT_FTT_END_FRESH)
    {
		ScriptDetail::Team &team_info = script->script_detail().__team;
		Time_Value used_times_tick = Time_Value::gettimeofday();
		BLongMap::iterator iter = team_info.__used_times_tick_map.find(this->role_id());
		if (iter != team_info.__used_times_tick_map.end())
			used_times_tick = Time_Value(iter->second);
		this->refresh_script_record_in_finish(script, used_times_tick);
    }

    return 0;
}

int MapPlayerScript::process_script_finish(BaseScript *script)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script->script_sort());
    if (script_json["prev_condition"]["fresh_times_type"].asInt() == GameEnum::SCRIPT_FTT_FINISH_FRESH ||
    		script_json["prev_condition"]["fresh_times_type"].asInt() == GameEnum::SCRIPT_FTT_END_FRESH)
    {
    	ScriptDetail::Team &team_info = script->script_detail().__team;
        Time_Value used_times_tick = Time_Value::gettimeofday();
        BLongMap::iterator iter = team_info.__used_times_tick_map.find(this->role_id());
        if (iter != team_info.__used_times_tick_map.end())
            used_times_tick = Time_Value(iter->second);
        this->refresh_script_record_in_finish(script, used_times_tick);
    }

	return 0;
}

int MapPlayerScript::enter_scene(const int type)
{
	BaseScript *script = this->fetch_script();
    if (script == 0)
    {
        MSG_USER("ERROR enter script scene => BaseScript NULL %ld %d", this->role_id(), 0);
        return -1;
    }

    if (script->is_finish_script() || script->is_failure_script())
    {
        MSG_USER("ERROR enter script scene => BaseScript finish[%d]/failure[%d] %ld %d",
        		script->is_finish_script(), script->is_failure_script(), this->role_id(), 0);
        return -1;
    }

    const Json::Value &script_json = CONFIG_INSTANCE->script(script->script_sort());
    if ((int)script->player_set().size() > script_json["prev_condition"]["max_teamer"].asInt())
    {
        MSG_USER("ERROR enter script scene => Script Player too more %d %d",
        		script->script_sort(), script->player_set().size());
        return -1;
    }

    if (script->is_self_script(this) == false)
    {
        MSG_USER("ERROR enter script scene => Script Owner(%ld,%ld) or Team(%d,%d) %d",
        		this->role_id(), script->owner_id(), this->team_id(), script->team_id(), script->script_sort());
        return -1;
    }

    if (script->player_enter_script_scene(this, this->scene_id()) != 0)
    {
        MSG_USER("ERROR enter script scene %ld %d", this->role_id(), script->script_sort());
        return -1;
    }

	ScriptScene* scene = script->fetch_current_scene();
    if (scene == NULL)
    {
        MSG_USER("ERROR enter script scene null %ld %d", this->role_id(), script->script_sort());
        return -1;
    }

	this->init_mover_scene(scene);
	MapPlayer::enter_scene(type);
    this->cache_tick().update_cache(MapPlayerEx::CACHE_SCRIPT);
    return 0;
}

int MapPlayerScript::exit_scene(const int type)
{
    int ret = MapPlayer::exit_scene(type);
//    switch (type)
//    {
//    case EXIT_SCENE_TRANSFER:
//    {
//    	ScriptScene *scene = dynamic_cast<ScriptScene *>(this->fetch_scene());
//    	JUDGE_BREAK(scene != 0 && scene->player_amount() <= 0);
//    	scene->holdup_scene();
//    	break;
//    }
//    }

	return ret;
}

int MapPlayerScript::sign_out(const bool is_save_player)
{
	JUDGE_RETURN(this->transfer_flag() == false, -1);

	//刷新退出
	BaseScript* script = this->fetch_script();
	JUDGE_RETURN(script != NULL, -1);
	JUDGE_RETURN(script->is_self_script(this) == true, -1);

	if (script->is_single_script() == true)
	{
		return script->stop_script_timer();
	}
	else
	{
		return script->player_exit_script(this, false);
	}
}

int MapPlayerScript::transfer_to_other_scene(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400110*, request, RETURN_TRANSFER_SCENE);
    CONDITION_NOTIFY_RETURN(GameCommon::is_script_scene(request->scene_id()) == true,
    		RETURN_TRANSFER_SCENE, ERROR_SCENE_NO_EXISTS);

    BaseScript *script = this->fetch_script();
    CONDITION_NOTIFY_RETURN(script != 0, RETURN_TRANSFER_SCENE, ERROR_CLIENT_OPERATE);

    int ret = script->player_transfer_script_scene(this, this->scene_id(), request->scene_id());
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_TRANSFER_SCENE, ret);

    return 0;
}

int MapPlayerScript::prepare_fight_skill(Message *msg)
{
    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != 0 && //
            script->is_finish_script() == false && //
            script->is_failure_script() == false, 0);
    JUDGE_RETURN(script->is_holdup() == false, 0);

    int ret = MapPlayer::prepare_fight_skill(msg);
    return ret;
}

//客户端每秒请求一次
int MapPlayerScript::request_script_detail_progress(void)
{
    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != 0, ERROR_CLIENT_OPERATE);

    Proto50400903 res;
    res.set_script_sort(this->script_sort());
    res.set_left_relive(script->left_relive_amount(this->role_id()));
    res.set_total_relive(script->total_relive_amount(this->role_id()));

    script->make_up_script_progress_detail(&res);

    ScriptPlayerDetail::ScriptRecord *record = this->script_record(script->script_sort());
    res.set_day_pass_time(record->__day_pass_times);

	FINER_PROCESS_RETURN(RETURN_SCRIPT_DETAIL_PROGRESS, &res);
}

ScriptPlayerDetail &MapPlayerScript::script_detail(void)
{
    return this->script_detail_;
}

ScriptPlayerDetail::ScriptRecord *MapPlayerScript::script_record(int script_sort)
{
	ScriptPlayerDetail::ScriptRecord &s_rec = this->script_detail_.__record_map[script_sort];
	s_rec.__script_sort = script_sort;
	return &s_rec;
}

ScriptPlayerDetail::TypeRecord *MapPlayerScript::type_record(int script_type)
{
	ScriptPlayerDetail::TypeRecordMap::iterator iter = this->script_detail_.__type_map.find(script_type);
	if (iter != this->script_detail_.__type_map.end())
	{
		return &(iter->second);
	}
	else
	{
		ScriptPlayerDetail::TypeRecord &t_rec = this->script_detail_.__type_map[script_type];
		t_rec.reset();
		t_rec.__script_type = script_type;

		return &t_rec;
	}
}

ScriptPlayerDetail::ScriptWaveRecord *MapPlayerScript::special_record(const int key)
{
	ScriptPlayerDetail::ScriptWaveMap::iterator iter = this->script_detail_.__script_wave_map.find(key);
	if (iter != this->script_detail_.__script_wave_map.end())
	{
		return &(iter->second);
	}
	else
	{
		ScriptPlayerDetail::ScriptWaveRecord &w_rec = this->script_detail_.__script_wave_map[key];
		w_rec.reset();
		w_rec.__script_wave_id = key;

		return &w_rec;
	}
}

ScriptPlayerDetail::LegendTopInfo &MapPlayerScript::top_info(int script_type)
{
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		return this->script_detail_.__legend_top_info;
	}
	else
	{
		return this->script_detail_.__sword_top_info;
	}
}

int MapPlayerScript::sync_transfer_script(void)
{
    Proto30400106 request;
    this->serialize_script(&request);
    return this->send_to_other_scene(this->scene_id(), request);
}

int MapPlayerScript::serialize_script(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400106 *, request, -1);
    ScriptPlayerDetail &detail = this->script_detail();

    request->set_script_id(detail.__script_id);
    request->set_script_sort(detail.__script_sort);
    request->set_prev_scene(detail.__prev_scene);
    request->set_prev_pixel_x(detail.__prev_coord.pixel_x());
    request->set_prev_pixel_y(detail.__prev_coord.pixel_y());
    request->set_prev_blood(detail.__prev_blood);
    request->set_prev_magic(detail.__prev_magic);
    request->set_trvl_total_pass(detail.__trvl_total_pass);
    request->set_skill_id(detail.__skill_id);

    for (ScriptPlayerDetail::ScriptRecordMap::iterator iter = detail.__record_map.begin();
            iter != detail.__record_map.end(); ++iter)
    {
        ScriptPlayerDetail::ScriptRecord &record = iter->second;
        ProtoScriptRecord *script_rec = request->add_script_record();
        script_rec->set_script_sort(record.__script_sort);
        script_rec->set_used_times(record.__used_times);
        script_rec->set_used_times_tick(record.__used_times_tick.sec());
        script_rec->set_enter_script_tick(record.__enter_script_tick.sec());
        script_rec->set_progress_id(record.__progress_id);
        script_rec->set_best_use_tick(record.__best_use_tick);
        script_rec->set_is_first_pass(record.__is_first_pass);
        script_rec->set_buy_left_times(record.__buy_times);
        script_rec->set_couple_buy_times(record.__couple_buy_times);
        script_rec->set_day_pass_times(record.__day_pass_times);
        script_rec->set_is_even_enter(record.__is_even_enter);
        script_rec->set_protect_beast_index(record.__protect_beast_index);
    }

    for (ScriptPlayerDetail::TypeRecordMap::iterator iter = detail.__type_map.begin();
        iter != detail.__type_map.end(); ++iter)
    {
    	ScriptPlayerDetail::TypeRecord &type_record = iter->second;
    	ProtoScriptType *script_type = request->add_script_type();
    	script_type->set_script_type(type_record.__script_type);
    	script_type->set_pass_wave(type_record.__pass_wave);
    	script_type->set_pass_chapter(type_record.__pass_chapter);
    	script_type->set_notify_wave(type_record.__notify_wave);
    	script_type->set_notify_chapter(type_record.__notify_chapter);
    	script_type->set_start_wave(type_record.__start_wave);
    	script_type->set_start_chapter(type_record.__start_chapter);
    	script_type->set_is_sweep(type_record.__is_sweep);
    	script_type->set_used_times_tick(type_record.__used_times_tick.sec());

    	for (IntMap::iterator it = type_record.__reward_map.begin();
    			it != type_record.__reward_map.end(); ++it)
    	{
    		ProtoPairObj* reward_map = script_type->add_reward_map();
    		reward_map->set_obj_id(it->first);
    		reward_map->set_obj_value(it->second);
    	}
    }

    for (ScriptPlayerDetail::ScriptWaveMap::iterator iter = detail.__script_wave_map.begin();
    		iter != detail.__script_wave_map.end(); ++iter)
    {
    	ScriptPlayerDetail::ScriptWaveRecord &special_record = iter->second;
    	ProtoScriptFirstAward *special_award = request->add_special_record();
    	special_award->set_script_wave_id(special_record.__script_wave_id);
    	special_award->set_is_get(special_record.__is_get);
    }

    request->set_pass_piece(detail.__piece_record.__pass_piece);
    request->set_pass_chapter(detail.__piece_record.__pass_chapter);
    for (ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator
    		iter = detail.__piece_record.__pass_chapter_map.begin();
    		iter != detail.__piece_record.__pass_chapter_map.end(); ++iter)
    {
    	ProtoPieceInfo *proto_piece_info = request->add_piece_info();
    	ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = iter->second;
    	proto_piece_info->set_chapter_key(chapter_info.__chapter_key);
    	proto_piece_info->set_used_sec(chapter_info.__used_sec);
    	proto_piece_info->set_used_times(chapter_info.__used_times);
    	proto_piece_info->set_today_pass_flag(chapter_info.__totay_pass_flag);
    }

    ProtoLegendTop *legend_top = request->mutable_legend_top();
    ProtoLegendTop *sword_top = request->mutable_sword_top();
    this->serialize_top_info(legend_top, detail.__legend_top_info);
    this->serialize_top_info(sword_top, detail.__sword_top_info);

//    request->set_pass_floor(detail.__legend_top_info.__pass_floor);
//    request->set_today_rank(detail.__legend_top_info.__today_rank);
//    request->set_is_sweep(detail.__legend_top_info.__is_sweep);
//
//    for (ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator
//    		iter = detail.__legend_top_info.__piece_map.begin();
//    		iter != detail.__legend_top_info.__piece_map.end(); ++iter)
//    {
//    	ProtoFloorInfo *proto_floor_info = request->add_floor_info();
//    	ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = iter->second;
//    	proto_floor_info->set_floor_id(floor_info.__floor_id);
//    	proto_floor_info->set_pass_tick(floor_info.__pass_tick);
//    	proto_floor_info->set_is_today_pass(floor_info.__totay_pass_flag);
//    }

    for (IntVec::iterator iter = detail.__first_script_vc.begin();
    		iter != detail.__first_script_vc.end(); ++iter)
    {
    	request->add_first_script(*iter);
    }

    for (IntMap::iterator iter = detail.__piece_record.__piece_star_award_map.begin();
    		iter != detail.__piece_record.__piece_star_award_map.end(); ++iter)
    {
    	JUDGE_CONTINUE(iter->second != 0);
    	request->add_piece_star_awarded(iter->first);
    	request->add_piece_star_awarded(iter->second);
    }

    return 0;
}

int MapPlayerScript::unserialize_script(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400106 *, request, -1);
    ScriptPlayerDetail &detail = this->script_detail();

    detail.__script_id = request->script_id();
    detail.__script_sort = request->script_sort();
    detail.__prev_scene = request->prev_scene();
    detail.__prev_coord.set_pixel(request->prev_pixel_x(), request->prev_pixel_y());
    detail.__prev_blood = request->prev_blood();
    detail.__prev_magic = request->prev_magic();
    detail.__trvl_total_pass = request->trvl_total_pass();
    detail.__skill_id = request->skill_id();

    for (int i = 0 ; i < request->script_record_size(); ++i)
    {
        const ProtoScriptRecord &script_rec = request->script_record(i);
        ScriptPlayerDetail::ScriptRecord &record = detail.__record_map[script_rec.script_sort()];
        record.reset();
        record.__script_sort = script_rec.script_sort();
        record.__used_times = script_rec.used_times();
        record.__used_times_tick.sec(script_rec.used_times_tick());
        record.__enter_script_tick.sec(script_rec.enter_script_tick());
        record.__progress_id = script_rec.progress_id();
        record.__best_use_tick = script_rec.best_use_tick();
        record.__is_first_pass = script_rec.is_first_pass();
        record.__buy_times = script_rec.buy_left_times();
        record.__couple_buy_times = script_rec.couple_buy_times();
        record.__day_pass_times = script_rec.day_pass_times();
        record.__is_even_enter = script_rec.is_even_enter();
        record.__protect_beast_index = script_rec.protect_beast_index();
    }

    for (int i = 0 ; i < request->script_type_size(); ++i)
    {
    	const ProtoScriptType &script_type = request->script_type(i);
    	ScriptPlayerDetail::TypeRecord &type_record = detail.__type_map[script_type.script_type()];
    	type_record.reset();
    	type_record.__script_type = script_type.script_type();
    	type_record.__pass_wave = script_type.pass_wave();
    	type_record.__pass_chapter = script_type.pass_chapter();
    	type_record.__notify_wave = script_type.notify_wave();
    	type_record.__notify_chapter = script_type.notify_chapter();
    	type_record.__start_wave = script_type.start_wave();
    	type_record.__start_chapter = script_type.start_chapter();
    	type_record.__is_sweep = script_type.is_sweep();
    	type_record.__used_times_tick.sec(script_type.used_times_tick());

    	for(int j = 0; j < script_type.reward_map_size(); ++j)
    	{
    		ProtoPairObj reward_map = script_type.reward_map(j);
    		int prop_id = reward_map.obj_id();
    		int prop_value = reward_map.obj_value();
    		type_record.__reward_map.insert(IntMap::value_type(prop_id, prop_value));
    	}
    }

    for (int i = 0; i < request->special_record_size(); ++i)
    {
    	const ProtoScriptFirstAward &special_award = request->special_record(i);
    	ScriptPlayerDetail::ScriptWaveRecord &special_record = detail.__script_wave_map[special_award.script_wave_id()];
    	special_record.reset();
    	special_record.__script_wave_id = special_award.script_wave_id();
    	special_record.__is_get = special_award.is_get();
    }

    detail.__piece_record.__pass_piece = request->pass_piece();
    detail.__piece_record.__pass_chapter = request->pass_chapter();
    for (int i = 0; i < request->piece_info_size(); ++i)
    {
    	const ProtoPieceInfo &proto_piece_info = request->piece_info(i);
    	ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = detail.__piece_record.__pass_chapter_map[proto_piece_info.chapter_key()];
    	chapter_info.reset();
    	chapter_info.__chapter_key = proto_piece_info.chapter_key();
    	chapter_info.__used_sec = proto_piece_info.used_sec();
    	chapter_info.__used_times = proto_piece_info.used_times();
    	chapter_info.__totay_pass_flag = proto_piece_info.today_pass_flag();
    }

    this->unserialize_top_info(request->legend_top(), detail.__legend_top_info);
    this->unserialize_top_info(request->sword_top(), detail.__sword_top_info);
//    detail.__legend_top_info.__pass_floor = request->pass_floor();
//    detail.__legend_top_info.__today_rank = request->today_rank();
//    detail.__legend_top_info.__is_sweep = request->is_sweep();
//    for (int i = 0; i < request->floor_info_size(); ++i)
//    {
//    	const ProtoFloorInfo &proto_floor_info = request->floor_info(i);
//    	ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = detail.__legend_top_info.__piece_map[proto_floor_info.floor_id()];
//    	floor_info.reset();
//    	floor_info.__floor_id = proto_floor_info.floor_id();
//    	floor_info.__pass_tick = proto_floor_info.pass_tick();
//    	floor_info.__totay_pass_flag = proto_floor_info.is_today_pass();
//    }

    for (int i = 0; i < request->first_script_size(); ++i)
    {
    	detail.__first_script_vc.push_back(request->first_script(i));
    }
    for (int i = 0; (i + 1) < request->piece_star_awarded_size(); i += 2)
    {
    	int key = request->piece_star_awarded(i), value = request->piece_star_awarded(i + 1);
    	detail.__piece_record.__piece_star_award_map[key] = value;
    }

    return 0;
}

void MapPlayerScript::serialize_top_info(ProtoLegendTop *top, ScriptPlayerDetail::LegendTopInfo &top_info)
{
	top->set_pass_floor(top_info.__pass_floor);
	top->set_today_rank(top_info.__today_rank);
	top->set_is_sweep(top_info.__is_sweep);

	for (ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator
	    	iter = top_info.__piece_map.begin();
	    	iter != top_info.__piece_map.end(); ++iter)
	{
		ProtoFloorInfo *proto_floor_info = top->add_floor_info();
	    ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = iter->second;
	    proto_floor_info->set_floor_id(floor_info.__floor_id);
	    proto_floor_info->set_pass_tick(floor_info.__pass_tick);
	    proto_floor_info->set_is_today_pass(floor_info.__totay_pass_flag);
	}
}

void MapPlayerScript::unserialize_top_info(const ProtoLegendTop &top, ScriptPlayerDetail::LegendTopInfo &top_info)
{
	top_info.__pass_floor = top.pass_floor();
	top_info.__today_rank = top.today_rank();
	top_info.__is_sweep = top.is_sweep();
	for (int i = 0; i < top.floor_info_size(); ++i)
	{
	    const ProtoFloorInfo &proto_floor_info = top.floor_info(i);
	    ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_info = top_info.__piece_map[proto_floor_info.floor_id()];
	    floor_info.reset();
	    floor_info.__floor_id = proto_floor_info.floor_id();
	    floor_info.__pass_tick = proto_floor_info.pass_tick();
	    floor_info.__totay_pass_flag = proto_floor_info.is_today_pass();
	}
}

int MapPlayerScript::script_sort(void)
{
    return this->script_detail_.__script_sort;
}

int MapPlayerScript::script_id(void)
{
    return this->script_detail_.__script_id;
}

int MapPlayerScript::script_chapter_key(void)
{
    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
    return (piece_rec.__pass_piece * 1000 + piece_rec.__pass_chapter);
}

int MapPlayerScript::convert_chapter_key(const int piece, const int chapter)
{
	return (piece * 1000 + chapter);
}

int MapPlayerScript::convert_script_wave_key(const int script_sort, const int wave)
{
	return (script_sort * 1000 + wave);
}

// 检查使用次数是否刷新
ScriptPlayerDetail::ScriptRecord *MapPlayerScript::refresh_script_record(int script_sort)
{
    ScriptPlayerDetail::ScriptRecord *record = this->script_record(script_sort);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(record->__used_times_tick <= nowtime, record);

	record->__buy_times = 0;
	record->__couple_buy_times = 0;
	record->__used_times = 0;
	record->__day_pass_times = 0;
	record->__used_times_tick = ::next_day(0, 0, nowtime);

	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	int script_type = script_json["type"].asInt();
	if (script_type == GameEnum::SCRIPT_T_CLIMB_TOWER)
	{
		ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
		for (ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.begin();
				iter != piece_rec.__pass_chapter_map.end(); ++iter)
		{
			ScriptPlayerDetail::PieceRecord::PieceChapterInfo &info = iter->second;
			info.__used_times = 0;
			info.__totay_pass_flag = 0;
		}
	}
	else if(script_type == GameEnum::SCRIPT_T_LEGEND_TOP || script_type == GameEnum::SCRIPT_T_SWORD_TOP)
	{
//		ScriptPlayerDetail::LegendTopInfo &top_info = this->script_detail_.__legend_top_info;
		ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(script_type);
		int is_sweep = 0;
		for (ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = top_info.__piece_map.begin();
				iter != top_info.__piece_map.end(); ++iter)
		{
			ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = iter->second;
			info.__pass_tick = 0;
			info.__totay_pass_flag = 0;
			is_sweep = 1;
		}
		top_info.__is_sweep = is_sweep;
		top_info.__today_rank = 0;
	}
	else if (this->is_wave_script(script_type) == true)
	{
		// 写在这里可能欠妥
		ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
		if (type_record == 0)
		{
			ScriptPlayerDetail::TypeRecord &t_rec = this->script_detail_.__type_map[script_type];
			t_rec.reset();
			t_rec.__script_type = script_type;
			type_record = &t_rec;
		}
		if (type_record->__used_times_tick <= nowtime)
		{
			if (script_type == GameEnum::SCRIPT_T_EXP)
			{
				if (type_record->__pass_wave > 0 || type_record->__pass_chapter > 0)
					type_record->__is_sweep = 1;
				else
					type_record->__is_sweep = 0;
			}
			else if (script_type == GameEnum::SCRIPT_T_LEAGUE_FB)
			{
//				type_record->__pass_wave = type_record->__start_wave;
//				type_record->__pass_chapter = type_record->__start_chapter;
				type_record->__start_wave = 0;
				type_record->__start_chapter = 0;
			}
			else
			{
				int sweep_need_chapter = CONFIG_INSTANCE->const_set("rama_open_sweep");
				if (type_record->__pass_chapter >= sweep_need_chapter)
				{
					type_record->__is_sweep = 1;
				}
				else
				{
					type_record->__pass_chapter = 0;
					type_record->__is_sweep = 0;
				}
				type_record->__pass_wave = 0;
			}
			type_record->__used_times_tick = next_day(0, 0, nowtime);
		}
	}

	if (this->is_script_compact_status())
	{
		const Json::Value &buy_json = script_json["prev_condition"]["buy_script_times"];
		record->__buy_times = int(buy_json.size());
	}

	return record;
}

int MapPlayerScript::update_script_used_times_in_enter(BaseScript *script, const Time_Value &tick)
{
    ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script->script_sort());

    MSG_DEBUG("update script used times %ld %s %d %d", this->role_id(), this->name(),
            script_record->__used_times_tick.sec(), tick.sec());

//    if (script_record->__used_times_tick <= tick || tick == Time_Value::zero)
    {
        script_record->__enter_script_tick = Time_Value::gettimeofday();
        this->process_increase_script_times(script->script_sort(), script->chapter_key());
    }


    return 0;
}

int MapPlayerScript::request_relive(Message *msg)
{
    return MapPlayer::request_relive(msg);
}

int MapPlayerScript::obtain_area_info(int request_flag)
{
	BaseScript *script = this->fetch_script();
	if (script == 0)
	{
		return MapPlayer::obtain_area_info(request_flag);
	}

	if (this->scene_id() < script->current_script_scene() ||
			(this->scene_id() == script->current_script_scene() && script->is_finish_script_scene()))
	{
//		Proto80400908 respond;
		script->make_up_finish_scene_detail(0);
		this->respond_to_client(ACTIVE_SCRIPT_SCENE_FINISH);
	}

	this->sword_script_set_skill(BaseScript::SKILL_SET);

	return MapPlayer::obtain_area_info(request_flag);
}

int MapPlayerScript::refresh_script_record_in_finish(BaseScript *script, const Time_Value &tick)
{
#ifdef LOCAL_DEBUG
    ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script->script_sort());

    MSG_DEBUG("fresh script %ld %s %d %d", this->role_id(), this->name(),
            script_record->__used_times_tick.sec(), tick.sec());
#endif

    if (script->is_finish_script() == true)
    {
    	if (script->is_climb_tower_script())
    	{
    		this->update_pass_chapter(script);
    	}
    	else if (script->is_top_script() == true)
    	{
    		this->update_pass_floor(script);
    	}

    }

    // tick 为次数有效的时间戳
    if (tick >= Time_Value::gettimeofday())
    {
        this->process_increase_script_times(script->script_sort(), script->chapter_key());
    }
    return 0;
}

int MapPlayerScript::update_pass_floor(BaseScript *script)
{
//	ScriptPlayerDetail::LegendTopInfo &top_rec = this->script_detail_.__legend_top_info;
	ScriptPlayerDetail::LegendTopInfo &top_rec = this->top_info(script->script_type());
	int floor_id = script->floor_id();
	top_rec.__pass_floor = floor_id;
	ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = top_rec.__piece_map.find(floor_id);
	if (iter == top_rec.__piece_map.end())
	{//新楼层
		ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = top_rec.__piece_map[floor_id];
		info.reset();
		info.__floor_id = floor_id;
		info.__pass_tick = Time_Value::gettimeofday().sec() + 60;
		info.__totay_pass_flag = true;
	}
	else
	{
		ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = iter->second;
		info.__pass_tick = Time_Value::gettimeofday().sec() + 60;
		info.__totay_pass_flag = true;
	}

	int script_type = script->script_type();
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		this->update_open_activity(script);
	}

	//更新问鼎江湖/武林论剑排行榜
	SCRIPT_SYSTEM->update_legend_top_rank(this, script_type, floor_id);

	this->process_pass_floor_award(script);
	this->process_first_pass_floor(script);

	return 0;
}

int MapPlayerScript::update_pass_exp(BaseScript *script)
{
	int script_type = script->script_type();
	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);

	int script_sort = script->script_sort();
	const Json::Value &script_json = script->script_conf();
	int next_script = script_json["next_script"].asInt();

	if (script->finish_wave() >= script->total_wave() && next_script > 0)
	{
		type_record->__pass_chapter += 1;
		type_record->__pass_wave = 0;
	}
	else
	{
		type_record->__pass_wave = script->finish_wave();
	}

	const Json::Value &exp_first_award = script_json["finish_condition"]["exp_first_award"];
	for (uint i = 0; i < exp_first_award.size(); ++i)
	{
		int wave_id = exp_first_award[i][0u].asInt();
		if (script->finish_wave() == wave_id)
		{
			int wave_chapter_key = this->convert_script_wave_key(script_sort, wave_id);
			ScriptPlayerDetail::ScriptWaveRecord *special_record = this->special_record(wave_chapter_key);
			special_record->__is_get = 1;

			Proto10400914 respond;
			respond.set_script_sort(script_sort);
			this->request_script_player_info(&respond);
		}
	}
	this->update_open_activity(script);
	this->update_sword_pool_info(script_type, 1);
	this->update_cornucopia_task_info(1, script_type);
	this->update_labour_task_info(1, script_type);
	this->process_pass_floor_award(script);

	return 0;
}

int MapPlayerScript::update_pass_lfb(BaseScript *script)
{
	int script_type = script->script_type();
	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);

	const Json::Value &script_json = script->script_conf();
	int next_script = script_json["next_script"].asInt();
	if (script->finish_wave() >= script->total_wave() && next_script > 0)
	{
		type_record->__start_chapter += 1;
		type_record->__start_wave = 0;
	}
	else
	{
		type_record->__start_wave = script->finish_wave();
	}

	if (type_record->__start_chapter > type_record->__pass_chapter)
	{
		type_record->__pass_chapter = type_record->__start_chapter;
		type_record->__pass_wave = type_record->__start_wave;
	}
	else if (type_record->__start_chapter == type_record->__pass_chapter)
	{
		if (type_record->__start_wave > type_record->__pass_wave)
			type_record->__pass_wave = type_record->__start_wave;
	}

	this->process_pass_floor_award(script);
	this->sync_script_wave(script);

	return 0;
}

int MapPlayerScript::update_pass_rama(BaseScript *script)
{
	int script_type = script->script_type();
	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);

	if (script->finish_wave() >= script->total_wave())
	{
		type_record->__pass_chapter += 1;
		type_record->__pass_wave = 0;

		if (type_record->__pass_chapter > type_record->__notify_chapter)
			type_record->__notify_chapter = type_record->__pass_chapter;
	}
	else
	{
		type_record->__pass_wave = script->finish_wave();
	}
	this->update_open_activity(script);
	this->process_pass_floor_award(script);

	return 0;
}

int MapPlayerScript::update_pass_chapter(BaseScript *script)
{
    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
    int key = script->chapter_key();
    ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.find(key);
    if (iter == piece_rec.__pass_chapter_map.end())
    {
    	ScriptPlayerDetail::PieceRecord::PieceChapterInfo &info = piece_rec.__pass_chapter_map[key];
    	info.reset();
        info.__chapter_key = key;
    	info.__used_sec = script->total_used_script_tick();
    }
    else
    {
    	ScriptPlayerDetail::PieceRecord::PieceChapterInfo &info = iter->second;
    	if (info.__used_sec > script->total_used_script_tick())
    		info.__used_sec = script->total_used_script_tick();
    }
//    this->process_pass_chapter_award(script);
    if (key >= (piece_rec.__pass_piece * 1000 + piece_rec.__pass_chapter))
    	this->sync_to_logic_single_script_rank(RANK_SINGLE_SCRIPT_ZYFM, key);

    int piece = script->piece(), chapter = script->chapter();
    if (piece_rec.__pass_piece > piece || (piece_rec.__pass_piece == piece && piece_rec.__pass_chapter >= chapter))
        return 0;

    // 首次通关
    piece_rec.__pass_piece = piece;
    piece_rec.__pass_chapter = chapter;
   
//    this->process_first_pass_chapter(script);

    return 0;
}

int MapPlayerScript::update_finish_script_rec(BaseScript *script, bool finish)
{
	ScriptPlayerDetail::ScriptRecord* script_rec = this->refresh_script_record(this->script_sort());
	JUDGE_RETURN(script_rec != NULL, -1);

	script_rec->__is_even_enter = 1;
	JUDGE_RETURN(finish == true, -1);

	script_rec->__day_pass_times += 1;
	script_rec->__award_star = script->script_detail().__star_lvl.size();

	int total_use_tick = script->total_used_script_tick();
	script_rec->__best_use_tick = std::min<int>(total_use_tick, script_rec->__best_use_tick);

	this->record_other_serial(SCRIPT_REL_OTHER_SERIAL, SUB_SCRIPT_FINISH,
			script_rec->__used_times, script_rec->__buy_times, script_rec->__script_sort);
	JUDGE_RETURN(script->script_type() == GameEnum::SCRIPT_T_TRVL, -1);

	this->script_detail_.__trvl_total_pass += 1;
    return 0;
}

int MapPlayerScript::update_open_activity(BaseScript *script)
{
	const Json::Value &script_json = script->script_conf();

	int type = 1; //BackSetActDetail::F_ACT_AIM_CHASE
	int day = script_json["open_activity"].asInt();

	if (script->script_type() == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		SubObj sub(day, script->floor_id());
		this->sync_open_activity_info(type, sub);
	}
	else
	{
		ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script->script_type());
		int pass_num = type_record->__pass_chapter * script->total_wave() + type_record->__pass_wave;

		SubObj sub(day, pass_num);
		this->sync_open_activity_info(type, sub);
	}

	return 0;
}

int MapPlayerScript::update_sword_pool_info(int script_type, int num)
{
	int task_id = 0;
	if (script_type == GameEnum::SCRIPT_T_ADVANCE)
	{
		task_id = GameEnum::SPOOL_TASK_ADVANCE_SCRIPT;
	}
	else if (script_type == GameEnum::SCRIPT_T_STORY)
	{
		task_id = GameEnum::SPOOL_TASK_STORY_SCRIPT;
	}
	else if (script_type == GameEnum::SCRIPT_T_EXP)
	{
		task_id = GameEnum::SPOOL_TASK_EXP_SCRIPT;
	}
	else if (script_type == GameEnum::SCRIPT_T_TRVL)
	{
		task_id = GameEnum::SPOOL_TASK_TRVL_SCRIPT;
	}
	JUDGE_RETURN(task_id != 0, 0);

	Proto31402901 inner_res;
	inner_res.set_left_add_flag(1);
	inner_res.set_left_add_num(num);
	inner_res.set_task_id(task_id);

	MSG_USER("MapPlayerScript, update_sword_pool_info, Proto31402901: %s", inner_res.Utf8DebugString().c_str());

	return this->send_to_logic_thread(inner_res);
}

int MapPlayerScript::request_first_start_script(Message *msg)
{
    BaseScript *script = this->fetch_script();
    CONDITION_NOTIFY_RETURN(script != NULL, RETURN_SCRIPT_FIRST_START,
    		ERROR_CLIENT_OPERATE);

    int ret = script->first_start_script();
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SCRIPT_FIRST_START, ret);

//    if (script->is_seven_star_script())
//    {
//    	this->request_script_detail_progress();
//    }

    return this->respond_to_client(RETURN_SCRIPT_FIRST_START);
}

int MapPlayerScript::request_stop_script(Message *msg)
{
    BaseScript *script = this->fetch_script();
    CONDITION_NOTIFY_RETURN(script != NULL,	RETURN_SCRIPT_STOP_SCRIPT,
    		ERROR_CLIENT_OPERATE);

    CONDITION_NOTIFY_RETURN(script->test_scene_flag(GameEnum::SCRIPT_SF_PREPARING_SCENE) == false,
    		RETURN_SCRIPT_STOP_SCRIPT, ERROR_CLIENT_OPERATE);

    int ret = script->stop_script_timer();
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SCRIPT_STOP_SCRIPT, ret);

    script->script_detail().__exit_script_state = GameEnum::SCRIPT_EXIT_STOPED;
    script->notify_script_progress_detail();

    FINER_PROCESS_NOTIFY(RETURN_SCRIPT_STOP_SCRIPT);
}

int MapPlayerScript::request_run_script(Message *msg)
{
//    BaseScript *script = this->fetch_script();
//    CONDITION_NOTIFY_RETURN(script != NULL,	RETURN_SCRIPT_RUN_SCRIPT,
//    		ERROR_CLIENT_OPERATE);
//
//    script->start_script_timer(msg);
//    script->script_detail().__exit_script_state = GameEnum::SCRIPT_EXIT_RUNNING;
//    script->notify_script_progress_detail();
//
//    FINER_PROCESS_NOTIFY(RETURN_SCRIPT_RUN_SCRIPT);
	return 0;
}

bool MapPlayerScript::is_movable_coord(const MoverCoord &coord)
{
    BaseScript *script = this->fetch_script();
    if (script == 0 || script->current_script_scene() != this->scene_id())
    	return MapPlayer::is_movable_coord(coord);

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
	if (scene_json["exec"].isMember("wave_spt_map") == false)
		return MapPlayer::is_movable_coord(coord);

	const Json::Value &wave_spt_json = scene_json["exec"]["wave_spt_map"];
	int current_wave = script->finish_wave() + 1, spt_id = this->scene_id();
	if (current_wave > script->total_wave())
		current_wave = script->total_wave();
	for (uint i = 0; i < wave_spt_json.size(); ++i)
	{
		if (wave_spt_json[i][0u].asInt() == current_wave)
		{
			spt_id = wave_spt_json[i][1u].asInt();
			break;
		}
	}

	return GameCommon::is_movable_coord(spt_id, coord);
}

int MapPlayerScript::validate_movable(const MoverCoord &step)
{
    BaseScript *script = this->fetch_script();
    if (script == 0 || script->current_script_scene() != this->scene_id())
        return MapPlayer::validate_movable(step);

    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    if (scene_json["exec"].isMember("wave_spt_map") == false)
        return MapPlayer::validate_movable(step);

    int ret = this->validate_fighter_movable();
    JUDGE_RETURN(ret == 0, ret);

    const Json::Value &wave_spt_json = scene_json["exec"]["wave_spt_map"];
    int current_wave = script->finish_wave() + 1, spt_id = this->scene_id();
    if (current_wave > script->total_wave())
        current_wave = script->total_wave();
    for (uint i = 0; i < wave_spt_json.size(); ++i)
    {
        if (wave_spt_json[i][0u].asInt() == current_wave)
        {
            spt_id = wave_spt_json[i][1u].asInt();
            break;
        }
    }

    if (GameCommon::is_movable_coord_no_border(spt_id, step) == false &&
    		CONFIG_INSTANCE->is_border_coord(spt_id, step.pos_x(), step.pos_y()) == false)
    {
        return ERROR_COORD_ILLEGAL;
    }
    
    return 0;
}

int MapPlayerScript::validate_relive_point(int check_type)
{
//    const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
//    const Json::Value &relive_json = script_json["relive"];

    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != NULL, ERROR_CLIENT_OPERATE);

    // 剩余复活次数校验
    int relive_times = script->left_relive_amount(this->role_id());
    JUDGE_RETURN(relive_times == -1 || relive_times > 0, ERROR_RELIVE_AMOUNT);

    return 0;
}

int MapPlayerScript::validate_relive_locate(const int item_id)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
    if (script_json.isMember("relive") == false)
        return MapPlayer::validate_relive_point(GameEnum::RELIVE_CONFIG_POINT);

    BaseScript *script = this->monitor()->find_script(this->space_id());
    JUDGE_RETURN(script != 0, ERROR_CLIENT_OPERATE);

    Time_Value nowtime = Time_Value::gettimeofday();

    const Json::Value &relive_json = script_json["relive"];
    const std::string &relive_type_str = relive_json["type"].asString();
    int type = 0;
    if (relive_type_str.length() >= GameEnum::RELIVE_LOCATE)
    {
        JUDGE_RETURN(relive_type_str[GameEnum::RELIVE_LOCATE - 1] == '1', ERROR_CONFIG_ERROR);
        type = GameEnum::RELIVE_LOCATE;
    }
    else
        return ERROR_CONFIG_NOT_EXIST;

    {
    	// 剩余复活次数校验
        int relive_times = script->left_relive_amount(this->role_id());
        JUDGE_RETURN(relive_times == -1 || relive_times > 0, ERROR_RELIVE_AMOUNT);


        int i = type - 1, use_item_id = item_id, use_amount = 1;
        if (relive_json.isMember("relive_item") == false)
            return 0;

        int used_times = script->used_relive_amout_by_item(this->role_id());
        JUDGE_RETURN(int(relive_json["item_times_amount"].size()) > used_times, ERROR_RELIVE_AMOUNT);
        use_amount = relive_json["item_times_amount"][used_times].asInt();

        if (int(relive_json["relive_item"].size()) > i)
            use_item_id = relive_json["relive_item"][i].asInt();

        JUDGE_RETURN(this->fight_detail_.__relive_tick < nowtime, -1);
        this->fight_detail_.__relive_tick = nowtime + Time_Value(3);

        this->request_use_item(INNER_RELIVE_USED_ITEM, ITEM_RELIVE_USE, use_item_id, use_amount);
    }
    return -1;
}

int MapPlayerScript::process_relive(const int relive_mode, MoverCoord &relive_coord)
{
	BaseScript *script = this->fetch_script();
	JUDGE_RETURN(script != NULL, -1);

//    const Json::Value &script_json = script->script_conf();
//    const Json::Value &relive_json = script_json["relive"];

    if (relive_mode == GameEnum::RELIVE_CONFIG_POINT)
    {
    	script->relive_coord_from_config_point(this->role_id(), relive_coord);
    }
    else
    {
    	relive_coord = this->location();
    }

    script->set_player_die_coords(this->location());
    script->reduce_relive_times(this->role_id(), relive_mode);

    return this->handle_player_relive(relive_coord, 1.0);
}

int MapPlayerScript::die_process(const int64_t fighter_id)
{
	this->process_script_player_die();
	return MapPlayer::die_process(fighter_id);
}

int MapPlayerScript::validate_prepare_attack_target(void)
{
//    if (this->is_in_seven_script())
//    {
//        GameFighter *defender = this->fetch_defender();
//        if (defender != 0 && defender->is_monster())
//        {
//            GameAI *game_ai = dynamic_cast<GameAI *>(defender);
//            BasicStatus *status = 0;
//            if (game_ai->find_status(BasicStatus::MINSUPPERMAN, status) == 0)
//                return ERROR_AI_BOSS_PROTECT;
//        }
//    }
    return MapPlayer::validate_prepare_attack_target();
}

int MapPlayerScript::process_relive_after_used_item(void)
{
	BaseScript *script = this->fetch_script();
	if (script != 0)
	{
		script->reduce_relive_times(this->role_id(), GameEnum::RELIVE_LOCATE);
	}
	return MapPlayer::process_relive_after_used_item();
}

int MapPlayerScript::schedule_move(const MoverCoord &step, const int toward, const Time_Value &arrive_tick)
{
    int ret = MapPlayer::schedule_move(step, toward, arrive_tick);
    JUDGE_RETURN(ret == 0, ret);

//    if (this->is_in_seven_script_6())
//    {
//        this->check_and_die_away_safe_range(this);
//    }

    return ret;
}

int MapPlayerScript::modify_blood_by_fight(const double value, const int fight_tips, const int64_t attackor, const int skill_id)
{
    BaseScript *script = this->fetch_script();
	if (value < 0 && fight_tips != FIGHT_TIPS_SCRIPT_AUTO 
            && fight_tips != FIGHT_TIPS_USE_PROPS)
	{
	    JUDGE_RETURN(script != 0 && //
	            script->is_finish_script() == false && //
	            script->is_failure_script() == false, 0);
	    JUDGE_RETURN(script->is_holdup() == false, 0);
	}
	
    int real_hurt = MapPlayer::modify_blood_by_fight(value, fight_tips, attackor, skill_id);

    if (real_hurt > 0 && script != 0)
    {
        script->update_player_hurt(this->role_id(), real_hurt);
    }

    return real_hurt;
}

int MapPlayerScript::request_update_script_matrix(Message *msg)
{
//    DYNAMIC_CAST_NOTIFY(Proto10400908 *, request, msg, RETURN_UPDATE_SCRIPT_MATRIX);
//
//    TowerDefenseScript *script = dynamic_cast<TowerDefenseScript *>(this->fetch_script());
//    CONDITION_NOTIFY_RETURN(script != 0, RETURN_UPDATE_SCRIPT_MATRIX, ERROR_CLIENT_OPERATE);
//    CONDITION_NOTIFY_RETURN(request->puppet() > 0, RETURN_UPDATE_SCRIPT_MATRIX, ERROR_CLIENT_OPERATE);
//    //CONDITION_NOTIFY_RETURN((script->is_single_script() == true || this->mover_id() == this->team_info().leader_id_), RETURN_UPDATE_SCRIPT_MATRIX, ERROR_CLIENT_OPERATE);
//
//    //int matrix_id = request->matrix_id();
//    //int matrix_level = script->matrix_spirit_level(matrix_id),
//    //    spirit_value = script->spirit_value();
//    //
//    //const Json::Value &matrix_json = CONFIG_INSTANCE->scene(script->current_script_scene())["exec"]["matrix"];
//    //CONDITION_NOTIFY_RETURN(matrix_json != Json::Value::null, RETURN_UPDATE_SCRIPT_MATRIX, ERROR_CONFIG_NOT_EXIST);
//
//    //for (uint i = 0; i < matrix_json.size(); ++i)
//    //{
//    //    if (matrix_json[i]["id"].asInt() != matrix_id)
//    //        continue;
//
//    //    CONDITION_NOTIFY_RETURN(matrix_level < int(matrix_json[i]["spirit"].size()), RETURN_UPDATE_SCRIPT_MATRIX, ERROR_SCRIPT_MATRIX_LEVEL);
//    //    int need_spirit = matrix_json[i]["spirit"][matrix_level].asInt();
//    //    CONDITION_NOTIFY_RETURN(spirit_value >= need_spirit, RETURN_UPDATE_SCRIPT_MATRIX, ERROR_SCRIPT_SPIRIT_VALUE);
//
//    //    script->set_spirit_value(spirit_value - need_spirit);
//    //    script->update_matrix_spirit_level(matrix_id, matrix_level + 1);
//    //    break;
//    //}
//
//    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
//    const Json::Value &puppet_json = scene_json["exec"]["puppet"];
//    CONDITION_NOTIFY_RETURN(puppet_json != Json::Value::null, RETURN_UPDATE_SCRIPT_MATRIX, ERROR_CONFIG_NOT_EXIST);
//
//    int puppet = request->puppet();
//    int puppet_sort = script->check_call_puppet(puppet, this->scene_id());
//    CONDITION_NOTIFY_RETURN(puppet_sort > 0, RETURN_UPDATE_SCRIPT_MATRIX, puppet_sort);
//
//    if (this->call_puppet(puppet_sort) == 0)
//    	script->update_puppet_call_flag(puppet);

    return 0;
}

int MapPlayerScript::inner_fetch_script_clean_times(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400919 *, request, -1);

    Proto31400902 respond;
    respond.set_type(request->type());
    respond.set_script_sort(request->script_sort());
    respond.set_script_type(request->scrit_type());
    respond.set_mult(request->mult());

    if (request->type() == GameEnum::SCRIPT_CT_SINGLE)
    {
    	CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == false, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_PLAYER_IN_SCRIPT);

    	if (request->script_sort() > 0)
    	{
    		int script_sort = request->script_sort();
    		const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    		CONDITION_NOTIFY_RETURN(script_json != Json::Value::null, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_CONFIG_NOT_EXIST);

    		int ret = this->make_up_script_clean_times(script_sort, &respond);
    		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_CLEAN_SINGLE_SCRIPT, ret);

    		int script_type = script_json["type"].asInt();
    		ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(script_type);
    		respond.set_top_floor(top_info.__pass_floor);
    	}
    	else
    	{
    		int script_type = request->scrit_type();
    		const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_type);
    		CONDITION_NOTIFY_RETURN(script_clean_json != Json::Value::null, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_CONFIG_NOT_EXIST);

    		const Json::Value &script_list = script_clean_json["script_list"];
    		CONDITION_NOTIFY_RETURN(script_list != Json::Value::null, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_CONFIG_NOT_EXIST);

    		for (uint i = 0; i < script_list.size(); ++i)
    		{
    			int script_sort = script_list[i].asInt();
    			const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    			CONDITION_NOTIFY_RETURN(script_json != Json::Value::null, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_CONFIG_NOT_EXIST);

    			int ret = this->make_up_script_clean_times(script_sort, &respond);
    			JUDGE_CONTINUE(ret == 0);

    			//剧情副本扫荡取消红点
    			if (script_type == GameEnum::SCRIPT_T_STORY)
    			{
					IntMap event_map;
					event_map[GameEnum::PA_EVENT_SCRIPT_STORY_SWEEP] = 0;
					this->check_pa_event_all_script(event_map);
    			}
    		}
    	}

        if (respond.script_list_size() <= 0)
            return this->respond_to_client_error(RETURN_CLEAN_SINGLE_SCRIPT, ERROR_NO_SCRIPT_CLEAN);
    }
    else
    {
    	CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == false, RETURN_CLEAN_ALL_SCRIPT, ERROR_PLAYER_IN_SCRIPT);
        const Json::Value &clean_order_json = CONFIG_INSTANCE->script_clean_order();
        for (uint i = 0; i < clean_order_json.size(); ++i)
        {
            this->make_up_script_clean_times(clean_order_json[i].asInt(), &respond);
        }
        if (respond.script_list_size() <= 0)
            return this->respond_to_client_error(RETURN_CLEAN_ALL_SCRIPT, ERROR_NO_SCRIPT_CLEAN);
    }
    this->cache_tick().update_cache(CACHE_SCRIPT);
    this->send_to_logic_thread(respond);

    this->drop_dragon_clean_tick_ = Time_Value::gettimeofday() + Time_Value(1);
    return 0;
}

int MapPlayerScript::fetch_script_clean_top_script(const int script_sort)
{
//	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//	const Json::Value &finish_cond_json = script_json["finish_condition"];
//	JUDGE_RETURN(finish_cond_json.isMember("share_finish_times"), script_sort);
//
//	int top_script_sort = script_sort;
//	for (int i = int(finish_cond_json["share_finish_times"].size()) - 1; i >= 0 ; --i)
//	{
//		top_script_sort = finish_cond_json["share_finish_times"][i].asInt();
//
//		const Json::Value &top_script_json = CONFIG_INSTANCE->script(top_script_sort);
//		JUDGE_CONTINUE(top_script_json != Json::Value::null);
//		JUDGE_CONTINUE(this->level() >= top_script_json["prev_condition"]["level"].asInt());
//
//	    ScriptPlayerDetail::ScriptRecord *top_script_record = this->refresh_script_record(top_script_sort);
//	    JUDGE_CONTINUE(top_script_record != NULL);
//	    JUDGE_CONTINUE(this->is_script_top_star_level(top_script_sort, top_script_record->__best_use_tick) == true);
//
//	    int left_times  = this->fetch_script_left_times(top_script_record, top_script_json);
//	    JUDGE_CONTINUE(left_times > 0);
//
//	    return top_script_sort;
//	}

	return script_sort;
}

int MapPlayerScript::make_up_script_clean_times(const int script_sort, Message *msg)
{
//    int ret = 0;
//    if (script_sort == GameEnum::SCRIPT_SORT_CLIMB_TOWER)
//    {
//        ret = this->make_up_chapter_script_clean_times(script_sort, msg);
//    }
//    else
//    {
//        ret = this->make_up_normal_script_clean_times(script_sort, msg);
//    }

    int ret = this->make_up_normal_script_clean_times(script_sort, msg);
    JUDGE_RETURN(ret == 0, ret);

    return 0;
}

int MapPlayerScript::make_up_normal_script_clean_times(const int script_sort, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400902 *, request, ERROR_SERVER_INNER);

    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

    const Json::Value &prev_condition = script_json["prev_condition"];
    JUDGE_RETURN(this->level() >= prev_condition["level"].asInt(), ERROR_PLAYER_LEVEL_LIMIT);

    if (prev_condition["is_league"].asInt() > 0)
        	CONDITION_NOTIFY_RETURN(this->league_id() > 0, RETURN_REQUEST_ENTER_SCRIPT, ERROR_LEAGUE_NO_EXIST);

    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
    JUDGE_RETURN(record != 0, ERROR_CLIENT_OPERATE);

    IntMap event_map;
    int script_type = script_json["type"].asInt();
    if (script_type == GameEnum::SCRIPT_T_ADVANCE || script_type == GameEnum::SCRIPT_T_STORY)
    {
    	JUDGE_RETURN(record->__is_first_pass > 0, ERROR_SCRIPT_NOT_PASSSED);
    }
    else if (this->is_wave_script(script_type) == true)
    {
        ScriptPlayerDetail::TypeRecord *type_info = this->type_record(script_type);
        if (script_type == GameEnum::SCRIPT_T_EXP || script_type == GameEnum::SCRIPT_T_RAMA)
        {
        	JUDGE_RETURN(type_info->__is_sweep == 1, ERROR_SCRIPT_CLEAN_TIMES);

        	request->set_pass_wave(type_info->__pass_wave);
        	request->set_pass_chapter(type_info->__pass_chapter);
        }
        else
        {
        	JUDGE_RETURN((type_info->__pass_chapter > type_info->__start_chapter)
        			|| (type_info->__pass_chapter == type_info->__start_chapter
        				&& type_info->__pass_wave > type_info->__start_wave),
        				ERROR_SCRIPT_CLEAN_TIMES);

        	if (type_info->__pass_wave == 0)
        	{
        		request->set_pass_wave(script_json["scene"][0u]["exec"]["wave"].asInt());
        		request->set_pass_chapter(type_info->__pass_chapter - 1);
        	}
        	else
        	{
        		request->set_pass_wave(type_info->__pass_wave);
        		request->set_pass_chapter(type_info->__pass_chapter);
        	}
        }

        if (script_type == GameEnum::SCRIPT_T_EXP)
        	event_map[GameEnum::PA_ENENT_SCRIPT_EXP_SWEEP] = 0;
        else if(script_type == GameEnum::SCRIPT_T_RAMA)
        	event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_SWEEP] = 0;
        else
        {
        	request->set_start_wave(type_info->__start_wave);
        	request->set_start_chapter(type_info->__start_chapter);

        	int chapter_wave = script_json["scene"][0u]["exec"]["wave"].asInt();
        	int total_wave = type_info->__pass_chapter * chapter_wave + type_info->__pass_wave;

        	Proto30100951 inner;
        	inner.set_wave(total_wave);
        	MAP_MONITOR->dispatch_to_logic(this, &inner);
        }
    }

    int left_times = this->fetch_script_left_times(record, script_json);

    if (prev_condition["is_tower"].asInt() == true)
    {
    	ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(script_type);
    	CONDITION_NOTIFY_RETURN((top_info.__pass_floor >= 1)&&(top_info.__is_sweep == 1),
    			RETURN_REQUEST_ENTER_SCRIPT, ERROR_MUST_SWEEP);

    	left_times = 1;

    	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
    		event_map[GameEnum::PA_EVENT_SCRIPT_LEGEND_SWEEP] = 0;
    	else if (script_type == GameEnum::SCRIPT_T_SWORD_TOP)
    		event_map[GameEnum::PA_EVENT_SCRIPT_SWORD_SWEEP] = 0;
    }

    //扫荡后取消红点
    this->check_pa_event_all_script(event_map);

    JUDGE_RETURN(left_times > 0, ERROR_SCRIPT_CLEAN_TIMES);

    this->process_increase_script_times(script_sort, 0, left_times, true);

    ProtoScriptClean *proto_clean = request->add_script_list();
    proto_clean->set_script_sort(script_sort);
    proto_clean->set_script_times(left_times);
    proto_clean->set_protect_beast_index(record->__protect_beast_index);

    int reset_times = record->__used_times - prev_condition["finish_times"].asInt();
    reset_times = reset_times < 0 ? 0 : reset_times;
    proto_clean->set_reset_times(reset_times);

    return 0;
}

int MapPlayerScript::make_up_chapter_script_clean_times(const int script_sort, Message *msg)
{
//    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
//
//    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
//    JUDGE_RETURN(record != 0, -1);
//
//    DYNAMIC_CAST_RETURN(Proto31400902 *, request, msg, ERROR_SERVER_INNER);
//
//    int total_times = script_json["prev_condition"]["finish_times"].asInt();
//    total_times += GameCommon::script_vip_extra_use_times(this->vip_type(), script_sort);
//
//    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//    const Json::Value &piece_json = script_json["prev_condition"]["piece"];
//    for (uint piece_index = 0; piece_index < piece_json.size(); ++piece_index)
//    {
//        int max_chapter = piece_json[piece_index].asInt();
//        for (int chapter_index = 0; chapter_index < max_chapter; ++chapter_index)
//        {
//            int chapter_key = (piece_index + 1) * 1000 + chapter_index + 1;
//            ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.find(chapter_key);
//            if (iter == piece_rec.__pass_chapter_map.end())
//                break;
//
//            ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = iter->second;
//            int left_times  = total_times - chapter_info.__used_times;
//            if (left_times <= 0 || this->is_script_top_star_level(script_sort, chapter_info.__used_sec) == false)
//                continue;
//
//            ProtoScriptClean *proto_clean = request->add_script_list();
//            proto_clean->set_script_sort(script_sort);
//            proto_clean->set_chapter_key(chapter_info.__chapter_key);
//            proto_clean->set_script_times(left_times);
//
//            this->process_increase_script_times(script_sort, chapter_key, left_times, true);
//        }
//    }

    return 0;
}

int MapPlayerScript::rollback_script_times(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto31400903 *, request, msg, -1);
//
//	for (int i = 0; i < request->rollback_script_size(); ++i)
//	{
//		const ProtoScriptClean &proto_roll =request->rollback_script(i);
//		ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(proto_roll.script_sort());
//		if (record == 0)
//			continue;
//
//		record->__used_times -= proto_roll.script_times();
//        if (record->__used_times < 0)
//            record->__used_times = 0;
//        this->sync_share_script_used_times(record->__script_sort, record->__used_times);
//
//        if (proto_roll.script_sort() == GameEnum::SCRIPT_SORT_CLIMB_TOWER && proto_roll.chapter_key() > 0)
//        {
//            ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//            ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator chapter_iter = piece_rec.__pass_chapter_map.find(proto_roll.chapter_key());
//            if (chapter_iter == piece_rec.__pass_chapter_map.end())
//            {
//                ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = piece_rec.__pass_chapter_map[proto_roll.chapter_key()];
//                chapter_info.reset();
//                chapter_info.__chapter_key = proto_roll.chapter_key();
//            }
//            else
//            {
//                ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = chapter_iter->second;
//                chapter_info.__used_times -= proto_roll.script_times();
//                chapter_info.__totay_pass_flag = 0;
//            }
//        }
//	}
//
//    this->process_check_pa_event_script_times();
//
//	this->cache_tick().update_cache(CACHE_SCRIPT);
	return 0;
}

int MapPlayerScript::request_chapter_script_detail(Message *msg)
{
//    DYNAMIC_CAST_NOTIFY(Proto10400909 *, request, msg, RETURN_CHAPTER_SCRIPT_DETAIL);
//
//    const Json::Value &script_json = CONFIG_INSTANCE->script(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//    CONDITION_NOTIFY_RETURN(script_json != Json::Value::null, RETURN_CHAPTER_SCRIPT_DETAIL, ERROR_CONFIG_NOT_EXIST);
//
//    const Json::Value &piece_json = script_json["prev_condition"]["piece"];
//    CONDITION_NOTIFY_RETURN(0 < request->piece() && request->piece() <= int(piece_json.size()), RETURN_CHAPTER_SCRIPT_DETAIL, ERROR_CLIENT_OPERATE);
//
//    const Json::Value &chapter_scene_json = script_json["prev_condition"]["chapter_scene"];
//
//    Proto50400909 respond;
//    respond.set_piece(request->piece());
//
//    this->refresh_script_record(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//
//    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//    int max_chapter = piece_json[request->piece() - 1].asInt(), prev_chapter_open = 1;
//    for (int i = 1; i <= max_chapter; ++i)
//    {
//        ProtoScriptChapter *proto_chapter = respond.add_chapter_info();
//        proto_chapter->set_chapter(i);
//
//        int key = request->piece() * 1000 + i;
//        ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.find(key);
//        if (iter != piece_rec.__pass_chapter_map.end())
//        {
//            ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = iter->second;
//            proto_chapter->set_is_passed(chapter_info.__totay_pass_flag);
//            proto_chapter->set_is_started(1);
//            proto_chapter->set_best_tick(chapter_info.__used_sec);
//            prev_chapter_open = 1;
////            proto_chapter->set_start_level(this->script_star_level(GameEnum::SCRIPT_SORT_CLIMB_TOWER, chapter_info.__used_sec));
//            proto_chapter->set_start_level(0);
//        }
//        else
//        {
//            proto_chapter->set_is_passed(0);
//            proto_chapter->set_is_started(0);
//            proto_chapter->set_best_tick(0);
//            if (prev_chapter_open == 1)
//            {
//                prev_chapter_open = 0;
//                for (uint j = 0; j < chapter_scene_json.size(); ++j)
//                {
//                    if (chapter_scene_json[j][0u].asInt() != request->piece())
//                        continue;
//                    if (i < chapter_scene_json[j][1u].asInt() || chapter_scene_json[j][2u].asInt() < i)
//                        continue;
//
//                    if (this->level() >= chapter_scene_json[j][4u].asInt())
//                        proto_chapter->set_is_started(1);
//                    break;
//                }
//            }
//        }
//    }
//
//    Proto30400903 inner_req;
//    inner_req.set_res_recogn(RETURN_CHAPTER_SCRIPT_DETAIL);
//    respond.SerializeToString(inner_req.mutable_res_body());
//
//    int script_scene = script_json["scene"][0u]["scene_id"].asInt();
//    CONDITION_NOTIFY_RETURN(script_scene > 0, RETURN_CHAPTER_SCRIPT_DETAIL, ERROR_CONFIG_ERROR);
//
//    MSG_USER("chapter script %d", script_scene);
//
//    return this->monitor()->dispatch_to_scene(this, script_scene, &inner_req);
	return 0;
}

int MapPlayerScript::request_extract_script_card(Message *msg)
{
//    DYNAMIC_CAST_NOTIFY(Proto10400910 *, request, msg, RETURN_EXTRACT_SCRIPT_CARD);
//
//    CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == true, RETURN_EXTRACT_SCRIPT_CARD, ERROR_NO_IN_SCRIPT);
//
//    int script_sort = this->script_sort();
//    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//    CONDITION_NOTIFY_RETURN(script_json != Json::Value::null, RETURN_EXTRACT_SCRIPT_CARD, ERROR_CONFIG_NOT_EXIST);
//    //CONDITION_NOTIFY_RETURN(0 < request->index() && request->index() <= int(script_json["finish_condition"]["award_item"].size()), RETURN_EXTRACT_SCRIPT_CARD, ERROR_CLIENT_OPERATE);
//
//    ScriptPlayerDetail::ScriptRecord *script_rec = this->refresh_script_record(script_sort);
//    CONDITION_NOTIFY_RETURN(script_rec->__award_star > 0, RETURN_EXTRACT_SCRIPT_CARD, ERROR_NO_SCRIPT_AWARD);
//
//    if (script_json["type"].asInt() == GameEnum::SCRIPT_T_MONSTER_TOWER)
//    {
//    	CONDITION_NOTIFY_RETURN(script_rec->__day_pass_times == 1, RETURN_EXTRACT_SCRIPT_CARD, ERROR_NO_SCRIPT_AWARD);
//    }
//
//    int chapter_key = 0;
//    BaseScript *script = this->fetch_script();
//    if (script != 0)
//    {
//        chapter_key = script->chapter_key();
//    }
//
//    Proto30400904 req;
//    req.set_script_sort(script_sort);
//    req.set_award_star(script_rec->__award_star);
//    req.set_index(request->index());
//    req.set_chapter_key(chapter_key);
//    this->send_to_logic_thread(req);
//
//    script_rec->__award_star = 0;
//
//    this->cache_tick().update_cache(MapPlayer::CACHE_SCRIPT);
    return 0;
}

int MapPlayerScript::request_script_piece_detail(void)
{
//    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//
//    const Json::Value &script_json = CONFIG_INSTANCE->script(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//    const Json::Value &piece_json = script_json["prev_condition"]["piece"];
//
//    int piece = piece_rec.__pass_piece;
//    if (piece == 0)
//        piece = 1;
//    if (piece <= int(piece_json.size()))
//    {
//        if (piece_rec.__pass_chapter == piece_json[piece - 1].asInt())
//            ++piece;
//    }
//
//    if (piece > int(piece_json.size()))
//        piece = int(piece_json.size());
//
//    Proto50400911 respond;
//    respond.set_max_piece(piece);
//
//    return this->respond_to_client(RETURN_SCRIPT_PIECE_DETAIL, &respond);
	return 0;
}

int MapPlayerScript::fetch_script_clean_tick(const int script_sort, int &left_times, int &use_tick, IntMap &script_times_map)
{
//    const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_sort);
//    JUDGE_RETURN(script_clean_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
//
//    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
//    JUDGE_RETURN(record != 0, ERROR_SERVER_INNER);
//
//    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
//
//    int total_times = script_json["prev_condition"]["finish_times"].asInt();
//    total_times += GameCommon::script_vip_extra_use_times(this->vip_type(), script_sort);
//
//    left_times = 0;
//    if (script_sort == GameEnum::SCRIPT_SORT_CLIMB_TOWER)
//    {
//        const Json::Value &piece_json = script_json["prev_condition"]["piece"];
//
//        int piece_id = 0, chapter_id = 0;
//        ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//        for (ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.begin();
//                iter != piece_rec.__pass_chapter_map.end(); ++iter)
//        {
//            ScriptPlayerDetail::PieceRecord::PieceChapterInfo &chapter_info = iter->second;
//            piece_id = chapter_info.__chapter_key / 1000;
//            chapter_id = chapter_info.__chapter_key % 1000;
//            if (piece_id > int(piece_json.size()) || piece_id <= 0)
//                continue;
//            if (chapter_id > piece_json[piece_id - 1].asInt() || chapter_id <= 0)
//                continue;
//
//            int single_left_times = total_times - chapter_info.__used_times;
//            if (single_left_times <= 0 || this->is_script_top_star_level(script_sort, chapter_info.__used_sec) == false)
//                continue;
//            left_times += single_left_times;
//        }
//    }
//    else
//    {
//    	JUDGE_RETURN(this->is_script_top_star_level(script_sort, record->__best_use_tick) == true, ERROR_SCRIPT_STAR_LEVEL);
//        left_times = this->fetch_script_left_times(record, script_json);
//        left_times -= script_times_map[script_sort];
//        if (left_times < 0)
//            left_times = 0;
//
//        if (left_times > 0)
//        {
//            if (script_json["finish_condition"].isMember("share_finish_times"))
//            {
//                const Json::Value &share_json = script_json["finish_condition"]["share_finish_times"];
//                for (uint i = 0; i < share_json.size(); ++i)
//                {
//                    script_times_map[share_json[i].asInt()] += left_times;
//                }
//            }
//        }
//    }
//    use_tick = left_times * script_clean_json["use_tick"].asInt();
//
//    int vip_clean_boost = GameCommon::script_clean_out_vip_boost(this->vip_type());
//    int vip_minus_tick = ::rint((use_tick * vip_clean_boost) / 100.0);
//    use_tick = MAX(use_tick - vip_minus_tick, 0);
//
//    JUDGE_RETURN(left_times > 0, ERROR_SCRIPT_CLEAN_TIMES);

    return 0;
}

int MapPlayerScript::request_single_clean_script_tick(Message *msg)
{
//    DYNAMIC_CAST_NOTIFY(Proto10400912 *, request, msg, RETURN_SINGLE_CLEAN_SCRIPT_TICK);
//    CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == false, RETURN_SINGLE_CLEAN_SCRIPT_TICK, ERROR_PLAYER_IN_SCRIPT);
//
//    int script_sort = request->script_sort();
//    CONDITION_NOTIFY_RETURN(script_sort > 0, RETURN_SINGLE_CLEAN_SCRIPT_TICK, ERROR_CLIENT_OPERATE);
//
//    int left_times = 0, use_tick = 0;
//    IntMap script_times_map;
//
//    int ret = 0;
//    {
//        ret = this->fetch_script_clean_tick(script_sort, left_times, use_tick, script_times_map);
//        CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SINGLE_CLEAN_SCRIPT_TICK, ret);
//    }
//    CONDITION_NOTIFY_RETURN(left_times > 0, RETURN_SINGLE_CLEAN_SCRIPT_TICK, ERROR_SCRIPT_FINISH_TIMES);
//
//    Proto50400912 respond;
//    respond.set_script_sort(script_sort);
//    respond.set_left_times(left_times);
//    respond.set_use_tick(use_tick);
//    return this->respond_to_client(RETURN_SINGLE_CLEAN_SCRIPT_TICK, &respond);
	return 0;
}

int MapPlayerScript::request_clean_all_script_tick(Message *msg)
{
//    CONDITION_NOTIFY_RETURN(this->is_in_script_mode() == false, RETURN_CLEAN_ALL_SCRIPT_TICK, ERROR_PLAYER_IN_SCRIPT);
//
//    IntMap script_times_map;
//    int use_tick = 0, left_times = 0, single_use_tick = 0, total_left_times = 0;
//    const Json::Value &clean_order_json = CONFIG_INSTANCE->script_clean_order();
//    for (uint i = 0; i < clean_order_json.size(); ++i)
//    {
//        int script_sort = clean_order_json[i].asInt();
//        single_use_tick = 0;
//        left_times = 0;
//        int ret = this->fetch_script_clean_tick(script_sort, left_times, single_use_tick, script_times_map);
//        if (ret != 0)
//            continue;
//
//        total_left_times += left_times;
//        use_tick += single_use_tick;
//    }
//    CONDITION_NOTIFY_RETURN(total_left_times > 0, RETURN_CLEAN_ALL_SCRIPT_TICK, ERROR_SCRIPT_FINISH_TIMES);
//
//    Proto50400913 respond;
//    respond.set_use_tick(use_tick);
//    return this->respond_to_client(RETURN_CLEAN_ALL_SCRIPT_TICK, &respond);
	return 0;
}

int MapPlayerScript::sync_team_script_use_times(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400905*, request, -1);
    int script_sort = request->script_sort();

	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	JUDGE_RETURN(Json::Value::null != script_json, -1);

	ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script_sort);
	int left_times = this->fetch_script_left_times(script_record, script_json);

    Proto30100701 sync_info;
    sync_info.set_script_sort(script_sort);
    sync_info.set_use_times(left_times);
    sync_info.set_oper_type(request->oper_type());

    return this->monitor()->dispatch_to_logic(this, &sync_info);
}

int MapPlayerScript::process_increase_script_times(const int script_sort, const int chapter_key, const int finish_times, const bool is_script_clean)
{
    ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script_sort);
    script_record->__progress_id = 0;
    script_record->__used_times += finish_times;
    this->sync_share_script_used_times(script_sort, script_record->__used_times);

    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    const Json::Value &prev_condition = script_json["prev_condition"];
    int script_type = script_json["type"].asInt();
    if (prev_condition["is_tower"].asInt() == true)
    {
    	ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(script_type);
    	top_info.__is_sweep = false;

    	int pass_floor = top_info.__pass_floor;
    	for (int i = 1; i <= pass_floor; ++i)
    	{
    		ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = top_info.__piece_map.find(i);
    		if (iter != top_info.__piece_map.end())
    		{
    			ScriptPlayerDetail::LegendTopInfo::FloorInfo &info = iter->second;
    			info.__pass_tick = Time_Value::gettimeofday().sec() + 60;
    			info.__totay_pass_flag = 1;
    		}
    	}
    	this->script_clean_sync_restore(script_sort);

    	Proto30400912 rank_inner;
    	rank_inner.set_role_id(this->role_id());
    	rank_inner.set_role_name(this->name());
    	rank_inner.set_fight_score(this->role_detail().__fight_force);
    	rank_inner.set_floor(pass_floor);
    	rank_inner.set_script_type(script_type);
    	MAP_MONITOR->dispatch_to_scene(this, GameEnum::LEGEND_TOP_SCENE, &rank_inner);

    	Proto10400914 res_script;
    	res_script.set_script_sort(script_sort);
    	this->request_script_player_info(&res_script);
    	return 0;
    }

    if (this->is_wave_script(script_type) == true)
    {
    	ScriptPlayerDetail::TypeRecord *type_info = this->type_record(script_type);
    	type_info->__is_sweep = false;

    	if (script_type == GameEnum::SCRIPT_T_LEAGUE_FB && is_script_clean == true)
    	{
    		type_info->__start_wave = type_info->__pass_wave;
    		type_info->__start_chapter = type_info->__pass_chapter;
    	}
    }

    // 更新剑池任务
    if (script_type == GameEnum::SCRIPT_T_EXP)
    {
    	if (is_script_clean == true)
    	{
			ScriptPlayerDetail::TypeRecord *type_info = this->type_record(script_type);
			int num = type_info->__pass_wave + type_info->__pass_chapter * 20;
			this->update_sword_pool_info(script_type, num);
    	}
    }
    else
    {
    	this->update_sword_pool_info(script_type, finish_times);
    }

    this->update_cornucopia_task_info(finish_times, script_type);
    this->update_labour_task_info(finish_times, script_type);
    this->check_pa_event_script_type_finish(script_type);
    this->script_clean_sync_restore(script_sort);
    this->request_script_type_info(script_sort);
    return 0;
}

//玩家死亡
int MapPlayerScript::process_script_player_die()
{
	BaseScript *script = this->fetch_script();

	JUDGE_RETURN(script != NULL, -1);
	JUDGE_RETURN(script->check_script_scene_failure() == 0, -1);

	return script->process_scene_tick_failure();
}

int MapPlayerScript::process_script_travel(BaseScript *script)
{
	ScriptPlayerDetail::ScriptRecord *script_rec = this->refresh_script_record(script->script_sort());
	JUDGE_RETURN(script_rec != NULL, -1);

	//正常奖励
	ThreeObjVec noraml_reward_vec;
	script->fetch_pass_total_reward(noraml_reward_vec,
			this->script_detail_.__trvl_total_pass);

	//发到背包的奖励
	RewardInfo normal_reward_info(false, this);
	GameCommon::make_up_reward_items(normal_reward_info, noraml_reward_vec);

	//通知客户端
	Proto80400906 req;
	script->make_up_script_finish_detail(this, &req, NULL);

	//星级
	IntVec& star_lvl = script->script_detail().__star_lvl;
	for (IntVec::iterator iter = star_lvl.begin(); iter != star_lvl.end(); ++iter)
	{
		req.add_star_level(*iter);
	}

	//首通奖励
	if (script_rec->__is_first_pass == false)
	{
	    req.set_is_first_pass(true);
	    script_rec->__is_first_pass = true;

	    ThreeObjVec first_reward_vec;
	    script->fetch_first_reward(first_reward_vec);

	    RewardInfo first_reward_info(false);
	    GameCommon::make_up_reward_items(first_reward_info, first_reward_vec);

	    SerialObj serial(ADD_FROM_SCRIPT_FIRST_PASS, script_rec->__script_sort);
	    this->request_add_mult_item(first_reward_info, serial);
	}

	//通关奖励
	{
	    SerialObj serail(ADD_FROM_SCRIPT_PASS, script_rec->__script_sort);
	    this->request_add_mult_item(normal_reward_info, serail);
	}

	//展示客户端的奖励
	for (ItemObjVec::iterator iter = normal_reward_info.item_vec_.begin();
			iter != normal_reward_info.item_vec_.end(); ++iter)
	{
	   	ProtoItem* proto = req.add_item();
	   	iter->serialize(proto);
	}

	FINER_PROCESS_RETURN(ACTIVE_FINISH_SCRIPT, &req);
}

//通关奖励
int MapPlayerScript::process_script_pass_award(BaseScript *script)
{
	ScriptPlayerDetail::ScriptRecord *script_rec = this->refresh_script_record(script->script_sort());
	JUDGE_RETURN(script_rec != NULL, -1);

	//正常奖励
	ThreeObjVec noraml_reward_vec;
	script->fetch_pass_total_reward(noraml_reward_vec,
			this->script_detail_.__trvl_total_pass);

	//发到背包的奖励
	RewardInfo normal_reward_info(false, this);
	GameCommon::make_up_reward_items(normal_reward_info, noraml_reward_vec);

	int mult = SCRIPT_SYSTEM->fetch_script_mult(script->script_type());

	//通知客户端
	Proto80400929 req;
	req.set_mult(1);
    script->make_up_script_finish_detail(this, NULL, &req);

    //星级
    IntVec& star_lvl = script->script_detail().__star_lvl;
	for (IntVec::iterator iter = star_lvl.begin(); iter != star_lvl.end(); ++iter)
	{
		req.add_star_level(*iter);
	}

	//首通奖励
    if (script_rec->__is_first_pass == false)
    {
    	req.set_is_first_pass(true);
    	script_rec->__is_first_pass = true;

    	ThreeObjVec first_reward_vec;
    	script->fetch_first_reward(first_reward_vec);

    	RewardInfo first_reward_info(false);
    	GameCommon::make_up_reward_items(first_reward_info, first_reward_vec);

    	SerialObj serial(ADD_FROM_SCRIPT_FIRST_PASS, script_rec->__script_sort);
    	this->request_add_mult_item(first_reward_info, serial);
    }

    //通关奖励
    {
    	SerialObj serail(ADD_FROM_SCRIPT_PASS, script_rec->__script_sort);
    	this->request_add_mult_item(normal_reward_info, serail, mult);
    }

    //更新资源找回
    script->sync_restore_pass(this);

    //针对经验副本,罗摩副本,帮派副本等已经发过的奖励
    ThreeObjVec history_reward_vec;
    script->fetch_wave_reward(history_reward_vec);
    GameCommon::make_up_reward_items(normal_reward_info, history_reward_vec);

    //展示客户端的奖励
   	for (ItemObjVec::iterator iter = normal_reward_info.item_vec_.begin();
   			iter != normal_reward_info.item_vec_.end(); ++iter)
   	{
   		ProtoItem* proto = req.add_item();
   		iter->serialize(proto);
   	}

	FINER_PROCESS_RETURN(ACTIVE_RETURN_FINISH_SCRIPT, &req);
}

int MapPlayerScript::process_script_pass_award(BaseScript *script, bool first)
{
	const Json::Value &script_json = script->script_conf();
    JUDGE_RETURN(script_json.empty() == false, -1);

    ThreeObjVec reward_vec;
    if (first == true)
	{
    	script->fetch_first_reward(reward_vec);
	}
    else
    {
    	int reward_id = script->fetch_normal_reward();
    	JUDGE_RETURN(reward_id > 0, -1);
    	reward_vec.push_back(ThreeObj(reward_id));
    }

    //更新资源找回
    script->sync_restore_pass(this);

    JUDGE_RETURN(reward_vec.empty() == false, -1);

    int mult = SCRIPT_SYSTEM->fetch_script_mult(script->script_type());

    Proto30400906 request;
    request.set_is_first_pass(first);
    request.set_script_sort(script->script_sort());
    request.set_chapter_key(script->chapter_key());
    request.set_floor(script->fetch_reward_index());
    request.set_mult(mult);

    for (ThreeObjVec::iterator iter = reward_vec.begin(); iter != reward_vec.end(); ++iter)
    {
    	JUDGE_CONTINUE(iter->id_ > 0);
        request.add_reward_id(iter->id_);
    }
    return this->send_to_logic_thread(request);
}

int MapPlayerScript::process_pass_floor_award(BaseScript *script)
{
	return this->process_script_pass_award(script, false);
}

int MapPlayerScript::process_first_pass_floor(BaseScript *script)
{
	return this->process_script_pass_award(script, true);
}

int MapPlayerScript::sync_script_wave(BaseScript *script)
{
	int script_type = script->script_type();
	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);

	const Json::Value &script_json = script->script_conf();
	int chapter_wave = script_json["scene"][0u]["exec"]["wave"].asInt();
	int total_wave = type_record->__start_chapter * chapter_wave + type_record->__start_wave;

	Proto30100951 inner;
	inner.set_wave(total_wave);
	MAP_MONITOR->dispatch_to_logic(this, &inner);

	return 0;
}

int MapPlayerScript::fetch_relive_data(const int relive_mode, int &blood_percent, int &item_id, int &item_amount)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
    const Json::Value &relive_json = script_json["relive"];
    
    int ret = fetch_relive_data_common(relive_json, relive_mode, blood_percent, item_id, item_amount);
    JUDGE_RETURN(ret == 0, ret);

    if (relive_json.isMember("item_times_amount"))
    {
        int used_relive_times = 0;
        BaseScript *script = this->fetch_script();
        if (script != 0)
            used_relive_times = script->used_relive_amout_by_item(this->role_id());
        if (used_relive_times < int(relive_json["item_times_amount"].size()))
            item_amount = relive_json["item_times_amount"][used_relive_times].asInt();
    }
    return 0;
}

int MapPlayerScript::set_script_wave_task(Message *msg)
{
	this->script_detail_.__task_listen = true;
	return 0;
}

int MapPlayerScript::check_script_wave_task(BaseScript *script)
{
	JUDGE_RETURN(this->script_detail_.__task_listen == true, -1);

	Proto31400601 inner;
	inner.set_script_sort(this->scene_id());

	int passed = script->fetch_task_wave();
	inner.set_pass_chapter(passed);
	return this->send_to_logic_thread(inner);
}

int MapPlayerScript::request_script_type_info(const int script_sort)
{
	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	JUDGE_RETURN(script_json != Json::Value::null, -1);

	int script_type = script_json["type"].asInt();
	if (this->is_wave_script(script_type) == true || script_type == GameEnum::SCRIPT_T_STORY)
	{
		const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
		for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
		     iter != script_map.end(); ++iter)
		{
			const Json::Value &script_info = *(iter->second);
			int type = script_info["type"].asInt();
			if (type == script_type)
			{
				Proto10400914 respond;
				respond.set_script_sort(iter->first);
				this->request_script_player_info(&respond);
			}
		}
	}
	else
	{
		Proto10400914 respond;
		respond.set_script_sort(script_sort);
		this->request_script_player_info(&respond);
	}

	return 0;
}

int MapPlayerScript::request_script_player_info(Message *msg)
{
    MSG_DYNAMIC_CAST_NOTIFY(Proto10400914*, request, RETURN_SCRIPT_PLAYER_INFO);

    int script_sort = request->script_sort();
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    CONDITION_NOTIFY_RETURN(script_json.empty() == false,
    		RETURN_SCRIPT_PLAYER_INFO, ERROR_CONFIG_NOT_EXIST);

    Proto50400914 respond;
    ProtoScriptInfo *script_info = respond.mutable_script_info();
    script_info->set_script_sort(script_sort);

    int script_type = script_json["type"].asInt();
    const Json::Value &prev_condition = script_json["prev_condition"];
    const Json::Value &finish_condition = script_json["finish_condition"];
    ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script_sort);

    if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
    {
    	ScriptPlayerDetail::LegendTopInfo &legend_top_info = this->script_detail_.__legend_top_info;
    	script_info->set_pass_floor(legend_top_info.__pass_floor);
    	script_info->set_today_rank(legend_top_info.__today_rank);
    	script_info->set_is_sweep(legend_top_info.__is_sweep);

    	int total_floor = prev_condition["total_floor"].asInt();
    	const Json::Value &special_item = finish_condition["special_item"];
		for (int i = 1; i <= total_floor; ++i)
		{
			ProtoFloorInfo *floor_info = script_info->add_floor_info();
			floor_info->set_floor_id(i);

			ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = legend_top_info.__piece_map.find(i);
			if (iter != legend_top_info.__piece_map.end())
			{
				ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_detail = iter->second;
				floor_info->set_is_today_pass(floor_detail.__totay_pass_flag);
				floor_info->set_pass_tick(floor_detail.__pass_tick);
			} else
			{
				floor_info->set_is_today_pass(0);
				floor_info->set_pass_tick(0);
			}
			floor_info->set_is_special(0); //初始化
			for (uint j = 0; j < special_item.size(); ++j)
			{
				if (special_item[j][0u].asInt() != i)
				{
					continue;
				}
				floor_info->set_is_special(1);
			}
		}

		return this->respond_to_client(RETURN_SCRIPT_PLAYER_INFO, &respond);
    }
    else if (script_type == GameEnum::SCRIPT_T_SWORD_TOP)
    {
        ScriptPlayerDetail::LegendTopInfo &sword_top_info = this->script_detail_.__sword_top_info;
        script_info->set_pass_floor(sword_top_info.__pass_floor);
        script_info->set_is_sweep(sword_top_info.__is_sweep);
        script_info->set_skill_id(this->script_detail_.__skill_id);

        return this->respond_to_client(RETURN_SCRIPT_PLAYER_INFO, &respond);
    }

    int left_times = this->fetch_script_left_times(script_record, script_json),
    left_buy_times = this->fetch_script_left_buy_times(script_record, script_json);

    int is_open = true;
    if (this->level() < prev_condition["level"].asInt())
    {
    	is_open = 0;
    }

    if (script_type == GameEnum::SCRIPT_T_ADVANCE)
    {
		int is_sweep = script_record->__is_first_pass > 0 ? 1 : 0;
		int reset_times = script_record->__used_times - prev_condition["finish_times"].asInt();
//		reset_times = reset_times < 0 ? 0 : reset_times;
		script_info->set_reset_times(reset_times);
		script_info->set_is_sweep(is_sweep);
	}
    else if (this->is_wave_script(script_type) == true)
    {
		ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
		script_info->set_pass_piece(type_record->__pass_wave);
		script_info->set_pass_chapter(type_record->__pass_chapter);
		script_info->set_start_wave(type_record->__start_wave);
		script_info->set_start_chapter(type_record->__start_chapter);
		script_info->set_is_sweep(type_record->__is_sweep);

		if (script_type == GameEnum::SCRIPT_T_EXP)
		{
			const Json::Value &exp_first_award = finish_condition["exp_first_award"];
			for (uint i = 0; i < exp_first_award.size(); ++i)
			{
				int wave_id = exp_first_award[i][0u].asInt();
				int wave_chapter_key = this->convert_script_wave_key(script_sort, wave_id);
				ScriptPlayerDetail::ScriptWaveRecord *special_record = this->special_record(wave_chapter_key);

				ProtoSpecialAwardInfo *special_info = script_info->add_special_info();
				special_info->set_script_wave_id(special_record->__script_wave_id);
				special_info->set_script_sort(script_sort);
				special_info->set_wave(wave_id);
				special_info->set_is_get(special_record->__is_get);
			}
		}
	}
    else if (script_type == GameEnum::SCRIPT_T_COUPLES)
    {
    	int couple_buy = script_record->__couple_buy_times;
        int left_get = prev_condition["get_times"].asInt() - couple_buy;
        left_get = left_get > 0 ? left_get : 0;

        script_info->set_left_get(left_get);
    }

    script_info->set_left_times(left_times);
    script_info->set_left_buy_times(left_buy_times);
    script_info->set_buy_times(script_record->__buy_times);
    script_info->set_is_first_pass(script_record->__is_first_pass);
    script_info->set_is_open(is_open);
    FINER_PROCESS_RETURN(RETURN_SCRIPT_PLAYER_INFO, &respond);
}

int MapPlayerScript::request_script_list_info(Message *msg)
{
    Proto31400905 request;
    return this->send_to_logic_thread(request);
}

int MapPlayerScript::process_script_list_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400905 *, request, -1);

    IntSet task_set;
    for (int i = 0; i < request->task_list_size(); ++i)
    {
        task_set.insert(request->task_list(i));
    }

    Proto50400918 respond;
    respond.set_trvl_total_pass(this->script_detail_.__trvl_total_pass);
    respond.set_max_trvl_pass(this->script_detail_.__max_trvl_pass);

    //发送红点的事件类型
    IntMap event_map;
    const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
    for (GameConfig::ConfigMap::const_iterator iter = script_map.begin(); 
            iter != script_map.end(); ++iter)
    {
        ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(iter->first);
        JUDGE_CONTINUE(script_record != NULL);

        const Json::Value &script_json = *(iter->second);
        const Json::Value &prev_condition = script_json["prev_condition"];
        const Json::Value &finish_condition = script_json["finish_condition"];

        ProtoScriptInfo *script_info = respond.add_script_list();
        script_info->set_script_sort(iter->first);

        int script_type = script_json["type"].asInt();
        if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
        {
        	ScriptPlayerDetail::LegendTopInfo &legend_top_info = this->script_detail_.__legend_top_info;
        	script_info->set_today_rank(legend_top_info.__today_rank);
        	script_info->set_pass_floor(legend_top_info.__pass_floor);
        	script_info->set_is_sweep(legend_top_info.__is_sweep);

        	if (legend_top_info.__is_sweep == true)
        	{
        		event_map[GameEnum::PA_EVENT_SCRIPT_LEGEND_SWEEP] = true;
        	}

        	int total_floor = prev_condition["total_floor"].asInt();
        	const Json::Value &special_item = finish_condition["special_item"];
        	for (int i = 1; i <= total_floor; ++i)
        	{
        		ProtoFloorInfo *floor_info = script_info->add_floor_info();
        		floor_info->set_floor_id(i);

        		ScriptPlayerDetail::LegendTopInfo::FloorMap::iterator iter = legend_top_info.__piece_map.find(i);
        		if (iter != legend_top_info.__piece_map.end())
        		{
					ScriptPlayerDetail::LegendTopInfo::FloorInfo &floor_detail = iter->second;
					floor_info->set_is_today_pass(floor_detail.__totay_pass_flag);
					floor_info->set_pass_tick(floor_detail.__pass_tick);
				}
        		else
				{
					floor_info->set_is_today_pass(0);
					floor_info->set_pass_tick(0);
				}
        		floor_info->set_is_special(0); //初始化
        		for (uint j = 0; j < special_item.size(); ++j)
        		{
					if (special_item[j][0u].asInt() != i) {
						continue;
					}
					floor_info->set_is_special(1);
				}
        	}
        	continue;
        }
        else if (script_type == GameEnum::SCRIPT_T_SWORD_TOP)
        {
        	ScriptPlayerDetail::LegendTopInfo &sword_top_info = this->script_detail_.__sword_top_info;
        	script_info->set_pass_floor(sword_top_info.__pass_floor);
        	script_info->set_is_sweep(sword_top_info.__is_sweep);
        	script_info->set_skill_id(this->script_detail_.__skill_id);

        	if (sword_top_info.__is_sweep == true)
        	{
        	    event_map[GameEnum::PA_EVENT_SCRIPT_SWORD_SWEEP] = true;
        	}
        	continue;
        }

        int left_times = this->fetch_script_left_times(script_record, script_json);
        int left_buy_times = this->fetch_script_left_buy_times(script_record, script_json);

        int is_open = 1;
        int task_id = prev_condition["task"].asInt();
        if (task_id > 0 && task_set.count(task_id) == 0 && script_record->__is_first_pass == 0)
        {
            is_open = 0;
        }
        if (this->level() < prev_condition["level"].asInt())
        {
            is_open = 0;
        }

        if (script_type == GameEnum::SCRIPT_T_ADVANCE)
        {
			int is_sweep = script_record->__is_first_pass > 0 ? 1 : 0;
			int reset_times = script_record->__used_times - prev_condition["finish_times"].asInt();
//			reset_times = reset_times < 0 ? 0 : reset_times;
			script_info->set_is_sweep(is_sweep);
			script_info->set_reset_times(reset_times);

			if (left_times > 0 && is_open == true)
			{
				event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = true;
			}
		}
        else if (this->is_wave_script(script_type) == true)
        {
        	ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
        	script_info->set_pass_piece(type_record->__pass_wave);
        	script_info->set_pass_chapter(type_record->__pass_chapter);
        	script_info->set_start_wave(type_record->__start_wave);
        	script_info->set_start_chapter(type_record->__start_chapter);
        	script_info->set_is_sweep(type_record->__is_sweep);

        	if (type_record->__is_sweep == true)
        	{
        		if (script_type == GameEnum::SCRIPT_T_EXP)
        			event_map[GameEnum::PA_ENENT_SCRIPT_EXP_SWEEP] = true;
        		else if (script_type == GameEnum::SCRIPT_T_RAMA)
        			event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_SWEEP] = true;
        	}

        	if (script_type == GameEnum::SCRIPT_T_EXP)
        	{
        		const Json::Value &exp_first_award = finish_condition["exp_first_award"];
        		for (uint i = 0; i < exp_first_award.size(); ++i)
        		{
        			int wave_id = exp_first_award[i][0u].asInt();
        			int wave_chapter_key = this->convert_script_wave_key(iter->first, wave_id);
        			ScriptPlayerDetail::ScriptWaveRecord *special_record = this->special_record(wave_chapter_key);

        			ProtoSpecialAwardInfo *special_info = script_info->add_special_info();
        			special_info->set_script_wave_id(special_record->__script_wave_id);
        			special_info->set_script_sort(iter->first);
        			special_info->set_wave(wave_id);
        			special_info->set_is_get(special_record->__is_get);

        			if (special_record->__is_get == 1)
        			{
        				event_map[GameEnum::PA_EVENT_SCRIPT_EXP_BOX] = true;
        			}
        		}
        	}
        	else if (script_type == GameEnum::SCRIPT_T_RAMA && script_record->__is_first_pass <= 0)
        	{
        		event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_FIGHT] = true;
        	}
        }
        else if (script_type == GameEnum::SCRIPT_T_STORY && left_times > 0 && is_open == true)
        {
        	if (script_record->__is_first_pass > 0)
        		event_map[GameEnum::PA_EVENT_SCRIPT_STORY_SWEEP] = true;
        	else
        		event_map[GameEnum::PA_EVENT_SCRIPT_STORY_FIGHT] = true;

        }
        else if (script_type == GameEnum::SCRIPT_T_VIP && left_times > 0)
        {
        	int vip_level = prev_condition["vip_level"].asInt();
        	if (this->vip_detail().__vip_level >= vip_level)
        	{
        		event_map[GameEnum::PA_EVENT_SCRIPT_VIP_FIGHT] = true;
        	}
        }
        else if (script_type == GameEnum::SCRIPT_T_COUPLES)
        {
        	int couple_buy = script_record->__couple_buy_times;
        	int left_get = prev_condition["get_times"].asInt() - couple_buy;
        	left_get = left_get > 0 ? left_get : 0;

        	script_info->set_left_get(left_get);
        }

        script_info->set_left_times(left_times);
        script_info->set_is_open(is_open);
        script_info->set_left_buy_times(left_buy_times);
        script_info->set_buy_times(script_record->__buy_times);
        script_info->set_is_first_pass(script_record->__is_first_pass);
    }

    if (this->script_detail_.is_have_trvl_red() == true)
    {
    	event_map[GameEnum::PA_EVENT_TRVL_TOTAL_TIMES] = true;
    }

    this->check_pa_event_all_script(event_map);

    FINER_PROCESS_RETURN(RETURN_SCRIPT_LIST_INFO, &respond);
}

int MapPlayerScript::login_add_couple_fb_times(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400916 *, request, -1);

	int couple_script_sort = GameEnum::SCRIPT_SORT_COUPLE;
	ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(couple_script_sort);
	JUDGE_RETURN(script_record != NULL, 0);

	script_record->__couple_buy_times += 1;

	Proto10400914 res_script;
	res_script.set_script_sort(couple_script_sort);
	return this->request_script_player_info(&res_script);
}

int MapPlayerScript::command_reset_script(void)
{
	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();

	Time_Value nowtime = Time_Value::gettimeofday();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		ScriptPlayerDetail::ScriptRecord *record = this->script_record(iter->first);
		if (record == 0)
		{
		    ScriptPlayerDetail::ScriptRecord &s_rec = this->script_detail_.__record_map[iter->first];
		    s_rec.reset();
		    s_rec.__script_sort = iter->first;
		    record = &s_rec;
		}
		record->__used_times_tick = Time_Value::zero;

		const Json::Value &script_json = *(iter->second);
		int script_type = script_json["type"].asInt();
		if (this->is_wave_script(script_type) == true)
		{
			ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
			if (type_record == 0)
			{
				ScriptPlayerDetail::TypeRecord &t_rec = this->script_detail_.__type_map[script_type];
				t_rec.reset();
				t_rec.__script_type = script_type;
				type_record = &t_rec;
			}
			type_record->__used_times_tick = Time_Value::zero;
		}
	}
	this->request_script_list_info(NULL);

	return 0;
}

int MapPlayerScript::test_reset_lfb_script()
{
	Time_Value nowtime = Time_Value::gettimeofday();
	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		const Json::Value &script_json = *(iter->second);
		int script_type = script_json["type"].asInt();
		JUDGE_CONTINUE(script_type == GameEnum::SCRIPT_T_LEAGUE_FB);

		ScriptPlayerDetail::ScriptRecord *record = this->script_record(iter->first);
		if (record == 0)
		{
			ScriptPlayerDetail::ScriptRecord &s_rec = this->script_detail_.__record_map[iter->first];
			s_rec.reset();
			s_rec.__script_sort = iter->first;
			record = &s_rec;
		}
		record->__used_times_tick = Time_Value::zero;

		if (this->is_wave_script(script_type) == true)
		{
			ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
			if (type_record == 0)
			{
				ScriptPlayerDetail::TypeRecord &t_rec = this->script_detail_.__type_map[script_type];
				t_rec.reset();
				t_rec.__script_type = script_type;
				type_record = &t_rec;
			}
			type_record->__used_times_tick = Time_Value::zero;
		}
	}
	this->request_script_list_info(NULL);

	return 0;
}

int MapPlayerScript::request_fetch_special_award(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10400939 *, request, -1);

	int script_sort = request->script_sort();
	int script_wave_id = request->script_wave_id();

	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	CONDITION_NOTIFY_RETURN(script_json.empty() == false,
			RETURN_SCRIPT_SPECIAL_AWARD, ERROR_CLIENT_OPERATE);

	ScriptPlayerDetail::ScriptWaveRecord *special_record = this->special_record(script_wave_id);
	CONDITION_NOTIFY_RETURN(special_record->__is_get == 1,
			RETURN_SCRIPT_SPECIAL_AWARD, ERROR_SPECIAL_AWARD_CAN_NOT_GET);

	int wave = script_wave_id - script_sort * 1000;
	const Json::Value &exp_first_award = script_json["finish_condition"]["exp_first_award"];
	int award_id = 0;
	for (uint i = 0; i < exp_first_award.size(); i++)
	{
		int wave_id = exp_first_award[i][0u].asInt();
		if (wave == wave_id)
		{
			award_id = exp_first_award[i][1u].asInt();
			break;
		}
	}
	CONDITION_NOTIFY_RETURN(award_id > 0, RETURN_SCRIPT_SPECIAL_AWARD, ERROR_NO_SCRIPT_AWARD);
	special_record->__is_get = 2;

	Proto30400906 req_logic;
	req_logic.set_is_first_pass(1);
	req_logic.set_script_sort(script_sort);
	req_logic.add_reward_id(award_id);
	this->send_to_logic_thread(req_logic);

	Proto50400939 respond;
	respond.set_script_wave_id(script_wave_id);
	respond.set_script_sort(script_sort);
	respond.set_is_get(special_record->__is_get);

	// 经验副本宝箱红点处理
	IntMap event_map;
	event_map[GameEnum::PA_EVENT_SCRIPT_EXP_BOX] = false;
	for (ScriptPlayerDetail::ScriptWaveMap::iterator iter = this->script_detail_.__script_wave_map.begin();
			iter != this->script_detail_.__script_wave_map.end(); ++iter)
	{
		ScriptPlayerDetail::ScriptWaveRecord &award_record = iter->second;
		JUDGE_CONTINUE(award_record.__is_get == 1);

		event_map[GameEnum::PA_EVENT_SCRIPT_EXP_BOX] = true;
		break;
	}

	this->check_pa_event_all_script(event_map);
	FINER_PROCESS_RETURN(RETURN_SCRIPT_SPECIAL_AWARD, &respond);
}

int MapPlayerScript::recovert_magic(const Time_Value &nowtime)
{
    BaseScript *script = this->fetch_script();
    if (script != 0 && script->is_holdup())
        return 0;

    return MapPlayer::recovert_magic(nowtime);
}

int MapPlayerScript::recovert_blood(const Time_Value &nowtime)
{
    BaseScript *script = this->fetch_script();
    if (script != 0 && script->is_holdup())
        return 0;

    return MapPlayer::recovert_blood(nowtime);
}

int MapPlayerScript::fetch_gold_for_buy_script_times(const int script_sort, const int inc_times)
{

	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
    const Json::Value &prev_cond_json = script_json["prev_condition"];

    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
    int buy_times = record->__buy_times, need_gold = 0;

    const Json::Value &buy_json = prev_cond_json["buy_script_times"];
    for (int i = 0; i < inc_times; ++i)
    {
		if (this->vip_detail().__vip_level > 0)
		{
//			int vip_buyed_times = buy_times - int(buy_json.size());
			int vip_buyed_times = buy_times;
			char field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0,};
			::snprintf(field_name, GameEnum::DEFAULT_MAX_NAME_LENGTH, "inc_buy_times_vip%d", this->vip_detail().__vip_level);
			field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH] = '\0';
			const Json::Value &vip_buy_json = prev_cond_json[field_name];
			JUDGE_RETURN(vip_buyed_times < int(vip_buy_json.size()), ERROR_SCRIPT_BUY_TIMES_LIMIT);

			need_gold += vip_buy_json[vip_buyed_times].asInt();
		}
		else
		{
			if (buy_times < int(buy_json.size()))
			{
				need_gold += prev_cond_json["buy_script_times"][buy_times].asInt();
			}
			else
			{
				return ERROR_SCRIPT_BUY_TIMES_LIMIT;
			}
		}
		++buy_times;
    }
    return need_gold;
}

int MapPlayerScript::fetch_script_id_use_type(IntVec &sort_vec, int script_type)
{
	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		const Json::Value &script_json = *(iter->second);
		int type = script_json["type"].asInt();
		JUDGE_CONTINUE(script_type == type);

		sort_vec.push_back(iter->first);
	}

	return 0;
}

bool MapPlayerScript::is_wave_script(int script_type)
{
	if (script_type == GameEnum::SCRIPT_T_EXP || script_type == GameEnum::SCRIPT_T_RAMA
				|| script_type == GameEnum::SCRIPT_T_LEAGUE_FB)
		return true;
	return false;
}

int MapPlayerScript::check_enter_condition(int script_sort, const Json::Value &cond)
{
	JUDGE_RETURN(cond != Json::Value::null, 0);

	int type = cond[0u].asInt();
	int value = cond[1u].asInt();
	switch (type)
	{
	case GameEnum::TYPE_OPEN_SERVER_DAY:
	{
		int open_day = CONFIG_INSTANCE->open_server_days();
		JUDGE_RETURN(open_day >= value, ERROR_OPEN_SERVER_DAY_NOT_ENOUGH);

		break;
	}
	default:
		break;
	}

	return 0;
}

int MapPlayerScript::request_script_add_times_gold(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10400919 *, request, msg, RETURN_SCRIPT_ADD_TIMES_GOLD);

    int script_sort = request->script_sort(), inc_times = request->inc_times(), cur_gold = request->cur_gold();
    CONDITION_NOTIFY_RETURN(script_sort > 0 && 0 < inc_times && inc_times < 100, RETURN_SCRIPT_ADD_TIMES_GOLD, ERROR_CLIENT_OPERATE);

    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    CONDITION_NOTIFY_RETURN(script_json != Json::Value::null, RETURN_SCRIPT_ADD_TIMES_GOLD, ERROR_CLIENT_OPERATE);

    const Json::Value &prev_json = script_json["prev_condition"];
    CONDITION_NOTIFY_RETURN(prev_json["is_couples"].asInt() == true && this->partner_id() > 0,
    		RETURN_SCRIPT_ADD_TIMES_GOLD, ERROR_COUPLE_CAN_BUY_COUPLE_FB);

    int need_gold = this->fetch_gold_for_buy_script_times(script_sort, inc_times);
    CONDITION_NOTIFY_RETURN(need_gold >= 0, RETURN_SCRIPT_ADD_TIMES_GOLD, ERROR_LEAGUE_MAX_BUY);

    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
    int left_buy_times = this->fetch_script_left_buy_times(record, script_json, cur_gold);

    Proto50400919 respond;
    respond.set_script_sort(request->script_sort());
    respond.set_gold(need_gold);
    respond.set_left_times(left_buy_times);
    return this->respond_to_client(RETURN_SCRIPT_ADD_TIMES_GOLD, &respond);
}

int MapPlayerScript::request_script_add_times(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10400920 *, request, msg, RETURN_SCRIPT_ADD_TIMES);
    int script_sort = request->script_sort(), inc_times = request->inc_times();

    CONDITION_NOTIFY_RETURN(script_sort > 0 && 0 < inc_times && inc_times < 100, RETURN_SCRIPT_ADD_TIMES, ERROR_CLIENT_OPERATE);

    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    if (script_json["prev_condition"]["is_couples"].asInt() == true)
    {
    	CONDITION_NOTIFY_RETURN(this->role_detail().__wedding_id > 0 && this->role_detail().__partner_id,
    			RETURN_SCRIPT_ADD_TIMES, ERROR_COUPLE_CAN_BUY_COUPLE_FB);
    }

    int need_gold = this->fetch_gold_for_buy_script_times(script_sort, inc_times);
    CONDITION_NOTIFY_RETURN(need_gold >= 0, RETURN_SCRIPT_ADD_TIMES, need_gold);

    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);

    record->__buy_times += inc_times;
    this->sync_share_script_buy_left_times(record->__script_sort, record->__buy_times);

    Proto31400904 inner_req;
    inner_req.set_script_sort(script_sort);
    inner_req.set_gold(need_gold);
    inner_req.set_inc_times(inc_times);
    return this->send_to_logic_thread(inner_req);
}

int MapPlayerScript::process_add_script_after_use_gold(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400904 *, request, msg, -1);

    int script_sort = request->script_sort(), ret = request->ret_code();
    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);

    Proto50400920 respond;
    respond.set_script_sort(script_sort);

    if (ret == 0)
    {
        const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
        int left_times = this->fetch_script_left_times(record, script_json);
        int left_buy_times = this->fetch_script_left_buy_times(record, script_json);

        Proto30100701 sync_info;
        sync_info.set_script_sort(script_sort);
        sync_info.set_use_times(left_times);
        this->monitor()->dispatch_to_logic(this, &sync_info);

        respond.set_left_times(left_times);
        respond.set_left_buy_times(left_buy_times);
        IntMap event_map;

        //剑池增加任务次数
        int script_type = script_json["type"].asInt();
        if (script_type == GameEnum::SCRIPT_T_ADVANCE)
        {
        	Proto31402901 inner_res;
        	int task_id = GameEnum::SPOOL_TASK_ADVANCE_SCRIPT;
        	inner_res.set_task_id(task_id);
        	inner_res.set_left_add_flag(2);
        	inner_res.set_left_add_num(request->inc_times());
        	this->send_to_logic_thread(inner_res);

        	MSG_USER("MapPlayerScript, process_add_script_after_use_gold, Proto31402901: %s", inner_res.Utf8DebugString().c_str());

        	event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = true;
        }
        this->check_pa_event_all_script(event_map);

        //夫妻副本同步次数
        const Json::Value &prev_json = script_json["prev_condition"];
        if (prev_json["is_couples"].asInt() == true && this->role_detail().__partner_id > 0)
        {
        	int couple_buy = record->__couple_buy_times;
        	int left_get = prev_json["get_times"].asInt() - couple_buy;
        	left_get = left_get > 0 ? left_get : 0;
        	respond.set_left_get(left_get);

        	MapPlayerEx* player = NULL;
        	if (MAP_MONITOR->find_player(this->role_detail().__partner_id, player) == 0)
        	{
        		ScriptPlayerDetail::ScriptRecord *partner_record = player->refresh_script_record(script_sort);
        		partner_record->__couple_buy_times += 1;

        		Proto50400920 couple_res;
        		couple_res.set_script_sort(script_sort);

        		int partner_left_times = player->fetch_script_left_times(partner_record, script_json);
        		int partner_left_buy_times = player->fetch_script_left_buy_times(partner_record, script_json);
        		couple_res.set_left_times(partner_left_times);
        		couple_res.set_left_buy_times(partner_left_buy_times);

        		int partner_couple_buy = partner_record->__couple_buy_times;
        		int partner_left_get = prev_json["get_times"].asInt() - partner_couple_buy;
        		partner_left_get = partner_left_get > 0 ? partner_left_get : 0;
        		couple_res.set_left_get(partner_left_get);

        		player->respond_to_client(RETURN_SCRIPT_ADD_TIMES, &couple_res);
        	}
        	else
        	{
        	    //夫妻不在线
        	    Proto30400914 inner;
        	    inner.set_role_id(this->role_detail().__partner_id);
        	    MAP_MONITOR->dispatch_to_scene(this, GameEnum::LEGEND_TOP_SCENE, &inner);
        	    MAP_MONITOR->dispatch_to_logic(this, IL_SYNC_COUPLE_FB_TIMES);
        	}
        }

        return this->respond_to_client(RETURN_SCRIPT_ADD_TIMES, &respond);
    }

    // failure rollback
    record->__buy_times -= request->inc_times();
    if (record->__buy_times < 0)
        record->__buy_times = 0;
    this->sync_share_script_buy_left_times(record->__script_sort, record->__buy_times);
    return this->respond_to_client_error(RETURN_SCRIPT_ADD_TIMES, ret);
}

int MapPlayerScript::process_after_reset_script(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400904 *, request, msg, -1);

	int script_sort = request->script_sort();
	if (script_sort > 0)
	{
		return this->process_add_script_after_use_gold(msg);
	}
	else
	{
		return this->request_script_type_reset_end(msg);
	}
}

int MapPlayerScript::request_script_type_reset_begin(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10400921 *, request, msg, RETURN_SCRIPT_TYPE_RESET);

	int script_type = request->script_type();

	IntVec sort_vec, send_vec;
	this->fetch_script_id_use_type(sort_vec, script_type);
	CONDITION_NOTIFY_RETURN(sort_vec.size() > 0, RETURN_SCRIPT_TYPE_RESET, ERROR_CLIENT_OPERATE);

	int total_gold = 0;
	for (IntVec::iterator iter = sort_vec.begin(); iter != sort_vec.end(); ++iter)
	{
		int script_sort = *iter;
		ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
		const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
		JUDGE_CONTINUE(script_json.empty() == false);

		int left_times = this->fetch_script_left_times(record, script_json);
		JUDGE_CONTINUE(left_times <= 0);

		int need_gold = this->fetch_gold_for_buy_script_times(script_sort, 1);
		if (need_gold > 0)
		{
			total_gold += need_gold;
			send_vec.push_back(script_sort);
		}
	}
	CONDITION_NOTIFY_RETURN(total_gold > 0 && send_vec.size() > 0, RETURN_SCRIPT_TYPE_RESET,
			ERROR_SCRIPT_BUY_TIMES_LIMIT);

	Proto31400904 inner_req;
	inner_req.set_script_type(script_type);
	inner_req.set_script_sort(0);
	inner_req.set_gold(total_gold);
	for (IntVec::iterator iter = send_vec.begin(); iter != send_vec.end(); ++iter)
		inner_req.add_sort_set(*iter);

	return this->send_to_logic_thread(inner_req);
}

int MapPlayerScript::request_script_type_reset_end(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto31400904 *, request, msg, -1);

	int ret = request->ret_code(), script_type = request->script_type();
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_SCRIPT_TYPE_RESET, ret);

	Proto50400921 respond;
	respond.set_script_type(script_type);

	int sort_size = request->sort_set_size();
	for (int i = 0; i < sort_size; ++i)
	{
		int script_sort = request->sort_set(i);
		ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
		record->__buy_times += 1;

		const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
		int left_times = this->fetch_script_left_times(record, script_json);
		int left_buy_times = this->fetch_script_left_buy_times(record, script_json);

		ProtoScriptReset *reset_info = respond.add_reset_info();
		reset_info->set_script_sort(script_sort);
		reset_info->set_left_times(left_times);
		reset_info->set_left_buy_times(left_buy_times);
	}

	IntMap event_map;

	//剑池增加任务次数
	if (script_type == GameEnum::SCRIPT_T_ADVANCE)
	{
		Proto31402901 inner_res;
		int task_id = GameEnum::SPOOL_TASK_ADVANCE_SCRIPT;
		inner_res.set_task_id(task_id);
		inner_res.set_left_add_flag(2);
		inner_res.set_left_add_num(sort_size);
		this->send_to_logic_thread(inner_res);

		event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = true;
	}
	this->check_pa_event_all_script(event_map);

	return this->respond_to_client(RETURN_SCRIPT_TYPE_RESET, &respond);
}

int MapPlayerScript::sync_share_script_used_times(const int script_sort, const int vary_times)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    if (script_json["finish_condition"].isMember("share_finish_times"))
    {
        const Json::Value &share_json = script_json["finish_condition"]["share_finish_times"];
        for (uint i = 0; i < share_json.size(); ++i)
        {
            int share_script_sort = share_json[i].asInt();
            if (share_script_sort == script_sort)
                continue;
            
            if (CONFIG_INSTANCE->script(share_script_sort) == Json::Value::null)
                continue;

            ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(share_script_sort);
            record->__progress_id = 0;
            record->__used_times = vary_times;
            if (record->__used_times < 0)
                record->__used_times = 0;
        }
    }
    return 0;
}

int MapPlayerScript::sync_share_script_buy_left_times(const int script_sort, const int vary_times)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    if (script_json["finish_condition"].isMember("share_finish_times"))
    {
        const Json::Value &share_json = script_json["finish_condition"]["share_finish_times"];
        for (uint i = 0; i < share_json.size(); ++i)
        {
            int share_script_sort = share_json[i].asInt();
            if (share_script_sort == script_sort)
                continue;
            
            if (CONFIG_INSTANCE->script(share_script_sort) == Json::Value::null)
                continue;

            ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(share_script_sort);
            record->__buy_times = vary_times;
            if (record->__buy_times < 0)
                record->__buy_times = 0;
        }
    }
    return 0;
}

int MapPlayerScript::call_puppet(const int puppet_sort)
{
//    ScriptScene *scene = dynamic_cast<ScriptScene *>(this->fetch_scene());
//    JUDGE_RETURN(scene != 0, -1);
//
//    return scene->call_puppet(puppet_sort, this, this->location());
	return 0;
}

double MapPlayerScript::fetch_addition_exp_percent(void)
{
//	double addition_exp_percent = 0;
//
//	BaseScript *script = this->fetch_script();
//	JUDGE_RETURN(script != 0, 0);
//
//	if(false == script->is_single_script())
//	{
//		int teamer_num = this->teamate_count() + this->replacement_count();
//		JUDGE_RETURN(teamer_num > 0 && teamer_num <= GameEnum::MAX_TEAMER_COUNT, -1);
//
//		const Json::Value &team_exp_json = CONFIG_INSTANCE->tiny("team_script_extra_exp");
//		JUDGE_RETURN(Json::Value::null != team_exp_json, -1);
//
//		double team_exp_percent = team_exp_json[(uint)(teamer_num - 1)].asDouble() / 100.0;
//		addition_exp_percent += team_exp_percent;
//		MSG_USER(team_exp_percent %f, team_exp_percent);
//	}
//
//	MSG_USER(MapPlayer::fetch_addition_exp_percent %f, MapPlayer::fetch_addition_exp_percent());
//	return addition_exp_percent += MapPlayer::fetch_addition_exp_percent();
	return MapPlayer::fetch_addition_exp_percent();
}

int MapPlayerScript::request_piece_total_star(Message *msg)
{
//    DYNAMIC_CAST_RETURN(Proto10400933 *, request, msg, -1);
//
//    int piece_total_star = this->piece_total_star(request->piece());
//    CONDITION_NOTIFY_RETURN(piece_total_star >= 0, RETURN_SCRIPT_PIECE_TOTAL_STAR, piece_total_star);
//
//    this->monitor()->inner_notify_player_assist_event(this, GameEnum::PA_EVENT_WJSL_SCRIPT, 0);
//
//    Proto50400933 respond;
//    respond.set_piece(request->piece());
//    respond.set_total_star(piece_total_star);
//
//    {
//    	int begin_key = this->convert_chapter_key(request->piece(), 0), end_key = this->convert_chapter_key(request->piece() + 1, 0);
//    	ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//    	IntMap::iterator begin_iter = piece_rec.__piece_star_award_map.upper_bound(begin_key),
//    			end_iter = piece_rec.__piece_star_award_map.lower_bound(end_key);
//    	for (IntMap::iterator iter = begin_iter; iter != end_iter; ++iter)
//    	{
//    		if (iter->second != 0)
//    			respond.add_drawed_star(iter->first % 1000);
//    	}
//    }
//    return this->respond_to_client(RETURN_SCRIPT_PIECE_TOTAL_STAR, &respond);
	return 0;
}

int MapPlayerScript::request_piece_total_star_award(Message *msg)
{
//    DYNAMIC_CAST_RETURN(Proto10400934 *, request, msg, -1);
//
//    int piece_total_star = this->piece_total_star(request->piece());
//
//    CONDITION_NOTIFY_RETURN(request->award_star() <= piece_total_star, RETURN_SCRIPT_PIECE_TOTAL_STAR_DRAW, ERROR_SCRIPT_TOTAL_STAR_AMOUNT);
//
//    int draw_key = this->convert_chapter_key(request->piece(), request->award_star());
//    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//    {
//    	IntMap::iterator iter = piece_rec.__piece_star_award_map.find(draw_key);
//    	CONDITION_NOTIFY_RETURN(iter == piece_rec.__piece_star_award_map.end() || iter->second == 0,
//    			RETURN_SCRIPT_PIECE_TOTAL_STAR_DRAW, ERROR_SCRIPT_AWARD_DRAWED);
//    }
//
//    this->update_drawed_star(request->piece(), request->award_star(), 1);
//
//    Proto31400906 inner_req;
//    inner_req.set_piece(request->piece());
//    inner_req.set_total_star(piece_total_star);
//    inner_req.set_draw_star(request->award_star());
//    return this->send_to_logic_thread(inner_req);
	return 0;
}

int MapPlayerScript::process_piece_total_star_after_award(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400906 *, request, msg, -1);
    if (request->proc_ret() != 0)
    {
        this->update_drawed_star(request->piece(), request->draw_star(), 0);
    }
    return this->respond_to_client_error(RETURN_SCRIPT_PIECE_TOTAL_STAR_DRAW, request->proc_ret());
}

int MapPlayerScript::piece_total_star(const int piece)
{
//    const Json::Value &script_json = CONFIG_INSTANCE->script(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);
//
//    const Json::Value &piece_json = script_json["prev_condition"]["piece"];
//    JUDGE_RETURN(0 < piece && piece <= int(piece_json.size()), ERROR_CONFIG_NOT_EXIST);
//
//    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
//
//    int max_chapter = piece_json[piece - 1].asInt(), total_star = 0;
//    for (int chapter_i = 1; chapter_i <= max_chapter; ++chapter_i)
//    {
//        int key = piece * 1000 + chapter_i;
//        ScriptPlayerDetail::PieceRecord::PassChapterMap::iterator iter = piece_rec.__pass_chapter_map.find(key);
//        if (iter == piece_rec.__pass_chapter_map.end())
//            continue;
//
////        ScriptPlayerDetail::PieceRecord::PieceChapterInfo &info = iter->second;
////        total_star += this->script_star_level(GameEnum::SCRIPT_SORT_CLIMB_TOWER, info.__used_sec);
//    }
//    return total_star;
	return 0;
}

int MapPlayerScript::update_drawed_star(const int piece, const int update_star, const int value)
{
	int chapter_key = this->convert_chapter_key(piece, update_star);
    ScriptPlayerDetail::PieceRecord &piece_rec = this->script_detail_.__piece_record;
    if (value == 0)
    	piece_rec.__piece_star_award_map.erase(chapter_key);
    else
    	piece_rec.__piece_star_award_map[chapter_key] = value;
    return 0;
}

int MapPlayerScript::process_fetch_climb_tower_info(Message *msg)
{
//	DYNAMIC_CAST_RETURN(Proto31400602 *, req, msg, -1);
//	Proto81400106 respond;
//	respond.ParseFromString(req->msg_body());
//
//	respond.set_pass_piece(this->script_detail_.__piece_record.__pass_piece);
//	respond.set_pass_chapter(this->script_detail_.__piece_record.__pass_chapter);
//
//	MSG_USER("task climb tower %ld %s %d %d", this->role_id(), this->name(),
//			respond.pass_piece(), respond.pass_chapter());
//	return this->respond_to_client(ACTIVE_FINISHED_TASK_SET, &respond);
	return 0;
}

int MapPlayerScript::read_script_compact_info(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400907 *, req, msg, -1);
    this->compact_type_ = req->type();
    this->expired_tick_.sec(req->expired_tick());

    if (this->is_script_compact_status())
    {
        for (ScriptPlayerDetail::ScriptRecordMap::iterator iter = this->script_detail_.__record_map.begin(); 
                iter != this->script_detail_.__record_map.end(); ++iter)
        {
            ScriptPlayerDetail::ScriptRecord &record = iter->second;
            const Json::Value &script_json = CONFIG_INSTANCE->script(record.__script_sort);
            record.__buy_times = int(script_json["prev_condition"]["buy_script_times"].size());
        }
    }
    return 0;
}

bool MapPlayerScript::is_script_compact_status(void)
{
    return (this->expired_tick_ >= Time_Value::gettimeofday());
}

int MapPlayerScript::fetch_script_left_times(const int script_sort)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    JUDGE_RETURN(script_json != Json::Value::null, -1);

    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
    return this->fetch_script_left_times(record, script_json);
}

int MapPlayerScript::fetch_script_left_times(ScriptPlayerDetail::ScriptRecord *record, const Json::Value &script_json)
{
    int left_times = script_json["prev_condition"]["finish_times"].asInt();

    left_times += GameCommon::script_vip_extra_use_times(this->vip_type(), record->__script_sort);
    left_times += record->__buy_times + record->__couple_buy_times - record->__used_times;

    return std::max<int>(left_times, 0);
}

int MapPlayerScript::fetch_script_left_buy_times(ScriptPlayerDetail::ScriptRecord *record, const Json::Value &script_json, const int cur_gold)
{
	if (cur_gold == INT_MAX)
	{
		int total_buy_times = script_json["prev_condition"]["buy_script_times"].size();

		if (this->vip_detail().__vip_level > 0)
		{
			char field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0,};
			::snprintf(field_name, GameEnum::DEFAULT_MAX_NAME_LENGTH, "inc_buy_times_vip%d", this->vip_detail().__vip_level);
			field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH] = '\0';
			const Json::Value &vip_buy_json = script_json["prev_condition"][field_name];
			total_buy_times = int(vip_buy_json.size());
		}

		total_buy_times -= record->__buy_times;
		if (total_buy_times < 0)
			return 0;
		return total_buy_times;
	}
	else
	{
		const Json::Value &prev_cond_json = script_json["prev_condition"];
		int buy_times = record->__buy_times, need_gold = 0, total_buy_times = 0;

		const Json::Value &buy_json = prev_cond_json["buy_script_times"];
		for (int i = 0; i < 100; ++i)
		{
			if (buy_times >= int(buy_json.size()))
			{
				if (this->vip_detail().__vip_level > 0)
				{
					int vip_buyed_times = buy_times - int(buy_json.size());
					char field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0,};
					::snprintf(field_name, GameEnum::DEFAULT_MAX_NAME_LENGTH, "inc_buy_times_vip%d", this->vip_detail().__vip_level);
					field_name[GameEnum::DEFAULT_MAX_NAME_LENGTH] = '\0';
					const Json::Value &vip_buy_json = prev_cond_json[field_name];
					if (vip_buyed_times >= int(vip_buy_json.size()))
						return total_buy_times;

					int inc_gold = vip_buy_json[vip_buyed_times].asInt();
					if (need_gold + inc_gold > cur_gold)
						return total_buy_times;
					need_gold += inc_gold;
					++total_buy_times;
				}
				else
				{
					return total_buy_times;
				}
			}
			else
			{
				int inc_gold = prev_cond_json["buy_script_times"][buy_times].asInt();
				if (need_gold + inc_gold > cur_gold)
					return total_buy_times;
				need_gold += inc_gold;
				++total_buy_times;
			}
			++buy_times;
		}
		return total_buy_times;
	}
}

int MapPlayerScript::script_relive_left_times(int &relive_total, int &relive_lefts)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(this->script_sort());
    const Json::Value &relive_json = script_json["relive"];

    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(0 != script, 0);

    if(!relive_json.isMember("item_times_amount"))
    {
    	if (relive_json["no_relive"].asInt() == 1)
    	{
    		relive_lefts = relive_total = 0;
    	}
    	else if (relive_json.isMember("relive_times"))
    	{
    		relive_total = relive_json["relive_times"].asInt();
    		relive_lefts = relive_total - script->used_relive_amout_by_item(this->role_id());
    		if (relive_lefts < 0)
    			relive_lefts = 0;
    	}
    	else
    	{
    		relive_lefts = relive_total = -1;
    	}
        return 0;
    }

    int total_relive_times = relive_json["item_times_amount"].size();
    int used_relive_times = script->used_relive_amout_by_item(this->role_id());
    int left_relive_times = total_relive_times - used_relive_times;

    relive_total = total_relive_times;
    relive_lefts = std::max(left_relive_times, 0);

    return 0;
}

// 副本星级
bool MapPlayerScript::is_script_top_star_level(const int script_sort, const int value)
{
//	JUDGE_RETURN(value > 0, false);
//
//	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//	const Json::Value &star_level_json = script_json["finish_condition"]["star_level"];
//	JUDGE_RETURN(star_level_json.size() > 0, false);
//
//	if (value <= star_level_json[(star_level_json.size() - 1)].asInt())
//		return true;
	return false;
}

int MapPlayerScript::process_pick_up_suceess(AIDropPack *drop_pack)
{
    MapPlayer::process_pick_up_suceess(drop_pack);

    BaseScript *script = this->fetch_script();
    JUDGE_RETURN(script != NULL, 0);

    if (drop_pack->drop_detail().goods_type() == AIDropDetail::TYPE_MONEY)
    {
        Money money;
        IntMap &money_map = drop_pack->drop_detail().money_map_;
        IntMap::iterator iter = money_map.find(GameEnum::MONEY_BIND_COPPER);
        if (iter != money_map.end())
            money.__bind_copper = iter->second;
        iter = money_map.find(GameEnum::MONEY_UNBIND_COPPER);
        if (iter != money_map.end())
            money.__copper = iter->second;
        script->sync_script_inc_money(this->role_id(), money);
    }
    else
    {
    	ItemObj item_obj;
    	item_obj.id_ = drop_pack->drop_detail().item_obj_.__id;
    	item_obj.amount_ = drop_pack->drop_detail().item_obj_.__amount;
    	item_obj.bind_ = drop_pack->drop_detail().item_obj_.__bind;
        script->sync_script_inc_item(this->role_id(), item_obj);
    }
    return 0;
}

int MapPlayerScript::process_check_pa_event_script_times(void)
{
//	const Json::Value &script_list_json = CONFIG_INSTANCE->tiny("script_list");
//    for (uint i = 0; i < script_list_json.size(); ++i)
//    {
//        int script_sort = script_list_json[i].asInt();
//
//        const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
//        JUDGE_CONTINUE(script_json != Json::Value::null);
//        JUDGE_CONTINUE(this->level() >= script_json["prev_condition"]["level"].asInt());
//
//        ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
//        JUDGE_CONTINUE(record != NULL);
//
//        if (this->fetch_script_left_times(record, script_json) > 0)
//        {
//            this->monitor()->inner_notify_player_assist_event(this, GameEnum::PA_EVENT_SCRIPT, 1);
//            return 0;
//        }
//    }
//    this->monitor()->inner_notify_player_assist_event(this, GameEnum::PA_EVENT_SCRIPT, 0);

    return 0;
}

int MapPlayerScript::check_pa_event_climb_tower_script(void)
{
//	// 无尽试炼提醒检查
//	ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//	const Json::Value &script_json = CONFIG_INSTANCE->script(GameEnum::SCRIPT_SORT_CLIMB_TOWER);
//	if (script_json != Json::Value::null &&
//			this->level() > script_json["prev_condition"]["level"].asInt() &&
//			this->validate_climb_tower_script_left_times(record, script_json) == 0)
//	{
//		this->monitor()->inner_notify_player_assist_event(this, GameEnum::PA_EVENT_WJSL_SCRIPT, 1);
//	}
//	else
//	{
//		this->monitor()->inner_notify_player_assist_event(this, GameEnum::PA_EVENT_WJSL_SCRIPT, 0);
//	}
	return 0;
}

int MapPlayerScript::check_pa_event_all_script(IntMap &event_map)
{
	for (IntMap::iterator iter = event_map.begin(); iter != event_map.end(); ++iter)
	{
		int event_id = iter->first;
		int value = iter->second;
		this->monitor()->inner_notify_player_assist_event(this, event_id, value);
	}
	return 0;
}

int MapPlayerScript::check_pa_event_script_type_finish(int type)
{
	IntMap event_map;
	switch(type)
	{
	case GameEnum::SCRIPT_T_ADVANCE:
	{
		event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = false;
		break;
	}
	case GameEnum::SCRIPT_T_STORY:
	{
		event_map[GameEnum::PA_EVENT_SCRIPT_STORY_FIGHT] = false;
		break;
	}
	case GameEnum::SCRIPT_T_VIP:
	{
		event_map[GameEnum::PA_EVENT_SCRIPT_VIP_FIGHT] = false;
		break;
	}
	case GameEnum::SCRIPT_T_RAMA:
	{
		event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_FIGHT] = false;
		break;
	}
	case GameEnum::SCRIPT_T_TRVL:
	{
		JUDGE_RETURN(this->script_detail_.is_have_trvl_red() == false, -1);
		event_map[GameEnum::PA_EVENT_TRVL_TOTAL_TIMES] = false;
		break;
	}
	default:
		return 0;
	}

	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(iter->first);
		JUDGE_CONTINUE(script_record != NULL);

		const Json::Value &script_json = *(iter->second);
		int script_type = script_json["type"].asInt();
		JUDGE_CONTINUE(type == script_type);

		int is_open = 1;
		const Json::Value prev_condition = script_json["prev_condition"];
		if (this->level() < prev_condition["level"].asInt())
			is_open = 0;

		int left_times = this->fetch_script_left_times(script_record, script_json);
		if (left_times > 0 && is_open == true)
		{
			if (script_type == GameEnum::SCRIPT_T_ADVANCE)
				event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = true;
			else if (script_type == GameEnum::SCRIPT_T_STORY)
				event_map[GameEnum::PA_EVENT_SCRIPT_STORY_FIGHT] = true;
			else if (script_type == GameEnum::SCRIPT_T_VIP)
			{
				int vip_level = script_json["prev_condition"]["vip_level"].asInt();
				int my_vip = this->vip_detail().__vip_level;
				if (my_vip >= vip_level)
					event_map[GameEnum::PA_EVENT_SCRIPT_VIP_FIGHT] = true;
			}
			else if (script_type == GameEnum::SCRIPT_T_RAMA && script_record->__is_first_pass <= 0)
			{
				event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_FIGHT] = true;
			}
		}
	}

	for (IntMap::iterator iter = event_map.begin(); iter != event_map.end(); ++iter)
	{
		int value = iter->second;
		JUDGE_CONTINUE(value == 0);
		this->monitor()->inner_notify_player_assist_event(this, iter->first, value);
	}
	return 0;
}

int MapPlayerScript::check_pa_event_script_level()
{
	//发送红点的事件类型
	IntMap event_map;
	const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
	for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
			iter != script_map.end(); ++iter)
	{
		ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(iter->first);
		JUDGE_CONTINUE(script_record != NULL);

		const Json::Value &script_json = *(iter->second);
		const Json::Value &prev_condition = script_json["prev_condition"];
		const Json::Value &finish_condition = script_json["finish_condition"];

		int script_type = script_json["type"].asInt();
		if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
		{
			ScriptPlayerDetail::LegendTopInfo &legend_top_info = this->script_detail_.__legend_top_info;
		    if (legend_top_info.__is_sweep == true)
		    {
		    	event_map[GameEnum::PA_EVENT_SCRIPT_LEGEND_SWEEP] = true;
		    }
		}
		else if (script_type == GameEnum::SCRIPT_T_SWORD_TOP)
		{
			ScriptPlayerDetail::LegendTopInfo &sword_top_info = this->script_detail_.__sword_top_info;
			if (sword_top_info.__is_sweep == true)
			{
				event_map[GameEnum::PA_EVENT_SCRIPT_SWORD_SWEEP] = true;
			}
		}
		else if (script_type == GameEnum::SCRIPT_T_EXP || script_type == GameEnum::SCRIPT_T_RAMA)
        {
            ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_type);
            if (type_record->__is_sweep == true)
            {
                if (script_type == GameEnum::SCRIPT_T_EXP)
                	event_map[GameEnum::PA_ENENT_SCRIPT_EXP_SWEEP] = true;
                else
                	event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_SWEEP] = true;
            }
            if (script_type == GameEnum::SCRIPT_T_EXP)
            {
            	const Json::Value &exp_first_award = finish_condition["exp_first_award"];
                for (uint i = 0; i < exp_first_award.size(); ++i)
                {
                	int wave_id = exp_first_award[i][0u].asInt();
                	int wave_chapter_key = this->convert_script_wave_key(iter->first, wave_id);
                	ScriptPlayerDetail::ScriptWaveRecord *special_record = this->special_record(wave_chapter_key);
                	if (special_record->__is_get == 1)
                	{
                		event_map[GameEnum::PA_EVENT_SCRIPT_EXP_BOX] = true;
                	}
                }
            }
            else if (script_type == GameEnum::SCRIPT_T_RAMA && script_record->__is_first_pass <= 0)
            {
            	event_map[GameEnum::PA_EVENT_SCRIPT_RAMA_FIGHT] = true;
            }
            continue;
        }

        JUDGE_CONTINUE(this->level() >= prev_condition["level"].asInt());

		int left_times = this->fetch_script_left_times(script_record, script_json);
		JUDGE_CONTINUE(left_times > 0);

        if (script_type == GameEnum::SCRIPT_T_ADVANCE)
        {
			event_map[GameEnum::PA_EVENT_SCRIPT_ADVANCE_FIGHT] = true;
        }
        else if (script_type == GameEnum::SCRIPT_T_TRVL)
        {
        	JUDGE_CONTINUE(this->script_detail_.is_have_trvl_red() == true);
        	event_map[GameEnum::PA_EVENT_TRVL_TOTAL_TIMES] = true;
        }
        else if (script_type == GameEnum::SCRIPT_T_STORY)
        {
            if (script_record->__is_first_pass > 0)
                event_map[GameEnum::PA_EVENT_SCRIPT_STORY_SWEEP] = true;
            else
                event_map[GameEnum::PA_EVENT_SCRIPT_STORY_FIGHT] = true;
        }
        else if (script_type == GameEnum::SCRIPT_T_VIP)
        {
        	int vip_level = script_json["prev_condition"]["vip_level"].asInt();
            int my_vip = this->vip_detail().__vip_level;
            JUDGE_CONTINUE(my_vip >= vip_level);
            event_map[GameEnum::PA_EVENT_SCRIPT_VIP_FIGHT] = true;
        }
	}
	return this->check_pa_event_all_script(event_map);
}


void MapPlayerScript::set_protect_beast_index(const int script_sort, const int index)
{
    ScriptPlayerDetail::ScriptRecord *record = this->refresh_script_record(script_sort);
    JUDGE_RETURN(record != NULL, ;);

    if (record->__protect_beast_index < index)
        record->__protect_beast_index = index;
}

int MapPlayerScript::process_fetch_couple_script_times(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto50101407 *, request, msg, -1);

    int couple_script_sort = GameEnum::SCRIPT_SORT_COUPLE;
    int left_times = this->fetch_script_left_times(couple_script_sort);
    request->set_left_script_times(left_times);
    return this->respond_to_client(RETURN_WEDDING_PANNEL, request);
}

int MapPlayerScript::couple_fb_select_key(Message *msg)
{
	BaseScript *script = this->fetch_script();
	CONDITION_NOTIFY_RETURN(script != NULL, RETURN_COUPLE_FB_SELECT_KEY, ERROR_CLIENT_OPERATE);
	DYNAMIC_CAST_RETURN(Proto10400941 *, request, msg, -1);

	int select_id = request->select_id();
	LongMap &couple_sel = script->script_detail().couple_sel_;
	CONDITION_NOTIFY_RETURN(couple_sel.count(this->role_id()) <= 0,
			RETURN_COUPLE_FB_SELECT_KEY, ERROR_CLIENT_OPERATE);

	couple_sel[this->role_id()] = select_id;
	return this->respond_to_client(RETURN_COUPLE_FB_SELECT_KEY);
}

int MapPlayerScript::sword_top_select_skill(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10400943 *, request, msg, -1);

	int index = request->index();
	const Json::Value &conf = CONFIG_INSTANCE->script_sword_skill(index);
	CONDITION_NOTIFY_RETURN(conf != Json::Value::null, RETURN_SELECT_SWORD_SKILL,
			ERROR_CONFIG_NOT_EXIST);

	ScriptPlayerDetail::LegendTopInfo &top_info = this->top_info(GameEnum::SCRIPT_T_SWORD_TOP);
	int need_floor = conf["get_lv"].asInt();
	CONDITION_NOTIFY_RETURN(top_info.__pass_floor >= need_floor,
			RETURN_SELECT_SWORD_SKILL, ERROR_FLOOR_NOT_ENOUGH);

	int skill_id = conf["skill_id"].asInt();
	this->remove_skill(skill_id);

	this->script_detail_.__skill_id = index;

	Proto50400943 respond;
	respond.set_index(index);

	return this->respond_to_client(RETURN_SELECT_SWORD_SKILL, &respond);
}

int MapPlayerScript::sword_script_set_skill(int type)
{
	BaseScript *script = this->fetch_script();
	JUDGE_RETURN(this->script_detail_.__skill_id > 0, 0);
	JUDGE_RETURN(script != NULL && script->script_type() == GameEnum::SCRIPT_T_SWORD_TOP, 0);

	const Json::Value &conf = CONFIG_INSTANCE->script_sword_skill(this->script_detail_.__skill_id);
	JUDGE_RETURN(conf != Json::Value::null, 0);

	int skill_id = conf["skill_id"].asInt();
	if (type == BaseScript::SKILL_SET)
		this->insert_skill(skill_id);
	else
		this->remove_skill(skill_id);

	return 0;
}

int MapPlayerScript::request_remove_stone_state(Message *msg)
{
//    DYNAMIC_CAST_RETURN(Proto10400937 *, request, msg, -1);
//
//    Int64 mover_id = request->mover_id();
//    CONDITION_NOTIFY_RETURN(mover_id != this->role_id(), RETURN_REMOVE_STONE_STATE, ERROR_CLIENT_OPERATE);
//
//    MapPlayerEx *player = this->find_player(mover_id);
//    CONDITION_NOTIFY_RETURN(player != NULL, RETURN_REMOVE_STONE_STATE, ERROR_CLIENT_OPERATE);
//    CONDITION_NOTIFY_RETURN(player->is_in_script_mode() && player->scene_id() == this->scene_id() &&
//    		player->space_id() == this->space_id(), RETURN_REMOVE_STONE_STATE, ERROR_CLIENT_OPERATE);
//
//
//    BasicStatus *status = NULL;
//    if (player->find_status(BasicStatus::STONE_PLAYER, status) == 0)
//    {
//        player->remove_status(status);
//        player->clean_status(BasicStatus::DIZZY);
//    }
//    return this->respond_to_client(RETURN_REMOVE_STONE_STATE);
	return 0;
}

int MapPlayerScript::get_monster_tower_pass_time_by_floor(int sort, const ScriptPlayerDetail &record)
{
	IntVec set;
	char str[6] = {0};
	::snprintf(str, sizeof(str),"%d",sort);
	for(int i = 1; i <= 4; ++i)
	{
		str[3] = (char)('0'+i);
		set.push_back(::atoi(str));
	}
	int ret = 0;
	for(IntVec::const_iterator it = set.begin(); it != set.end(); ++it)
	{
		int sort_id = *it;
		if(record.__record_map.find(sort_id) != record.__record_map.end())
		{
			if(sort == sort_id)
			{
				ret += record.__record_map.at(sort_id).__day_pass_times;
			}
			else if(is_same_day(record.__record_map.at(sort_id).__used_times_tick, next_day(0,0,Time_Value::gettimeofday())))
			{
				ret += record.__record_map.at(sort_id).__day_pass_times;
			}
		}
	}
	return ret;
}

int MapPlayerScript::update_monster_tower_pass_time(int sort, ScriptPlayerDetail &record)
{
	IntVec set;
	char str[6] = {0};
	::snprintf(str, sizeof(str),"%d",sort);
	for(int i = 1; i <= 4; ++i)
	{
		str[3] = (char)('0'+i);
		set.push_back(::atoi(str));
	}
	int pass_time = record.__record_map.at(sort).__day_pass_times;
	Time_Value time = record.__record_map.at(sort).__used_times_tick;
	for(IntVec::iterator it = set.begin(); it != set.end(); ++it)
	{
		int sort_id = *it;
		if(record.__record_map.find(sort_id) != record.__record_map.end())
		{
			if(sort != sort_id)
			{
				record.__record_map.at(sort_id).__day_pass_times = pass_time;
				record.__record_map.at(sort_id).__used_times_tick = time;
			}
		}
	}
	return 0;
}


int MapPlayerScript::gather_goods_begin(Message* msg)
{
	return MapPlayer::gather_goods_begin(msg);
}

int MapPlayerScript::gather_goods_done(Message* msg)
{
	return MapPlayer::gather_goods_done(msg);
}

int MapPlayerScript::fetch_legend_top_rank(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10400940 *, request, msg, -1);

	Proto30400911 inner;
	inner.set_script_type(GameEnum::SCRIPT_T_LEGEND_TOP);
	inner.set_num1(request->num1());
	inner.set_num2(request->num2());
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::LEGEND_TOP_SCENE, &inner);
}

int MapPlayerScript::fetch_sword_top_rank(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto10400942 *, request, msg, -1);

	Proto30400911 inner;
	inner.set_script_type(GameEnum::SCRIPT_T_SWORD_TOP);
	inner.set_page(request->page());
	return MAP_MONITOR->dispatch_to_scene(this, GameEnum::LEGEND_TOP_SCENE, &inner);
}

int MapPlayerScript::script_red_point_check_uplvl()
{
	return this->check_pa_event_script_level();
}

int MapPlayerScript::script_sync_restore_info(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto30400902 *, request, -1);

	int script_sort = request->script_sort();
	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	JUDGE_RETURN(script_json.empty() == false, -1);

	int transfer_date = 0;
	int event_id = 0;
	if (script_json["type"].asInt() == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		ScriptPlayerDetail::LegendTopInfo &legend_top = this->script_detail_.__legend_top_info;
		transfer_date = legend_top.__pass_floor + 1;
		event_id = GameEnum::ES_ACT_LEGEND_TOP;
	}
	// 经验副本，罗摩副本
	else if (script_json["type"].asInt() == GameEnum::SCRIPT_T_EXP)
	{
		ScriptPlayerDetail::TypeRecord *type_record = this->type_record(script_json["type"].asInt());
		int wave = script_json["scene"][0u]["exec"]["wave"].asInt();
		int pass_piece = type_record->__pass_wave;
		int pass_chapter = type_record->__pass_chapter;
		transfer_date = pass_piece + pass_chapter * wave;

		event_id = GameEnum::ES_ACT_EXP_FB;
	}

	this->check_script_restore_info(script_json, event_id, transfer_date);

	JUDGE_RETURN(event_id > 0, -1);

	this->sync_restore_info(event_id, transfer_date, 1);
	return 0;
}

int MapPlayerScript::script_clean_sync_restore(int script_sort)
{
	int event_id = 0;
	int transfer_date = 0;
	const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
	int script_type = script_json["type"].asInt();

//	ScriptPlayerDetail::ScriptRecord *script_record = this->refresh_script_record(script_sort);
	if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP)
	{
		transfer_date = this->script_detail_.__legend_top_info.__pass_floor;
		event_id = GameEnum::ES_ACT_LEGEND_TOP;
	}
	else if (script_type == GameEnum::SCRIPT_T_EXP)
	{
		ScriptPlayerDetail::TypeRecord *type_info = this->type_record(script_type);
		transfer_date = type_info->__pass_wave + type_info->__pass_chapter * 20;
		event_id = GameEnum::ES_ACT_EXP_FB;
	}

	this->check_script_restore_info(script_json, event_id, transfer_date);

	JUDGE_RETURN(event_id > 0, -1);

	this->sync_restore_info(event_id, transfer_date, 1);
	return 0;
}

int MapPlayerScript::check_script_restore_info(const Json::Value &script_json, int &event_id, int &value)
{
	int script_type = script_json["type"].asInt();
	if (script_type == GameEnum::SCRIPT_T_STORY)
	{
		int ext_type = script_json["priority"].asInt();
		switch (ext_type)
		{
		case 1:
			event_id = GameEnum::ES_ACT_STORY_FB_1;
			break;
		case 2:
			event_id = GameEnum::ES_ACT_STORY_FB_2;
			break;
		case 3:
			event_id = GameEnum::ES_ACT_STORY_FB_3;
			break;
		case 4:
			event_id = GameEnum::ES_ACT_STORY_FB_4;
			break;
		case 5:
			event_id = GameEnum::ES_ACT_STORY_FB_5;
			break;
		case 6:
			event_id = GameEnum::ES_ACT_STORY_FB_6;
			break;
		case 7:
			event_id = GameEnum::ES_ACT_STORY_FB_7;
			break;
		}

		value = 1;
	}
	else if (script_type == GameEnum::SCRIPT_T_ADVANCE)
	{
		int ext_type = script_json["priority"].asInt();
		switch (ext_type)
		{
		case 1:
			event_id = GameEnum::ES_ACT_FUN_MOUNT;
			break;
		case 2:
			event_id = GameEnum::ES_ACT_FUN_MAGIC_EQUIP;
			break;
		case 3:
			event_id = GameEnum::ES_ACT_FUN_LING_BEAST;
			break;
		case 4:
			event_id = GameEnum::ES_ACT_FUN_XIAN_WING;
			break;
		case 5:
			event_id = GameEnum::ES_ACT_FUN_GOD_SOLIDER;
			break;
		case 6:
			event_id = GameEnum::ES_ACT_FUN_BEAST_EQUIP;
			break;
		case 7:
			event_id = GameEnum::ES_ACT_FUN_BEAST_MOUNT;
			break;

		case 8 :
			event_id = GameEnum::ES_ACT_FUN_BEAST_WING;
			break;
		case 9 :
			event_id = GameEnum::ES_ACT_FUN_BEAST_MAO;
			break;
		case 10 :
			event_id = GameEnum::ES_ACT_FUN_TIAN_GANG;
			break;

		}

		value = 1;
	}
	else if (script_type == GameEnum::SCRIPT_T_VIP)
	{
		int ext_type = script_json["priority"].asInt();
		switch (ext_type)
		{
		case 1:
			event_id = GameEnum::ES_ACT_VIP_1_FB;
			break;
		case 2:
			event_id = GameEnum::ES_ACT_VIP_3_FB;
			break;
		case 3:
			event_id = GameEnum::ES_ACT_VIP_6_FB;
			break;
		case 4:
			event_id = GameEnum::ES_ACT_VIP_9_FB;
			break;
		}
		value = 1;
	}
	else if (script_type == GameEnum::SCRIPT_T_TRVL)
	{
		value = 1;
	}
	return 0;
}

int MapPlayerScript::update_branch_task_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400026 *, request, -1);

	int type = request->id();
	if (type == GameEnum::BRANCH_LEGEND_TOP)
	{
		this->sync_branch_task_info(type, this->script_detail_.__legend_top_info.__pass_floor);
	}
	else if (type == GameEnum::BRANCH_EXP_FB)
	{
		ScriptPlayerDetail::TypeRecord *type_record = this->type_record(GameEnum::SCRIPT_T_EXP);

		int wave_num = 0;
		const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();
		for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
				iter != script_map.end(); ++iter)
		{
			const Json::Value &script_json = *(iter->second);
			JUDGE_CONTINUE(script_json != Json::Value::null);

			if (script_json["type"].asInt() == GameEnum::SCRIPT_T_EXP)
			{
				wave_num = script_json["scene"][0u]["exec"]["wave"].asInt();
				break;
			}
		}
		int pass_piece = type_record->__pass_wave;
		int pass_chapter = type_record->__pass_chapter;
		int transfer_date = pass_piece + pass_chapter * wave_num;

		this->sync_branch_task_info(type, transfer_date);
	}
	else
	{
		ScriptPlayerDetail::ScriptRecord* script_record = this->script_record(type);
		this->sync_branch_task_info(type, script_record->__is_first_pass);
	}

	return 0;

}

int MapPlayerScript::update_cornucopia_task_info(int num, int type)
{
	int task_id = 0;
	switch(type)
	{
		case GameEnum::SCRIPT_T_ADVANCE:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_ADVANCE_SCRIPT;
			break;
		}
		case GameEnum::SCRIPT_T_TRVL:
		{
			task_id = GameEnum::CORNUCOPIA_TASK_TRVL_SCRIPT;
			break;
		}
		default:
		{
			return -1;
		}
	}
	Proto31403200 task_info;
	task_info.set_task_id(task_id);
	task_info.set_task_finish_count(num);
	MSG_USER("MapPlayerScript, update_cornucopia_task_info, Proto31403200: %s", task_info.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}

int MapPlayerScript::update_labour_task_info(int num, int type)
{
	int task_id = 0;
	switch(type)
	{
		case GameEnum::SCRIPT_T_TRVL:
		{
			task_id = GameEnum::LABOUR_TASK_TRVL_SCRIPT;
			break;
		}
		default:
		{
			return -1;
		}
	}
	Proto31403201 task_info;
	task_info.set_task_id(task_id);
	task_info.set_task_finish_count(num);
	MSG_USER("MapPlayerScript, update_labour_task_info, Proto31403201: %s", task_info.Utf8DebugString().c_str());
	return MAP_MONITOR->dispatch_to_logic(this, &task_info);
}
