/*
 * AdvanceScript.cpp
 *
 *  Created on: 2016年8月18日
 *      Author: lyw
 */

#include "AdvanceScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"
#include "MapPlayerScript.h"

AdvanceScript::AdvanceScript() {
	// TODO Auto-generated constructor stub

}

AdvanceScript::~AdvanceScript() {
	// TODO Auto-generated destructor stub
}

void AdvanceScript::reset(void)
{
	BaseScript::reset();
}

int AdvanceScript::sync_restore_pass(MapPlayerScript* player)
{
	const Json::Value &script_json = this->script_conf();
	int script_type = script_json["type"].asInt();
	int event_id, value = 1;
	if (script_type == GameEnum::SCRIPT_T_STORY)
	{
		int ext_type = script_json["priority"].asInt();
		switch (ext_type)
		{
		case 1:
			event_id = GameEnum::BRANCH_STORY_FB_1;
			break;
		case 2:
			event_id = GameEnum::BRANCH_STORY_FB_2;
			break;
		case 3:
			event_id = GameEnum::BRANCH_STORY_FB_3;
			break;
		case 4:
			event_id = GameEnum::BRANCH_STORY_FB_4;
			break;
		case 5:
			event_id = GameEnum::BRANCH_STORY_FB_5;
			break;
		case 6:
			event_id = GameEnum::BRANCH_STORY_FB_6;
			break;
		case 7:
			event_id = GameEnum::BRANCH_STORY_FB_7;
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
			event_id = GameEnum::BRANCH_MOUNT;
			break;
		case 2:
			event_id = GameEnum::BRANCH_MAGIC_EQUIP;
			break;
		case 3:
			event_id = GameEnum::BRANCH_LING_BEAST;
			break;
		case 4:
			event_id = GameEnum::BRANCH_XIAN_WING;
			break;
		case 5:
			event_id = GameEnum::BRANCH_GOD_SOLIDER;
			break;
		case 6:
			event_id = GameEnum::BRANCH_BEAST_EQUIP;
			break;
		case 7:
			event_id = GameEnum::BRANCH_BEAST_MOUNT;
			break;

		case 8 :
			event_id = GameEnum::BRANCH_BEAST_WING;
			break;
		case 9 :
			event_id = GameEnum::BRANCH_BEAST_MAO;
			break;
		case 10 :
			event_id = GameEnum::BRANCH_TIAN_GANG;
			break;

		}

		value = 1;
	}
	/*
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
	*/
	else if (script_type == GameEnum::SCRIPT_T_TRVL)
	{
		value = 1;
	}

	player->sync_branch_task_info(event_id, value);
	return 0;
}

void AdvanceScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
