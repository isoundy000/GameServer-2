/*
 * LegendTopScript.cpp
 *
 *  Created on: 2016年8月16日
 *      Author: lyw
 */

#include "LegendTopScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "MapPlayerEx.h"

LegendTopScript::LegendTopScript() {
	// TODO Auto-generated constructor stub

}

LegendTopScript::~LegendTopScript() {
	// TODO Auto-generated destructor stub
}

void LegendTopScript::reset(void) {
	BaseScript::reset();
}

int LegendTopScript::fetch_task_wave() {
	return this->floor_id();
}

int LegendTopScript::fetch_reward_index() {
	return this->floor_id();
}

int LegendTopScript::fetch_first_reward(ThreeObjVec& reward_vec) {
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int floor_id = this->floor_id();
	const Json::Value &first_floor_award_json =
			script_json["finish_condition"]["first_floor_award"];

	for (uint i = 0; i < first_floor_award_json.size(); ++i) {
		JUDGE_CONTINUE(first_floor_award_json[i][0u].asInt() == floor_id);

		int reward_id = first_floor_award_json[i][1u].asInt();
		reward_vec.push_back(ThreeObj(reward_id, true));

		break;
	}

	//特殊奖励
	const Json::Value &special_award_json =
			script_json["finish_condition"]["special_item"];
	for (uint i = 0; i < special_award_json.size(); ++i) {
		JUDGE_CONTINUE(special_award_json[i][0u].asInt() == floor_id);

		int reward_id = special_award_json[i][1u].asInt();
		reward_vec.push_back(reward_id);

		break;
	}

	return 0;
}

int LegendTopScript::fetch_normal_reward() {
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int floor_id = this->floor_id();
	const Json::Value &floor_award_json =
			script_json["finish_condition"]["pass_award_item"];

	for (uint i = 0; i < floor_award_json.size(); ++i) {
		JUDGE_CONTINUE(floor_award_json[i][0u].asInt() == floor_id);
		return floor_award_json[i][1u].asInt();
	}

	return 0;
}

int LegendTopScript::fetch_monster_coord(MoverCoord &coord) {
	return 0;
}

int LegendTopScript::sync_restore_pass(MapPlayerScript* player) {
	JUDGE_RETURN(player != NULL, -1);

	int event_id = GameEnum::ES_ACT_LEGEND_TOP;
	int floor_id = this->floor_id();
	player->sync_restore_info(event_id, floor_id, 0);
	player->sync_branch_task_info(GameEnum::BRANCH_LEGEND_TOP, floor_id);
	//更新成就
	player->notify_ML_to_update_achievement(GameEnum::LEGEND_TOP_SCRIPT,
			floor_id);

	return 0;
}

void LegendTopScript::recycle_self_to_pool(void) {
	this->monitor()->script_factory()->push_script(this);
}

