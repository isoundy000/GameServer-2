/*
 * TMArenaPrep.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#include "TMArenaPrep.h"

TMArenaPrep::TMArenaPrep()
{
	// TODO Auto-generated constructor stub
	this->t_radius_ = 0;
}

TMArenaPrep::~TMArenaPrep()
{
	// TODO Auto-generated destructor stub
}

void TMArenaPrep::tmarena_prep_init()
{
	this->init_scene(0, GameEnum::TRVL_MARENA_PREP_SCENE_ID);
	this->run_scene();

	const Json::Value& conf = this->conf();
	this->t_radius_ = conf["r"].asInt();
}

MoverCoord TMArenaPrep::fetch_enter_pos()
{
	const Json::Value& enter_pos = this->conf()["enter_pos"];

	MoverCoord center_pos = GameCommon::fetch_rand_conf_pos(enter_pos);
	return this->rand_coord(center_pos, this->t_radius_);
}
