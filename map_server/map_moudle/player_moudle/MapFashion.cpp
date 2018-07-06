/*
 * MapFashion.cpp
 *
 *  Created on: 2017年1月23日
 *      Author: lyw
 */

#include "MapFashion.h"
#include "ProtoDefine.h"

MapFashion::MapFashion() {
	// TODO Auto-generated constructor stub

}

MapFashion::~MapFashion() {
	// TODO Auto-generated destructor stub
}

void MapFashion::reset_fashion(void)
{
	this->fashion_detail_.reset();
}

int MapFashion::update_fashion_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400410*, request, -1);

	this->fashion_detail_.level_ = request->level();
	this->fashion_detail_.exp_ = request->exp();
	this->fashion_detail_.open_ = request->open();
	this->fashion_detail_.select_id_ = request->select_id();
	this->fashion_detail_.sel_color_id_ = request->sel_color_id();

	int notify_flag = request->notify_flag();
	JUDGE_RETURN(notify_flag > 0, 0);

	return this->refresh_fashion_shape();
}

int MapFashion::refresh_fashion_shape()
{
	this->notify_update_player_info(GameEnum::PLAYER_INFO_FASHION,
			this->fashion_detail_.select_id_);

	return 0;
}

int MapFashion::fetch_fashion_id()
{
	JUDGE_RETURN(this->fashion_detail_.open_ == true, 0);
	return this->fashion_detail_.select_id_;
}

int MapFashion::fetch_fashion_color()
{
	JUDGE_RETURN(this->fashion_detail_.open_ == true, 0);
	return this->fashion_detail_.sel_color_id_;
}

RoleFashion &MapFashion::fashion_detail()
{
	return this->fashion_detail_;
}
