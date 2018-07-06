/*
 * TArenaPrepScene.h
 *
 *  Created on: Mar 22, 2017
 *      Author: peizhibi
 */

#ifndef TARENAPREPSCENE_H_
#define TARENAPREPSCENE_H_

#include "Scene.h"

class TArenaPrepScene : public Scene
{
public:
	TArenaPrepScene();
	virtual ~TArenaPrepScene();

	void tarena_prep_init();

	MoverCoord fetch_enter_pos();

private:
	int t_radius_;

};

typedef Singleton<TArenaPrepScene> TArenaPrepSceneSingle;
#define TARENA_PREP_SCENE   (TArenaPrepSceneSingle::instance())

#endif /* TARENAPREPSCENE_H_ */
