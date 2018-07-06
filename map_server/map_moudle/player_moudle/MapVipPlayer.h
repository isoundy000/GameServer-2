/*
 * MapVipPlayer.h
 *
 *  Created on: 2013-12-6
 *      Author: root
 */

#ifndef MAPVIPPLAYER_H_
#define MAPVIPPLAYER_H_

#include "GameFighter.h"

class MapVipPlayer : virtual public GameFighter
{
public:
	MapVipPlayer();
	virtual ~MapVipPlayer();
	void reset();

	bool is_vip();
	int vip_type();

	int set_vip_type(int vip_type);
	int sync_vip_info(Message* msg);

	BaseVipDetail &vip_detail();

    int sync_transfer_vip();
    int serialize_vip(Message *msg);
    int unserialize_vip(Message *msg);

private:
	BaseVipDetail vip_detail_;
};

#endif
