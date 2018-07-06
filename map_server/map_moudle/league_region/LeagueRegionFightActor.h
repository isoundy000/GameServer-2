/*
 * LeagueRegionFightPlayer.h
 *
 *  Created on: Mar 28, 2017
 *      Author: root
 */

#ifndef LEAGUEREGIONFIGHTPLAYER_H_
#define LEAGUEREGIONFIGHTPLAYER_H_

#include "MapPlayer.h"

class LeagueRegionFightActor : virtual public MapPlayer
{
public:
	LeagueRegionFightActor();
	virtual ~LeagueRegionFightActor();

	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
	virtual int die_process(const int64_t fighter_id);
	virtual int request_relive(Message* msg);
	virtual int modify_blood_by_fight(const double value, const int fight_tips = FIGHT_TIPS_BLANK,
			const int64_t attackor = 0, const int skill_id = 0);

	int get_last_region_info(Message *msg);
	int get_region_bet_info(Message *msg);
	int get_bet_support_apply(Message *msg);

	int player_request_join_war(Message *msg);
	int player_request_change_mode(Message *msg);
	int lrf_request_change_mode_done(Message* msg);
	int quit_lrf_change_mode();
	int fetch_lrf_war_rank();

	int lrf_die_process(Int64 fighter_id);
	int lrf_modify_blood(double value, Int64 attackor);

	int lrf_scene();
	int update_hickty_info(const Json::Value& weapon_conf);
};

#endif /* LEAGUEREGIONFIGHTPLAYER_H_ */
