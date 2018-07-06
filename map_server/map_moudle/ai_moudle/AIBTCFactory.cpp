/*
 * AIBTCFactory.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: peizhibi
 */

#include "AIBTCFactory.h"
#include "AIBasicPre.h"
#include "AIBasicAction.h"
#include "AIBTCName.h"

AIBTCFactory::AIBTCFactory()
{
	// TODO Auto-generated constructor stub

}

AIBTCFactory::~AIBTCFactory()
{
	// TODO Auto-generated destructor stub
}

#define NEW_ACTION_NODE(type, class_type) \
do { \
    if (action_name == type) \
    { \
        action_node = new class_type; \
        action_node->set_node_name(action_name); \
        return action_node; \
    } \
} while (0)

BehaviorNode* AIBTCFactory::create_action_node(const std::string& action_name,
		const Json::Value &node_json)
{
	BehaviorNode* action_node = NULL;

    NEW_ACTION_NODE(AIBTCName::DieAction, AIDieAction);
    NEW_ACTION_NODE(AIBTCName::SleepAction, AISleepAction);
    NEW_ACTION_NODE(AIBTCName::RecycleAction, AIRecycleAction);
    NEW_ACTION_NODE(AIBTCName::AliveRecycleAction, AIAliveRecycleAction);
    NEW_ACTION_NODE(AIBTCName::SelectAction, AISelectAction);
    NEW_ACTION_NODE(AIBTCName::AutoAction, AIAutoAction);
    NEW_ACTION_NODE(AIBTCName::PatrolAction, AIRandMoveAction);
    NEW_ACTION_NODE(AIBTCName::BackAction, AIBackAction);
    NEW_ACTION_NODE(AIBTCName::IdleBackAction, AIIdleBackAction);
    NEW_ACTION_NODE(AIBTCName::AttackAction, AIAttackAction);
    NEW_ACTION_NODE(AIBTCName::ChaseAction, AIChaseAction);
    NEW_ACTION_NODE(AIBTCName::PatrolRouteAction, AIPatrolRouteAction);
    NEW_ACTION_NODE(AIBTCName::PatrolRouteBAction, AIPatrolRouteBAction);
    NEW_ACTION_NODE(AIBTCName::FollowAction, AIFollowAction);
    NEW_ACTION_NODE(AIBTCName::KeepAwayAction, AIKeepAwayAction);
    NEW_ACTION_NODE(AIBTCName::MoveAction, AIMoveAction);
    NEW_ACTION_NODE(AIBTCName::PauseAction, AIPauseAction);
    NEW_ACTION_NODE(AIBTCName::MovetoAction, AIMovetoAction);
    NEW_ACTION_NODE(AIBTCName::ComboAttackAction, AIComboAttackAction);
    NEW_ACTION_NODE(AIBTCName::NormalAttackAction, AINormalAttackAction);
    NEW_ACTION_NODE(AIBTCName::IntervalFinAction, AIIntervalFinAction);
    NEW_ACTION_NODE(AIBTCName::RemoveBuffAction, AIRemoveBuffAction);
    NEW_ACTION_NODE(AIBTCName::AddBuffAction, AIAddBuffAction);
    NEW_ACTION_NODE(AIBTCName::RelAddSkillAction, AIRelAddSkillAction);
    NEW_ACTION_NODE(AIBTCName::AddSkillAction, AIAddSkillAction);
    NEW_ACTION_NODE(AIBTCName::DirectChaseAction, AIDirectChaseAction);
    NEW_ACTION_NODE(AIBTCName::GenGiftboxAction, AIGenGiftboxAction);
    NEW_ACTION_NODE(AIBTCName::LimitDisChaseAction, LimitDisChaseAction);
    NEW_ACTION_NODE(AIBTCName::ReturnPatrolAction, ReturnPatrolAction);
    NEW_ACTION_NODE(AIBTCName::NotifyBossRemoveAttr, NotifyBossRemoveAttr);
    NEW_ACTION_NODE(AIBTCName::NotifyShowBoss, NotifyShowBoss);
    NEW_ACTION_NODE(AIBTCName::QIXIRUNAWAY,MonsterRunAwayInQiXi);
    NEW_ACTION_NODE(AIBTCName::QIXIPatrolAction,MonsterPatrolInQiXi);
	if (action_node == NULL)
	{
		MSG_USER("Error Action Name %s", action_name.c_str());
	}

	return action_node;
}

#define NEW_PRECOND_NODE(type, class_type) \
do { \
    if (precond_name == type) \
    { \
        pre_cond = new class_type; \
        pre_cond->set_pre_name(precond_name); \
        return pre_cond; \
    } \
} while (0)

