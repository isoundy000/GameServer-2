/*
 * MLScriptClean.cpp
 *
 * Created on: 2014-04-24 16:01
 *     Author: lyz
 */

#include "MLScriptClean.h"
#include "MapLogicPlayer.h"
#include "MapTaskStruct.h"

#include "GameFont.h"
#include "ScriptStruct.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MapLogicTasker.h"

MLScriptClean::~MLScriptClean(void)
{ /*NULL*/ }

void MLScriptClean::reset_script_clean(void)
{
    this->clean_detail_.reset();
    this->reward_info_set_.clear();
}

MapLogicPlayer *MLScriptClean::player(void)
{
    return dynamic_cast<MapLogicPlayer *>(this);
}

int MLScriptClean::cal_reset_multiple(const Json::Value reset_multiple, int reset_times)
{
	JUDGE_RETURN(reset_multiple != Json::Value::null, 1);

	int reset_mult = 1;
	for (uint i = 0; i < reset_multiple.size(); ++i)
	{
		if (reset_times == int(i + 1))
		{
			reset_mult = reset_multiple[i].asInt();
			break;
		}
	}

	return reset_mult;
}

int MLScriptClean::request_script_clean_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400902 *, request, -1);

	Proto51401209 respond;
	respond.set_script_sort(request->script_sort());
	respond.set_script_type(request->script_type());

	for (ScriptCleanDetail::ScriptInfoVec::iterator iter = this->clean_detail_.__current_script_vc.begin();
		iter != this->clean_detail_.__current_script_vc.end(); ++iter)
	{
		ScriptCleanDetail::ScriptInfo &curr_info = *iter;
		int script_sort = curr_info.__script_sort;

		const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
		JUDGE_RETURN(script_json != Json::Value::null, -1);

		ScriptCleanDetail::ScriptItemInfo &script_item = this->clean_detail_.__script_item_map[script_sort];
		script_item.__id = script_sort;

		int script_type = script_json["type"].asInt();
		const Json::Value &finish_condition = script_json["finish_condition"];
		const Json::Value &prev_condition = script_json["prev_condition"];
		if (finish_condition.isMember("award_item"))
		{
			int award_item = finish_condition["award_item"].asInt();
//			ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
			ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
			item_obj.__item_id = award_item;

			if (script_type == GameEnum::SCRIPT_T_ADVANCE)
			{
				const Json::Value reset_multiple = prev_condition["reset_multiple"];
				int reset_times = curr_info.__reset_times;
				int reset_mult = this->cal_reset_multiple(reset_multiple, reset_times);
				item_obj.__amount += reset_mult;

				respond.set_reset_times(reset_times);
			} else
			{
				item_obj.__amount += 1;
			}
		}

		if (finish_condition.isMember("pass_award_item"))
		{
			{
				const Json::Value &pass_award_item = finish_condition["pass_award_item"];
				int top_num = 0;
				if (script_type == GameEnum::SCRIPT_T_LEGEND_TOP || script_type == GameEnum::SCRIPT_T_SWORD_TOP)
				{
					top_num = this->clean_detail_.__top_floor;
					respond.set_top_floor(top_num);
				}
				else
				{
					top_num = this->clean_detail_.__pass_wave;
					respond.set_pass_wave(top_num);
					respond.set_pass_chapter(this->clean_detail_.__pass_chapter);
				}
				for (uint i = 0; i < pass_award_item.size(); ++i)
				{
					int top_id = pass_award_item[i][0u].asInt();
					if (top_num >= top_id)
					{
						int award_item = pass_award_item[i][1u].asInt();
//						ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
						ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
						item_obj.__item_id = award_item;
						item_obj.__amount += 1;
					}
				}
			}

			const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_sort);
			const Json::Value &before_script_id = script_clean_json["before_script_id"];
			for (uint i = 0; i < before_script_id.size(); ++i)
			{
				int before_script = before_script_id[i].asInt();
				const Json::Value &before_json = CONFIG_INSTANCE->script(before_script);
				const Json::Value &finish_condition = before_json["finish_condition"];
				const Json::Value &pass_award_item = finish_condition["pass_award_item"];

				for (uint j = 0; j < pass_award_item.size(); ++j)
				{
					int award_item = pass_award_item[j][1u].asInt();
//					ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
					ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
					item_obj.__item_id = award_item;
					item_obj.__amount += 1;
				}
			}
		}

		if (finish_condition.isMember("league_award_item"))
		{
			if (this->clean_detail_.__pass_chapter > this->clean_detail_.__start_chapter)
			{
				{
					const Json::Value &league_award_item = finish_condition["league_award_item"];
					for (uint i = 0; i < league_award_item.size(); ++i)
					{
						int wave_id = league_award_item[i][0u].asInt();
						if (this->clean_detail_.__pass_wave >= wave_id)
						{
							int award_item = league_award_item[i][1u].asInt();
//							ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
							ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
							item_obj.__item_id = award_item;
							item_obj.__amount += 1;
						}
					}
				}

				const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_sort);
				const Json::Value &before_script_id = script_clean_json["before_script_id"];
				int chapter = this->clean_detail_.__pass_chapter - this->clean_detail_.__start_chapter;
				for (uint i = before_script_id.size() - chapter; i < before_script_id.size(); ++i)
				{
					int before_script = before_script_id[i].asInt();
					const Json::Value &before_json = CONFIG_INSTANCE->script(before_script);
					const Json::Value &finish_condition = before_json["finish_condition"];
					const Json::Value &league_award_item = finish_condition["league_award_item"];
					for (uint j = 0; j < league_award_item.size(); ++j)
					{
						int wave_id = league_award_item[i][0u].asInt();
						if (i == before_script_id.size() - chapter)
						{
							JUDGE_CONTINUE(wave_id > this->clean_detail_.__start_wave);
						}

						int award_item = league_award_item[j][1u].asInt();
//						ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
						ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
						item_obj.__item_id = award_item;
						item_obj.__amount += 1;
					}
				}
			}
			else
			{
				const Json::Value &league_award_item = finish_condition["league_award_item"];
				for (uint i = 0; i < league_award_item.size(); ++i)
				{
					int wave_id = league_award_item[i][0u].asInt();
					if (this->clean_detail_.__pass_wave >= wave_id &&
							wave_id > this->clean_detail_.__start_wave)
					{
						int award_item = league_award_item[i][1u].asInt();
//						ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[award_item];
						ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[award_item];
						item_obj.__item_id = award_item;
						item_obj.__amount += 1;
					}
				}
			}
		}

		if (finish_condition.isMember("exp_award_item"))
		{
			{
				int exp_award_item = finish_condition["exp_award_item"].asInt();
				int pass_wave = this->clean_detail_.__pass_wave;
				int pass_chapter = this->clean_detail_.__pass_chapter;
				for (int i = 0; i < pass_wave; ++i)
				{
//					ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[exp_award_item];
					ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[exp_award_item];
					item_obj.__item_id = exp_award_item;
					item_obj.__amount += 1;
				}

				respond.set_pass_wave(pass_wave);
				respond.set_pass_chapter(pass_chapter);
			}

			const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_sort);
			const Json::Value &before_script_id = script_clean_json["before_script_id"];
			for (uint i = 0; i < before_script_id.size(); ++i)
			{
				int before_script = before_script_id[i].asInt();
				const Json::Value &before_json = CONFIG_INSTANCE->script(before_script);
				const Json::Value &finish_condition = before_json["finish_condition"];
				int exp_award_item = finish_condition["exp_award_item"].asInt();
				int total_wave = before_json["scene"][0u]["exec"]["wave"].asInt();

				for (int j = 0; j < total_wave; ++j)
				{
//					ScriptCleanDetail::CleanItemInfo &item_obj = this->clean_detail_.__item_map[exp_award_item];
					ScriptCleanDetail::CleanItemInfo &item_obj = script_item.__item_map[exp_award_item];
					item_obj.__item_id = exp_award_item;
					item_obj.__amount += 1;
				}
			}
		}
	}

	// 客户端展示
	MapLogicPlayer *player = this->player();
	TipsPlayer tips(player);
	IntMap tip_item_map;

	int mult = this->clean_detail_.__mult;

	for (ScriptCleanDetail::ScriptItemMap::iterator iter = this->clean_detail_.__script_item_map.begin();
			iter != this->clean_detail_.__script_item_map.end(); ++iter)
	{
		ScriptCleanDetail::ScriptItemInfo &script_item = iter->second;

		for (ScriptCleanDetail::ItemMap::iterator it = script_item.__item_map.begin();
				it != script_item.__item_map.end(); ++it)
		{
			ScriptCleanDetail::CleanItemInfo &item_obj = it->second;

			const Json::Value& reward_json = CONFIG_INSTANCE->reward(item_obj.__item_id);
			JUDGE_CONTINUE(reward_json.empty() == false);

			RewardInfo reward_info(true, NULL, this->player());
			GameCommon::make_up_reward_items(reward_info, reward_json);

			for (ItemObjVec::iterator item_iter = reward_info.item_vec_.begin();
					item_iter != reward_info.item_vec_.end(); ++item_iter)
			{
				item_iter->amount_ *= item_obj.__amount;
				item_iter->amount_ *= mult;

				ItemObj &get_obj = script_item.__get_item_map[item_iter->id_];
				get_obj.id_ = item_iter->id_;
				get_obj.amount_ += item_iter->amount_;
				get_obj.bind_ = item_iter->bind_;

				tip_item_map[item_iter->id_] += item_iter->amount_;
			}

			for (IntMap::iterator resouce_iter = reward_info.resource_map_.begin();
					resouce_iter != reward_info.resource_map_.end(); ++resouce_iter)
			{
				resouce_iter->second *= item_obj.__amount;
				resouce_iter->second *= mult;

				ItemObj &get_obj = script_item.__get_item_map[resouce_iter->first];
				get_obj.id_ = resouce_iter->first;
				get_obj.amount_ += resouce_iter->second;

				tip_item_map[resouce_iter->first] += resouce_iter->second;
			}

			if (reward_info.exp_ > 0)
			{
				reward_info.exp_ *= item_obj.__amount;
				reward_info.exp_ *= mult;

				ItemObj &get_obj = script_item.__get_item_map[GameEnum::ITEM_ID_PLAYER_EXP];
				get_obj.id_ = GameEnum::ITEM_ID_PLAYER_EXP;
				get_obj.amount_ += reward_info.exp_;

				tip_item_map[GameEnum::ITEM_ID_PLAYER_EXP] += reward_info.exp_;
			}

			this->reward_info_set_.push_back(reward_info);
		}

		for (ScriptCleanDetail::ItemMap::iterator it = script_item.__drop_map.begin();
				it != script_item.__drop_map.end(); ++it)
		{
			ScriptCleanDetail::CleanItemInfo &drop_obj = it->second;

			const Json::Value& reward_json = CONFIG_INSTANCE->reward(drop_obj.__item_id);
			JUDGE_CONTINUE(reward_json.empty() == false);

			RewardInfo reward_info(true, NULL, this->player());;
			GameCommon::make_up_reward_items(reward_info, reward_json);

			for (IntMap::iterator resouce_iter = reward_info.resource_map_.begin();
					resouce_iter != reward_info.resource_map_.end(); ++resouce_iter)
			{
				resouce_iter->second *= drop_obj.__amount;
				resouce_iter->second *= mult;

				ItemObj &get_obj = script_item.__get_drop_map[resouce_iter->first];
				get_obj.id_ = resouce_iter->first;
				get_obj.amount_ += resouce_iter->second;
				tip_item_map[resouce_iter->first] += resouce_iter->second;
			}

			for (ItemObjVec::iterator item_iter = reward_info.item_vec_.begin();
					item_iter != reward_info.item_vec_.end(); ++item_iter)
			{
				item_iter->amount_ *= drop_obj.__amount;
				item_iter->amount_ *= mult;

				ItemObj &get_obj = script_item.__get_drop_map[item_iter->id_];
				get_obj.id_ = item_iter->id_;
				get_obj.amount_ += item_iter->amount_;
				get_obj.bind_ = item_iter->bind_;

				tip_item_map[item_iter->id_] += item_iter->amount_;
			}

			if (reward_info.exp_ > 0)
			{
				reward_info.exp_ *= drop_obj.__amount;
				reward_info.exp_ *= mult;

				ItemObj &get_obj = script_item.__get_drop_map[GameEnum::ITEM_ID_PLAYER_EXP];
				get_obj.id_ = GameEnum::ITEM_ID_PLAYER_EXP;
				get_obj.amount_ += reward_info.exp_;

				tip_item_map[GameEnum::ITEM_ID_PLAYER_EXP] += reward_info.exp_;
			}

			this->reward_info_set_.push_back(reward_info);
		}
	}

	for (ScriptCleanDetail::ScriptItemMap::iterator iter = this->clean_detail_.__script_item_map.begin();
			iter != this->clean_detail_.__script_item_map.end(); ++iter)
	{
		ScriptCleanDetail::ScriptItemInfo &script_item = iter->second;

		ProtoCleanInfo *proto_clean = respond.add_clean_item();
		proto_clean->set_script_sort(script_item.__id);

		for (ItemObjMap::iterator get_iter = script_item.__get_item_map.begin();
				get_iter != script_item.__get_item_map.end(); ++get_iter)
		{
			ItemObj &get_obj = get_iter->second;

			ProtoItem *proto_item = proto_clean->add_item_list();
			get_obj.serialize(proto_item);
		}

		for (ItemObjMap::iterator drop_iter = script_item.__get_drop_map.begin();
				drop_iter != script_item.__get_drop_map.end(); ++drop_iter)
		{
			ItemObj &drop_obj = drop_iter->second;

			ProtoItem *proto_drop = proto_clean->add_drop_list();
			drop_obj.serialize(proto_drop);
		}
	}

	// 飘字
	for (IntMap::iterator iter = tip_item_map.begin(); iter != tip_item_map.end(); ++iter)
	{
		tips.push_goods(iter->first, iter->second);
	}

	return this->player()->respond_to_client(RETURN_SCRIPT_CLEAN_INFO_NEW, &respond);
}

