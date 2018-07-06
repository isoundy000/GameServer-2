/*
 * AdvanceScript.h
 *
 *  Created on: 2016年8月18日
 *      Author: lyw
 */

#ifndef ADVANCESCRIPT_H_
#define ADVANCESCRIPT_H_

#include "BaseScript.h"

class AdvanceScript : public BaseScript
{
public:
	AdvanceScript();
	virtual ~AdvanceScript();
	virtual void reset(void);
	virtual int sync_restore_pass(MapPlayerScript* player);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* ADVANCESCRIPT_H_ */
