/*
 * MLOfflineAwards.h
 *
 *  Created on: 2016年8月18日
 *      Author: lzy
 */

#ifndef MLOFFLINEAWARDS_H_
#define MLOFFLINEAWARDS_H_


#include "MLPacker.h"


enum
{
    OFFLINE_NORMAL_NUM = 0,
    OFFLINE_DOUBLE_VIP_NUM = 101,
    OFFLINE_TRIPLE_VIP_NUM = 103,
    OFFLINE_END
};

class MLOfflineRewards : virtual public MLPacker
{
public:
	MLOfflineRewards();
	virtual ~MLOfflineRewards();
	void reset();

	int login_offline_rewards();
	int logout_offline_rewards();
	int offline_rewards_info();
	int fetch_offline_rewards(Message* msg);
	int notify_offline_rewards(int level);

	bool validate_offline_rewards_level_limit(void);

	OfflineRewardsDetail& offline_rewards_detail();

	int sync_transfer_offline_rewards(int scene_id);
	int read_transfer_offline_rewards(Message* msg);

private:
	OfflineRewardsDetail __offline_reward;
};

#endif /* MLOFFLINEAWARDS_H_ */