int MLScriptClean::request_start_clean_single_script(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto11401202 *, request, -1);

    MapLogicPlayer *player = this->player();

    int script_sort = request->script_sort();
    int script_type = request->script_type();
    CONDITION_PLAYER_NOTIFY_RETURN((script_sort <= 0 && script_type > 0)
    		|| (script_sort > 0 && script_type <= 0),
    		RETURN_CLEAN_SINGLE_SCRIPT, ERROR_CLIENT_OPERATE);

    int script = 0;
    if (script_type > 0)
    	script = script_type;
    else
    	script = script_sort;

    bool is_vip = false, is_level = false;
    const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script);
    CONDITION_PLAYER_NOTIFY_RETURN(script_clean_json != Json::Value::null,
    		RETURN_CLEAN_SINGLE_SCRIPT,ERROR_CLIENT_OPERATE);

    if (script_clean_json.isMember("vip"))
    {
    	if (player->vip_detail().__vip_level >= script_clean_json["vip"].asInt())
    		is_vip = true;
    }
    if (script_clean_json.isMember("level"))
    {
    	if (player->role_level() >= script_clean_json["level"].asInt())
    		is_level = true;
    }
    CONDITION_PLAYER_NOTIFY_RETURN(is_vip == true || is_level == true, RETURN_CLEAN_SINGLE_SCRIPT, ERROR_SCRIPT_CLEAN_VIP_LEVEL);

    Proto30400918 req;
    req.set_prev_scene_id(player->scene_id());
    req.set_type(GameEnum::SCRIPT_CT_SINGLE);
    req.set_script_sort(script_sort);
    req.set_scrit_type(script_type);
    return MAP_MONITOR->dispatch_to_scene(player, GameEnum::LEGEND_TOP_SCENE, &req);
}

