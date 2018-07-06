/*
 * LeagueFbScript.cpp
 *
 *  Created on: 2017年2月28日
 *      Author: lyw
 */

#include "LeagueFbScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ProtoClient016.pb.h"
#include "MapPlayerEx.h"

LeagueFbScript::LeagueFbScript() {
	// TODO Auto-generated constructor stub

}

LeagueFbScript::~LeagueFbScript() {
	// TODO Auto-generated destructor stub
}

void LeagueFbScript::reset(void)
{
	this->is_add_ = 0;
	BaseScript::reset();
}

int LeagueFbScript::fetch_first_reward(ThreeObjVec& reward_vec)
{
	return 0;
}

int LeagueFbScript::check_add_player_buff()
{
	JUDGE_RETURN(this->script_detail_.__cheer_num > 0 ||
			this->script_detail_.__encourage_num > 0, 0);
	JUDGE_RETURN(this->script_detail_.__player_set.size() > 0, 0);
	JUDGE_RETURN(this->is_add_ == false, 0);

	int cheer_num = this->script_detail_.__cheer_num;
	int encourage_num = this->script_detail_.__encourage_num;

	IntMap buff_map;
	const Json::Value &cheer_attr_buff = CONFIG_INSTANCE->lfb_cheer_attr(cheer_num);
	const Json::Value &encourage_attr_buff = CONFIG_INSTANCE->lfb_cheer_attr(encourage_num);
	this->add_buff_map(cheer_attr_buff, buff_map);
	this->add_buff_map(encourage_attr_buff, buff_map);
	JUDGE_RETURN(buff_map.size() > 0, 0);

	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		for (IntMap::iterator it = buff_map.begin(); it != buff_map.end(); ++it)
		{
			BasicStatus* c_status = NULL;
			if (player->find_status(it->first, c_status) != 0)
			{
				player->insert_defender_status(player, it->first, 0, 86400, 0, 0, it->second);
			}
		}
	}
	this->is_add_ = true;

	return 0;
}

int LeagueFbScript::fetch_normal_reward()
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int wave = this->finish_wave();
	const Json::Value &floor_award_json = script_json["finish_condition"]["league_award_item"];

	for (uint i = 0; i < floor_award_json.size(); ++i)
	{
		JUDGE_CONTINUE(floor_award_json[i][0u].asInt() == wave);

		int reward_id = floor_award_json[i][1u].asInt();
		this->update_reward_map(reward_id);
		return reward_id;
	}

	return 0;
}

int LeagueFbScript::fetch_wave_reward(ThreeObjVec& reward_vec)
{
	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(this->script_type());
		for (IntMap::iterator it = type_record->__reward_map.begin();
				it != type_record->__reward_map.end(); ++it)
		{
			int reward_id = it->first;
			int amount = it->second;
			for (int i = 0; i < amount; ++i)
			{
				reward_vec.push_back(ThreeObj(reward_id));
			}
		}
		type_record->__reward_map.clear();
	}

	return 0;
}

int LeagueFbScript::script_finish_flag()
{
	return this->pass_wave() == true ? BaseScript::WIN : BaseScript::LOSE;
}

int LeagueFbScript::process_script_stage_finish()
{
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	JUDGE_RETURN(wave_info.__finish_wave > this->piece(), -1);

	this->reset_left_tick();
	this->script_detail_.__piece.__piece = wave_info.__finish_wave;

	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->update_pass_lfb(this);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(this->script_type());
		if (type_record->__start_wave > type_record->__notify_wave ||
				type_record->__start_chapter > type_record->__notify_chapter)
			wave_info.__pass_wave = true;
	}

	return 0;
}

int LeagueFbScript::process_script_player_finish(MapPlayerEx* player)
{
	BaseScript::process_script_player_finish(player);

	const Json::Value &script_json = this->script_conf();
	int next_script = script_json["next_script"].asInt();
	JUDGE_RETURN(next_script > 0, -1);

	const Json::Value &next_script_json = CONFIG_INSTANCE->script(next_script);
	int level = next_script_json["prev_condition"]["level"].asInt();
	JUDGE_RETURN(player->level() >= level, -1);

	// 通知客户端开启传送门
	const Json::Value &transfer_json = script_json["finish_condition"]["transfer"];
	JUDGE_RETURN(transfer_json != Json::Value::null, -1);

	Proto80400927 active_req;
	active_req.set_scene_id(this->script_sort());
	active_req.set_target_scene_id(next_script);
	active_req.set_type(transfer_json["type"].asInt());
	active_req.set_posx(transfer_json["posX"].asInt());
	active_req.set_posy(transfer_json["posY"].asInt());
	return this->monitor()->dispatch_to_client_from_gate(player, &active_req);
}

void LeagueFbScript::update_reward_map(int reward_id)
{
	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(this->script_type());
		++(type_record->__reward_map[reward_id]);
	}
}

void LeagueFbScript::add_buff_map(const Json::Value &json, IntMap &buff_map)
{
	JUDGE_RETURN(json != Json::Value::null, ;);

	const Json::Value &buff_list = json["buff_list"];
	const Json::Value &attr_json = json["attr"];

	for (uint i = 0; i < buff_list.size(); ++i)
	{
		int buff_id = buff_list[i].asInt();
		JUDGE_CONTINUE(buff_id > 0);

		int percent = attr_json[i].asInt();
		buff_map[buff_id] += percent;
	}
}

void LeagueFbScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
