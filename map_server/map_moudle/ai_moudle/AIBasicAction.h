/*
 * AIBasicAction.h
 *
 *  Created on: Jun 21, 2013
 *      Author: peizhibi
 */

#ifndef AIBASICACTION_H_
#define AIBASICACTION_H_

#include "BehaviorNode.h"
#include "AIStruct.h"

class BaseAIAction : public BevActionNode
{
protected:
	virtual int DoExecute(NodeOutputParam &output, NodeInputParam &input);
	virtual int DoAIExecute(GameAI* game_ai) = 0;

private:
	void dump_info(GameAI* game_ai);
};

/*
 * ai die
 * "node_action": "die_action"
 * */
class AIDieAction : public BaseAIAction
{
public:
	AIDieAction(double die_time = GameEnum::AI_DIE_TIME);

protected:
	virtual int DoAIExecute(GameAI* game_ai);

private:
	double die_time_;
};

/*
 * ai recycle
 * "node_action": "recycle_action"
 * */
class AIRecycleAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai recycle with alive
 * "node_action": "alive_recycle_action"
 * */
class AIAliveRecycleAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai sleep
 * "node_action": "sleep_action"
 * */
class AISleepAction : public BaseAIAction
{
public:
	AISleepAction(int sleep_time = GameEnum::AI_SLEEP_TIME);

protected:
	virtual int DoAIExecute(GameAI* game_ai);

private:
	int sleep_time_;
};

/*
 * pause
 */
class AIPauseAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * Auto
 * "node_action": "auto_action"
 * */
class AIAutoAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);

private:
	int ai_speak_action(GameAI* game_ai);
};

/*
 * ai move action
 * */
class AIMoveAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};


/*
 * Select Fighter
 * "node_action": "select_action"
 * */
class AISelectAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * Back
 * "node_action": "back_action"
 * */
class AIBackAction : public AIMoveAction
{
public:
	AIBackAction(void);

protected:
	virtual int DoAIExecute(GameAI* game_ai);

	int check_and_restore(GameAI* game_ai);
	int is_back_finish(GameAI* game_ai);
};

/*
 * Idle Back
 * "node_action": "idle_back_action"
 * */
class AIIdleBackAction : public AIMoveAction
{
public:
	AIIdleBackAction(void);

protected:
	virtual int DoAIExecute(GameAI* game_ai);

	bool is_idle_back_finish(GameAI* game_ai);
};

/*
 * ai rand move
 * "node_action": "patrol_action"
 * */
class AIRandMoveAction : public AIMoveAction
{
public:
	AIRandMoveAction(int rand_radius = GameEnum::AI_RAND_MOVE_RADIUS);

protected:
	virtual int DoAIExecute(GameAI* game_ai);

private:
	int rand_radius_;
};

/*
 * ai chase aim fighter
 * "node_action": "chase_action"
 * */
class AIChaseAction : public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai direct chase aim fighter, no around aim;
 * */
class AIDirectChaseAction : public AIMoveAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai attack aim fighter
 * "node_action": "attack_action"
 * */
class AIAttackAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai patrol route
 * "node_action" : "patrol_route_action"
 * */
class AIPatrolRouteAction : public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * ai patrol route
 * "node_action" : "patrol_route_b_action"
 * */
class AIPatrolRouteBAction : public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};


/*
 * ai follow
 * "node_action": "follow_action"
 * */
class AIFollowAction : public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * 移开离目标一定距离
 */
class AIKeepAwayAction : public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 *  moveto_action 移动指定目标点
 */
class AIMovetoAction : public AIMoveAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};


/*
 *  发动一套技能
 */
class AIComboAttackAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};


/*
 *  发动一套顺序随机的技能
 */
class AIRamdonComboAttackAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 *  发动普通攻击
 */
class AINormalAttackAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

/*
 * 定时器完成
 */
class AIIntervalFinAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

class AIAddBuffAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

class AIRemoveBuffAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

class AIRelAddSkillAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

class AIAddSkillAction : public BaseAIAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};

class AIGenGiftboxAction : public BaseAIAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};

class LimitDisChaseAction : public AIChaseAction
{
public:
	LimitDisChaseAction(int range_ = GameEnum::AI_CHASE_BACK_DISTANCE);
protected:
    virtual int DoAIExecute(GameAI* game_ai);
private:
    int range_;
};

class ReturnPatrolAction : public AIMoveAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};

class NotifyBossRemoveAttr : public BaseAIAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};
class NotifyShowBoss : public BaseAIAction
{
protected:
    virtual int DoAIExecute(GameAI* game_ai);
};
/*
 * 荒野奇袭怪物逃跑
 */
class MonsterRunAwayInQiXi:public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);
};
/*
 * 荒野奇袭怪物巡逻
 */
class MonsterPatrolInQiXi:public AIMoveAction
{
protected:
	virtual int DoAIExecute(GameAI* game_ai);

};

#endif /* AIBASICACTION_H_ */