bool MLScriptClean::has_clean_script_award(void)
{
//	if (this->clean_detail_.__exp > 0 ||
//			this->clean_detail_.__money.__bind_copper > 0 ||
//			this->clean_detail_.__money.__bind_gold > 0 ||
//			this->clean_detail_.__money.__copper > 0 ||
//			this->clean_detail_.__money.__gold > 0 ||
//			this->clean_detail_.__item_map.size() > 0 ||
//			this->clean_detail_.__drop_map.size() > 0 ||
//			this->clean_detail_.__savvy > 0) {
//		return true;
//	}
	if (this->reward_info_set_.size() > 0)
	{
		return true;
	}
	return false;
}

int MLScriptClean::request_draw_clean_script_award(void)
{
    MapLogicPlayer *player = this->player();

    bool has_award = this->has_clean_script_award();
    CONDITION_PLAYER_NOTIFY_RETURN(has_award == true, RETURN_SCRIPT_CLEAN_AWARD, ERROR_NO_SCRIPT_CLEAN_AWARD);

    for (RewardInfoSet::iterator iter = this->reward_info_set_.begin();
    		iter != this->reward_info_set_.end(); ++iter)
    {
    	SerialObj obj(ADD_FROM_SCRIPT_CLEAN);
    	player->insert_package(obj, *iter);
    }

    this->clean_detail_.reset_award();
    this->reward_info_set_.clear();

    player->update_player_assist_single_event(GameEnum::PA_EVENT_SCRIPT_CLEAN, 0);

    return 0;
}

