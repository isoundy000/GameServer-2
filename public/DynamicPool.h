/*
 * DynamicPool.h
 *
 * Created on: 2016-06-04 16:42
 *     Author: lyz
 */

#ifndef _DYNAMICPOOL_H_
#define _DYNAMICPOOL_H_

#include "ObjectPoolEx.h"
#include "Date_Time.h"

template <class CObject>
class DynamicPool : public ObjectPoolEx<CObject>
{
public:
    typedef ObjectPoolEx<CObject> SUPPER_POOL;
public:
    virtual ~DynamicPool(void);
    virtual int push(CObject *obj);
};

template<class CObject>
DynamicPool<CObject>::~DynamicPool(void)
{ /*NULL*/ }

template<class CObject>
int DynamicPool<CObject>::push(CObject *obj)
{
    if (obj == 0)
    {
        MSG_USER("ERROR push obejct is null: %s", SUPPER_POOL::SUPPER_POOL::type_name_.c_str());
        return -1;
    }

    obj->reset();

#ifdef NO_CACHE
    delete obj;
#else
    GUARD(typename SUPPER_POOL::MUTEX_TYPE, mon, this->lock_);

    typename SUPPER_POOL::SUPPER_POOL::Obj_Map::iterator obj_iter = this->obj_map_.find((uint64_t)(obj));
    if (obj_iter == this->obj_map_.end())
    {
    	LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) can't find this Object Block %s %x!!!",
    			SUPPER_POOL::SUPPER_POOL::type_name_.c_str(), obj);
        return -1;
    }
    typename SUPPER_POOL::SUPPER_POOL::ObjectNode *node = obj_iter->second;
    if (node->__object != (void *)obj)
    {
    	LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) is inner error %s %x!!!",
    			SUPPER_POOL::SUPPER_POOL::type_name_.c_str(), obj);
        return -1;
    }
    if (node->__is_in_used == false)
    {
    	LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) is no used %s %x!!!",
    			SUPPER_POOL::SUPPER_POOL::type_name_.c_str(), obj);
        return -1;
    }

    node->__is_in_used = false;
    this->free_obj_list_.push_front(node);
    ++this->free_obj_list_size_;
    --this->used_obj_size_;

    if (this->free_obj_list_.size() > 100)
    {
//        Date_Time now_date(Time_Value::gettimeofday());
//        if (2 <= now_date.hour() && now_date.hour() <= 18)
        {
            int i = 0;
            while (this->free_obj_list_.size() > 20 && i < 100)
            {
                ++i;
                node = this->free_obj_list_.back();
                if (node->__is_in_used == true)
                    continue;
                this->free_obj_list_.pop_back();
                obj = (CObject *)(node->__object);
                
                typename SUPPER_POOL::SUPPER_POOL::Obj_Map::iterator iter = this->obj_map_.find((uint64_t)(obj));
                if (iter != this->obj_map_.end())
                    this->obj_map_.erase(iter);

                if (obj != NULL)
                    delete obj;
                delete node;
                --this->free_obj_list_size_;
                --this->total_obj_size_;
            }
        }
    }
#endif
    return 0;
}

#endif //_DYNAMICPOOL_H_
