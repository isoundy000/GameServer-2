/*
 * SealBossScript.cpp
 *
 * Created on: 2015-07-18 19:26
 *     Author: lyz
 */

#include "SealBossScript.h"
#include "MapMonitor.h"
#include "ScriptFactory.h"


SealBossScript::~SealBossScript(void)
{ /*NULL*/ }

void SealBossScript::reset(void)
{
	BaseScript::reset();
}

void SealBossScript::recycle_self_to_pool(void)
{
    this->monitor()->script_factory()->push_script(this);
}