ScriptCleanDetail &MLScriptClean::script_clean_detail(void)
{
    return this->clean_detail_;
}

int MLScriptClean::script_clean_state(void)
{
    return this->clean_detail_.__state;
}

int MLScriptClean::process_rollback_script(void)
{
    Proto31400903 req;
    for (size_t i = 0; i < this->clean_detail_.__current_script_vc.size(); ++i)
    {
        ScriptCleanDetail::ScriptInfo &cur_info = this->clean_detail_.__current_script_vc[i];
        if (this->clean_detail_.__finish_script_vc.size() <= i)
        {
            ProtoScriptClean *proto_roll = req.add_rollback_script();
            proto_roll->set_script_sort(cur_info.__script_sort);
            proto_roll->set_script_times(cur_info.__use_times);
            proto_roll->set_chapter_key(cur_info.__chapter_key);
        }
        else
        {
            ScriptCleanDetail::ScriptInfo &finish_info = this->clean_detail_.__finish_script_vc[i];
            if (finish_info.__use_times >= cur_info.__use_times)
                continue;

            ProtoScriptClean *proto_roll = req.add_rollback_script();
            proto_roll->set_script_sort(cur_info.__script_sort);
            proto_roll->set_script_times(cur_info.__use_times - finish_info.__use_times);
            proto_roll->set_chapter_key(cur_info.__chapter_key);
        }
    }
    if (req.rollback_script_size() <= 0)
    	return 0;

    return this->player()->send_to_map_thread(req);
}

