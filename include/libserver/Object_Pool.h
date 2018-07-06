// -*- C++ -*-
/*
 * Object_Pool.h
 *
 *  Created on: Mar 23, 2012
 *      Author: ChenLong
 */

#ifndef OBJECT_POOL_H_
#define OBJECT_POOL_H_

#include "Thread_Mutex.h"
#include "Mutex_Guard.h"
#include "Lib_Log.h"
#include <list>
#include <string>
#include <algorithm>
#include <sstream>
#include <typeinfo>
#include "boost/unordered_map.hpp"

template <typename Obj, typename LOCK = Spin_Lock>
class Object_Pool {
public:
	struct ObjectNode
	{
		bool __is_in_used;
		void *__object;
	};
	typedef std::list<ObjectNode *> Obj_List;
	typedef boost::unordered_map<uint64_t, ObjectNode *> Obj_Map;

	const static int used_obj_list_bucket = 131073;

public:
	Object_Pool(void);

	virtual ~Object_Pool(void);

	virtual Obj *pop(void);

	virtual int push(Obj *obj);

	void clear(void);

	void dump_info(void);

    void dump_info_to_stream(std::ostringstream &stream);

	void dump_info_i(void);

	void debug_info(void);

	size_t sum_size(void);

	size_t free_obj_list_size(void);

	size_t used_obj_list_size(void);


protected:
	Obj_List free_obj_list_;
	size_t free_obj_list_size_;
	size_t used_obj_size_;
	size_t total_obj_size_;

	Obj_Map obj_map_; /// 使用hash_set防止push时std::list::remove操作的低效

	LOCK lock_;

	std::string type_name_;
};

template <typename Obj, typename LOCK>
Object_Pool<Obj, LOCK>::Object_Pool(void)
: free_obj_list_size_(0),
  used_obj_size_(0),
  total_obj_size_(0),
  obj_map_(used_obj_list_bucket)
{
	this->type_name_ = typeid(Obj).name();
}

template <typename Obj, typename LOCK>
Object_Pool<Obj, LOCK>::~Object_Pool(void) {
	this->clear();
}

template <typename Obj, typename LOCK>
void Object_Pool<Obj, LOCK>::dump_info(void) {
	GUARD(LOCK, mon, this->lock_);
	dump_info_i();
}

template <typename Obj, typename LOCK>
void Object_Pool<Obj, LOCK>::dump_info_to_stream(std::ostringstream &stream) {
    GUARD(LOCK, mon, this->lock_);
    stream << "free_obj_list_siz = " << free_obj_list_size_
    		<< ", used_obj_size = " << used_obj_size_
    		<< ", total_obj_size = " << total_obj_size_;
}

template <typename Obj, typename LOCK>
void Object_Pool<Obj, LOCK>::dump_info_i(void) {
	/*if (++dump_log_ > dump_log_interval_)*/ {
		LOG_DEBUG_INFO("free_obj_list_siz = %u, used_obj_list_size = %u", free_obj_list_size_, used_obj_size_);
//		dump_log_ = 0;
	}
}

template <typename Obj, typename LOCK>
void Object_Pool<Obj, LOCK>::debug_info(void) {
	LOG_DEBUG_INFO("free_obj_list_size_ = %u, free_obj_list_size_ = %u, used_size = %u",
			free_obj_list_size_, free_obj_list_.size(), used_obj_size_);
}

