/*
 * MLOnceRewards.h
 *
 *  Created on: Aug 18, 2014
 *      Author: jinxing
 */

#ifndef MLONCEREWARDS_H_
#define MLONCEREWARDS_H_

#include "MLPacker.h"

struct OnceRewardRecords
{
	int __update_res_flag;	// 是否已领取资源包下载完成奖励
	void reset(void);
};

/*
 * 一次性礼包
 * */
class MLOnceRewards: virtual public MLPacker
{
public:
	MLOnceRewards();
	virtual ~MLOnceRewards();
	void reset(void);

	OnceRewardRecords &once_reward_records(void);

	int process_query_update_res_rewards(Message *msg);
	int process_get_update_res_rewards(Message *msg);

	int sync_transfer_once_rewards(int scene_id);
	int read_transfer_once_rewards(Message *msg);

private:
	OnceRewardRecords records_;
};

#endif /* MLONCEREWARDS_H_ */
