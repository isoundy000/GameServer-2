/*
 * TrvlWbossPreScene.h
 *
 *  Created on: 2017年4月27日
 *      Author: lyw
 */

#ifndef TRVLWBOSSPRESCENE_H_
#define TRVLWBOSSPRESCENE_H_

#include "Scene.h"

class TrvlWbossPreScene : public Scene
{
public:
	TrvlWbossPreScene();
	virtual ~TrvlWbossPreScene();
	virtual void reset(void);

	void init_wboss_pre();

};

#endif /* TRVLWBOSSPRESCENE_H_ */
