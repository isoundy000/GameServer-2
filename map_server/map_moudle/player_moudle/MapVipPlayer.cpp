/*
 * MapVipPlayer.cpp
 *
 *  Created on: 2013-12-6
 *      Author: root
 */

#include "MapPlayerEx.h"
#include "MapVipPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

MapVipPlayer::MapVipPlayer(void)
{
	// TODO Auto-generated constructor stub
}

MapVipPlayer::~MapVipPlayer(void)
{
	// TODO Auto-generated destructor stub
}

void MapVipPlayer::reset(void)
{
	this->vip_detail_.reset();
}

//是否是VIP玩家
bool MapVipPlayer::is_vip(void)
{
	return this->vip_detail_.is_vip();
}

int MapVipPlayer::vip_type(void)
{
	return this->vip_detail_.__vip_type;
}

int MapVipPlayer::set_vip_type(int vip_type)
{
	this->vip_detail_.set_vip_type(vip_type);

	MapPlayer *player = dynamic_cast<MapPlayer*>(this);
	JUDGE_RETURN(player != NULL, -1);

	player->role_detail().__vip_type = vip_type;
	return 0;
}

int MapVipPlayer::sync_vip_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400801*, request, -1);

	int pre_vip_type = this->vip_detail().__vip_type;
	this->set_vip_type(request->vip_type());

	this->vip_detail_.__start_time = request->vip_start_time();
	this->vip_detail_.__expired_time = request->expired_time();

	if (request->vip_type() != pre_vip_type && request->is_notify() == true)
	{
		this->notify_update_player_info(GameEnum::PLAYER_INFO_VIP);
	}

	return 0;
}

BaseVipDetail &MapVipPlayer::vip_detail(void)
{
	return this->vip_detail_;
}

int MapVipPlayer::sync_transfer_vip(void)
{
	Proto30400107 request;
	this->serialize_vip(&request);
	return this->send_to_other_scene(this->scene_id(), request);
}

int MapVipPlayer::serialize_vip(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400107 *, request, -1);
    BaseVipDetail &detail = this->vip_detail();

    request->set_vip_type(detail.__vip_type);
    request->set_vip_start_time(detail.__start_time);
    request->set_expired_time(detail.__expired_time);

    return 0;
}

int MapVipPlayer::unserialize_vip(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400107 *, request, -1);

    this->set_vip_type(request->vip_type());

    BaseVipDetail &detail = this->vip_detail();
    detail.__start_time = request->vip_start_time();
    detail.__expired_time = request->expired_time();

    return 0;
}

