/*
 * SealBossScript.h
 *
 * Created on: 2015-07-18 19:24
 *     Author: lyz
 */

#ifndef _SEALBOSSSCRIPT_H_
#define _SEALBOSSSCRIPT_H_

#include "BaseScript.h"

class SealBossScript : public BaseScript
{
public:
    virtual ~SealBossScript(void);
    virtual void reset(void);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif //_SEALBOSSSCRIPT_H_
