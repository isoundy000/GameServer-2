/*
 * RamaScript.h
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#ifndef RAMASCRIPT_H_
#define RAMASCRIPT_H_

#include "BaseScript.h"

class RamaScript : public BaseScript
{
public:
	RamaScript();
	virtual ~RamaScript();
	virtual void reset(void);

	virtual int fetch_reward_index();
    virtual int fetch_first_reward(ThreeObjVec& reward_vec);
    virtual int fetch_normal_reward();
    virtual int fetch_wave_reward(ThreeObjVec& reward_vec);

    virtual int script_finish_flag();
    virtual int process_script_stage_finish();	//阶段完成处理

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* RAMASCRIPT_H_ */
