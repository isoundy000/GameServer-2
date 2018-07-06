/*
 * StoryScript.h
 *
 *  Created on: 2016年8月26日
 *      Author: lyw
 */

#ifndef STORYSCRIPT_H_
#define STORYSCRIPT_H_

#include "BaseScript.h"

class StoryScript : public BaseScript
{
public:
	StoryScript();
	virtual ~StoryScript();
	virtual void reset(void);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* STORYSCRIPT_H_ */
