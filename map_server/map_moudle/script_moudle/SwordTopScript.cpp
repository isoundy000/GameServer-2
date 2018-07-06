/*
 * SwordTopScript.cpp
 *
 *  Created on: 2017年3月17日
 *      Author: lyw
 */

#include "SwordTopScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "MapPlayerEx.h"

SwordTopScript::SwordTopScript() {
	// TODO Auto-generated constructor stub

}

SwordTopScript::~SwordTopScript() {
	// TODO Auto-generated destructor stub
}

void SwordTopScript::reset(void)
{
	BaseScript::reset();
}

int SwordTopScript::fetch_task_wave()
{
	return this->floor_id();
}

int SwordTopScript::fetch_reward_index()
{
	return this->floor_id();
}

int SwordTopScript::fetch_first_reward(ThreeObjVec& reward_vec)
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int floor_id = this->floor_id();
	const Json::Value &first_floor_award_json = script_json["finish_condition"]["first_floor_award"];

	for (uint i = 0; i < first_floor_award_json.size(); ++i)
	{
		JUDGE_CONTINUE(first_floor_award_json[i][0u].asInt() == floor_id);

		int reward_id = first_floor_award_json[i][1u].asInt();
		reward_vec.push_back(ThreeObj(reward_id, true));

		break;
	}

	return 0;
}

int SwordTopScript::fetch_normal_reward()
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int floor_id = this->floor_id();
	const Json::Value &floor_award_json = script_json["finish_condition"]["pass_award_item"];

	for (uint i = 0; i < floor_award_json.size(); ++i)
	{
		JUDGE_CONTINUE(floor_award_json[i][0u].asInt() == floor_id);
		return floor_award_json[i][1u].asInt();
	}

	return 0;
}

int SwordTopScript::fetch_monster_coord(MoverCoord &coord)
{
	return 0;
}

void SwordTopScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
