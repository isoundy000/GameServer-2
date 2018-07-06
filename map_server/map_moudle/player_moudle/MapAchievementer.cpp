/*
 * MapAchievementer.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: louis
 */

#include "MapAchievementer.h"
#include "MapPlayer.h"
#include "ProtoDefine.h"

MapAchievementer::MapAchievementer() {
	// TODO Auto-generated constructor stub

}

MapAchievementer::~MapAchievementer() {
	// TODO Auto-generated destructor stub
}

int MapAchievementer::notify_ML_to_update_achievement(int ach_index, int cur_value, Int64 special_value, int ach_type)
{
	Proto31401901 request;
	request.set_ach_index(ach_index);
	request.set_ach_type(ach_type);
	request.set_cur_value(cur_value);
	request.set_special_value(special_value);
	return this->send_to_logic_thread(request);
}

