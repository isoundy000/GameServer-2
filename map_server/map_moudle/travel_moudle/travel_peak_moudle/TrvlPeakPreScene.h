/*
 * TrvlPeakPreScene.h
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#ifndef TRVLPEAKPRESCENE_H_
#define TRVLPEAKPRESCENE_H_

#include "Scene.h"

class TrvlPeakPreScene : public Scene
{
public:
	TrvlPeakPreScene();
	virtual ~TrvlPeakPreScene();

	void peak_prep_init();

	MoverCoord fetch_enter_pos();

private:
	int t_radius_;

};

#endif /* TRVLPEAKPRESCENE_H_ */