template <typename Obj, typename LOCK>
Obj * Object_Pool<Obj, LOCK>::pop(void) {
#ifdef NO_CACHE
	Obj *obj = new Obj;
#else
	GUARD(LOCK, mon, this->lock_);

	ObjectNode *node = 0;
	Obj *obj = 0;
	if (free_obj_list_size_ != 0) {
		node = this->free_obj_list_.front();
		free_obj_list_.pop_front();
		if (node->__is_in_used == true) {
			LOG_USER_TRACE("Object[%s] is used %x", this->type_name_.c_str(), node->__object);
		}
		obj = (Obj *)(node->__object);
		node->__is_in_used = true;
		--free_obj_list_size_;
		++used_obj_size_;
	} else {
		while (1) {
			if ((obj = new Obj) == 0) {
				LOG_USER_TRACE("new object[%s] return 0", this->type_name_.c_str());
				continue;
			} else {
				if ((node = new ObjectNode) == 0) {
					delete obj;
					obj = 0;
					LOG_USER_TRACE("new ObjectNode return 0");
					continue;
				}
				node->__object = obj;
				node->__is_in_used = true;
				++used_obj_size_;
				++total_obj_size_;
				if (obj_map_.insert(typename Obj_Map::value_type((uint64_t)(obj), node)).second == false)
				{
					LOG_USER_TRACE("insert obj map error %s %x", this->type_name_.c_str(), obj);
				}
				break;
			}
		}
	}
#endif
	return obj;
}

template <typename Obj, typename LOCK>
int Object_Pool<Obj, LOCK>::push(Obj *obj) {
#ifdef NO_CACHE
	delete obj;
#else
	GUARD(LOCK, mon, this->lock_);

	if (obj == 0)
		return -1;

    // modify 2013-06-04
	//typename Obj_Set::iterator used_obj_it = this->used_obj_list_.find(obj);
	//if (used_obj_it == this->used_obj_list_.end()) {
	//	LOG_USER_TRACE("***** Object_Pool<Obj, LOCK>::push(Obj *obj) can't find this Object Block %x!!!", obj);
	//	//LOG_ABORT();
	//	return -1;
	//}
	//this->used_obj_list_.erase(used_obj_it);
    //
	typename Obj_Map::iterator obj_iter = obj_map_.find((uint64_t)(obj));
	if (obj_iter == obj_map_.end())
    {
		LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) can't find this Object Block %s %x!!!",
	    		this->type_name_.c_str(), obj);
	    return -1;
    }
	ObjectNode *node = obj_iter->second;
	if (node->__object != (void *)obj)
	{
		LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) is inner error %s %x!!!",
	    		this->type_name_.c_str(), obj);
	    return -1;
	}
	if (node->__is_in_used == false)
	{
		LOG_USER_INFO("***** Object_Pool<Obj, LOCK>::push(Obj *obj) is no used %s %x!!!",
	    		this->type_name_.c_str(), obj);
	    return -1;
	}

	node->__is_in_used = false;
	this->free_obj_list_.push_back(node);
	++free_obj_list_size_;
	--used_obj_size_;
#endif
	return 0;
}

template <typename Obj, typename LOCK>
void Object_Pool<Obj, LOCK>::clear(void) {
	GUARD(LOCK, mon, this->lock_);

	for (typename Obj_List::iterator it = this->free_obj_list_.begin();
			it != this->free_obj_list_.end(); ++it) {
		if ((*it)->__object != 0)
			delete ((Obj *)((*it)->__object));
		delete *it;
	}
	//for (typename Obj_Set::iterator it = this->used_obj_list_.begin();
	//		it != this->used_obj_list_.end(); ++it) {
	//	delete *it;
	//}

	this->total_obj_size_ = 0;
	this->used_obj_size_ = 0;
	this->free_obj_list_size_ = 0;
	this->free_obj_list_.clear();
	this->obj_map_.clear();
}

template <typename Obj, typename LOCK>
size_t Object_Pool<Obj, LOCK>::sum_size(void) {
	GUARD(LOCK, mon, this->lock_);

	return total_obj_size_;
}

template <typename Obj, typename LOCK>
size_t Object_Pool<Obj, LOCK>::free_obj_list_size(void) {
	return free_obj_list_size_;
}

template <typename Obj, typename LOCK>
size_t Object_Pool<Obj, LOCK>::used_obj_list_size(void) {
	return used_obj_size_;
}

#endif /* OBJECT_POOL_H_ */