int MLScriptClean::process_start_script_clean(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400902 *, request, msg, -1);

    Time_Value nowtime = Time_Value::gettimeofday();

    this->clean_detail_.reset();
    int total_tick = 0;
    ScriptCleanDetail::ScriptInfo script_info;
    for (int i = 0; i < request->script_list_size(); ++i)
    {
        const ProtoScriptClean &proto_clean = request->script_list(i);

        const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(proto_clean.script_sort());
        if (script_clean_json == Json::Value::null)
            continue;

        script_info.reset();
        script_info.__script_sort = proto_clean.script_sort();
        script_info.__use_times = proto_clean.script_times();
        script_info.__chapter_key = proto_clean.chapter_key();
        script_info.__reset_times = proto_clean.reset_times();
        script_info.__protect_beast_index = proto_clean.protect_beast_index();
        this->clean_detail_.__current_script_vc.push_back(script_info);
    }

    this->clean_detail_.__reset_times = request->reset_times();
    this->clean_detail_.__top_floor = request->top_floor();
    this->clean_detail_.__pass_wave = request->pass_wave();
    this->clean_detail_.__pass_chapter = request->pass_chapter();
    this->clean_detail_.__start_wave = request->start_wave();
    this->clean_detail_.__start_chapter = request->start_chapter();

    int mult = request->mult() > 0 ? request->mult() : 1;
    this->clean_detail_.__mult = mult;

    this->clean_detail_.__state = GameEnum::SCRIPT_CS_START;
    this->clean_detail_.__begin_tick = nowtime;
    this->clean_detail_.__end_tick = nowtime + Time_Value(total_tick);
    this->clean_detail_.__terminate_tick = next_day(0, 0, nowtime);

    this->player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SCRIPT_CLEAN);

    if (request->type() == GameEnum::SCRIPT_CT_SINGLE)
    {
    	this->generate_award(this->clean_detail_.__end_tick);

        Proto31400909 deduct_vit;
        deduct_vit.set_script_sort(request->script_sort());
        this->player()->send_to_map_thread(deduct_vit);

        //扫荡副本id流水
        this->player()->record_other_serial(SCRIPT_CLEAN_SERIAL, request->script_sort(), request->script_type());

        this->request_script_clean_info(msg);

        Proto51401202 respond;
        this->player()->respond_to_client(RETURN_CLEAN_SINGLE_SCRIPT, &respond);

        for (IntVec::iterator iter = this->clean_detail_.__clean_sort_list.begin();
                iter != this->clean_detail_.__clean_sort_list.end(); ++iter)
            this->script_clean_finish_hook(*iter);

        this->request_draw_clean_script_award();
        this->clean_detail_.reset();

        return 0;
    }
    else
    {
        Proto51401203 respond;
        respond.set_left_tick(total_tick);
        return this->player()->respond_to_client(RETURN_CLEAN_ALL_SCRIPT, &respond);
    }
}

int MLScriptClean::generate_award(const Time_Value &nowtime)
{
	for (ScriptCleanDetail::ScriptInfoVec::iterator iter = this->clean_detail_.__current_script_vc.begin();
			iter != this->clean_detail_.__current_script_vc.end(); ++iter)
	{
		ScriptCleanDetail::ScriptInfo &curr_info = *iter;
		this->generate_normal_script_award(curr_info.__script_sort);
	}

    this->player()->cache_tick().update_cache(MapLogicPlayer::CACHE_SCRIPT_CLEAN);
	return 0;
}

