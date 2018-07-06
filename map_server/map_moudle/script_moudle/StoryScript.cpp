/*
 * StoryScript.cpp
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#include "StoryScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"

StoryScript::StoryScript() {
	// TODO Auto-generated constructor stub

}

StoryScript::~StoryScript() {
	// TODO Auto-generated destructor stub
}

void StoryScript::reset(void)
{
	BaseScript::reset();
}

void StoryScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
