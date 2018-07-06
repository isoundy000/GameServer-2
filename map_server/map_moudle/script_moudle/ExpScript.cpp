/*
 * ExpScript.cpp
 *
 * Created on: 2013-12-27 19:37
 *     Author: lyz
 */

#include "ExpScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ProtoClient016.pb.h"
#include "MapPlayerEx.h"

ExpScript::ExpScript(void) { /*NULL*/
}

ExpScript::~ExpScript(void) { /*NULL*/
}

void ExpScript::reset(void) {
	BaseScript::reset();
}

int ExpScript::fetch_first_reward(ThreeObjVec& reward_vec) {
	return 0;
}

int ExpScript::fetch_normal_reward() {
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	const Json::Value &finish_condition = script_json["finish_condition"];
	this->update_reward_map(finish_condition["exp_award_item"].asInt());

	return finish_condition["exp_award_item"].asInt();
}

int ExpScript::fetch_wave_reward(ThreeObjVec& reward_vec) {
//	const Json::Value &script_json = this->script_conf();
//	JUDGE_RETURN(script_json.empty() == false, -1);
//	JUDGE_RETURN(this->finish_wave() > this->begin_wave(), -1);

	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter) {
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(
				this->script_type());
		for (IntMap::iterator it = type_record->__reward_map.begin();
				it != type_record->__reward_map.end(); ++it) {
			int reward_id = it->first;
			int amount = it->second;
			for (int i = 0; i < amount; ++i) {
				reward_vec.push_back(ThreeObj(reward_id));
			}
		}
		type_record->__reward_map.clear();
	}
//	const Json::Value &finish_condition = script_json["finish_condition"];
//	int reward_id = finish_condition["exp_award_item"].asInt();
//	int finish_wave = this->finish_wave() - this->begin_wave();
//	for (int i = 0; i < finish_wave; ++i)
//	{
//		reward_vec.push_back(ThreeObj(reward_id));
//	}

	return 0;
}

int ExpScript::sync_restore_pass(MapPlayerScript* player) {
	JUDGE_RETURN(player != NULL, -1);

	int event_id = GameEnum::ES_ACT_EXP_FB;
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	ScriptPlayerDetail::TypeRecord *type_record = player->type_record(
			script_json["type"].asInt());
	int wave = script_json["scene"][0u]["exec"]["wave"].asInt();
	int pass_piece = type_record->__pass_wave;
	int pass_chapter = type_record->__pass_chapter;
	int transfer_date = pass_piece + pass_chapter * wave;

	player->sync_restore_info(event_id, transfer_date, 0);
	player->sync_branch_task_info(GameEnum::BRANCH_EXP_FB, transfer_date);

	//更新成就
	player->notify_ML_to_update_achievement(GameEnum::EXP_SCRIPT,
			transfer_date);

	return 0;
}

int ExpScript::script_pass_chapter() {
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;

	int get_exp = 0;
	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter) {
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(
				this->script_type());
		if (type_record->__pass_wave > type_record->__notify_wave
				|| type_record->__pass_chapter > type_record->__notify_chapter)
			wave_info.__pass_wave = true;

		ThreeObjVec reward_vec;
		for (IntMap::iterator it = type_record->__reward_map.begin();
				it != type_record->__reward_map.end(); ++it) {
			int reward_id = it->first;
			int amount = it->second;
			for (int i = 0; i < amount; ++i) {
				reward_vec.push_back(ThreeObj(reward_id));
			}
		}

		RewardInfo reward_info(false);
		GameCommon::make_up_reward_items(reward_info, reward_vec);
		for (ItemObjVec::iterator it = reward_info.item_vec_.begin();
				it != reward_info.item_vec_.end(); ++it) {
			get_exp += it->amount_;
		}
	}

	return get_exp;
}

int ExpScript::script_finish_flag() {
	return this->pass_wave() == true ? BaseScript::WIN : BaseScript::LOSE;
}

int ExpScript::process_script_stage_finish()	//阶段完成处理
{
	ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
	JUDGE_RETURN(wave_info.__finish_wave > this->piece(), -1);

//    wave_info.__pass_wave = true;
	this->reset_left_tick();
	this->script_detail_.__piece.__piece = wave_info.__finish_wave;

	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter) {
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->update_pass_exp(this);
		player->check_script_wave_task(this);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(
				this->script_type());
		if (type_record->__pass_wave > type_record->__notify_wave
				|| type_record->__pass_chapter > type_record->__notify_chapter)
			wave_info.__pass_wave = true;
	}

	return 0;
}

int ExpScript::process_script_player_finish(MapPlayerEx* player) {
	BaseScript::process_script_player_finish(player);

	const Json::Value &script_json = this->script_conf();
	int next_script = script_json["next_script"].asInt();
	JUDGE_RETURN(next_script > 0, -1);

	// 通知客户端开启传送门
	const Json::Value &transfer_json =
			script_json["finish_condition"]["transfer"];
	JUDGE_RETURN(transfer_json != Json::Value::null, -1);

	Proto80400927 active_req;
	active_req.set_scene_id(this->script_sort());
	active_req.set_target_scene_id(next_script);
	active_req.set_type(transfer_json["type"].asInt());
	active_req.set_posx(transfer_json["posX"].asInt());
	active_req.set_posy(transfer_json["posY"].asInt());
	return this->monitor()->dispatch_to_client_from_gate(player, &active_req);
}

void ExpScript::recycle_self_to_pool(void) {
	this->monitor()->script_factory()->push_script(this);
}

void ExpScript::update_reward_map(int reward_id) {
	for (BLongSet::iterator iter = this->script_detail_.__player_set.begin();
			iter != this->script_detail_.__player_set.end(); ++iter) {
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		ScriptPlayerDetail::TypeRecord *type_record = player->type_record(
				this->script_type());
		++(type_record->__reward_map[reward_id]);
	}
}

