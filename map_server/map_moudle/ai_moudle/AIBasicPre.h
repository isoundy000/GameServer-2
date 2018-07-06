/*
 * AIBasicPre.h
 *
 *  Created on: Jun 22, 2013
 *      Author: peizhibi
 */

#ifndef AIBASICPRE_H_
#define AIBASICPRE_H_

#include "BehaviorNode.h"
#include "AIStruct.h"

class BaseAIPrecond : public BevNodePrecondition
{
public:
    virtual void reset(void){ /*void*/ }
    virtual bool ExternalCondition(NodeInputParam &input);

private:
    virtual bool AIExternalCond(GameAI* game_ai) = 0;
};

/*
 * die
 * */
class AIDiePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * area_recycle_pre
 * */
class AreaRecyclePre : public BaseAIPrecond
{
private:
	virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * sleep
 * */
class AISleepPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * pause
 */
class AIPausePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * auto
 * */
class AIAutoPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * move
 * */
class AIMovePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * rand move
 * */
class AIRandMovePre : public BaseAIPrecond
{
public:
	AIRandMovePre(void);
private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int move_interval_;
    int move_max_last_;
};

/*
 * no aim
 * */
class AINoAimPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * have aim
 * */
class AIHaveAimPre : public BaseAIPrecond
{
protected:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * back
 * */
class AIBackPre : public BaseAIPrecond
{
protected:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * idle back
 * */
class AIIdleBackPre : public BaseAIPrecond
{
public:
	AIIdleBackPre(int back_distance = GameEnum::AI_CHASE_BACK_DISTANCE);
	void check_birth_distance(GameAI* game_ai);

private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int idle_back_distance_;
};

/*
 * select the last attacker in current time.
 * if had selected, don't select again.
 * */
class AISelectModeAPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * if player's level is too lower, attack it.
 * */
class AISelectModeBPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * select fighter from around
 * */
class AISelectModeCPre : public BaseAIPrecond
{
public:
	AISelectModeCPre(int radius = GameEnum::AI_SELECT_RADIUS);

private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * random select fighter from full scene
 * */
class AISelectModeDPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};


/*
 * 选择不超过怪物出生点的玩家
 * */
class AISelectModeGPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 使用外部函数(game_ai::aim_select() )选择攻击目标
 * */
class AISelectModeOutsidePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};


/*
 * select protected npc as fighter(only for script)
 * "ext_precond" : "select_mode_npc"
 * */
class AISelectModeNpcPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

class AISelectModeMonsterPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

class AISelectModeMonsterLowBlood : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * chase aim
 * */
class AIChasePre : public BaseAIPrecond
{
public:
	AIChasePre(int attack_distance = GameEnum::DEFAULT_USE_SKILL_DISTANCE);

private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int attack_distance_;
};

/*
 * circle chase aim
 *  */
class AICircleChasePre : public BaseAIPrecond
{
public:
    AICircleChasePre(int distance = 0);

private:
    virtual bool AIExternalCond(GameAI *game_ai);

private:
    int attack_distance_;
};

/*
 * attack aim
 * */
class AIAttackPre : public BaseAIPrecond
{
public:
	AIAttackPre(int attack_distance = GameEnum::DEFAULT_USE_SKILL_DISTANCE);

private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int attack_distance_;
};

/*
 * recycle interval tick
 * */
class AIRecycleTickPre : public BaseAIPrecond
{
public:
    AIRecycleTickPre(int tick = 10);

private:
    virtual bool AIExternalCond(GameAI *game_ai);
};

/*
 * attack interval tick
 */
class AIAttackIntervalPre : public BaseAIPrecond
{
public:
	AIAttackIntervalPre(int tick = 2);

private:
    virtual bool AIExternalCond(GameAI *game_ai);

private:
    int default_interval_;
};

/*
 *跟随召唤者
 * */
class AIFollowPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 与目标保持最小的距离
 * */
class AIAimDistancePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 向内圈空位补上
 */
class AINearInnerPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 检测附近仙盟人
 * */
class AINearLeaguePre : public BaseAIPrecond
{
public:
	AINearLeaguePre(int distance = 10);

private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int distance_;
};

/*
 * 检测附近人
 * */
class AINearPlayerPre : public BaseAIPrecond
{
public:
	AINearPlayerPre(int distance = 10);

private:
    virtual bool AIExternalCond(GameAI* game_ai);

private:
    int distance_;
};

/*
 *多个怪才会触发
 */
class AINotSinglePre : public BaseAIPrecond
{
private:
	virtual bool AIExternalCond(GameAI* game_ai);
};

class AIPlayerAlivePre : public BaseAIPrecond
{
private:
	virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 与其他怪物保持距离判断
 * */
class AIDistancePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 定时器时间到达
 */
class AIIntervalPre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 距离某些生物的距离在 distance 之内
 */
class AIWithinDistancePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 距离某些生物的距离在 distance 之外
 */
class AIBeyondDistancePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 * 距离所有玩家一定距离之外
 */
class AIAllRoleDistancePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);

    MoverCoord generate_distance_point(GameAI *game_ai, std::vector<MoverCoord> &coord_list, const int distance);
};

/*
 * 判断相关的怪物是否已死亡
 */
class AIRelAiDiePre : public BaseAIPrecond
{
private:
    virtual bool AIExternalCond(GameAI* game_ai);
};

/*
 *追逐目标超出巡逻点范围
 */
class ChaseBeyondDistancePre : public AIHaveAimPre
{
public:
	ChaseBeyondDistancePre(int range_ = GameEnum::AI_CHASE_BACK_DISTANCE);
private:
    virtual bool AIExternalCond(GameAI* game_ai);
    int range_;
};

class QIXIRUNAWAYPre:public BaseAIPrecond
{

private:
	virtual bool AIExternalCond(GameAI* game_ai);
};

#endif /* AIBASICPRE_H_ */
