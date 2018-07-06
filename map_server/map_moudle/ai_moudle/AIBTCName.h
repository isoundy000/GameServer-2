/*
 * AIBTCName.h
 *
 *  Created on: Jun 24, 2013
 *      Author: peizhibi
 */

#ifndef AIBTCNAME_H_
#define AIBTCNAME_H_

#include <string>

/*
 * ai behavior tree component name
 * */
struct AIBTCName
{
	/*
	 * ai precondition
	 * */
	static std::string AutoPre;
	static std::string DiePre;
	static std::string SleepPre;
	static std::string PatrolPre;
	static std::string NoAimPre;
	static std::string HaveAimPre;
	static std::string SelectModeA;
	static std::string SelectModeB;
	static std::string SelectModeC;
	static std::string SelectModeD;
	static std::string SelectModeNpc;
	static std::string SelectModeE;
	static std::string SelectModeF;
	static std::string SelectModeG;
	static std::string SelectModeOutside;
	static std::string SelectModeMonsterLowBlood;
	static std::string BackPre;
	static std::string IdleBackPre;
	static std::string ChasePre;
	static std::string AttackPre;
    static std::string RecycleTickPre;
    static std::string CircleChasePre;
    static std::string AttackIntervalPre;
    static std::string FollowPre;
    static std::string AimDistancePre;
    static std::string NearInnerPre;
    static std::string PausePre;
    static std::string SelectModeMonster;
    static std::string AreaRecyclePre;
    static std::string CheckNearLeaguePre;
    static std::string CheckNearPlayerPre;
    static std::string NotSinglePre;
    static std::string PlayerAlivePre;
    static std::string AIDistancePre;
    static std::string AIIntervalPre;
    static std::string AIWithinDistancePre;
    static std::string AIBeyondDistancePre;
    static std::string AllRoleDistancePre;
    static std::string RelAiDiePre;
    static std::string ChaseBeyondDistancePre;
    static std::string QIXIRUNAWAYPre;
	/*
	 * ai action
	 * */
	static std::string AutoAction;
	static std::string DieAction;
	static std::string SleepAction;
	static std::string RecycleAction;
	static std::string AliveRecycleAction;
	static std::string PatrolAction;
	static std::string BackAction;
	static std::string IdleBackAction;
	static std::string AttackAction;
	static std::string ChaseAction;
	static std::string SelectAction;
    static std::string PatrolRouteAction;
    static std::string PatrolRouteBAction;
    static std::string SelectNpcAction;
    static std::string FollowAction;
    static std::string KeepAwayAction;
    static std::string MoveAction;
    static std::string PauseAction;
    static std::string MovetoAction;
    static std::string ComboAttackAction;
    static std::string NormalAttackAction;
    static std::string IntervalFinAction;
    static std::string RemoveBuffAction;
    static std::string AddBuffAction;
    static std::string RelAddSkillAction;
    static std::string AddSkillAction;
    static std::string DirectChaseAction;
    static std::string GenGiftboxAction;
    static std::string LimitDisChaseAction;
    static std::string ReturnPatrolAction;
    static std::string NotifyBossRemoveAttr;
    static std::string NotifyShowBoss;
    static std::string QIXIRUNAWAY;
    static std::string QIXIPatrolAction;
	/*
	 * ai condition flag
	 * */
	static std::string NO_MOVE;
	static std::string NO_ATTACK;
	static std::string NO_BE_ATTACKED;
	static std::string NO_AIM;
	static std::string RECYCLE_ON_ATTACKED;


    /*
     * ai field name
     * */
    static std::string RECYCLE_TICK_FIELD;
    static std::string NPC_SORT_FIELD;
    static std::string PATROL_PATH_FIELD;
    static std::string ATTACK_INTERVAL_FIELD;
    static std::string SELECT_C_RADIUS_FIELD;
    static std::string CIRCLE_CHASE_FIELD;
    static std::string MAX_CHASE_RADIUS_FIELD;
    static std::string ATTACK_DISTANCE_FIELD;
    static std::string AI_DISTANCE_FIELD;
    static std::string AI_NORMAL_SKILL;
    static std::string AI_COMBO_SKILL_LIST;
    static std::string AI_RANDOM_COMBO_SKILL_LIST;
    static std::string AI_ADD_BUFF;
    static std::string AI_REMOVE_BUFF;
};


#endif /* AIBTCNAME_H_ */
