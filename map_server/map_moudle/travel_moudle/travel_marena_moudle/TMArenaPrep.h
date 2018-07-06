/*
 * TMArenaPrep.h
 *
 *  Created on: Apr 11, 2017
 *      Author: peizhibi
 */

#ifndef TMARENAPREP_H_
#define TMARENAPREP_H_

#include "Scene.h"

class TMArenaPrep : public Scene
{
public:
	TMArenaPrep();
	virtual ~TMArenaPrep();

	void tmarena_prep_init();
	MoverCoord fetch_enter_pos();

private:
	int t_radius_;
};

typedef Singleton<TMArenaPrep> TMArenaPrepSingle;
#define TMARENA_PREP   (TMArenaPrepSingle::instance())

#endif /* TMARENAPREP_H_ */
