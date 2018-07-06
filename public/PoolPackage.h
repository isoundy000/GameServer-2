/*
 * PoolPackage.h
 *
 *  Created on: Jun 28, 2013
 *      Author: peizhibi
 */

#ifndef POOLPACKAGE_H_
#define POOLPACKAGE_H_

#include "ObjectPoolEx.h"
#include "boost/unordered_map.hpp"

/*
 * 包含Pool和Map的功能
 * */
template<class CObject, class Index = int>
class PoolPackage
{
public:
	typedef ObjectPoolEx<CObject> CObjectPool;

	typedef boost::unordered_map<Index, Index> IndexMap;
	typedef boost::unordered_map<Index, CObject*> CObjectMap;

public:
	PoolPackage();
	virtual ~PoolPackage();

	CObject* pop_object();
	int push_object(CObject* object);

	CObject* find_object(Index object_index);
	CObject* find_and_pop_object(Index object_index);
	CObject* find_pop_bind_object(Index object_index);

	int bind_object(Index object_index, CObject* obj);
	int unbind_object(Index object_index);

	int unbind_and_push(Index object_index);
	int unbind_and_push(Index object_index, CObject* obj);

	IndexMap fetch_index_map();
	CObjectPool& cobject_pool();

	void clear(void);
    void report_pool_info(std::ostringstream &stream);

    int size();

private:
	CObject* find_object_i(Index object_index);
	int bind_object_i(Index object_index, CObject* obj);

private:
	RE_MUTEX map_mutex_;
	CObjectPool cobject_pool_;

	IndexMap index_map_;
	CObjectMap cobject_map_;
};

template<class CObject, class Index>
PoolPackage<CObject, Index>::PoolPackage()
{
}

template<class CObject, class Index>
PoolPackage<CObject, Index>::~PoolPackage()
{
	for (typename CObjectMap::iterator iter = this->cobject_map_.begin();
			iter != this->cobject_map_.end(); ++iter)
	{
		this->cobject_pool_.push(iter->second);
	}

	this->index_map_.clear();
	this->cobject_map_.clear();
}

template<class CObject, class Index>
CObject* PoolPackage<CObject, Index>::pop_object()
{
	return this->cobject_pool_.pop();
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::push_object(CObject* object)
{
	return this->cobject_pool_.push(object);
}

template<class CObject, class Index>
CObject* PoolPackage<CObject, Index>::find_object(Index object_index)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	return this->find_object_i(object_index);
}

template<class CObject, class Index>
CObject* PoolPackage<CObject, Index>::find_object_i(Index object_index)
{
	JUDGE_RETURN(this->cobject_map_.count(object_index) > 0, NULL);
	return this->cobject_map_[object_index];
}

template<class CObject, class Index>
CObject* PoolPackage<CObject, Index>::find_and_pop_object(Index object_index)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);

	CObject* obj = this->find_object_i(object_index);
	JUDGE_RETURN(obj == NULL, obj);

	return this->pop_object();
}

template<class CObject, class Index>
CObject* PoolPackage<CObject, Index>::find_pop_bind_object(Index object_index)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);

	CObject* obj = this->find_object_i(object_index);
	JUDGE_RETURN(obj == NULL, obj);

	obj = this->pop_object();
	JUDGE_RETURN(obj != NULL, NULL);

	this->bind_object_i(object_index, obj);
	return obj;
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::bind_object(Index object_index, CObject* obj)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	return this->bind_object_i(object_index, obj);
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::bind_object_i(Index object_index, CObject* obj)
{
	this->cobject_map_[object_index] = obj;
	this->index_map_[object_index] = object_index;
	return 0;
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::unbind_object(Index object_index)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	this->index_map_.erase(object_index);

	JUDGE_RETURN(this->cobject_map_.count(object_index) > 0, -1);
	this->cobject_map_.erase(object_index);

	return 0;
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::unbind_and_push(Index object_index)
{
	CObject* obj = this->find_object(object_index);
	JUDGE_RETURN(obj != NULL, -1);

	this->unbind_object(object_index);
	this->push_object(obj);
	return 0;
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::unbind_and_push(Index object_index, CObject* obj)
{
	this->unbind_object(object_index);
	this->push_object(obj);
	return 0;
}

template<class CObject, class Index>
typename PoolPackage<CObject, Index>::IndexMap PoolPackage<CObject, Index>::fetch_index_map()
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	return this->index_map_;
}

template<class CObject, class Index>
typename PoolPackage<CObject, Index>::CObjectPool& PoolPackage<CObject, Index>::cobject_pool()
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	return this->cobject_pool_;
}

template<class CObject, class Index>
void PoolPackage<CObject, Index>::clear(void)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	this->cobject_pool_.clear();
	this->index_map_.clear();
	this->cobject_map_.clear();
}

template<class CObject, class Index>
void PoolPackage<CObject, Index>::report_pool_info(std::ostringstream &stream)
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	this->cobject_pool_.dump_info_to_stream(stream);
}

template<class CObject, class Index>
int PoolPackage<CObject, Index>::size()
{
	GUARD(RE_MUTEX, mon, this->map_mutex_);
	return this->index_map_.size();
}

#endif /* POOLPACKAGE_H_ */
