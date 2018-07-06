/*
 * UpclassScript.h
 *
 * Created on: 2014-02-13 15:38
 *     Author: lyz
 */

#ifndef _UPCLASSSCRIPT_H_
#define _UPCLASSSCRIPT_H_

#include "BaseScript.h"

// 升阶副本
class UpclassScript : public BaseScript
{
public:
    UpclassScript(void);
    virtual ~UpclassScript(void);
    virtual void reset(void);

    virtual int generate_new_wave(const int scene_id, const int wave_id);

protected:
    virtual void recycle_self_to_pool(void);
};

#endif //_UPCLASSSCRIPT_H_