int MLScriptClean::generate_normal_script_award(const int script_sort)
{
    const Json::Value &script_clean_json = CONFIG_INSTANCE->script_clean_out(script_sort);
    JUDGE_RETURN(script_clean_json != Json::Value::null, -1);

    IntVec script_set;
    if (script_clean_json.isMember("before_script_id"))
    {
    	const Json::Value &before_script = script_clean_json["before_script_id"];
    	for (uint i = 0; i < before_script.size(); ++i)
    	{
    		int clean_script = before_script[i].asInt();
    		script_set.push_back(clean_script);
    	}
    }
    else
    {
    	script_set.push_back(script_sort);
    }
    JUDGE_RETURN(script_set.size() > 0, -1);

    for (IntVec::iterator iter = script_set.begin(); iter != script_set.end(); ++iter)
    {
    	const Json::Value &clean_json = CONFIG_INSTANCE->script_clean_out(*iter);
    	JUDGE_CONTINUE(clean_json != Json::Value::null);

    	ScriptCleanDetail::ScriptItemInfo &script_item = this->clean_detail_.__script_item_map[(*iter)];
    	script_item.__id = *iter;

    	const Json::Value &floor_item = clean_json["floor_item"];
    	if (floor_item != Json::Value::null)
    	{
    		int top_floor = this->clean_detail_.__top_floor;
    		for (uint i = 0; i < floor_item.size(); ++i)
			{
				int floor_id = floor_item[i][0u].asInt();
				if (top_floor >= floor_id)
				{
					for (uint j = 0; j < floor_item[i][1u].size(); ++j)
					{
						int item_id = floor_item[i][1u][j].asInt();
//						ScriptCleanDetail::CleanItemInfo &drop_obj = this->clean_detail_.__drop_map[item_id];
						ScriptCleanDetail::CleanItemInfo &drop_obj = script_item.__drop_map[item_id];
						drop_obj.__item_id = item_id;
						drop_obj.__amount += 1;
					}
				}
			}
    	}

    	const Json::Value &item_json = clean_json["item"];
		if(item_json != Json::Value::null)
		{
			for (uint i = 0; i < item_json.size(); ++i)
			{
				int item_id = item_json[i].asInt();
//				ScriptCleanDetail::CleanItemInfo &drop_obj = this->clean_detail_.__drop_map[item_id];
				ScriptCleanDetail::CleanItemInfo &drop_obj = script_item.__drop_map[item_id];
				drop_obj.__item_id = item_id;
				drop_obj.__amount += 1;
			}
		}
    }

    return 0;
}

int MLScriptClean::notify_script_clean_doing(void)
{
    if (this->script_clean_state() != GameEnum::SCRIPT_CS_START)
    {
        bool has_award = this->has_clean_script_award();
        JUDGE_RETURN(has_award == true, 0);
    }

    return this->player()->respond_to_client(ACTIVE_SCRIPT_CLEAN_DOING, 0);
}

int MLScriptClean::process_script_first_pass_award(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400906 *, request, -1);

    SerialObj obj;
    obj.type_  = ADD_FROM_SCRIPT_PASS;
    obj.value_ = request->script_sort();
    obj.sub_   = request->chapter_key();

    int mult = request->mult() > 0 ? request->mult() : 1;

    if (request->is_first_pass() == 1)
    {
    	obj.type_ = ADD_FROM_SCRIPT_FIRST_PASS;
    }

    if (obj.sub_ == 0)
    {
    	obj.sub_ = request->floor();
    }

    for (int i = 0; i < request->reward_id_size(); ++i)
    {
    	int reward_id = request->reward_id(i);
    	for (int j = 0; j < mult; ++j)
    		this->player()->add_reward(reward_id, obj, true);
    }

    return 0;
}

int MLScriptClean::process_script_add_times_use_gold(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400904 *, request, msg, -1);

    MapLogicPlayer *player = this->player();

    int script_sort = request->script_sort(), script_type = request->script_type();
    int need_gold = request->gold();
    Money money(need_gold);
    if (player->validate_money(money) == false)
    {
        request->set_ret_code(ERROR_PACKAGE_GOLD_AMOUNT);
        return player->send_to_map_thread(*request);
    }

    int sub_serial = script_sort > 0 ? script_sort : script_type;
    player->pack_money_sub(money, SerialObj(SUB_MONEY_SCRIPT_ADD_TIMES, sub_serial));
    return player->send_to_map_thread(*request);
}

int MLScriptClean::process_script_list_info(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400905 *, request, -1);

    MapLogicTasker::TaskIdSet &task_set = this->player()->task_submited_once_set();
    for (MapLogicTasker::TaskIdSet::iterator iter = task_set.begin(); iter != task_set.end(); ++iter)
    {
        request->add_task_list(*iter);
    }

    MapLogicTasker::TaskMap &task_map = this->player()->task_map();
    for (MapLogicTasker::TaskMap::iterator iter = task_map.begin(); iter != task_map.end(); ++iter)
    {
        TaskInfo *info = iter->second;
        if (info->is_accepted() || info->is_finish() || info->is_submit())
        {
            request->add_task_list(info->__task_id);
        }
    }

    return this->player()->send_to_map_thread(*request);
}

