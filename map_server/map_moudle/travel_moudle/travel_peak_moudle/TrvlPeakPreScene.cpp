/*
 * TrvlPeakPreScene.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#include "TrvlPeakPreScene.h"

TrvlPeakPreScene::TrvlPeakPreScene() {
	// TODO Auto-generated constructor stub
	this->t_radius_ = 0;
}

TrvlPeakPreScene::~TrvlPeakPreScene() {
	// TODO Auto-generated destructor stub
}

void TrvlPeakPreScene::peak_prep_init()
{
	this->init_scene(0, GameEnum::TRVL_PEAK_PRE_SCENE_ID);
	this->run_scene();

	const Json::Value& conf = this->conf();
	this->t_radius_ = conf["r"].asInt();
}

MoverCoord TrvlPeakPreScene::fetch_enter_pos()
{
	const Json::Value& enter_pos = this->conf()["enter_pos"];

	MoverCoord center_pos = GameCommon::fetch_rand_conf_pos(enter_pos);
	return this->rand_coord(center_pos, this->t_radius_);
}

