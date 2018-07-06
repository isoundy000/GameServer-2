/*
 * RamaScript.cpp
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#include "RamaScript.h"
#include "MapMonitor.h"
#include "MapPlayerEx.h"
#include "ScriptFactory.h"
#include "ScriptSystem.h"

RamaScript::RamaScript() {
	// TODO Auto-generated constructor stub

}

RamaScript::~RamaScript() {
	// TODO Auto-generated destructor stub
}

void RamaScript::reset(void)
{
	BaseScript::reset();
}

int RamaScript::fetch_reward_index()
{
	return this->finish_wave();
}

int RamaScript::fetch_first_reward(ThreeObjVec& reward_vec)
{
	return 0;
}

int RamaScript::fetch_normal_reward()
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	int floor_id = this->finish_wave();
	const Json::Value &floor_award_json = script_json["finish_condition"]["pass_award_item"];

	for (uint i = 0; i < floor_award_json.size(); ++i)
	{
		JUDGE_CONTINUE(floor_award_json[i][0u].asInt() == floor_id);
		return floor_award_json[i][1u].asInt();
	}

	return 0;
}

int RamaScript::fetch_wave_reward(ThreeObjVec& reward_vec)
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);
	JUDGE_RETURN(this->finish_wave() > this->begin_wave(), -1);

	int mult = SCRIPT_SYSTEM->fetch_script_mult(this->script_type());

	const Json::Value &floor_award_json = script_json["finish_condition"]["pass_award_item"];
	for (uint i = 0; i < floor_award_json.size(); ++i)
	{
		JUDGE_CONTINUE(floor_award_json[i][0u].asInt() > this->begin_wave() &&
				floor_award_json[i][0u].asInt() <= this->finish_wave());

		int reward_id = floor_award_json[i][1u].asInt();
		for (int i = 0; i < mult; ++i)
			reward_vec.push_back(ThreeObj(reward_id));
	}

	return 0;
}

int RamaScript::script_finish_flag()
{
	return this->pass_wave() == true ? BaseScript::WIN : BaseScript::LOSE;
}

int RamaScript::process_script_stage_finish()
{
    ScriptDetail::Wave &wave_info = this->script_detail_.__wave;
    JUDGE_RETURN(wave_info.__finish_wave > this->piece(), -1);

	wave_info.__pass_wave = true;

	this->reset_left_tick();
    this->script_detail_.__piece.__piece = wave_info.__finish_wave;

	for (BLongSet::iterator iter = this->player_set().begin();
			iter != this->player_set().end(); ++iter)
	{
		MapPlayerEx *player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);

		player->update_pass_rama(this);
		player->check_script_wave_task(this);
	}

	return 0;
}

void RamaScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}

