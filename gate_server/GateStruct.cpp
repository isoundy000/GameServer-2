/*
 * GateStruct.cpp
 *
 * Created on: 2013-04-15 14:50
 *     Author: lyz
 */

#include "GateStruct.h"
#include "GateMonitor.h"

void GateRoleDetail::reset(void)
{
	BaseRoleInfo::reset();

    this->__force = 0;
    this->__camp_id = 0;
    this->__pos_x = 0;
    this->__pos_y = 0;
    this->__pixel_x = 0;
    this->__pixel_y = 0;
    this->__scene_mode = 0;
    this->__space_id = 0;

    this->__net_type.clear();	// 网络类型
    this->__sys_version.clear();	// 系统版本
    this->__sys_model.clear();	// 机型
    this->__mac.clear();		// MAC地址
    this->__uc_sid.clear();
    this->__imei.clear();
}

CombineServerInfo::CombineServerInfo()
{
	this->port_ = 0;
	this->check_tick_  = 0;
	this->update_tick_ = 0;
}