BevNodePrecondition* AIBTCFactory::create_ext_precond(const std::string& precond_name,
		const Json::Value &node_json)
{
	BevNodePrecondition* pre_cond = NULL;

    NEW_PRECOND_NODE(AIBTCName::DiePre, AIDiePre);
    NEW_PRECOND_NODE(AIBTCName::SleepPre, AISleepPre);
    NEW_PRECOND_NODE(AIBTCName::AutoPre, AIAutoPre);
    NEW_PRECOND_NODE(AIBTCName::NoAimPre, AINoAimPre);
    NEW_PRECOND_NODE(AIBTCName::HaveAimPre, AIHaveAimPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeA, AISelectModeAPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeB, AISelectModeBPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeC, AISelectModeCPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeD, AISelectModeDPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeNpc, AISelectModeNpcPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeG, AISelectModeGPre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeOutside, AISelectModeOutsidePre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeMonsterLowBlood, AISelectModeMonsterLowBlood);
    NEW_PRECOND_NODE(AIBTCName::ChasePre, AIChasePre);
    NEW_PRECOND_NODE(AIBTCName::AttackPre, AIAttackPre);
    NEW_PRECOND_NODE(AIBTCName::BackPre, AIBackPre);
    NEW_PRECOND_NODE(AIBTCName::IdleBackPre, AIIdleBackPre);
    NEW_PRECOND_NODE(AIBTCName::RecycleTickPre, AIRecycleTickPre);
    NEW_PRECOND_NODE(AIBTCName::PatrolPre, AIRandMovePre);
    NEW_PRECOND_NODE(AIBTCName::CircleChasePre, AICircleChasePre);
    NEW_PRECOND_NODE(AIBTCName::AttackIntervalPre, AIAttackIntervalPre);
    NEW_PRECOND_NODE(AIBTCName::FollowPre, AIFollowPre);
    NEW_PRECOND_NODE(AIBTCName::AimDistancePre, AIAimDistancePre);
    NEW_PRECOND_NODE(AIBTCName::NearInnerPre, AINearInnerPre);
    NEW_PRECOND_NODE(AIBTCName::PausePre, AIPausePre);
    NEW_PRECOND_NODE(AIBTCName::SelectModeMonster, AISelectModeMonsterPre);
    NEW_PRECOND_NODE(AIBTCName::AreaRecyclePre, AreaRecyclePre);
    NEW_PRECOND_NODE(AIBTCName::NotSinglePre, AINotSinglePre);
    NEW_PRECOND_NODE(AIBTCName::PlayerAlivePre, AIPlayerAlivePre);
    NEW_PRECOND_NODE(AIBTCName::AIDistancePre, AIDistancePre);
    NEW_PRECOND_NODE(AIBTCName::AIIntervalPre, AIIntervalPre);
    NEW_PRECOND_NODE(AIBTCName::AIWithinDistancePre, AIWithinDistancePre);
    NEW_PRECOND_NODE(AIBTCName::AIBeyondDistancePre, AIBeyondDistancePre);
    NEW_PRECOND_NODE(AIBTCName::AllRoleDistancePre, AIAllRoleDistancePre);
    NEW_PRECOND_NODE(AIBTCName::RelAiDiePre, AIRelAiDiePre);
    NEW_PRECOND_NODE(AIBTCName::ChaseBeyondDistancePre, ChaseBeyondDistancePre);
    NEW_PRECOND_NODE(AIBTCName::QIXIRUNAWAYPre, QIXIRUNAWAYPre);
    if (precond_name == AIBTCName::CheckNearLeaguePre)
    {
    	if (node_json.isMember("distance") == true)
    	{
    		pre_cond = new AINearLeaguePre(node_json["distance"].asInt());
    	}
    	else
    	{
    		pre_cond = new AINearLeaguePre;
    	}

        pre_cond->set_pre_name(precond_name);
        return pre_cond;
    }

    if (precond_name == AIBTCName::CheckNearPlayerPre)
    {
    	if (node_json.isMember("distance") == true)
    	{
    		pre_cond = new AINearPlayerPre(node_json["distance"].asInt());
    	}
    	else
    	{
    		pre_cond = new AINearPlayerPre;
    	}

        pre_cond->set_pre_name(precond_name);
        return pre_cond;
    }

	if (pre_cond == NULL)
	{
		MSG_USER("Error Precondition Name %s", precond_name.c_str());
	}

	return pre_cond;
}
