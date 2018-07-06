/*
 * MLHiddenTreasure.h
 *
 *  Created on: 2016年12月6日
 *      Author: lyw
 */

#ifndef MLHIDDENTREASURE_H_
#define MLHIDDENTREASURE_H_

#include "MLPacker.h"

enum OPEN_STATUS
{
	NOT_OPEN = 0,
	HAS_OPEN = 1,
	END_OPEN = 2
};

class MLHiddenTreasure : virtual public MLPacker
{
public:
	MLHiddenTreasure();
	virtual ~MLHiddenTreasure();
	void reset(void);

	void active_hi_treasure();
	void hi_treasure_next_day();

	int fetch_hi_treasure_info();
	int fetch_hi_treasure_reward();
	int buy_hi_treasure_item(Message *msg);

	int sync_transfer_hi_treasure(int scene_id);	// 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_hi_treasure(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

	HiddenTreasureDetail& hi_treasure_detail();

	void test_hi_treasure_reset();
	void test_set_hi_treasure_day(int day);

protected:
	void hi_treasure_send_mail(Int64 refresh_tick);	//未领取奖励发送邮件

private:
	HiddenTreasureDetail hi_treasure_;
};

#endif /* MLHIDDENTREASURE_H_ */
