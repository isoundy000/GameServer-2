/*
 * NewStoryScript.h
 *
 *  Created on: 2016年11月1日
 *      Author: lyw
 */

#ifndef NEWSTORYSCRIPT_H_
#define NEWSTORYSCRIPT_H_

#include "BaseScript.h"

class NewStoryScript : public BaseScript
{
public:
	NewStoryScript();
	virtual ~NewStoryScript();
	virtual void reset(void);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif /* NEWSTORYSCRIPT_H_ */
