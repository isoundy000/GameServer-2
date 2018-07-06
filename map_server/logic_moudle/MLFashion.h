/*
 * MLFashion.h
 *
 *  Created on: 2017年1月20日
 *      Author: lyw
 */

#ifndef MLFASHION_H_
#define MLFASHION_H_

#include "MLPacker.h"

class ProtoFashionDetail;

class MLFashion : virtual public MLPacker
{
public:
	MLFashion();
	virtual ~MLFashion();
	void reset(void);

	int request_fetch_fashion_info();
	int request_fashion_add_color(Message* msg);
	int request_change_fashion_id(Message* msg);	//无用
	int request_change_fashion_color(Message* msg);	//无用
	int request_change_fashion_style(Message* msg);

	int add_fashion(int fashion_id);
	int send_change_mail(const Json::Value &conf);
	int ckeck_can_add_fashion(const Json::Value &conf);

	// transfer
	int sync_transfer_fashion(int scene_id);
	int read_transfer_fashion(Message* msg);

	void check_handle_fashion_timeout();

	void fashion_login_operate();		//登录检测操作
	void fashion_handle_player_levelup();
	void fashion_handle_player_task(int task_id);

	void login_check_send_fashion();	//登录检测老玩家补领时装

	RoleFashion& fashion_detail();

protected:
	void check_level_open_fashion();
	void check_task_open_fashion(int task_id);
	void active_up_fashion_level();
	int cal_fashion_fight_attr(int fashion_id);	//计算单件时装属性
	int cal_total_fashion_attr();				//计算时装总属性
	int refresh_fashion_attr_add(const int enter_type = 0); 	//刷新时装总属性
	int send_fashion_by_cond(const Json::Value& conf);

	void serialize_fashion_info(ProtoFashionDetail* info, RoleFashion::FashionInfo *fashion_info);

	void notify_fashion_style(int notify = true);//0:不通知，1:通知外形

private:
	RoleFashion fashion_detail_;
};

#endif /* MLFASHION_H_ */
