/*
 * TArenaPrepScene.cpp
 *
 *  Created on: Mar 22, 2017
 *      Author: peizhibi
 */

#include "TArenaPrepScene.h"

TArenaPrepScene::TArenaPrepScene() {
	// TODO Auto-generated constructor stub
	this->t_radius_ = 0;
}

TArenaPrepScene::~TArenaPrepScene() {
	// TODO Auto-generated destructor stub
}

void TArenaPrepScene::tarena_prep_init()
{
	this->init_scene(0, GameEnum::TRVL_ARENA_PREP_SCENE_ID);
	this->run_scene();

	const Json::Value& conf = this->conf();
	this->t_radius_ = conf["r"].asInt();
}


MoverCoord TArenaPrepScene::fetch_enter_pos()
{
	const Json::Value& enter_pos = this->conf()["enter_pos"];

	MoverCoord center_pos = GameCommon::fetch_rand_conf_pos(enter_pos);
	return this->rand_coord(center_pos, this->t_radius_);
}
