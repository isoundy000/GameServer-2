/*
 * NewAdvanceScript.cpp
 *
 *  Created on: 2016年11月1日
 *      Author: lyw
 */

#include "NewAdvanceScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"

NewAdvanceScript::NewAdvanceScript() {
	// TODO Auto-generated constructor stub

}

NewAdvanceScript::~NewAdvanceScript() {
	// TODO Auto-generated destructor stub
}

void NewAdvanceScript::reset(void)
{
	BaseScript::reset();
}

void NewAdvanceScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}
