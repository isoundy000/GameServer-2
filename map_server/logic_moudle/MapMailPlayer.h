/*
 * MapMailPlayer.h
 *
 *  Created on: 2013-7-12
 *      Author: xie
 */

#ifndef MAPMAILPLAYER_H_
#define MAPMAILPLAYER_H_

#include "MLPacker.h"

struct MailBox
{
    typedef std::map<Int64, MailInformation*> MailMap;
    MailMap __mail_map;

    MailBox(void);

    void reset(void);
};

class MapMailPlayer : virtual public MLPacker
{
public:
	MapMailPlayer(void);
	virtual ~MapMailPlayer(void);

    void reset(void);

	MailBox& mail_box(void);

	int obtain_mail_list(Message* msg);
	int obtain_mail_info(Message* msg);

	int pick_up_single_mail(Message* msg);
	int pick_up_all_mail(void);

	int delete_mail(Message* msg);
	int send_mail(int sid, Message* msg);
	int send_test_mail();

	int after_send_mail(Transaction *transaction);
	int after_load_mail_offline(Transaction *transaction);
	int after_load_player_to_send_mail(Transaction *transaction);

	int mail_time_up(const Time_Value &nowtime);
	int map_logic_mail_box_reset();

	static bool check_has_attached(MailInformation* mail_info);
	static int calc_mail_left_minute(MailInformation *mail_info);

	int sync_transfer_mail_box_info(int scene_id); // 角色切换场景时, 同步信息到其他地图进程
	int read_transfer_mail_box_info(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

	int fetch_new_mail_amount(void);

	static int handle_mail_no_reciever(Transaction *transaction);

private:
	int delete_expired_time_mail(void);
	int pick_up_single_mail(MailInformation* mail_info);

	int check_send_mail_interval();
	bool is_leagal_interval_for_send_mail();
	int modify_send_mail_interval();
	int player_mail_box_max_size();
	int calc_deposit_mail_num();

	int make_up_mail_serial_detail(int opra_type, MailInformation* mail_info);
protected:
	MailBox mail_box_;
	int send_mail_count_;
	Int64 send_mail_cool_tick_;
};

#endif /* MAPMAILPLAYER_H_ */