int MLScriptClean::serialize_script_clean(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400130 *, request, -1);

    request->set_role_id(this->player()->role_id());
    request->set_exp(this->clean_detail_.__exp);
    request->set_savvy(this->clean_detail_.__savvy);
    request->set_reset_times(this->clean_detail_.__reset_times);
    request->set_top_floor(this->clean_detail_.__top_floor);
    request->set_pass_wave(this->clean_detail_.__pass_wave);
    request->set_pass_chapter(this->clean_detail_.__pass_chapter);
    request->set_start_wave(this->clean_detail_.__start_wave);
    request->set_start_chapter(this->clean_detail_.__start_chapter);
    request->set_mult(this->clean_detail_.__mult);

    ProtoMoney *proto_money = request->mutable_money();
    proto_money->set_bind_copper(this->clean_detail_.__money.__bind_copper);
    proto_money->set_bind_gold(this->clean_detail_.__money.__bind_gold);
    proto_money->set_copper(this->clean_detail_.__money.__copper);
    proto_money->set_gold(this->clean_detail_.__money.__gold);

    request->set_state(this->clean_detail_.__state);
    request->set_begin_tick(this->clean_detail_.__begin_tick.sec());
    request->set_end_tick(this->clean_detail_.__end_tick.sec());
    request->set_terminate_tick(this->clean_detail_.__terminate_tick.sec());
    request->set_next_check_tick(this->clean_detail_.__next_check_tick.sec());

    {
        MapLogicPlayer *player = this->player();
        request->set_compact_type(player->script_compact_detail().__compact_type);
        request->set_start_tick(player->script_compact_detail().__start_tick.sec());
        request->set_expired_tick(player->script_compact_detail().__expired_tick.sec());
        request->set_sys_notify(player->script_compact_detail().__sys_notify);
    }

    for (ScriptCleanDetail::ScriptInfoVec::iterator iter = this->clean_detail_.__current_script_vc.begin(); 
            iter != this->clean_detail_.__current_script_vc.end(); ++iter)
    {
        ScriptCleanDetail::ScriptInfo &script_info = *iter;

        ProtoScriptClean *proto_script_info = request->add_current_script_list();
        proto_script_info->set_script_sort(script_info.__script_sort);
        proto_script_info->set_chapter_key(script_info.__chapter_key);
        proto_script_info->set_script_times(script_info.__use_times);
        proto_script_info->set_script_use_tick(script_info.__use_tick);
        proto_script_info->set_reset_times(script_info.__reset_times);
    }
    for (ScriptCleanDetail::ScriptInfoVec::iterator iter = this->clean_detail_.__finish_script_vc.begin(); 
            iter != this->clean_detail_.__finish_script_vc.end(); ++iter)
    {
        ScriptCleanDetail::ScriptInfo &script_info = *iter;

        ProtoScriptClean *proto_script_info = request->add_finish_script_list();
        proto_script_info->set_script_sort(script_info.__script_sort);
        proto_script_info->set_chapter_key(script_info.__chapter_key);
        proto_script_info->set_script_times(script_info.__use_times);
        proto_script_info->set_script_use_tick(script_info.__use_tick);
    }
    for (ScriptCleanDetail::ItemMap::iterator iter = this->clean_detail_.__item_map.begin();
            iter != this->clean_detail_.__item_map.end(); ++iter)
    {
        ScriptCleanDetail::CleanItemInfo &item_obj = iter->second;
        ProtoItem *proto_item = request->add_item_list();
        proto_item->set_id(item_obj.__item_id);
        proto_item->set_amount(item_obj.__amount);
    }
    for (ScriptCleanDetail::ItemMap::iterator iter = this->clean_detail_.__drop_map.begin();
            iter != this->clean_detail_.__drop_map.end(); ++iter)
    {
    	ScriptCleanDetail::CleanItemInfo &drop_obj = iter->second;
    	ProtoItem *proto_drop = request->add_drop_list();
    	proto_drop->set_id(drop_obj.__item_id);
    	proto_drop->set_amount(drop_obj.__amount);
    }
    return 0;
}

