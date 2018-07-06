/*
 * MapTeamer.h
 *
 *  Created on: Jul 29, 2013
 *      Author: peizhibi
 */

#ifndef MAPTEAMER_H_
#define MAPTEAMER_H_

#include "GameFighter.h"

class MapTeamer : virtual public GameFighter
{
public:
	MapTeamer();
	virtual ~MapTeamer();

	void reset();
	MapTeamInfo& team_info(int type = 0);

	int teamer_state(int type = 0);
	int team_index(int type = 0);
	int team_empty(int type = 0);

	int read_team_info(Message* msg);
	int req_team_enter_script(Message* msg);
	int req_team_get_ready(Message* msg);

	int update_team_blood_info(int type = 0);

	virtual int team_notify_teamer_blood(int type = 0);
	int team_fetch_teamer_blood(int typ = 0);

	int sync_transfer_team(void);
	int read_transfer_team(Message *msg);

	int teamate_count();
	int replacement_count();

    int make_up_teamer_move(Block_Buffer *buff);
    int make_up_teamer_appear(Block_Buffer *buff);

    int notify_teamer_move(void);
    int notify_teamer_disappear(Scene *scene);

    int request_open_mini_map_pannel(void);
    int request_close_mini_map_pannel(void);

    int fetch_teamer_appear_info(void);
    int fetch_teamer_near_info(Message* msg);

protected:
	int notify_all_teamer_info(int type, int recogn, Message* msg,
			bool with_self = false);

protected:
	MapTeamInfo team_info_[GameEnum::TOTAL_TEAM];
};

#endif /* MAPTEAMER_H_ */
