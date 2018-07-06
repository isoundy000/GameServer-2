/*
 * GateStruct.h
 *
 * Created on: 2013-04-15 14:50
 *     Author: lyz
 */

#ifndef _GATESTRUCT_H_
#define _GATESTRUCT_H_

#include "PubStruct.h"

class GateMonitor;

struct GateRoleDetail : public BaseRoleInfo
{
    int __force;
    int __camp_id;
    int __pos_x;
    int __pos_y;
    int __pixel_x;
    int __pixel_y;
    int __scene_mode;
    int __space_id;

    string __net_type;	// 网络类型
    string __sys_version;	// 系统版本
    string __sys_model;	// 机型
    string __mac;		// MAC地址
    string __uc_sid;	// uc ＳＤＫ的sid
    string __imei;	//设备号

    void reset(void);
};

struct CombineServerInfo
{
	string server_flag_;

	string ip_;
	int port_;

	Int64 check_tick_;
	Int64 update_tick_;

	CombineServerInfo();
};

#endif //_GATESTRUCT_H_
