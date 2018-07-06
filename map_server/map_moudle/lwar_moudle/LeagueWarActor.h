/*
 * LeagueWarActor.h
 *
 *  Created on: 2016年9月20日
 *      Author: lyw
 */

#ifndef LEAGUEWARACTOR_H_
#define LEAGUEWARACTOR_H_

#include "MapPlayer.h"

class LeagueWarActor : virtual public MapPlayer
{
public:
	LeagueWarActor();
	virtual ~LeagueWarActor();

	void reset();

	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
	virtual int die_process(const int64_t fighter_id);
	virtual int request_relive(Message* msg);

	int request_join_league_war(void);
	int handle_exit_lwar_scene(void);
	int league_war_scene();
	int lwar_enter_scene_type();

	int on_enter_lwar_scene(const int type);
	int on_exit_lwar_scene(const int type);
	int on_lwar_relive(Message* msg);

	int request_league_war_info(void);
	int request_change_space(Message* msg);
	int request_league_war_score();

	int validate_player_change();

private:
	int lwar_die_process(Int64 fighter_id);

	ThreeObjVec league_score_rank_;
};

#endif /* LEAGUEWARACTOR_H_ */
