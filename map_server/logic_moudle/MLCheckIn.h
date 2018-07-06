/*
 * MLCheckIn.h
 *
 *  Created on: 2014年1月22日
 *      Author: xing
 */

#ifndef MLCHECKIN_H_
#define MLCHECKIN_H_
#include "MapStruct.h"

#include "MLPacker.h"

class MLCheckIn: virtual public MLPacker {
public:
	enum CYCLE_MODE {
		// 分别表示 单配置循环, 顺序循环, 随机循环
		CYCLE_MODE_SINGLE = 1,
		CYCLE_MODE_LIST = 2,
		CYCLE_MODE_RANDOM = 3
	};
	MLCheckIn();
	virtual ~MLCheckIn();
	void reset(void);

	CheckInDetail &check_in_detail();

	bool need_new_check_in_cycle(void); // 是否需要生成新的签到周期
	int new_check_in_cycle(void); // 生成新的签到周期

	bool has_check_in_award(void); // 是否可领取奖励
	bool has_total_check_in_award(void); // 是否可领取奖励
    bool has_check_in_award_again(void); //是否可再领

	int has_charge_money(void);
	int check_need_popup(void);
	int total_check_need_popup();

	int fetch_check_in_info(Message *msg);
	int request_check_in(Message *msg); // 领取签到奖励
	int request_check_in_again(Message *msg); // 领取签到奖励
	int request_total_check_in(Message *msg); //累计签到
//	int send_check_in_award(int award_id, int award_num, int is_bind, int id, int index); // 发放奖励
	int check_in_pa_event();
	int set_notify_msg_check_in_info(Message *msg); // 设置 msg 中 check_in 的状态
											// MapLogicPlayer::notify_player_welfare_status()

	int sync_transfer_check_in(int scene_id); // 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_check_in(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

	void test_check_in(void);
private:
    CheckInDetail check_in_detail_;
};

#endif /* MLCHECKIN_H_ */
