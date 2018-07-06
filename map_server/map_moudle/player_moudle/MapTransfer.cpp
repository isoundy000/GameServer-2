/*
 * MapTransfer.cpp
 *
 *  Created on: 2017年4月7日
 *      Author: lyw
 */

#include "MapTransfer.h"
#include "MapPlayer.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"

MapTransfer::MapTransfer() {
	// TODO Auto-generated constructor stub

}

MapTransfer::~MapTransfer() {
	// TODO Auto-generated destructor stub
}

void MapTransfer::reset_transfer(void)
{
	this->transfer_detail_.reset();
}

MapPlayer *MapTransfer::transfer_player(void)
{
	return dynamic_cast<MapPlayer *>(this);
}

double MapTransfer::fetch_sub_skill_use_rate(FighterSkill* skill)
{
	JUDGE_RETURN(skill->__sub_rate_skill_2 > 0, 0);

	TransferDetail &transfer_detail = this->transfer_detail();
	JUDGE_RETURN(transfer_detail.is_in_transfer() == true, 0);

	TransferDetail::TransferInfo *info = transfer_detail.fetch_transfer_info();
	JUDGE_RETURN(info != NULL, 0);
	JUDGE_RETURN(info->skill_info_.count(skill->__sub_rate_skill_2) > 0, 0);

	FighterSkill *sub_skill = info->skill_info_[skill->__sub_rate_skill_2];
	JUDGE_RETURN(sub_skill != NULL, 0);

	return sub_skill->detail()["percent"].asInt();
}

int MapTransfer::update_transfer_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400411*, request, -1);

	this->delete_last_transfer(request->type());

	TransferDetail &transfer_detail = this->transfer_detail();
	transfer_detail.transfer_tick_ = request->transfer_tick();
	transfer_detail.last_ = request->last();
	transfer_detail.active_id_ = request->active_id();
	transfer_detail.reduce_cool_ = request->reduce_cool();
	transfer_detail.add_time_ = request->add_time();

	if (transfer_detail.active_id_ > 0)
	{
		TransferDetail::TransferInfo &transfer_info = transfer_detail.transfer_map_[transfer_detail.active_id_];
		const ProtoTransferInfo &transfer_proto = request->transfer_info();
		transfer_info.unserialize(transfer_proto);
	}

	this->create_transfer_skill(request->type());

	JUDGE_RETURN(request->notify() == true, 0);
	this->refresh_transfer_shape();

	return 0;
}

int MapTransfer::player_use_transfer()
{
	return this->send_to_logic_thread(CLIENT_USER_TRANSFER);
}

int MapTransfer::create_transfer_skill(int type)
{
	TransferDetail &transfer_detail = this->transfer_detail();

	TransferDetail::TransferInfo *info = transfer_detail.fetch_transfer_info();
	JUDGE_RETURN(info != NULL, 0);

	MapPlayer *player = this->transfer_player();

	switch (type)
	{
	case TransferDetail::CHANGE_TRANSFER:
	case TransferDetail::CD_TRANSFER:
	{
		this->insert_skill(info->transfer_skill_);
		player->fetch_noraml_skill_list();

		break;
	}
	case TransferDetail::USE_TRANSFER:
	{
		info->create_skill();
		player->fetch_transfer_skill_list();

		break;
	}
	case TransferDetail::SYNC_TRANSFER:
	{
		this->insert_skill(info->transfer_skill_);
		JUDGE_RETURN(transfer_detail.is_in_transfer() == true, 0);

		info->create_skill();

		break;
	}
	default:
		break;
	}

	return 0;
}

int MapTransfer::delete_last_transfer(int type)
{
	TransferDetail &transfer_detail = this->transfer_detail();

	TransferDetail::TransferInfo *info = transfer_detail.fetch_transfer_info();
	JUDGE_RETURN(info != NULL, 0);

	switch (type)
	{
	case TransferDetail::CHANGE_TRANSFER:
	{
		this->remove_skill(info->transfer_skill_);
		info->delete_skill();
		transfer_detail.transfer_map_.clear();

		break;
	}
	case TransferDetail::CD_TRANSFER:
	{
		for (IntMap::iterator it = info->skill_map_.begin();
				it != info->skill_map_.end(); ++it)
		{
			this->remove_skill(it->first);
		}
		info->delete_skill();

		MapPlayer *player = this->transfer_player();
		player->fetch_noraml_skill_list();
		break;
	}
	default:
		break;
	}

	return 0;
}

