/*
 * AreaMonitor.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: peizhibi
 */

#include "AreaMonitor.h"
#include "AreaField.h"

#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

AreaMonitor::AreaMonitor()
{
	// TODO Auto-generated constructor stub
	this->area_field_package_ = new PoolPackage<AreaField>;
}

AreaMonitor::~AreaMonitor()
{
	// TODO Auto-generated destructor stub
}

int AreaMonitor::create_area_field(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400438*, request, -1);

	int area_index = request->area_index();
	JUDGE_RETURN(this->area_field_package_->find_object(area_index) == NULL, -1);

	AreaField* area_field = this->area_field_package_->pop_object();
	area_field->start_area_field(msg);

	MapPlayerEx* copy_player = MAP_MONITOR->player_pool()->pop();
	JUDGE_RETURN(copy_player != NULL, -1);

	MoverCoord coord;
	coord.set_pixel(CONFIG_INSTANCE->athletics_base()["second_loc"][0u].asInt(),
			CONFIG_INSTANCE->athletics_base()["second_loc"][1u].asInt());

	MoverDetail& mover_detail = copy_player->mover_detail();
	mover_detail.__location = coord;

	copy_player->set_scene_mode(SCENE_MODE_LEAGUE);
	copy_player->init_mover_scene(area_field);

	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_LOAD_AREA_COPY_PLAYER;
	shop_mode->input_argv_.type_int64_ = request->second_id();
	shop_mode->output_argv_.type_void_ = copy_player;

	MAP_MONITOR->db_map_load_mode_begin(shop_mode);
	this->area_field_package_->bind_object(area_index, area_field);

	return 0;
}

int AreaMonitor::recycle_area_field(AreaField* area_field)
{
	return this->area_field_package_->unbind_and_push(area_field->area_index(),
			area_field);
}

AreaField* AreaMonitor::find_area_field(int area_index)
{
	return this->area_field_package_->find_object(area_index);
}

