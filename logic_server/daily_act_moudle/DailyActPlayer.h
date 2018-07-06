/*
 * DailyActPlayer.h
 *
 *  Created on: 2017年3月21日
 *      Author: lyw
 */

#ifndef DAILYACTPLAYER_H_
#define DAILYACTPLAYER_H_

#include "BaseLogicPlayer.h"

class DailyActPlayer : virtual public BaseLogicPlayer
{
public:
	DailyActPlayer();
	virtual ~DailyActPlayer();

	void reset();

	int fetch_daily_act_list();

	int request_total_double_info();	//全民双倍
	int is_xuanji_double();
};

#endif /* DAILYACTPLAYER_H_ */
