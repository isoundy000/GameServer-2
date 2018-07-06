/*
 * SendActReward.h
 *
 *  Created on: 2016年11月21日
 *      Author: lzy
 */

#ifndef SENDACTREWARD_H_
#define SENDACTREWARD_H_

#include "LogicStruct.h"

typedef std::map<Int64, IntMap> ActRewardMap;

class SendActReward
{
public :
	class RefreshTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

	};

public:
	SendActReward();
	virtual ~SendActReward();

	void init(void);
	int reset(void);

	int send_player_act_reward(Message* msg);

private:
	RefreshTimer refresh_timer_;
	ActRewardMap reward_map_;
};

typedef Singleton<SendActReward> SendActRewardSingleton;
#define SEND_ACTREWARD  SendActRewardSingleton::instance()

#endif /* SENDACTREWARD_H_ */
