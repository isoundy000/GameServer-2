/*
 * BTTreePool.h
 *
 *  Created on: May 24, 2013
 *      Author: peizhibi
 */

#ifndef BTTREEPOOL_H_
#define BTTREEPOOL_H_

#include "Object_Pool.h"
#include "BehaviorNode.h"
#include "Thread_Mutex.h"

/*
 * Behavior Tree Pool
 * */
class BTTreePool : public Object_Pool<BehaviorNode, RE_MUTEX>
{
public:
	typedef Object_Pool<BehaviorNode, RE_MUTEX> BASE_POOL;
public:
	BTTreePool(const std::string& tree_name);
	virtual ~BTTreePool();

    virtual BehaviorNode *pop(void);

private:
    std::string tree_name_;
};

#endif /* BTNODEPOOL_H_ */
