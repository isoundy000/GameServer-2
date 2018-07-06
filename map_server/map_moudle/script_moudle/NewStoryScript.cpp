/*
 * NewStoryScript.cpp
 *
 *  Created on: 2016年11月1日
 *      Author: lyw
 */

#include "NewStoryScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"

NewStoryScript::NewStoryScript() {
	// TODO Auto-generated constructor stub

}

NewStoryScript::~NewStoryScript() {
	// TODO Auto-generated destructor stub
}

void NewStoryScript::reset(void)
{
	BaseScript::reset();
}

void NewStoryScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}

