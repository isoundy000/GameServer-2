/*
 * MLTransfer.h
 *
 *  Created on: 2017年3月29日
 *      Author: lyw
 */

#ifndef MLTRANSFER_H_
#define MLTRANSFER_H_

#include "MLPacker.h"

//变身系统
class MLTransfer : virtual public MLPacker
{
public:
	MLTransfer();
	virtual ~MLTransfer();
	void reset(void);

	//英魂
	int request_spirit_info();
	int request_make_spirit(Message* msg);
	int request_up_stage();

	//变身
	int request_transfer_info();
	int request_change_transfer_id(Message* msg);
	int change_transfer_id(int transfer_id);
	int request_use_transfer();
	int fetch_open_reward();
	int request_buy_transfer(Message* msg);

	int add_transfer(int id, int dur_time); //变身id, 持续时间
	void show_broad(int transfer_id, int show_id);

	void check_handle_transfer_timeout();

	// transfer
	int sync_transfer_trans(int scene_id);
	int read_transfer_trans(Message* msg);

	void transfer_login_operate();	//登录检测操作
	void transfer_login_obtain();
	void transfer_handle_player_levelup();
	void transfer_handle_player_task(int task_id);
	void check_spirit_day_reset();

	TransferDetail &transfer_detail();

	void test_reset_transfer_cd();

protected:
	void check_level_open_transfer();
	void check_task_open_transfer(int task_id);
	void init_transfer();

	void active_up_spirit_level();
	void create_transfer_in_time(int notify = true, int send_skill = true);

	int cal_spirit_advence();	//计算变身cd减少,时间增加
	int cal_one_transfer_attr(TransferDetail::TransferInfo &transfer_info);
	int cal_total_transfer_attr();	//计算总属性
	int refresh_transfer_attr_add(const int enter_type = 0); 	//刷新总属性
	int fetch_make_spirit_mult(const Json::Value& rate_json);

	void handle_transfer_time_out();
	void handle_transfer_cd_time_out();

	void send_transfer_to_map(int type, int notify = true);//0:不通知，1:通知外形

private:
	TransferDetail transfer_detail_;

};

#endif /* MLTRANSFER_H_ */
