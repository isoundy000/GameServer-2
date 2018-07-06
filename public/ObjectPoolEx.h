/*
 * ObjectPoolEx.h
 *
 * Created on: 2013-01-04 10:40
 *     Author: glendy
 */

#ifndef _OBJECTPOOLEX_H_
#define _OBJECTPOOLEX_H_

#include "Object_Pool.h"
#include "Log.h"

template<class CObject>
class ObjectPoolEx : public Object_Pool<CObject, RE_MUTEX>
{
public:
    typedef Object_Pool<CObject, RE_MUTEX> SUPPER_POOL;
    typedef RE_MUTEX MUTEX_TYPE;
public:
    virtual ~ObjectPoolEx(void);
    virtual CObject *pop(void);
    virtual int push(CObject *obj);
};

template<class CObject>
ObjectPoolEx<CObject>::~ObjectPoolEx(void)
{ /*NULL*/ }

template<class CObject>
inline CObject *ObjectPoolEx<CObject>::pop(void)
{
    CObject *obj = SUPPER_POOL::pop();
    if (obj == 0)
        return obj;

    obj->reset();
    return obj;
}

template<class CObject>
int ObjectPoolEx<CObject>::push(CObject *obj)
{
    if (obj == 0)
    {
        MSG_USER("ERROR push obejct is null: %s", this->type_name_.c_str());
        return -1;
    }

    obj->reset();
    return SUPPER_POOL::push(obj);
}

#endif //_OBJECTPOOLEX_H_