int MapTransfer::use_transfer(int id)
{
	TransferDetail::TransferInfo info(id);
	info.add_skill();

	for (IntMap::iterator iter = info.skill_map_.begin();
			iter != info.skill_map_.end(); ++iter)
	{
		this->insert_skill(iter->first);
	}

	{
		Proto51403006 respond;
		respond.set_transfer_id(id);
		respond.set_transfer_tick(::time(NULL));
		respond.set_last(Time_Value::DAY);
		this->respond_to_client(RETURN_USER_TRANSFER, &respond);
	}

	{
		MapPlayer* player = this->transfer_player();
		player->fetch_transfer_skill_list(info.skill_map_);
		player->role_detail().hickty_id_ = id;

		this->notify_update_player_info(GameEnum::PLAYER_INFO_TRANSFER, id);
		this->notify_update_player_info(GameEnum::PLAYER_INFO_SWORD_POOL);
		this->notify_update_player_info(GameEnum::PLAYER_INFO_MAGICWEAPON);
	}
	return 0;
}

int MapTransfer::remove_transfer(int id)
{
	TransferDetail::TransferInfo info(id);
	info.add_skill();

	for (IntMap::iterator iter = info.skill_map_.begin();
			iter != info.skill_map_.end(); ++iter)
	{
		this->remove_skill(iter->first);
	}

	MapPlayer* player = this->transfer_player();
	player->fetch_noraml_skill_list();
	player->role_detail().hickty_id_ = 0;
	player->role_detail().health_ = 0;

	this->respond_to_client(ACTIVE_REGION_QUIT_TRANFER);
	this->notify_update_player_info(GameEnum::PLAYER_INFO_TRANSFER,
			player->fetch_show_mode_id());
	this->notify_update_player_info(GameEnum::PLAYER_INFO_SWORD_POOL);
	this->notify_update_player_info(GameEnum::PLAYER_INFO_MAGICWEAPON);
	return 0;
}

int MapTransfer::refresh_transfer_shape()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	int transfer_id = 0;
	if (transfer_detail.is_in_transfer() == true)
		transfer_id = transfer_detail.active_id_;

	this->notify_update_player_info(GameEnum::PLAYER_INFO_TRANSFER,
			transfer_id);

	return 0;
}

int MapTransfer::trans_refresh_transfer_shape()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	int transfer_id = 0;
	if (transfer_detail.is_in_transfer() == true)
		transfer_id = transfer_detail.active_id_;

	MapPlayer *pleyer = this->transfer_player();

	Proto80400208 respond;
	respond.set_role_id(pleyer->role_id());
	respond.set_update_type(GameEnum::PLAYER_INFO_TRANSFER);
	respond.set_value(transfer_id);
	return this->respond_to_client(ACTIVE_UPDATE_PLAYER_INFO, &respond);
}

int MapTransfer::fetch_transfer_id()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	if (transfer_detail.is_in_transfer() == true)
		return transfer_detail.active_id_;

	return 0;
}

SkillMap *MapTransfer::fetch_transfer_skill()
{
	TransferDetail::TransferInfo *info = this->transfer_detail().fetch_transfer_info();
	JUDGE_RETURN(info != NULL, NULL);

	return &info->skill_info_;
}

bool MapTransfer::check_is_in_transfer_time()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	return transfer_detail.is_in_transfer();
}

bool MapTransfer::check_is_in_cool_time()
{
	TransferDetail &transfer_detail = this->transfer_detail();
	return transfer_detail.is_in_cool();
}

TransferDetail& MapTransfer::transfer_detail()
{
	return this->transfer_detail_;
}

