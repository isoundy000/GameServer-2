/*
 * UpclassScript.cpp
 *
 * Created on: 2014-02-13 15:39
 *     Author: lyz
 */

#include "UpclassScript.h"

#include "MapMonitor.h"
#include "ProtoDefine.h"

#include "ScriptFactory.h"

UpclassScript::UpclassScript(void)
{ /*NULL*/ }

UpclassScript::~UpclassScript(void)
{ /*NULL*/ }

void UpclassScript::reset(void)
{
    BaseScript::reset();
}

int UpclassScript::generate_new_wave(const int scene_id, const int wave_id)
{
	int ret = BaseScript::generate_new_wave(scene_id, wave_id);
	if (wave_id != 2)
		return ret;

	Proto80400913 respond;
	respond.set_floor(this->current_script_floor());
	this->notify_all_player(&respond);
	return ret;
}

void UpclassScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

