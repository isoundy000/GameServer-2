/*
 * BehaviorStruct.h
 *
 * Created on: 2013-05-17 15:59
 *     Author: lyz
 */

#ifndef _BEHAVIORSTRUCT_H_
#define _BEHAVIORSTRUCT_H_

#include "PubStruct.h"

class GameAI;
class BehaviorNode;

// behavior tree enum
namespace BT
{
	enum BevRunningStatus
	{
		BEV_RS_IDL          = 0,
		BEV_RS_EXECUTING    = 1,
		BEV_RS_FINISH       = 2,
		BEV_RS_FAILURE      = 3,
		BEV_RS_END
	};
}

struct BevChildNode
{
    int __priority;
    BehaviorNode *__child;

    BevChildNode(const int priority, BehaviorNode *child);
    void reset(void);
};

struct AnyData
{
	GameAI* game_ai_;
};

typedef AnyData NodeInputParam;
typedef AnyData NodeOutputParam;

#endif //_BEHAVIORSTRUCT_H_
