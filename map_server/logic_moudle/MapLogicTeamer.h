/*
 * MapLogicTeamer.h
 *
 *  Created on: Jul 29, 2013
 *      Author: peizhibi
 */

#ifndef MAPLOGICTEAMER_H_
#define MAPLOGICTEAMER_H_

#include "MLPacker.h"

class MapLogicTeamer : virtual public MLPacker
{
public:
	MapLogicTeamer();
	virtual ~MapLogicTeamer();

	void reset();
	MapTeamInfo& team_info(int type = 0);

	int teamer_state(int type = 0);
	int team_index(int type = 0);

	int team_learn_circle(Message* msg);
	int read_logic_team_info(Message* msg);

	int sync_transfer_team_info(int scene_id);
	int read_transfer_team_info(Message* msg);

private:
	MapTeamInfo team_info_[GameEnum::TOTAL_TEAM];
};

#endif /* MAPTEAMER_H_ */
