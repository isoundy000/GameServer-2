/*
 * ClimbTowerScript.h
 *
 * Created on: 2014-02-13 15:33
 *     Author: lyz
 */

#ifndef _CLIMBTOWERSCRIPT_H_
#define _CLIMBTOWERSCRIPT_H_

#include "BaseScript.h"

// 爬塔副本(章节副本)
class ClimbTowerScript : public BaseScript
{
public:
    ClimbTowerScript(void);
    virtual ~ClimbTowerScript(void);
    virtual void reset(void);

protected:
    virtual void recycle_self_to_pool(void);

    virtual int init_script_first_scene(void);
};

#endif //_CLIMBTOWERSCRIPT_H_
