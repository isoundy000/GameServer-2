/*
 * TinyLogicPlayer.h
 *
 *  Created on: Aug 12, 2014
 *      Author: peizhibi
 */

#ifndef TINYLOGICPLAYER_H_
#define TINYLOGICPLAYER_H_

#include "BaseLogicPlayer.h"

class TinyLogicPlayer : virtual public BaseLogicPlayer
{
public:
	TinyLogicPlayer();
	virtual ~TinyLogicPlayer();

	void reset_tiny();

	/*
	 * game notice
	 * */
	int check_and_game_notice_tips();
	int fetch_game_notice_info();
	int draw_game_notice_reward_begin();
	int draw_game_notice_reward_done(Message* msg);
};

#endif /* TINYLOGICPLAYER_H_ */
