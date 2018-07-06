/*
 * MLVipPlayer.h
 *
 *  Created on: 2013-12-3
 *      Author: root
 */

#ifndef MLVIPPLAYER_H_
#define MLVIPPLAYER_H_

#include "MLPacker.h"

class MLVipPlayer :virtual public MLPacker
{
public:
	MLVipPlayer();
	virtual ~MLVipPlayer();

	void reset();
	void check_and_adjust_vip();
	void on_vip_status_upgrade(int vip_type);

	bool is_vip();
	bool is_max_vip();

	int vip_type(void);
	MLVipDetail& vip_detail(void);

	int gain_vip_gift(Message* msg);
	int fetch_vip_info(void);

	int justify_recharge_gold();
	int vip_free_transfer(void);
	int notify_sync_vip_info(bool is_notify);

	int vip_addtion_exp(const int inc_value);
    int calc_vip_prop(IntMap& prop_map);

	int sync_transfer_vip(int scene_id);
	int read_transfer_vip(Message* msg);

	//超级VIP相关
	int fetch_super_vip_info_begin(void);
	int fetch_super_vip_info_end(Message* msg);
	bool check_open_super_vip(int type, int limit);

private:
	MLVipDetail vip_detail_;
};
#endif /* MLVIPPLAYER_H_ */
