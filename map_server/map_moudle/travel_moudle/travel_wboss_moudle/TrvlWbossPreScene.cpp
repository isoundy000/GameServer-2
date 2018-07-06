/*
 * TrvlWbossPreScene.cpp
 *
 *  Created on: 2017年4月27日
 *      Author: lyw
 */

#include "TrvlWbossPreScene.h"

TrvlWbossPreScene::TrvlWbossPreScene() {
	// TODO Auto-generated constructor stub

}

TrvlWbossPreScene::~TrvlWbossPreScene() {
	// TODO Auto-generated destructor stub
}

void TrvlWbossPreScene::reset(void)
{
	Scene::reset();
}

void TrvlWbossPreScene::init_wboss_pre()
{
	this->init_scene(0, GameEnum::TRVL_WBOSS_SCENE_ID_READY);
	this->start_scene();
}