int MLScriptClean::unserialize_script_clean(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31400130 *, request, msg, -1);

    ScriptCleanDetail &detail = this->clean_detail_;
    detail.reset();
    int total_size = request->current_script_list_size();
    for (int i = 0; i < total_size; ++i)
    {
        const ProtoScriptClean &proto_clean = request->current_script_list(i);
        ScriptCleanDetail::ScriptInfo script_info;
        script_info.__script_sort = proto_clean.script_sort();
        script_info.__chapter_key = proto_clean.chapter_key();
        script_info.__use_times = proto_clean.script_times();
        script_info.__use_tick = proto_clean.script_use_tick();
        script_info.__reset_times = proto_clean.reset_times();
        detail.__current_script_vc.push_back(script_info);
    }
    total_size = request->finish_script_list_size();
    for (int i = 0; i < total_size; ++i)
    {
        const ProtoScriptClean &proto_clean = request->finish_script_list(i);
        ScriptCleanDetail::ScriptInfo script_info;
        script_info.__script_sort = proto_clean.script_sort();
        script_info.__chapter_key = proto_clean.chapter_key();
        script_info.__use_times = proto_clean.script_times();
        script_info.__use_tick = proto_clean.script_use_tick();
        detail.__finish_script_vc.push_back(script_info);
    }
    total_size = request->item_list_size();
    for (int i = 0; i < total_size; ++i)
    {
        const ProtoItem &proto_item = request->item_list(i);
        ScriptCleanDetail::CleanItemInfo &item_obj = detail.__item_map[proto_item.id()];
        item_obj.__item_id = proto_item.id();
        item_obj.__amount = proto_item.amount();
    }
    total_size = request->drop_list_size();
    for (int i = 0; i < total_size; ++i)
    {
    	const ProtoItem &proto_drop = request->drop_list(i);
    	ScriptCleanDetail::CleanItemInfo &drop_obj = detail.__drop_map[proto_drop.id()];
    	drop_obj.__item_id = proto_drop.id();
    	drop_obj.__amount = proto_drop.amount();
    }

    detail.__exp = request->exp();
    detail.__savvy = request->savvy();
    detail.__money.__bind_copper = request->money().bind_copper();
    detail.__money.__bind_gold = request->money().bind_gold();
    detail.__money.__copper = request->money().copper();
    detail.__money.__gold = request->money().gold();
    detail.__state = request->state();
    detail.__begin_tick.sec(request->begin_tick());
    detail.__end_tick.sec(request->end_tick());
    detail.__terminate_tick.sec(request->terminate_tick());
    detail.__next_check_tick.sec(request->next_check_tick());
    detail.__reset_times = request->reset_times();
    detail.__top_floor = request->top_floor();
    detail.__pass_wave = request->pass_wave();
    detail.__pass_chapter = request->pass_chapter();
    detail.__start_wave = request->start_wave();
    detail.__start_chapter = request->start_chapter();
    detail.__mult = request->mult();

    {
        ScriptCompactDetail &compact_detail = this->player()->script_compact_detail();
        compact_detail.reset();
        compact_detail.__compact_type = request->compact_type();
        compact_detail.__start_tick.sec(request->start_tick());
        compact_detail.__expired_tick.sec(request->expired_tick());
        compact_detail.__sys_notify = request->sys_notify();
    }

    return 0;
}

int MLScriptClean::sync_transfer_script_clean(const int scene_id)
{
    Proto31400130 request;
    this->serialize_script_clean(&request);
    return this->player()->send_to_other_logic_thread(scene_id, request);
}

int MLScriptClean::process_piece_total_star_award(Message *msg)
{
	return 0;
}

int MLScriptClean::script_clean_finish_hook(const int script_sort)
{
    // 经验找回增加完成次数
    this->player()->sync_exp_restore_event_done(script_sort, this->clean_detail_.__begin_tick);

    return 0;
}

int MLScriptClean::process_activity_score_award(const int script_sort, const int chapter_key, const int star_lvl)
{
    const Json::Value &script_json = CONFIG_INSTANCE->script(script_sort);
    JUDGE_RETURN(script_json != Json::Value::null, ERROR_CONFIG_NOT_EXIST);

    int award_score = 0;
    if(script_json["finish_condition"].isMember("award_score"))
    {
    	award_score = script_json["finish_condition"]["award_score"].asInt();
    }
    else
    {
    	if(script_json["finish_condition"].isMember("award_score_lvl") &&
    			script_json["finish_condition"]["award_score_lvl"].isArray() &&
    			star_lvl <= (int)script_json["finish_condition"]["award_score_lvl"].size())
    	{
    		award_score = script_json["finish_condition"]["award_score_lvl"][star_lvl - 1].asInt();
    	}
    }
    JUDGE_RETURN(award_score > 0, 0);

    return 0;
}

int MLScriptClean::process_script_other_award(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30400909 *, request, msg, -1);

    int script_sort = request->script_sort();
    int star_lvl = request->star_lvl();

    // 当日首次通关才发积分奖励
    if (request->day_pass_times() == 1)
    {
    	this->process_activity_score_award(script_sort, 0, star_lvl);
    }

    return 0;
}

int MLScriptClean::process_script_enter_check_pack(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400901 *, request, -1);

    MapLogicPlayer *player = this->player();
    CONDITION_PLAYER_NOTIFY_RETURN(request->need_pack_space() <= player->pack_left_capacity(),
    		RETURN_REQUEST_ENTER_SCRIPT, ERROR_PACKAGE_NO_CAPACITY);

    return player->monitor()->dispatch_to_scene(player, request->script_scene(), request);
}
