/*
 * SwordTopScript.h
 *
 *  Created on: 2017年3月17日
 *      Author: lyw
 */

#ifndef SWORDTOPSCRIPT_H_
#define SWORDTOPSCRIPT_H_

#include "BaseScript.h"

class SwordTopScript : public BaseScript
{
public:
	SwordTopScript();
	virtual ~SwordTopScript();

	virtual void reset(void);

	virtual int fetch_task_wave();
	virtual int fetch_reward_index();
	virtual int fetch_first_reward(ThreeObjVec& reward_vec);
	virtual int fetch_normal_reward();
	virtual int fetch_monster_coord(MoverCoord &coord);

protected:
	virtual void recycle_self_to_pool(void);

};

#endif /* SWORDTOPSCRIPT_H_ */
