/*
 * VipScript.cpp
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#include "VipScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"

VipScript::VipScript() {
	// TODO Auto-generated constructor stub

}

VipScript::~VipScript() {
	// TODO Auto-generated destructor stub
}

void VipScript::reset(void)
{
	BaseScript::reset();
}

int VipScript::fetch_monster_coord(MoverCoord &coord)
{
	return 0;
}

void VipScript::recycle_self_to_pool(void)
{
	this->monitor()->script_factory()->push_script(this);
}

