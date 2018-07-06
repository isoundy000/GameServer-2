/*
 * CouplesScript.cpp
 *
 * Created on: 2015-06-11 14:59
 *     Author: lyz
 */

#include "CouplesScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "ScriptAI.h"
#include "ScriptScene.h"

CouplesScript::~CouplesScript(void)
{ /*NULL*/ }

void CouplesScript::reset(void)
{
    BaseScript::reset();
}

int CouplesScript::fetch_normal_reward()
{
	const Json::Value &script_json = this->script_conf();
	JUDGE_RETURN(script_json.empty() == false, -1);

	const Json::Value &award_item_json = script_json["finish_condition"]["award_item"];
	LongMap couple_sel = this->script_detail().couple_sel_;
	JUDGE_RETURN(couple_sel.size() >= 2, -1);

	LongMap::iterator iter = couple_sel.begin();
	Int64 select_id = iter->second;
	for (; iter != couple_sel.end(); ++iter)
	{
		JUDGE_CONTINUE(select_id != iter->second);

		return award_item_json[1u].asInt();
	}

	return award_item_json[0u].asInt();
}

void CouplesScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

