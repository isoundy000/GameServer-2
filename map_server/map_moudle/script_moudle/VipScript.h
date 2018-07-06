/*
 * VipScript.h
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#ifndef VIPSCRIPT_H_
#define VIPSCRIPT_H_

#include "BaseScript.h"

class VipScript : public BaseScript
{
public:
	VipScript();
	virtual ~VipScript();
	virtual void reset(void);
	virtual int fetch_monster_coord(MoverCoord &coord);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* VIPSCRIPT_H_ */
