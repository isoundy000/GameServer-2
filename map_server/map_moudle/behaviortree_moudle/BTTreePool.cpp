/*
 * BTNodePool.cpp
 *
 *  Created on: May 24, 2013
 *      Author: peizhibi
 */

#include "BTTreePool.h"
#include "BTCFactory.h"

BTTreePool::BTTreePool(const std::string& tree_name)
{
	// TODO Auto-generated constructor stub
	this->tree_name_ = tree_name;

}

BTTreePool::~BTTreePool()
{
	// TODO Auto-generated destructor stub
}

BehaviorNode *BTTreePool::pop(void)
{
	GUARD(RE_MUTEX, mon, this->lock_);

	BASE_POOL::ObjectNode *node = NULL;
	BehaviorNode *obj = NULL;
	if (this->free_obj_list_.empty() == false)
	{
		node = this->free_obj_list_.front();
		this->free_obj_list_.pop_front();

		obj = (BehaviorNode *)(node->__object);
		node->__is_in_used = true;
		--this->free_obj_list_size_;
		++this->used_obj_size_;
	}
	else
	{
		obj = BTCFACTORY->create_ai_tree(this->tree_name_);

		JUDGE_RETURN(obj != NULL, NULL);

		node = new ObjectNode();
		node->__is_in_used = true;
		node->__object = obj;
		this->obj_map_.insert(BASE_POOL::Obj_Map::value_type((uint64_t)(obj), node));
		++this->used_obj_size_;
		++this->total_obj_size_;
	}

	return obj;
}
