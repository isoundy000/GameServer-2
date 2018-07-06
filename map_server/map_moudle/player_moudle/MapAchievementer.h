/*
 * MapAchievementer.h
 *
 *  Created on: Jun 23, 2014
 *      Author: louis
 */

#ifndef MAPACHIEVEMENTER_H_
#define MAPACHIEVEMENTER_H_

#include "GameFighter.h"

class MapAchievementer : virtual public GameFighter
{
public:
	MapAchievementer();
	virtual ~MapAchievementer();

	int notify_ML_to_update_achievement(int ach_index, int cur_value, Int64 special_value = 0, int ach_type = 0);
};

#endif /* MAPACHIEVEMENTER_H_ */
