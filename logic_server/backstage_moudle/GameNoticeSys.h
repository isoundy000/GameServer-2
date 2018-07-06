/*
 * GameNoticeSys.h
 *
 *  Created on: Aug 12, 2014
 *      Author: peizhibi
 */

#ifndef GAMENOTICESYS_H_
#define GAMENOTICESYS_H_

#include "LogicStruct.h"

class GameNoticeSys
{
public:
	class RewardTimer : public GameTimer
	{
    	virtual int type(void);
        virtual int handle_timeout(const Time_Value &nowtime);
	};

public:
	GameNoticeSys();
	~GameNoticeSys();

	void init();
	void fina();

	int request_load_notice();
	int after_load_notice(DBShopMode* shop_mode);

	int is_need_view(Int64 last_view);
	GameNoticeDetial& notice_detail();

	int handle_timeout();

private:
	RewardTimer reward_timer_;
	GameNoticeDetial notice_detail_;
};

typedef Singleton<GameNoticeSys> 	GameNoticeSysSingle;
#define GAME_NOTICE_SYS           	GameNoticeSysSingle::instance()

#endif /* GAMENOTICESYS_H_ */
