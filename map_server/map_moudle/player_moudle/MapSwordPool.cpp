/*
 * MapSwordPool.cpp
 *
 *  Created on: 2016年11月3日
 *      Author: lyw
 */

#include "MapSwordPool.h"
#include "ProtoDefine.h"

MapSwordPool::MapSwordPool() {
	// TODO Auto-generated constructor stub

}

MapSwordPool::~MapSwordPool() {
	// TODO Auto-generated destructor stub
}

void MapSwordPool::reset_spool(void)
{
	SwordPoolDetail& spool = this->spool_detail_;
	spool.reset();
}

int MapSwordPool::update_spool_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400408*, request, -1);

	SwordPoolDetail& spool = this->spool_detail_;
	spool.level_ = request->level();
	spool.exp_ = request->exp();
	spool.open_ = request->open();
	spool.stype_lv_ = request->style_lvl();

	int notify_flag = request->notify_flag();
	JUDGE_RETURN(notify_flag > 0, 0);

	return this->refresh_spool_shape(spool);
}

int MapSwordPool::refresh_spool_shape(SwordPoolDetail& spool_detail)
{
	this->notify_update_player_info(GameEnum::PLAYER_INFO_SWORD_POOL,
			spool_detail.stype_lv_);

	return 0;
}

int MapSwordPool::fetch_spool_style_lvl()
{
	JUDGE_RETURN(this->is_lrf_change_mode() == false, 0);

	SwordPoolDetail& spool = this->spool_detail_;
	JUDGE_RETURN(spool.open_ == true, 0);

	return spool.stype_lv_;
}
