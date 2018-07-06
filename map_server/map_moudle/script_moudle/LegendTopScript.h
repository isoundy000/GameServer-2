/*
 * LegendTopScript.h
 *
 *  Created on: 2016年8月16日
 *      Author: lyw
 */

#ifndef LEGENDTOPSCRIPT_H_
#define LEGENDTOPSCRIPT_H_

#include "BaseScript.h"

class LegendTopScript : public BaseScript
{
public:
	LegendTopScript();
	virtual ~LegendTopScript();

	virtual void reset(void);

	virtual int fetch_task_wave();
	virtual int fetch_reward_index();
	virtual int fetch_first_reward(ThreeObjVec& reward_vec);
	virtual int fetch_normal_reward();
	virtual int fetch_monster_coord(MoverCoord &coord);
	virtual int sync_restore_pass(MapPlayerScript* player);//更新资源找回

protected:
	virtual void recycle_self_to_pool(void);

};

#endif /* LEGENDTOPSCRIPT_H_ */
