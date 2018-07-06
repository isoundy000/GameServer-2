/*
 * BehaviorStruct.cpp
 *
 * Created on: 2013-05-17 16:03
 *     Author: lyz
 */

#include "BehaviorStruct.h"


BevChildNode::BevChildNode(const int priority, BehaviorNode *child)
{
	this->__priority = priority;
	this->__child = child;
}

void BevChildNode::reset(void)
{

}
