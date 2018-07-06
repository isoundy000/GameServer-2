/*
 * CouplesScript.h
 *
 * Created on: 2015-06-11 14:53
 *     Author: lyz
 */

#ifndef _COUPLESSCRIPT_H_
#define _COUPLESSCRIPT_H_

#include "BaseScript.h"

// 夫妻副本
class CouplesScript : public BaseScript
{
public:
public:
    virtual ~CouplesScript(void);
    virtual void reset(void);
    virtual int fetch_normal_reward();
    
protected:
    virtual void recycle_self_to_pool(void);
};

#endif //_COUPLESSCRIPT_H_
