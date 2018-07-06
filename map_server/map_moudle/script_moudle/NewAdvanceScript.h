/*
 * NewAdvanceScript.h
 *
 *  Created on: 2016年11月1日
 *      Author: lyw
 */

#ifndef NEWADVANCESCRIPT_H_
#define NEWADVANCESCRIPT_H_

#include "BaseScript.h"

class NewAdvanceScript : public BaseScript
{
public:
	NewAdvanceScript();
	virtual ~NewAdvanceScript();
	virtual void reset(void);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* NEWADVANCESCRIPT_H_ */
