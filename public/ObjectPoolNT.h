/*
 * ObjectPoolNT.h
 *
 * Created on: 2014-11-25 11:29
 *     Author: lyz
 */

#ifndef _OBJECTPOOLNT_H_
#define _OBJECTPOOLNT_H_

#include "Object_Pool.h"
#include "Log.h"

template <class CObject>
class ObjectPoolNT : public Object_Pool<CObject, NULL_MUTEX>
{
public:
    typedef Object_Pool<CObject, NULL_MUTEX> SUPPER_POOL;
public:
    virtual ~ObjectPoolNT(void);
    virtual CObject *pop(void);
    virtual int push(CObject *obj);
};

template<class CObject>
ObjectPoolNT<CObject>::~ObjectPoolNT(void)
{ /*NULL*/ }

template<class CObject>
inline CObject *ObjectPoolNT<CObject>::pop(void)
{
    CObject *obj = SUPPER_POOL::pop();
    if (obj == 0)
        return obj;

    obj->reset();
    return obj;
}

template<class CObject>
int ObjectPoolNT<CObject>::push(CObject *obj)
{
    if (obj == 0)
    {
        MSG_USER("ERROR push obejct is null: %s", type_name_.c_str());
        return -1;
    }

    obj->reset();
    return SUPPER_POOL::push(obj);
}

#endif //_OBJECTPOOLNT_H_
