/*
 * MapTinyPlayer.h
 *
 *  Created on: Apr 1, 2014
 *      Author: peizhibi
 */

#ifndef MAPTINYPLAYER_H_
#define MAPTINYPLAYER_H_

#include "GameFighter.h"

class MapTinyPlayer : virtual public GameFighter
{
public:
	MapTinyPlayer();
	virtual ~MapTinyPlayer();

	void reset_tiny();
	void reset_tiny_everyday();
	void tiny_fb_recover_blood();

	int fetch_buy_blood_value();
	int notify_blood_contain_info();
	int check_recover_blood_scene();
	int check_and_add_cur_blood(int add_value);
	int check_and_use_health(ProtoItem* proto_item, int add_value);

	int check_timeout_cont_blood();
	int timeout_use_cont_blood();

	int set_auto_cont_blood(Message* msg);
	int set_cont_blood_notips(Message* msg);


	//jump value
	void start_jump_value_time();
	void check_jump_value_timeout();

	void insert_attack_me(Int64 fighter_id);
	void check_attack_me();
	void clear_attack_me();

	int check_and_add_event_cut(Int64 attackor, int test = false);
	int update_event_cut();

	BloodContainer& blood_container();

	// transfer
	int sync_transfer_tiny();
	int read_transfer_tiny(Message* msg);

private:
	int condition_attack_me(Int64 fighter_id);
	void notify_attack(Int64 fighter_id);

protected:
	SecondTimeout jump_value_;
	BloodContainer blood_container_;

	BLongSet attack_set;//攻击我的玩家列表
	Int64 cur_select_role;
};

#endif /* MAPTINYPLAYER_H_ */
